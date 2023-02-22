#include "main/lsp/ConvertToSingletonClassMethod.h"
#include "absl/strings/match.h"
#include "main/lsp/AbstractRewriter.h"
#include "main/sig_finder/sig_finder.h"

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
        auto trees = typechecker.getResolved({file});
        ENFORCE(!trees.empty());
        auto &rootTree = trees[0].tree;

        auto ctx = core::Context(gs, core::Symbols::root(), file);
        auto queryLoc = definition.termLoc.copyWithZeroLength();
        auto parsedSig = sig_finder::SigFinder::findSignature(ctx, rootTree, queryLoc);
        if (parsedSig.has_value()) {
            if (!parsedSig->argTypes.empty()) {
                auto firstArgLoc = parsedSig->argTypes[0].nameLoc;
                auto insertSigParamRange = Range::fromLoc(gs, firstArgLoc.adjustLen(gs, 0, 0));
                auto sigParamText = fmt::format("this: {}, ", definition.symbol.data(gs)->owner.show(gs));
                edits.emplace_back(make_unique<TextEdit>(move(insertSigParamRange), move(sigParamText)));
            } else if (parsedSig->returnsLoc.exists()) {
                auto insertSigParamsRange = Range::fromLoc(gs, parsedSig->returnsLoc.adjustLen(gs, 0, 0));
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

        if (sendResp->callerSideName == core::Names::callWithSplat() ||
            sendResp->callerSideName == core::Names::callWithBlock() ||
            sendResp->callerSideName == core::Names::callWithSplatAndBlock()) {
            // These are too hard... skipping for the time being.
            return;
        }

        if (sendResp->dispatchResult->secondary != nullptr) {
            // If the call site is not trivial, don't attepmt to rename.
            // The type check error will inform the user that this needs to be fixed manually.
            return;
        }

        auto replaceLoc = sendResp->termLoc();
        ENFORCE(edits.find(replaceLoc) == edits.end(), "Tried to edit the same call site twice...");

        fmt::memory_buffer buf;
        fmt::format_to(back_inserter(buf), "{}.{}", owner.show(gs), sendResp->callerSideName.show(gs));
        auto receiverSource = sendResp->isPrivateOk ? "self"sv : sendResp->receiverLoc().source(gs).value();
        if (sendResp->argLocOffsets.empty()) {
            fmt::format_to(back_inserter(buf), "({})", receiverSource);
        } else {
            auto file = sendResp->file;
            auto firstArgLoc = core::Loc(file, sendResp->argLocOffsets.front());
            auto lastArgLoc = core::Loc(file, sendResp->argLocOffsets.back());
            if (!firstArgLoc.exists() || !lastArgLoc.exists()) {
                return;
            }
            auto argSource = firstArgLoc.join(lastArgLoc).source(gs).value();
            fmt::format_to(back_inserter(buf), "({}, {})", receiverSource, argSource);
        }

        if (sendResp->dispatchResult->main.blockPreType != nullptr) {
            auto blockLocStart = sendResp->argLocOffsets.empty()
                                     ? sendResp->funLocOffsets.copyEndWithZeroLength()
                                     : sendResp->argLocOffsets.back().copyEndWithZeroLength();
            auto blockLocEnd = sendResp->termLocOffsets.copyEndWithZeroLength();
            auto blockLoc = core::Loc(sendResp->file, blockLocStart.join(blockLocEnd));
            if (auto maybeBlockSource = blockLoc.source(gs)) {
                string_view blockSource = (!maybeBlockSource->empty() && absl::StartsWith(*maybeBlockSource, "()"))
                                              ? maybeBlockSource->substr(2)
                                              : *maybeBlockSource;
                fmt::format_to(back_inserter(buf), "{}", blockSource);
            }
        }

        edits[replaceLoc] = to_string(buf);
    }

    void addSymbol(const core::SymbolRef symbol) override {
        if (!symbol.isMethod()) {
            return;
        }
        getQueue()->tryEnqueue(symbol);

        // This doesn't make any attempt to handle methods that are overridden.
        //
        // Technically speaking, this code action doesn't make a ton of sense if the method is
        // overriden, because converting to a singleton method will kill dynamic dispatch.
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
