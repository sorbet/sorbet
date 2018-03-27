#include "dsl/Minitest.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names/dsl.h"
#include "core/core.h"
#include "core/errors/dsl.h"
#include "dsl/dsl.h"

using namespace std;

namespace ruby_typer {
namespace dsl {

unique_ptr<ast::Expression> recurse(core::MutableContext ctx, unique_ptr<ast::Expression> body);

unique_ptr<ast::Expression> prepareBody(core::MutableContext ctx, unique_ptr<ast::Expression> body) {
    body = recurse(ctx, move(body));

    auto bodySeq = ast::cast_tree<ast::InsSeq>(body.get());
    if (bodySeq) {
        for (auto &exp : bodySeq->stats) {
            exp = recurse(ctx, move(exp));
        }
        bodySeq->expr = recurse(ctx, move(bodySeq->expr));
    }
    return body;
}

string to_s(core::Context ctx, unique_ptr<ast::Expression> &arg) {
    auto argLit = ast::cast_tree<ast::Literal>(arg.get());
    std::string argString;
    if (argLit != nullptr) {
        if (argLit->isString(ctx)) {
            return argLit->asString(ctx).toString(ctx);
        } else if (argLit->isSymbol(ctx)) {
            return argLit->asSymbol(ctx).toString(ctx);
        }
    }
    auto argConstant = ast::cast_tree<ast::ConstantLit>(arg.get());
    if (argConstant != nullptr) {
        return argConstant->cnst.toString(ctx);
    }
    return arg->toString(ctx);
}

unique_ptr<ast::Expression> replaceDSLSingle(core::MutableContext ctx, ast::Send *send) {
    if (send->block == nullptr) {
        return nullptr;
    }

    auto self = ast::cast_tree<ast::Self>(send->recv.get());
    if (self == nullptr) {
        return nullptr;
    }

    if (send->args.size() == 0 && send->fun == core::Names::before()) {
        return ast::MK::Method0(send->loc, core::Names::initialize(), prepareBody(ctx, move(send->block->body)));
    }

    if (send->args.size() != 1) {
        return nullptr;
    }
    auto &arg = send->args[0];
    auto argString = to_s(ctx, arg);

    vector<unique_ptr<ast::Expression>> stats;
    if (send->fun == core::Names::describe()) {
        ast::ClassDef::ANCESTORS_store ancestors;
        ancestors.emplace_back(ast::MK::Self(arg->loc));
        ast::ClassDef::RHS_store rhs;
        rhs.emplace_back(prepareBody(ctx, move(send->block->body)));
        auto name = ast::MK::Constant(arg->loc, ast::MK::EmptyTree(arg->loc),
                                      ctx.state.enterNameConstant("<class_" + argString + ">"));
        return ast::MK::Class(send->loc, move(name), move(ancestors), move(rhs), ast::ClassDefKind::Class);
    } else if (send->fun == core::Names::it()) {
        auto name = ctx.state.enterNameUTF8("<test_" + argString + ">");
        return ast::MK::Method0(send->loc, move(name), prepareBody(ctx, move(send->block->body)));
    }

    return nullptr;
}

unique_ptr<ast::Expression> recurse(core::MutableContext ctx, unique_ptr<ast::Expression> body) {
    auto bodySend = ast::cast_tree<ast::Send>(body.get());
    if (bodySend) {
        auto change = replaceDSLSingle(ctx, bodySend);
        if (change) {
            return change;
        }
    }
    return body;
}

vector<unique_ptr<ast::Expression>> Minitest::replaceDSL(core::MutableContext ctx, ast::Send *send) {
    vector<unique_ptr<ast::Expression>> stats;
    auto exp = replaceDSLSingle(ctx, send);
    if (exp != nullptr) {
        stats.emplace_back(move(exp));
    }
    return stats;
}

} // namespace dsl
}; // namespace ruby_typer
