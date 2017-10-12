#include "parser/parser.h"

// makes lldb work. Do not remove please
template class std::unique_ptr<ruby_typer::parser::Node>;
namespace ruby_typer {
namespace parser {

void Node::printTabs(std::stringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}

void Node::printNode(std::stringstream &to, unique_ptr<Node> &node, ast::ContextBase &ctx, int tabs) {
    if (node) {
        to << node->toString(ctx, tabs) << std::endl;
    } else {
        to << "NULL" << std::endl;
    }
}

}; // namespace parser
} // namespace ruby_typer
