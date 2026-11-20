#include "main/lsp/ConvertToSingletonClassMethod.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "core/sig_finder/sig_finder.h"
#include "main/lsp/AbstractRewriter.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

unique_ptr<TextDocumentEdit> createMethodDefEdit(const core::GlobalState &gs, LSPTypecheckerDelegate &typechecker,
                                                 const LSPConfiguration &config,
                                                 const core::lsp::MethodDefResponse &definition) {
    const auto &maybeSource = definition.declLoc().source(gs);
    if (!maybeSource.has_value()) {
        return nullptr;
    }
    const auto &source = maybeSource.value();
    if (!absl::StartsWith(source, "def ")) {
        // Maybe this is an attr_reader or a prop or something. Abort.
        return nullptr;
    }
    auto file = definition.declLoc().file();

    auto shortName = definition.name.shortName(gs);
    auto insertAt = source.find(shortName);
    if (insertAt == string::npos) {
        return nullptr;
    }

    auto insertSelfLoc = definition.declLoc().adjustLen(gs, insertAt, 0);
    if (!insertSelfLoc.exists()) {
        return nullptr;
    }

    auto insertParamLoc = definition.declLoc().adjustLen(gs, insertAt + shortName.size(), 0);
    if (!insertParamLoc.exists()) {
        return nullptr;
    }
    bool needsParens = true;
    if (insertParamLoc.adjustLen(gs, 0, 1).source(gs) == "(") {
        insertParamLoc = insertParamLoc.adjustLen(gs, 1, 0);
        needsParens = false;
    }

    auto insertSelfRange = Range::fromLoc(gs, insertSelfLoc);
    ENFORCE(insertSelfRange != nullptr);
    auto insertParamRange = Range::fromLoc(gs, insertParamLoc);
    ENFORCE(insertParamRange != nullptr);

    auto uri = config.fileRef2Uri(gs, definition.declLoc().file());
    auto tdi = make_unique<VersionedTextDocumentIdentifier>(move(uri), JSONNullObject());
    vector<unique_ptr<TextEdit>> edits;
    edits.emplace_back(make_unique<TextEdit>(move(insertSelfRange), "self."));
    if (needsParens) {
        edits.emplace_back(make_unique<TextEdit>(move(insertParamRange), "(this)"));
    } else if (definition.symbol.data(gs)->parameters.empty()) {
        edits.emplace_back(make_unique<TextEdit>(move(insertParamRange), "this"));
    } else {
        edits.emplace_back(make_unique<TextEdit>(move(insertParamRange), "this, "));
    }

    if (definition.symbol.data(gs)->hasSig()) {
        auto resolvedTree = typechecker.getResolved(file);
        auto &rootTree = resolvedTree.tree;

        auto ctx = core::Context(gs, core::Symbols::root(), file);
        auto queryLoc = definition.declLoc().copyWithZeroLength();
        auto parsedSig = sig_finder::SigFinder::findSignature(ctx, rootTree, queryLoc);
        if (parsedSig.has_value()) {
            if (!parsedSig->sig.argTypes.empty()) {
                auto firstArgLoc = parsedSig->sig.argTypes[0].nameLoc;
                auto insertSigParamRange = Range::fromLoc(gs, ctx.locAt(firstArgLoc).adjustLen(gs, 0, 0));
                auto sigParamText = fmt::format("this: {}, ", definition.symbol.data(gs)->owner.show(gs));
                edits.emplace_back(make_unique<TextEdit>(move(insertSigParamRange), move(sigParamText)));
            } else if (parsedSig->sig.returnsLoc.exists()) {
                auto insertSigParamsRange =
                    Range::fromLoc(gs, ctx.locAt(parsedSig->sig.returnsLoc).adjustLen(gs, 0, 0));
                auto sigParamText = fmt::format("params(this: {}).", definition.symbol.data(gs)->owner.show(gs));
                edits.emplace_back(make_unique<TextEdit>(move(insertSigParamsRange), move(sigParamText)));
            }
        }
    }

    return make_unique<TextDocumentEdit>(move(tdi), move(edits));
}

// `(x).foo` desugars to just `x.foo`: there's no AST node that records the loc of the parens.
// But `x` might not be valid as an argument expression without them: `(a rescue b).foo` is fine,
// but `A.foo(a rescue b)` is a syntax error.
//
// This helper tries to find those surrounding parens by scanning outward from the receiver's loc,
// bounded by the range of the overall send's loc.
//
// When in doubt, it returns the receiver's loc unchanged, which is what the code action used before
// it looked for parens (i.e., no worse than before).
//
// The cursors `open` and `close` only ever move away from the receiver, and stay inside the send,
// so the loop terminates, and it never goes back to a character it has already passed.
core::LocOffsets receiverWithParens(string_view source, core::LocOffsets send, core::LocOffsets receiver) {
    // (The receiver of the `x.foo` that `x&.foo` desugars to is a temporary that holds `x`, with a zero-width loc.)
    if (receiver.empty()) {
        return receiver;
    }

    // Make sure the receiver is inside the send.
    // A receiver without parens starts where its send starts, so the scan below stops right away.
    if (!(send.beginPos() <= receiver.beginPos() && receiver.endPos() <= send.endPos())) {
        return receiver;
    }

    // `[begin, end)` is always the receiver wrapped in zero or more pairs of matched parens.
    auto begin = receiver.beginPos(), end = receiver.endPos();
    // These cursors search outwards from the current receiver to find the next pair.
    auto open = begin, close = end;
    while (true) {
        ENFORCE(send.beginPos() <= begin && begin <= receiver.beginPos());
        ENFORCE(receiver.endPos() <= end && end <= send.endPos());

        // Skip over whitespace
        while (open > send.beginPos() && absl::ascii_isspace(source[open - 1])) {
            open--;
        }
        while (close < send.endPos() && absl::ascii_isspace(source[close])) {
            close++;
        }

        if (open == send.beginPos() || close == send.endPos() || source[open - 1] != '(' || source[close] != ')') {
            // Reached the bounds of the send, or didn't find parens on both sides.
            break;
        }

        // Found matching open/close parentheses, let's consume and look for more.
        begin = --open;
        end = ++close;

        ENFORCE(begin < receiver.beginPos() && receiver.endPos() < end);
        ENFORCE(source[begin] == '(' && source[end - 1] == ')');
    }

    // Scanning backwards can land on a paren in a comment, not the real paren:
    //     ( # (
    //       x).foo
    // Scanning forwards can't: whitespace and `)` never start a comment.
    //
    // Keep scanning to the beginning of the line to try to find something that might be a comment
    // (either a `#` or an `=` character, as in `=end`). We might find one that doesn't hide any of the
    // parens, like a comment on a line above them, but oh well:
    //     begin # hi
    //       (x) end.foo
    // (The scan never looks before the send, so code like `"#"; (x).foo` is fine--you have to get
    // some really contrived cases like that `begin #` for this to matter.)
    //
    // If we find something, give up and return the receiver. This is pessimistic and might generate a
    // bad edit anyways (e.g., in cases where preserving the parens was truly required), but oh well.
    for (auto i = open; i > send.beginPos() && source[i - 1] != '\n'; i--) {
        if (source[i - 1] == '#' || source[i - 1] == '=') {
            return receiver;
        }
    }

    return {begin, end};
}

class MethodCallSiteRewriter : public AbstractRewriter {
    core::MethodRef method;
    core::ClassOrModuleRef owner;

public:
    MethodCallSiteRewriter(const core::GlobalState &gs, const LSPConfiguration &config, core::MethodRef method)
        : AbstractRewriter(gs, config), method(method), owner(method.data(gs)->owner) {
        getQueue()->tryEnqueue(method);

        // This doesn't make any attempt to handle methods that are overridden.
        //
        // Technically speaking, this code action doesn't make a ton of sense if the method is
        // overridden, because converting to a singleton method will kill dynamic dispatch.
        //
        // We have two options:
        // 1.  Assume that there are no overrides of this method.
        //     Pro: no additional work
        //     Con: might not catch some callsites on child methods
        // 2.  Detect when there are overrides of this method, and mark the code action `invalid` with an error
        //     Pro: "safer" because we can error instead of silently changing the meaning of the program
        //     Con: requires a full scan of the symbol table
        //
        // For the time being, we're taking option (1).
        // Option (2) would involve a call to addSubclassRelatedMethods or something similar here.
    }

    ~MethodCallSiteRewriter() {}

    void rename(unique_ptr<core::lsp::QueryResponse> &response) override {
        if (invalid) {
            return;
        }

        auto sendResp = response->isSend();
        if (sendResp == nullptr || !sendResp->receiverLocOffsets.exists()) {
            return;
        }

        if (sendResp->originalName == core::Names::callWithSplat() ||
            sendResp->originalName == core::Names::callWithBlockPass() ||
            sendResp->originalName == core::Names::callWithSplatAndBlockPass()) {
            // These are too hard... skipping for the time being.
            return;
        }

        if (sendResp->dispatchResult->secondary != nullptr) {
            // If the call site is not trivial, don't attepmt to rename.
            // The type check error will inform the user that this needs to be fixed manually.
            return;
        }

        // Both the range to replace with the class name and the text to move into the arguments
        // include the receiver's parens, so that the moved text is still a valid expression.
        auto receiverLoc =
            sendResp->receiverLoc().exists()
                ? core::Loc(sendResp->file, receiverWithParens(sendResp->file.data(gs).source(),
                                                               sendResp->termLocOffsets, sendResp->receiverLocOffsets))
                : sendResp->termLoc().copyWithZeroLength();

        ENFORCE(edits.find(receiverLoc) == edits.end(), "Tried to edit the same call site twice...");
        edits[receiverLoc] = fmt::format("{}{}", owner.show(gs), sendResp->isPrivateOk ? "." : "");

        auto receiverSource = string(sendResp->isPrivateOk ? "self" : receiverLoc.source(gs).value());

        if (!sendResp->argLocOffsets.empty()) {
            auto newFirstArgLoc = core::Loc(sendResp->file, sendResp->argLocOffsets.front().copyWithZeroLength());

            ENFORCE(edits.find(newFirstArgLoc) == edits.end(), "Tried to edit the same call site twice...");
            edits[newFirstArgLoc] = fmt::format("{}, ", receiverSource);
        } else if (sendResp->funLocOffsets.exists() && !sendResp->funLocOffsets.empty() &&
                   sendResp->locOffsetsWithoutBlock.exists() &&
                   sendResp->funLocOffsets.endPos() <= sendResp->locOffsetsWithoutBlock.endPos()) {
            auto newFirstArgLoc =
                core::Loc(sendResp->file, sendResp->funLocOffsets.endPos(), sendResp->locOffsetsWithoutBlock.endPos());

            if (newFirstArgLoc.empty()) {
                auto checkForEmptyParens = newFirstArgLoc.adjustLen(gs, 0, 2);
                if (checkForEmptyParens.source(gs) == "()") {
                    newFirstArgLoc = checkForEmptyParens;
                }
            }

            ENFORCE(edits.find(newFirstArgLoc) == edits.end(), "Tried to edit the same call site twice...");
            edits[newFirstArgLoc] = fmt::format("({})", receiverSource);
        } else {
            ENFORCE(false, "You've found a test case, please add one!");
            auto newFirstArgLoc = core::Loc(sendResp->file, sendResp->funLocOffsets.copyEndWithZeroLength());
            edits[newFirstArgLoc] = fmt::format("({})", receiverSource);
        }
    }
};

} // namespace

vector<unique_ptr<TextDocumentEdit>> convertToSingletonClassMethod(LSPTypecheckerDelegate &typechecker,
                                                                   const LSPConfiguration &config,
                                                                   const core::lsp::MethodDefResponse &definition) {
    auto &gs = typechecker.state();

    auto methodDefEdit = createMethodDefEdit(gs, typechecker, config, definition);
    if (methodDefEdit == nullptr) {
        config.logger->error("Failed to createMethodDefEdit for convertToSingletonClassMethod");
        return {};
    }

    auto renamer = MethodCallSiteRewriter{gs, config, definition.symbol};
    renamer.getEdits(typechecker);
    auto callSiteEdits = renamer.buildTextDocumentEdits();
    if (!callSiteEdits.has_value()) {
        config.logger->error("Failed to buildTextDocumentEdits for convertToSingletonClassMethod");
        return {};
    }

    auto res = move(callSiteEdits.value());
    res.emplace_back(move(methodDefEdit));

    return res;
}

} // namespace sorbet::realmain::lsp
