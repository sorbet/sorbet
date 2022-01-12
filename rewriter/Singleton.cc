#include "absl/algorithm/container.h"
#include <algorithm> // for std::move

#include "ast/Helpers.h"
#include "core/GlobalState.h"
#include "rewriter/Singleton.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

bool isFinal(const ast::ExpressionPtr &stmt) {
    auto *send = ast::cast_tree<ast::Send>(stmt);
    if (send == nullptr) {
        return false;
    }

    if (!send->recv.isSelfReference()) {
        return false;
    }

    if (send->hasPosArgs() || send->hasKwArgs()) {
        return false;
    }

    if (send->fun != core::Names::declareFinal()) {
        return false;
    }

    return true;
}

bool isIncludeSingleton(const ast::ExpressionPtr &stmt) {
    const auto *send = ast::cast_tree<ast::Send>(stmt);
    if (send == nullptr) {
        return false;
    }

    if (!send->recv.isSelfReference()) {
        return false;
    }

    if (send->fun != core::Names::include()) {
        return false;
    }

    if (send->numPosArgs() != 1 || send->hasKwArgs()) {
        return false;
    }

    auto *sym = ast::cast_tree<ast::UnresolvedConstantLit>(send->getPosArg(0));
    if (sym == nullptr || sym->cnst != core::Names::Constants::Singleton()) {
        return false;
    }

    return true;
}

} // namespace

void Singleton::run(core::MutableContext ctx, ast::ClassDef *cdef) {
    auto *it = absl::c_find_if(cdef->rhs, isIncludeSingleton);
    if (it == cdef->rhs.end()) {
        return;
    }

    auto finalKlass = absl::c_any_of(cdef->rhs, isFinal);
    auto loc = it->loc();

    ast::ClassDef::RHS_store newRHS;
    newRHS.reserve(cdef->rhs.size() + 2);

    std::move(cdef->rhs.begin(), it, std::back_inserter(newRHS));

    {
        auto sig = ast::MK::Sig0(
            loc, ast::MK::Send0(loc, ast::MK::T(loc), core::Names::attachedClass(), loc.copyWithZeroLength()));
        if (finalKlass) {
            ast::cast_tree_nonnull<ast::Send>(sig).addPosArg(ast::MK::Symbol(loc, core::Names::final_()));
        }
        newRHS.emplace_back(std::move(sig));
    }

    {
        auto method = ast::MK::SyntheticMethod0(loc, loc, core::Names::instance(), ast::MK::RaiseUnimplemented(loc));
        ast::cast_tree_nonnull<ast::MethodDef>(method).flags.isSelfMethod = true;
        newRHS.emplace_back(std::move(method));
    }

    std::move(it, cdef->rhs.end(), std::back_inserter(newRHS));

    cdef->rhs = std::move(newRHS);

    return;
}

} // namespace sorbet::rewriter
