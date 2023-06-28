#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/namer.h"

#include "TypeMembers.h"

using namespace std;

namespace sorbet::rewriter {

void TypeMembers::run(core::MutableContext ctx, ast::ClassDef *cdef) {
    UnorderedMap<core::NameRef, core::LocOffsets> typeMembers;

    for (auto &expr : cdef->rhs) {
        auto assn = ast::cast_tree<ast::Assign>(expr);
        if (!assn) {
            continue;
        }

        auto rhs = ast::cast_tree<ast::Send>(assn->rhs);
        if (!rhs || !rhs->recv.isSelfReference() ||
            (rhs->fun != core::Names::typeMember() && rhs->fun != core::Names::typeTemplate())) {
            continue;
        }

        auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(assn->lhs);
        if (!lhs) {
            continue;
        }

        auto it = typeMembers.find(lhs->cnst);
        if (it != typeMembers.end()) {
            if (auto e = ctx.beginError(lhs->loc, core::errors::Namer::InvalidTypeDefinition)) {
                auto memTem = rhs->fun == core::Names::typeMember() ? "member" : "template";
                e.setHeader("Duplicate type {} `{}`", memTem, lhs->cnst.show(ctx));
                e.addErrorLine(ctx.locAt(it->second), "Previous definition");
                e.replaceWith(fmt::format("Delete duplicate type {}", memTem), ctx.locAt(expr.loc()), "");
            }
            expr = ast::MK::EmptyTree();
            continue;
        }

        typeMembers[lhs->cnst] = expr.loc();
    }
}

} // namespace sorbet::rewriter
