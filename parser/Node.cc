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
        fmt::format_to(std::back_inserter(to), "{}", "  ");
        i++;
    }
}

void Node::printNode(fmt::memory_buffer &to, const unique_ptr<Node> &node, const core::GlobalState &gs,
                     int tabs) const {
    if (node) {
        fmt::format_to(std::back_inserter(to), "{}\n", node->toStringWithTabs(gs, tabs));
    } else {
        fmt::format_to(std::back_inserter(to), "NULL\n");
    }
}

void Node::printNodeJSON(fmt::memory_buffer &to, const unique_ptr<Node> &node, const core::GlobalState &gs,
                         int tabs) const {
    if (node) {
        fmt::format_to(std::back_inserter(to), "{}", node->toJSON(gs, tabs));
    } else {
        fmt::format_to(std::back_inserter(to), "null");
    }
}

void Node::printNodeJSONWithLocs(fmt::memory_buffer &to, const unique_ptr<Node> &node, const core::GlobalState &gs,
                                 core::FileRef file, int tabs) const {
    if (node) {
        fmt::format_to(std::back_inserter(to), "{}", node->toJSONWithLocs(gs, file, tabs));
    } else {
        fmt::format_to(std::back_inserter(to), "null");
    }
}

void Node::printNodeWhitequark(fmt::memory_buffer &to, const unique_ptr<Node> &node, const core::GlobalState &gs,
                               int tabs) const {
    if (node) {
        fmt::format_to(std::back_inserter(to), "{}", node->toWhitequark(gs, tabs));
    } else {
        fmt::format_to(std::back_inserter(to), "nil");
    }
}

} // namespace sorbet::parser
