#include "dsl/DSLBuilder.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names/dsl.h"
#include "core/core.h"
#include "dsl/dsl.h"
#include "dsl/util.h"

using namespace std;

namespace sorbet {
namespace dsl {
vector<std::unique_ptr<ast::Expression>> DSLBuilder::replaceDSL(core::MutableContext ctx, ast::Send *send) {
    vector<unique_ptr<ast::Expression>> empty;

    bool nilable = false;
    unique_ptr<ast::Expression> type;
    core::NameRef name = core::NameRef::noName();

    if (send->fun == core::Names::dslOptional()) {
        nilable = true;
    } else if (send->fun == core::Names::dslRequired()) {
    } else {
        return empty;
    }

    if (send->args.size() < 2) {
        return empty;
    }
    auto *sym = ast::cast_tree<ast::Literal>(send->args[0].get());
    if (sym == nullptr || !sym->isSymbol(ctx)) {
        return empty;
    }
    name = sym->asSymbol(ctx);

    auto *klass = ast::cast_tree<ast::ConstantLit>(send->args[1].get());
    if (klass == nullptr) {
        return empty;
    }
    type = ASTUtil::dupType(send->args[1].get());

    ast::Hash *opts = nullptr;
    if (send->args.size() > 2) {
        opts = ast::cast_tree<ast::Hash>(send->args[2].get());
    }
    if (opts != nullptr) {
        if (ASTUtil::getHashValue(ctx, opts, core::Names::default_())) {
            nilable = false;
        }
    }

    auto loc = send->loc;

    vector<unique_ptr<ast::Expression>> stats;

    // def self.<prop>
    stats.emplace_back(ast::MK::Sig1(loc, ast::MK::Symbol(loc, name), ASTUtil::dupType(type.get()),
                                     ast::MK::Ident(loc, core::Symbols::NilClass())));
    stats.emplace_back(ast::MK::Method1(loc, name, ast::MK::Local(loc, name), ast::MK::EmptyTree(loc), true));

    // def self.get_<prop>
    core::NameRef getName = ctx.state.enterNameUTF8("get_" + name.data(ctx).toString(ctx));
    stats.emplace_back(ast::MK::Sig0(loc, ASTUtil::dupType(type.get())));
    stats.emplace_back(
        ast::MK::Method(loc, getName, ast::MethodDef::ARGS_store(), ast::MK::Unsafe(loc, ast::MK::Nil(loc)), true));

    // def <prop>()
    stats.emplace_back(ast::MK::Sig0(loc, ASTUtil::dupType(type.get())));
    stats.emplace_back(
        ast::MK::Method(loc, name, ast::MethodDef::ARGS_store(), ast::MK::Unsafe(loc, ast::MK::Nil(loc))));

    return stats;
}
} // namespace dsl
} // namespace sorbet
