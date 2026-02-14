#include "main/lsp/requests/document_symbol.h"
#include "absl/strings/match.h"
#include "ast/treemap/treemap.h"
#include "common/sort/sort.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/json_types.h"

#include "absl/algorithm/container.h"

using namespace std;

namespace sorbet::realmain::lsp {
namespace {

struct Node {
    // The parent node, or nullptr if this is `<root>`.
    shared_ptr<Node> parent;

    // The symbol defined.
    core::SymbolRef symbol;

    // The location of the full declaration in the file being processed (i.e. the whole class definition).
    core::Loc loc;

    // The location of only the declaration (i.e. the `def` of a method definition).
    core::Loc declLoc;

    // Propagated from the `MethodDef` when `symbol` is a method.
    bool isAttrBestEffortUIOnly = false;

    // Any child symbols.
    vector<shared_ptr<Node>> children;

    Node(shared_ptr<Node> parent, core::SymbolRef symbol, core::Loc loc)
        : parent{move(parent)}, symbol{symbol}, loc{loc}, declLoc{loc} {}

    unique_ptr<DocumentSymbol> genNode(const core::GlobalState &gs) const {
        if (!this->symbol.exists()) {
            return nullptr;
        }
        if (hideSymbol(gs, this->symbol)) {
            return nullptr;
        }

        auto range = Range::fromLoc(gs, this->loc);
        auto selectionRange = Range::fromLoc(gs, this->declLoc);
        bool isAttr = this->symbol.isMethod() && this->isAttrBestEffortUIOnly;
        auto kind = symbolRef2SymbolKind(gs, this->symbol, isAttr);

        string name;
        auto owner = this->symbol.owner(gs);
        if (owner.isClassOrModule() && owner.asClassOrModuleRef().data(gs)->attachedClass(gs).exists()) {
            name = "self.";
        }
        auto symName = this->symbol.name(gs).show(gs);
        string_view view{symName};
        if (absl::StartsWith(view, "<") && view.size() > 1) {
            string_view describeStr = "<describe '";
            string_view itStr = "<it '";
            if (absl::StartsWith(view, describeStr) || absl::StartsWith(view, itStr)) {
                view.remove_prefix(1);
                view.remove_suffix(1);
            }
        }
        name += view;

        auto result = make_unique<DocumentSymbol>(move(name), kind, move(range), move(selectionRange));

        // Previous versions of VSCode have a bug that requires this non-optional field to be present.
        // This previously tried to include the method signature but due to issues where large signatures were not
        // readable when put on one line and given that currently details are only visible in the outline view but not
        // seen in the symbol search. Additionally, no other language server implementations we could find used this
        // field.
        result->detail = "";

        result->children = this->genChildren(gs);

        return result;
    }

    vector<unique_ptr<DocumentSymbol>> genChildren(const core::GlobalState &gs) const {
        vector<unique_ptr<DocumentSymbol>> result;

        result.reserve(this->children.size());
        for (auto &child : this->children) {
            if (auto res = child->genNode(gs)) {
                result.emplace_back(move(res));
            }
        }

        return result;
    }
};

// Reify the outline of the tree, so that we can post-process it into a documentSymbol response.
class Outliner {
    shared_ptr<Node> scope;

    Outliner(core::Loc fullFile) : scope{make_shared<Node>(nullptr, core::Symbols::root(), fullFile)} {}

    // When processing all the members of a class/module, only add the ones that occur syntactically within the current
    // node.
    void addChild(const core::Context ctx, core::SymbolRef child) {
        for (auto childLoc : child.locs(ctx)) {
            if (childLoc.file() == ctx.file && this->scope->loc.contains(childLoc)) {
                this->scope->children.emplace_back(make_shared<Node>(this->scope, child, childLoc));
                return;
            }
        }
    }

public:
    void preTransformClassDef(core::Context ctx, const ast::ClassDef &klass) {
        // <root> is eagerly added in the constructor, so we exit early here to avoid accidentally adding it twice.
        if (klass.symbol == core::Symbols::root()) {
            return;
        }

        auto next = make_shared<Node>(this->scope, klass.symbol, ctx.locAt(klass.loc));
        next->declLoc = ctx.locAt(klass.declLoc);
        this->scope->children.emplace_back(next);
        this->scope = move(next);

        for (auto member : klass.symbol.data(ctx)->members()) {
            if (member.second.isClassOrModule() || member.second.isMethod()) {
                continue;
            }

            this->addChild(ctx, member.second);
        }

        auto singleton = klass.symbol.data(ctx)->lookupSingletonClass(ctx);
        if (singleton.exists()) {
            for (auto member : singleton.data(ctx)->members()) {
                if (member.second.isClassOrModule() || member.second.isMethod()) {
                    continue;
                }

                this->addChild(ctx, member.second);
            }
        }
    }

    void postTransformClassDef(core::Context ctx, const ast::ClassDef &klass) {
        // <root> is eagerly added in the constructor, so we exit early here to avoid accidentally assigning
        // `scope` to its parent, which will be `nullptr`.
        if (klass.symbol == core::Symbols::root()) {
            return;
        }

        fast_sort(this->scope->children,
                  [](auto &left, auto &right) { return left->loc.beginPos() < right->loc.beginPos(); });

        this->scope = this->scope->parent;
    }

    void preTransformMethodDef(core::Context ctx, const ast::MethodDef &method) {
        // We avoid using `this->addChild` here because we know the method occurs within `this->scope`, and we also want
        // to give it a different `declLoc` and propagate `isAttrBestEffortUIOnly`.
        auto node = make_shared<Node>(this->scope, method.symbol, ctx.locAt(method.loc));
        node->declLoc = ctx.locAt(method.declLoc);
        node->isAttrBestEffortUIOnly = method.flags.isAttrBestEffortUIOnly;
        this->scope->children.emplace_back(node);
    }

    static vector<unique_ptr<DocumentSymbol>> documentSymbol(core::Context ctx, const ast::ExpressionPtr &tree) {
        core::Loc fullFile{ctx.file, core::LocOffsets{0, static_cast<uint32_t>(ctx.file.data(ctx).source().size())}};

        Outliner outliner{fullFile};

        ast::ConstShallowWalk::apply(ctx, outliner, tree);

        ENFORCE(outliner.scope != nullptr);
        ENFORCE(outliner.scope->symbol == core::Symbols::root());
        return outliner.scope->genChildren(ctx);
    }
};

} // namespace

DocumentSymbolTask::DocumentSymbolTask(const LSPConfiguration &config, MessageId id,
                                       unique_ptr<DocumentSymbolParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentDocumentSymbol), params(move(params)) {}

bool DocumentSymbolTask::isDelayable() const {
    return true;
}

unique_ptr<ResponseMessage> DocumentSymbolTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentDocumentSymbol);

    const core::GlobalState &gs = typechecker.state();
    vector<unique_ptr<DocumentSymbol>> result;
    const auto &uri = params->textDocument->uri;
    auto fref = config.uri2FileRef(gs, uri);
    if (!fref.exists()) {
        response->result = move(result);
        return response;
    }

    auto resolved = typechecker.getResolved(fref);
    if (resolved.tree == nullptr) {
        response->result = move(result);
        return response;
    }

    core::Context ctx{gs, core::Symbols::root(), fref};
    response->result = Outliner::documentSymbol(ctx, resolved.tree);

    return response;
}

} // namespace sorbet::realmain::lsp
