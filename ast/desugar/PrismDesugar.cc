#include <algorithm>

#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_replace.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/desugar/PrismDesugar.h"
#include "ast/verifier/verifier.h"
#include "common/common.h"
#include "common/strings/formatting.h"
#include "core/Names.h"
#include "core/errors/desugar.h"
#include "core/errors/internal.h"

// Redefinition to avoid including parser/prism/Parser.h which has additional dependencies
namespace sorbet::parser::Prism {
struct PrismFallback {};
} // namespace sorbet::parser::Prism

namespace sorbet::ast::prismDesugar {

using namespace std;

namespace {

// Translate a tree to an expression.
ExpressionPtr node2TreeImpl(unique_ptr<parser::Node> &what) {
    ENFORCE(what != nullptr);
    ENFORCE(what->hasDesugaredExpr(), "Node has no desugared expression");

    auto expr = what->takeDesugaredExpr();
    ENFORCE(expr != nullptr, "Node has null desugared expr");

    return expr;
}

ExpressionPtr liftTopLevel(core::LocOffsets loc, ExpressionPtr what) {
    ClassDef::RHS_store rhs;
    ClassDef::ANCESTORS_store ancestors;
    ancestors.emplace_back(MK::Constant(loc, core::Symbols::todo()));
    auto insSeq = cast_tree<InsSeq>(what);
    if (insSeq) {
        rhs.reserve(insSeq->stats.size() + 1);
        for (auto &stat : insSeq->stats) {
            rhs.emplace_back(move(stat));
        }
        rhs.emplace_back(move(insSeq->expr));
    } else {
        rhs.emplace_back(move(what));
    }
    return make_expression<ClassDef>(loc, loc, core::Symbols::root(), MK::EmptyTree(), move(ancestors), move(rhs),
                                     ClassDef::Kind::Class);
}
} // namespace

ExpressionPtr node2Tree(core::MutableContext ctx, unique_ptr<parser::Node> what, bool preserveConcreteSyntax) {
    try {
        // Callers should not pass null - when Prism falls back, use legacy desugar instead
        ENFORCE(what != nullptr, "node2Tree called with null tree");

        auto liftedClassDefLoc = what->loc;
        auto result = node2TreeImpl(what);
        if (result.loc().exists()) {
            // If the desugared expression has a different loc, we want to use that. This can happen
            // because (:block (:send)) desugars to (:send (:block)), but the (:block) node just has
            // the loc of the `do ... end`, while the (:send) has the whole loc
            //
            // But if we desugared to EmptyTree (either intentionally or because there was an
            // unsupported node type), we want to use the loc of the original node.
            liftedClassDefLoc = result.loc();
        }
        result = liftTopLevel(liftedClassDefLoc, move(result));
        auto verifiedResult = Verifier::run(ctx, move(result));
        return verifiedResult;
    } catch (SorbetException &) {
        throw;
    }
}
} // namespace sorbet::ast::prismDesugar
