#include "main/lsp/ConvertToSingletonClassMethod.h"
#include "main/lsp/AbstractRewriter.h"
using namespace std;

namespace sorbet::realmain::lsp {

namespace {

unique_ptr<TextDocumentEdit> createMethodDefEdit(const core::GlobalState &gs, const LSPConfiguration &config,
                                                 const core::lsp::MethodDefResponse &definition) {
    const auto &maybeSource = definition.termLoc.source(gs);
    if (!maybeSource.has_value()) {
        return nullptr;
    }
    const auto &source = maybeSource.value();

    auto insertAt = source.find(definition.name.shortName(gs));
    if (insertAt == string::npos) {
        return nullptr;
    }

    auto insertAtLoc = definition.termLoc.adjustLen(gs, insertAt, 0);
    if (!insertAtLoc.exists()) {
        return nullptr;
    }

    auto insertAtRange = Range::fromLoc(gs, insertAtLoc);
    ENFORCE(insertAtRange != nullptr);

    auto uri = config.fileRef2Uri(gs, definition.termLoc.file());
    auto tdi = make_unique<VersionedTextDocumentIdentifier>(move(uri), JSONNullObject());
    vector<unique_ptr<TextEdit>> edits;
    edits.emplace_back(make_unique<TextEdit>(move(insertAtRange), "self."));
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

        // auto loc = response->getLoc();

        // // TODO(jez) I think because we're skipping T.any we should be able to ignore this,
        // // and anywhere that that fails we should build proper support for.
        // ENFORCE(edits.find(loc) == edits.end());

        // TODO(jez)
        // (1 + 1).even?    ->   A.even?((1 + 1))
        // x.foo(y)         ->   A.foo(x, y)
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

    auto methodDefEdit = createMethodDefEdit(gs, config, definition);
    // TODO(jez) Handle if methodDefEdit is nullptr, abort
    ENFORCE(methodDefEdit != nullptr);
    res.emplace_back(move(methodDefEdit));

    return res;
}

} // namespace sorbet::realmain::lsp
