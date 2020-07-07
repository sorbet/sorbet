#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/namer.h"

#include "TypeMembers.h"

using namespace std;

namespace sorbet::rewriter {

void TypeMembers::run(core::MutableContext ctx, ast::ClassDef *cdef) {
    UnorderedSet<core::NameRef> typeMembers;

    for (auto &expr : cdef->rhs) {
        auto assn = ast::cast_tree<ast::Assign>(expr);
        if (!assn) {
            continue;
        }

        auto rhs = ast::cast_tree<ast::Send>(assn->rhs);
        if (!rhs || !rhs->recv.isSelfReference() || rhs->fun != core::Names::typeMember()) {
            continue;
        }

        auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(assn->lhs);
        if (!lhs) {
            continue;
        }

        if (typeMembers.contains(lhs->cnst)) {
            if (auto e = ctx.beginError(lhs->loc, core::errors::Namer::InvalidTypeDefinition)) {
                e.setHeader("Duplicate type member `{}`", lhs->cnst.data(ctx)->show(ctx));
            }
            expr = ast::MK::EmptyTree();
            return;
        }

        typeMembers.insert(lhs->cnst);
    }
}

} // namespace sorbet::rewriter
