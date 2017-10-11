#include "Desugar.h"
#include "ast/ast.h"
#include "common/common.h"

namespace ruby_typer {
namespace ast {
namespace desugar {
using namespace parser;

std::unique_ptr<Stat> Desugar::yesPlease(parser::Node *what) {
    if (what == nullptr)
        return std::unique_ptr<EmptyTree>();

    typecase(what, [](Alias *a) { std::cout << "This is alias!" << std::endl; },
             [](And *b) { std::cout << "This is And!" << std::endl; });
}
} // namespace desugar
} // namespace ast
} // namespace ruby_typer