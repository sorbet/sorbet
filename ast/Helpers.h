#ifndef SRUBY_AST_HELPERSS_H
#define SRUBY_AST_HELPERSS_H

#include "ast/ast.h"
#include "core/Names/desugar.h"

namespace ruby_typer {
namespace ast {

class MK {
public:
    static std::unique_ptr<Expression> Send(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                            Send::ARGS_store args, u4 flags = 0, std::unique_ptr<Block> blk = nullptr) {
        auto send = std::make_unique<ast::Send>(loc, move(recv), fun, move(args), move(blk));
        send->flags = flags;
        return move(send);
    }

    static std::unique_ptr<Expression> Send1(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                             std::unique_ptr<Expression> arg1) {
        Send::ARGS_store nargs;
        nargs.emplace_back(move(arg1));
        return std::make_unique<ast::Send>(loc, move(recv), fun, move(nargs));
    }

    static std::unique_ptr<Expression> Send2(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                             std::unique_ptr<Expression> arg1, std::unique_ptr<Expression> arg2) {
        Send::ARGS_store nargs;
        nargs.emplace_back(move(arg1));
        nargs.emplace_back(move(arg2));
        return std::make_unique<ast::Send>(loc, move(recv), fun, move(nargs));
    }

    static std::unique_ptr<Expression> Send3(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                             std::unique_ptr<Expression> arg1, std::unique_ptr<Expression> arg2,
                                             std::unique_ptr<Expression> arg3) {
        Send::ARGS_store nargs;
        nargs.emplace_back(move(arg1));
        nargs.emplace_back(move(arg2));
        nargs.emplace_back(move(arg3));
        return std::make_unique<ast::Send>(loc, move(recv), fun, move(nargs));
    }

    static std::unique_ptr<Expression> Send0(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun) {
        Send::ARGS_store nargs;
        return std::make_unique<ast::Send>(loc, move(recv), fun, move(nargs));
    }

    static std::unique_ptr<Expression> Ident(core::Loc loc, core::SymbolRef symbol) {
        return std::make_unique<ast::Ident>(loc, symbol);
    }

    static std::unique_ptr<Reference> Local(core::Loc loc, core::NameRef name) {
        return std::make_unique<ast::UnresolvedIdent>(loc, UnresolvedIdent::Local, name);
    }

    static std::unique_ptr<Expression> cpRef(core::Loc loc, Reference &name) {
        if (UnresolvedIdent *nm = cast_tree<ast::UnresolvedIdent>(&name)) {
            return std::make_unique<ast::UnresolvedIdent>(loc, nm->kind, nm->name);
        }
        if (ast::Ident *id = cast_tree<ast::Ident>(&name)) {
            return std::make_unique<ast::Ident>(loc, id->symbol);
        }
        Error::notImplemented();
    }

    static std::unique_ptr<Expression> Assign(core::Loc loc, std::unique_ptr<Expression> lhs,
                                              std::unique_ptr<Expression> rhs) {
        if (auto *s = cast_tree<ast::Send>(lhs.get())) {
            s->args.emplace_back(move(rhs));
            return lhs;
        }

        return std::make_unique<ast::Assign>(loc, move(lhs), move(rhs));
    }

    static std::unique_ptr<Expression> Assign(core::Loc loc, core::NameRef name, std::unique_ptr<Expression> rhs) {
        return Assign(loc, Local(loc, name), move(rhs));
    }

    static std::unique_ptr<Expression> If(core::Loc loc, std::unique_ptr<Expression> cond,
                                          std::unique_ptr<Expression> thenp, std::unique_ptr<Expression> elsep) {
        return std::make_unique<ast::If>(loc, move(cond), move(thenp), move(elsep));
    }

    static std::unique_ptr<Expression> EmptyTree(core::Loc loc) {
        return std::make_unique<ast::EmptyTree>(loc);
    }

    static std::unique_ptr<Expression> Self(core::Loc loc) {
        return std::make_unique<ast::Self>(loc, core::Symbols::todo());
    }

    static std::unique_ptr<Expression> InsSeq(core::Loc loc, InsSeq::STATS_store stats,
                                              std::unique_ptr<Expression> expr) {
        return std::make_unique<ast::InsSeq>(loc, move(stats), move(expr));
    }

    static std::unique_ptr<Expression> Splat(core::Loc loc, std::unique_ptr<Expression> arg) {
        auto to_a = Send0(loc, move(arg), core::Names::to_a());
        return Send1(loc, Ident(loc, core::Symbols::Magic()), core::Names::splat(), move(to_a));
    }

    static std::unique_ptr<Expression> InsSeq1(core::Loc loc, std::unique_ptr<Expression> stat,
                                               std::unique_ptr<Expression> expr) {
        InsSeq::STATS_store stats;
        stats.emplace_back(move(stat));
        return InsSeq(loc, move(stats), move(expr));
    }

    static std::unique_ptr<Expression> True(core::Loc loc) {
        return std::make_unique<ast::BoolLit>(loc, true);
    }

    static std::unique_ptr<Expression> False(core::Loc loc) {
        return std::make_unique<ast::BoolLit>(loc, false);
    }

    static std::unique_ptr<Expression> Constant(core::Loc loc, std::unique_ptr<Expression> scope, core::NameRef name) {
        return std::make_unique<ast::ConstantLit>(loc, move(scope), name);
    }

    static std::unique_ptr<Expression> Int(core::Loc loc, int64_t val) {
        return std::make_unique<ast::IntLit>(loc, val);
    }
};

} // namespace ast
} // namespace ruby_typer

#endif // SRUBY_TREES_H
