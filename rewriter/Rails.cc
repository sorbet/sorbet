#include "rewriter/Rails.h"
#include "absl/container/inlined_vector.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/rewriter.h"
#include "rewriter/util/Util.h"

using namespace std;

namespace sorbet::rewriter {

void Rails::run(core::MutableContext ctx, ast::ClassDef *cdef) {
    if (cdef->ancestors.size() != 1) {
        return;
    }
    auto send = ast::cast_tree<ast::Send>(cdef->ancestors[0]);
    if (!send) {
        return;
    }
    if (send->fun != core::Names::squareBrackets()) {
        return;
    }

    static constexpr core::NameRef activeRecordMigration[] = {
        core::Names::Constants::ActiveRecord(),
        core::Names::Constants::Migration(),
    };

    if (!ASTUtil::isRootScopedSyntacticConstant(send->recv, activeRecordMigration)) {
        return;
    }
    if (send->numPosArgs() != 1 && !send->hasKwArgs()) {
        return;
    }
    auto arg = ast::cast_tree<ast::Literal>(send->getPosArg(0));
    if (!arg) {
        return;
    }
    if (!core::isa_type<core::FloatLiteralType>(arg->value)) {
        return;
    }
    auto &f = core::cast_type_nonnull<core::FloatLiteralType>(arg->value);
    char version[5];
    snprintf(version, sizeof(version), "V%.1f", f.value);
    absl::c_replace(version, '.', '_');

    auto versionName = ctx.state.enterNameConstant(version);
    // send->recv is already a UCL (e.g. ActiveRecord::Migration). Flatten it with the
    // new segments instead of nesting UCLs (which would violate the ENFORCE).
    auto recvUcl = ast::cast_tree<ast::UnresolvedConstantLit>(send->recv);
    ENFORCE(recvUcl != nullptr);
    absl::InlinedVector<core::NameRef, 4> names;
    absl::InlinedVector<core::LocOffsets, 4> locs;
    for (auto &[n, l] : recvUcl->parts()) {
        names.push_back(n);
        locs.push_back(l);
    }
    names.push_back(core::Names::Constants::Compatibility());
    locs.push_back(arg->loc);
    names.push_back(versionName);
    locs.push_back(arg->loc);
    cdef->ancestors.emplace_back(ast::MK::UnresolvedConstant(
        std::move(recvUcl->rootScope_), names, locs));
    cdef->ancestors.erase(cdef->ancestors.begin(), cdef->ancestors.begin() + 1);
}

}; // namespace sorbet::rewriter
