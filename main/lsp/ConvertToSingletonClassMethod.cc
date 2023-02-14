#include "main/lsp/ConvertToSingletonClassMethod.h"
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
        : AbstractRewriter(gs, config), method(method), owner(method.data(gs)->owner) {
        // TODO(jez) Mark invalid if it isn't "final in practice", because the dynamic dispatch will
        // go away
    }
    ~MethodCallSiteRewriter() {}

    void rename(unique_ptr<core::lsp::QueryResponse> &response, const core::SymbolRef originalSymbol) override {
        if (invalid) {
            return;
        }

        auto sendResp = response->isSend();
        if (sendResp == nullptr || !sendResp->receiverLocOffsets.exists()) {
            return;
        }

        if (sendResp->dispatchResult->secondary != nullptr) {
            // If the call site is not trivial, don't attepmt to rename.
            // The type check error will inform the user that this needs to be fixed manually.
            return;
        }

        if (sendResp->dispatchResult->main.blockPreType != nullptr) {
            // Skip over call sites that pass a block right now.
            // TODO(jez) We can probably handle these
            return;
        }

        // // TODO(jez) I think because we're skipping T.any we should be able to ignore this,
        // // and anywhere that that fails we should build proper support for.
        // ENFORCE(edits.find(loc) == edits.end());

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

        edits[sendResp->termLoc()] = to_string(buf);
    }

    void addSymbol(const core::SymbolRef symbol) override {
        if (!symbol.isMethod()) {
            return;
        }
        getQueue()->tryEnqueue(symbol);

        // TODO(jez) How much breaks if we assume that the method is final?
        // Right now my gut is not much.
        // TODO(jez) Is it easy to make the edit if the method is not final?
        // Right now my gut is that it's hard/not worth.
        // addSubclassRelatedMethods(gs, symbol.asMethodRef(), getQueue());
    }
};

} // namespace

vector<unique_ptr<TextDocumentEdit>> convertToSingletonClassMethod(LSPTypecheckerDelegate &typechecker,
                                                                   const LSPConfiguration &config,
                                                                   const core::lsp::MethodDefResponse &definition) {
    auto &gs = typechecker.state();

    auto renamer = make_shared<MethodCallSiteRewriter>(gs, config, definition.symbol);
    renamer->getEdits(typechecker, definition.symbol);
    auto callSiteEdits = renamer->buildTextDocumentEdits();

    vector<unique_ptr<TextDocumentEdit>> res;
    if (callSiteEdits.has_value()) {
        res = move(callSiteEdits.value());
    }

    auto methodDefEdit = createMethodDefEdit(gs, typechecker, config, definition);
    // TODO(jez) Handle if methodDefEdit is nullptr, abort
    ENFORCE(methodDefEdit != nullptr);
    res.emplace_back(move(methodDefEdit));

    return res;
}

} // namespace sorbet::realmain::lsp
