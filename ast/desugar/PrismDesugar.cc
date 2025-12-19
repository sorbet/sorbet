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

ExpressionPtr node2Tree(core::MutableContext ctx, unique_ptr<parser::Node> what, bool preserveConcreteSyntax) {
    try {
        // Callers should not pass null - when Prism falls back, use legacy desugar instead
        ENFORCE(what != nullptr, "node2Tree called with null tree");
        ENFORCE(what->hasDesugaredExpr(), "Node has no desugared expression");

        auto result = what->takeDesugaredExpr();
        ENFORCE(result != nullptr, "Node has null desugared expr");

        auto verifiedResult = Verifier::run(ctx, move(result));
        return verifiedResult;
    } catch (SorbetException &) {
        throw;
    }
}
} // namespace sorbet::ast::prismDesugar
