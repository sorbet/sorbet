#include "dsl/Sinatra.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names/dsl.h"
#include "core/core.h"
#include "core/errors/dsl.h"
#include "dsl/dsl.h"

using namespace std;

namespace sorbet {
namespace dsl {

vector<unique_ptr<ast::Expression>> Sinatra::replaceDSL(core::MutableContext ctx, ast::MethodDef *mdef) {
    vector<unique_ptr<ast::Expression>> empty;

    if (mdef->name != core::Names::registered()) {
        return empty;
    }
    if (!mdef->isSelf()) {
        return empty;
    }

    mdef->name = core::Names::instanceRegistered();
    mdef->flags &= ~ast::MethodDef::SelfMethod;

    vector<unique_ptr<ast::Expression>> ret;
    auto loc = mdef->loc;

    ret.emplace_back(ast::MK::Send1(
        loc, ast::MK::Self(loc), core::Names::include(),
        ast::MK::UnresolvedConstant(
            loc, ast::MK::UnresolvedConstant(loc, ast::MK::EmptyTree(loc), core::Symbols::Sinatra().data(ctx).name),
            core::Symbols::SinatraBase().data(ctx).name)));

    auto inseq = ast::cast_tree<ast::InsSeq>(mdef->rhs.get());
    if (inseq) {
        auto stats = move(inseq->stats);
        inseq->stats.clear();
        for (auto &stat : stats) {
            typecase(stat.get(),
                     [&](ast::Send *send) {
                         if (send->fun == core::Names::helpers() && send->args.size() == 1) {
                             ret.emplace_back(ast::MK::Send1(send->loc, ast::MK::Self(loc), core::Names::include(),
                                                             move(send->args[0])));
                         } else {
                             inseq->stats.emplace_back(move(stat));
                         }
                     },
                     [&](ast::Expression *e) { inseq->stats.emplace_back(move(stat)); });
        }
        if (inseq->stats.empty()) {
            mdef->rhs = move(inseq->expr);
        }
    }

    ret.emplace_back(ast::MK::Method(loc, core::Names::instanceRegistered(), move(mdef->args), move(mdef->rhs),
                                     ast::MethodDef::DSLSynthesized));

    return ret;
}

} // namespace dsl
}; // namespace sorbet
