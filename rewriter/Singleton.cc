#include "rewriter/Singleton.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

bool isSingleton(const ast::TreePtr &expr) {
    if (auto *sym = ast::cast_tree<ast::UnresolvedConstantLit>(expr)) {
        return sym->cnst == core::Names::Constants::Singleton();
    }
    return false;
}

} // namespace

std::vector<ast::TreePtr> Singleton::run(core::MutableContext ctx, const ast::Send *send) {
    vector<ast::TreePtr> stmts{};

    if (!send->recv.isSelfReference()) {
        return stmts;
    }

    if (send->fun != core::Names::include()) {
        return stmts;
    }

    if (send->args.size() != 1) {
        return stmts;
    }

    if (!isSingleton(send->args.front())) {
        return stmts;
    }

    auto loc = send->loc;

    {
        auto sig = ast::MK::Sig0(loc, ast::MK::Send0(loc, ast::MK::T(loc), core::Names::attachedClass()));
        ast::cast_tree_nonnull<ast::Send>(sig).args.emplace_back(ast::MK::Symbol(loc, core::Names::final_()));
        stmts.emplace_back(std::move(sig));
    }

    {
        auto method = ast::MK::SyntheticMethod0(loc, loc, core::Names::instance(), ast::MK::RaiseUnimplemented(loc));
        ast::cast_tree_nonnull<ast::MethodDef>(method).flags.isSelfMethod = true;
        stmts.emplace_back(std::move(method));
    }

    stmts.emplace_back(send->deepCopy());

    return stmts;
}

} // namespace sorbet::rewriter
