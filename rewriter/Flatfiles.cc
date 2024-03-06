#include "rewriter/Flatfiles.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {
optional<core::NameRef> getFieldName(core::MutableContext ctx, ast::Send &send) {
    if (auto propLit = ast::cast_tree<ast::Literal>(send.getPosArg(0))) {
        if (propLit->isSymbol()) {
            return propLit->asSymbol();
        }
    }
    if (send.numPosArgs() >= 2) {
        if (auto propLit = ast::cast_tree<ast::Literal>(send.getPosArg(1))) {
            if (propLit->isSymbol()) {
                return propLit->asSymbol();
            }
        }
    }
    return nullopt;
}

ast::Send *asFlatfileDo(ast::ExpressionPtr &stat) {
    auto *send = ast::cast_tree<ast::Send>(stat);
    if (send != nullptr && send->hasBlock() && send->fun == core::Names::flatfile()) {
        return send;
    } else {
        return nullptr;
    }
}

void handleFieldDefinition(core::MutableContext ctx, ast::ExpressionPtr &stat, vector<ast::ExpressionPtr> &methods) {
    if (auto send = ast::cast_tree<ast::Send>(stat)) {
        if ((send->fun != core::Names::from() && send->fun != core::Names::field() &&
             send->fun != core::Names::pattern()) ||
            !send->recv.isSelfReference() || send->numPosArgs() < 1) {
            return;
        }
        auto name = getFieldName(ctx, *send);
        if (!name) {
            return;
        }

        methods.emplace_back(ast::MK::Sig0(send->loc, ast::MK::Untyped(send->loc)));
        methods.emplace_back(ast::MK::SyntheticMethod0(send->loc, send->loc, *name, ast::MK::Nil(send->loc)));
        auto var = ast::MK::ResolvedLocal(send->loc, core::Names::arg0());
        auto setName = name->addEq(ctx);
        methods.emplace_back(ast::MK::Sig1(send->loc, ast::MK::Symbol(send->loc, core::Names::arg0()),
                                           ast::MK::Untyped(send->loc), ast::MK::Untyped(send->loc)));
        methods.emplace_back(
            ast::MK::SyntheticMethod1(send->loc, send->loc, setName, move(var), ast::MK::Nil(send->loc)));
    }
}

void Flatfiles::run(core::MutableContext ctx, ast::ClassDef *klass) {
    if (klass->kind != ast::ClassDef::Kind::Class || klass->ancestors.empty()) {
        return;
    }

    vector<ast::ExpressionPtr> methods;
    for (auto &stat : klass->rhs) {
        if (auto flatfileBlock = asFlatfileDo(stat)) {
            auto *block = flatfileBlock->block();
            if (auto *insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
                for (auto &stat : insSeq->stats) {
                    handleFieldDefinition(ctx, stat, methods);
                }
                handleFieldDefinition(ctx, insSeq->expr, methods);
            } else {
                handleFieldDefinition(ctx, block->body, methods);
            }
        }
    }

    if (methods.empty()) {
        klass = nullptr;
    }

    for (auto &m : methods) {
        klass->rhs.emplace_back(move(m));
    }
}
}; // namespace sorbet::rewriter
