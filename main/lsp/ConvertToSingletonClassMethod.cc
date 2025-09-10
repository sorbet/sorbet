#include "main/lsp/ConvertToSingletonClassMethod.h"
#include "absl/strings/match.h"
#include "core/sig_finder/sig_finder.h"
#include "main/lsp/AbstractRewriter.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

unique_ptr<TextDocumentEdit> createMethodDefEdit(const core::GlobalState &gs, LSPTypecheckerDelegate &typechecker,
                                                 const LSPConfiguration &config,
                                                 const core::lsp::MethodDefResponse &definition) {
    const auto &maybeSource = definition.termLoc.source(gs);
    if (!maybeSource.has_value()) {
        return nullptr;
    }
    const auto &source = maybeSource.value();
    if (!absl::StartsWith(source, "def ")) {
        // Maybe this is an attr_reader or a prop or something. Abort.
        return nullptr;
    }
    auto file = definition.termLoc.file();

    auto shortName = definition.name.shortName(gs);
    auto insertAt = source.find(shortName);
    if (insertAt == string::npos) {
        return nullptr;
    }

    auto insertSelfLoc = definition.termLoc.adjustLen(gs, insertAt, 0);
    if (!insertSelfLoc.exists()) {
        return nullptr;
    }

    auto insertParamLoc = definition.termLoc.adjustLen(gs, insertAt + shortName.size(), 0);
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

    auto uri = config.fileRef2Uri(gs, definition.termLoc.file());
    auto tdi = make_unique<VersionedTextDocumentIdentifier>(move(uri), JSONNullObject());
    vector<unique_ptr<TextEdit>> edits;
    edits.emplace_back(make_unique<TextEdit>(move(insertSelfRange), "self."));
    if (needsParens) {
        edits.emplace_back(make_unique<TextEdit>(move(insertParamRange), "(this)"));
    } else if (definition.symbol.data(gs)->arguments.empty()) {
        edits.emplace_back(make_unique<TextEdit>(move(insertParamRange), "this"));
    } else {
        edits.emplace_back(make_unique<TextEdit>(move(insertParamRange), "this, "));
    }

    if (definition.symbol.data(gs)->hasSig()) {
        auto resolvedTree = typechecker.getResolved(file);
        auto &rootTree = resolvedTree.tree;

        auto ctx = core::Context(gs, core::Symbols::root(), file);
        auto queryLoc = definition.termLoc.copyWithZeroLength();
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

class MethodCallSiteRewriter : public AbstractRewriter {
    core::MethodRef method;
    core::ClassOrModuleRef owner;

public:
    MethodCallSiteRewriter(const core::GlobalState &gs, const LSPConfiguration &config, core::MethodRef method)
        : AbstractRewriter(gs, config), method(method), owner(method.data(gs)->owner) {}
    ~MethodCallSiteRewriter() {}

    void rename(unique_ptr<core::lsp::QueryResponse> &response, const core::SymbolRef originalSymbol) override {
        if (invalid) {
            return;
        }

        auto sendResp = response->isSend();
        if (sendResp == nullptr || !sendResp->receiverLocOffsets.exists()) {
            return;
        }

        if (sendResp->originalName == core::Names::callWithSplat() ||
            sendResp->originalName == core::Names::callWithBlockPass() ||
            sendResp->originalName == core::Names::callWithSplatAndBlock()) {
            // These are too hard... skipping for the time being.
            return;
        }

        if (sendResp->dispatchResult->secondary != nullptr) {
            // If the call site is not trivial, don't attepmt to rename.
            // The type check error will inform the user that this needs to be fixed manually.
            return;
        }

        auto receiverLoc =
            sendResp->receiverLoc().exists() ? sendResp->receiverLoc() : sendResp->termLoc().copyWithZeroLength();

        ENFORCE(edits.find(receiverLoc) == edits.end(), "Tried to edit the same call site twice...");
        edits[receiverLoc] = fmt::format("{}{}", owner.show(gs), sendResp->isPrivateOk ? "." : "");

        auto receiverSource = string(sendResp->isPrivateOk ? "self" : sendResp->receiverLoc().source(gs).value());
        if (absl::StrContains(receiverSource, ";")) {
            // It never hurts to parenthesize an argument, it's still technically valid.
            //
            // If there's a `;` in the receiver, it could either be in a string (`"foo;bar".example`)
            // OR it could be a statement separator: `(x; y).example`, and `example(x; y)` is a syntax error.
            //
            // Let's defend against this by parenthesizing, despite how rare this is likely to be.
            receiverSource = fmt::format("({})", receiverSource);
        }

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

    void addSymbol(const core::SymbolRef symbol) override {
        if (!symbol.isMethod()) {
            return;
        }
        getQueue()->tryEnqueue(symbol);

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

    auto renamer = make_shared<MethodCallSiteRewriter>(gs, config, definition.symbol);
    renamer->getEdits(typechecker, definition.symbol);
    auto callSiteEdits = renamer->buildTextDocumentEdits();
    if (!callSiteEdits.has_value()) {
        config.logger->error("Failed to buildTextDocumentEdits for convertToSingletonClassMethod");
        return {};
    }

    auto res = move(callSiteEdits.value());
    res.emplace_back(move(methodDefEdit));

    return res;
}

} // namespace sorbet::realmain::lsp
