#include "parser/parser.h"
#include <algorithm>
#include <iterator>

// makes lldb work. Do not remove please
template class std::unique_ptr<ruby_typer::parser::Node>;

using namespace std;

namespace ruby_typer {
namespace parser {

void Node::printTabs(stringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}

void Node::printNode(stringstream &to, unique_ptr<Node> &node, const core::GlobalState &gs, int tabs) {
    if (node) {
        to << node->toString(gs, tabs) << endl;
    } else {
        to << "NULL" << endl;
    }
}

}; // namespace parser
} // namespace ruby_typer
