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
        to << node->toString(gs, tabs) << '\n';
    } else {
        to << "NULL" << '\n';
    }
}

void Node::printNodeJSON(stringstream &to, unique_ptr<Node> &node, const core::GlobalState &gs, int tabs) {
    if (node) {
        to << node->toJSON(gs, tabs);
    } else {
        to << "null";
    }
}

// https://stackoverflow.com/questions/7724448/simple-json-string-escape-for-c
std::string Node::escapeJSON(std::string from) {
    std::ostringstream ss;
    for (auto ch : from) {
        switch (ch) {
            case '\\':
                ss << "\\\\";
                break;
            case '"':
                ss << "\\\"";
                break;
            case '/':
                ss << "\\/";
                break;
            case '\b':
                ss << "\\b";
                break;
            case '\f':
                ss << "\\f";
                break;
            case '\n':
                ss << "\\n";
                break;
            case '\r':
                ss << "\\r";
                break;
            case '\t':
                ss << "\\t";
                break;
            default:
                ss << ch;
                break;
        }
    }
    return ss.str();
}

}; // namespace parser
} // namespace ruby_typer
