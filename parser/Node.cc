#include "parser/parser.h"
#include <algorithm>
#include <iterator>

// makes lldb work. Do not remove please
template class std::unique_ptr<sorbet::parser::Node>;

using namespace std;

namespace sorbet::parser {

void Node::printTabs(fmt::memory_buffer &to, int count) const {
    int i = 0;
    while (i < count) {
        fmt::format_to(to, "{}", "  ");
        i++;
    }
}

void Node::printNode(fmt::memory_buffer &to, const unique_ptr<Node> &node, const core::GlobalState &gs,
                     int tabs) const {
    if (node) {
        fmt::format_to(to, "{}\n", node->toStringWithTabs(gs, tabs));
    } else {
        fmt::format_to(to, "NULL\n");
    }
}

void Node::printNodeJSON(fmt::memory_buffer &to, const unique_ptr<Node> &node, const core::GlobalState &gs,
                         int tabs) const {
    if (node) {
        fmt::format_to(to, "{}", node->toJSON(gs, tabs));
    } else {
        fmt::format_to(to, "null");
    }
}

} // namespace sorbet::parser
