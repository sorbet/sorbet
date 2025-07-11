#include "main/lsp/KwargsFinder.h"
#include "ast/Trees.h"
#include "ast/treemap/treemap.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/NameRef.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

struct SendTraversal {
    core::LocOffsets funLoc;
    vector<core::NameRef> kwargs;

    SendTraversal(core::LocOffsets funLoc) : funLoc{funLoc} {}

    ast::QueryControl preTransformSend(core::Context ctx, const ast::Send &send) {
        if (!send.funLoc.contains(this->funLoc)) {
            return send.loc.contains(this->funLoc) ? ast::QueryControl::Continue : ast::QueryControl::Skip;
        }

        for (const auto &pair : send.kwArgPairs()) {
            if (auto lit = ast::cast_tree<ast::Literal>(pair.key)) {
                if (lit->isSymbol()) {
                    kwargs.emplace_back(lit->asSymbol());
                }
            }
        }

        // We've traversed everything necessary.
        return ast::QueryControl::Done;
    }

    ast::QueryControl preTransformMethodDef(core::Context ctx, const ast::MethodDef &method) {
        if (!method.loc.contains(this->funLoc)) {
            return ast::QueryControl::Skip;
        }

        return ast::QueryControl::Continue;
    }
};

}; // namespace

vector<core::NameRef> KwargsFinder::findKwargs(const core::GlobalState &gs, const ast::ParsedFile &ast,
                                               core::LocOffsets funLoc) {
    core::Context ctx{gs, core::Symbols::root(), ast.file};
    SendTraversal traversal{funLoc};
    ast::TreeQuery::apply(ctx, traversal, ast.tree);
    return move(traversal.kwargs);
}

} // namespace sorbet::realmain::lsp
