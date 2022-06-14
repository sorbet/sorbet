#include "parser/parser.h"
#include "parser/Node_gen_case.h"
#include <algorithm>
#include <iterator>

// makes lldb work. Do not remove please
template class std::unique_ptr<sorbet::parser::Node, sorbet::parser::NodeDeleter>;

using namespace std;

namespace sorbet::parser {

void Node::deleteTagged() {
#define DELETE_NODE(name) delete static_cast<name *>(this);
    GENERATE_TAG_SWITCH(this->tag, DELETE_NODE);
#undef DELETE_NODE
}

string Node::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
#define TOSTRING(name) return static_cast<const name *>(this)->toStringWithTabs(gs, tabs);
    GENERATE_TAG_SWITCH(this->tag, TOSTRING);
#undef TOSTRING
}

string Node::toJSON(const core::GlobalState &gs, int tabs) {
#define TOJSON(name) return static_cast<name *>(this)->toJSON(gs, tabs);
    GENERATE_TAG_SWITCH(this->tag, TOJSON);
#undef TOJSON
}

string Node::toJSONWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) {
#define TOJSON(name) return static_cast<name *>(this)->toJSONWithLocs(gs, file, tabs);
    GENERATE_TAG_SWITCH(this->tag, TOJSON);
#undef TOJSON
}

string Node::toWhitequark(const core::GlobalState &gs, int tabs) {
#define TOWHITEQUARK(name) return static_cast<name *>(this)->toWhitequark(gs, tabs);
    GENERATE_TAG_SWITCH(this->tag, TOWHITEQUARK);
#undef TOWHITEQUARK
}

string Node::nodeName() const {
#define NODENAME(name) return static_cast<const name *>(this)->nodeName();
    GENERATE_TAG_SWITCH(this->tag, NODENAME);
#undef NODENAME
}

void Node::printTabs(fmt::memory_buffer &to, int count) const {
    int i = 0;
    while (i < count) {
        fmt::format_to(std::back_inserter(to), "{}", "  ");
        i++;
    }
}

void Node::printNode(fmt::memory_buffer &to, const unique_ptr<Node, NodeDeleter> &node, const core::GlobalState &gs,
                     int tabs) const {
    if (node) {
        fmt::format_to(std::back_inserter(to), "{}\n", node->toStringWithTabs(gs, tabs));
    } else {
        fmt::format_to(std::back_inserter(to), "NULL\n");
    }
}

void Node::printNodeJSON(fmt::memory_buffer &to, const unique_ptr<Node, NodeDeleter> &node, const core::GlobalState &gs,
                         int tabs) const {
    if (node) {
        fmt::format_to(std::back_inserter(to), "{}", node->toJSON(gs, tabs));
    } else {
        fmt::format_to(std::back_inserter(to), "null");
    }
}

void Node::printNodeJSONWithLocs(fmt::memory_buffer &to, const unique_ptr<Node, NodeDeleter> &node, const core::GlobalState &gs,
                                 core::FileRef file, int tabs) const {
    if (node) {
        fmt::format_to(std::back_inserter(to), "{}", node->toJSONWithLocs(gs, file, tabs));
    } else {
        fmt::format_to(std::back_inserter(to), "null");
    }
}

void Node::printNodeWhitequark(fmt::memory_buffer &to, const unique_ptr<Node, NodeDeleter> &node, const core::GlobalState &gs,
                               int tabs) const {
    if (node) {
        fmt::format_to(std::back_inserter(to), "{}", node->toWhitequark(gs, tabs));
    } else {
        fmt::format_to(std::back_inserter(to), "nil");
    }
}

} // namespace sorbet::parser
