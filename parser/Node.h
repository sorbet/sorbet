#ifndef SORBET_PARSER_NODE_H
#define SORBET_PARSER_NODE_H

#include "common/common.h"
#include "core/core.h"
#include <memory>
#include <string>

namespace sorbet::parser {

#include "parser/Node_gen_tag.h"

struct NodeDeleter;
class NodePtr;

class Node {
    friend NodeDeleter;
    void deleteTagged();

public:
    Node(NodeTag tag, core::LocOffsets loc) : tag(tag), loc(loc) {
        ENFORCE(loc.exists(), "Location of parser node is none");
    }
    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string toString(const core::GlobalState &gs) const {
        return toStringWithTabs(gs);
    }
    std::string toJSON(const core::GlobalState &gs, int tabs = 0);
    std::string toJSONWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0);
    std::string toWhitequark(const core::GlobalState &gs, int tabs = 0);
    std::string nodeName() const;
    NodeTag tag;
    core::LocOffsets loc;

protected:
    void printTabs(fmt::memory_buffer &to, int count) const;
    void printNode(fmt::memory_buffer &to, const NodePtr &node, const core::GlobalState &gs,
                   int tabs) const;
    void printNodeJSON(fmt::memory_buffer &to, const NodePtr &node, const core::GlobalState &gs,
                       int tabs) const;
    void printNodeJSONWithLocs(fmt::memory_buffer &to, const NodePtr &node, const core::GlobalState &gs,
                               core::FileRef file, int tabs) const;
    void printNodeWhitequark(fmt::memory_buffer &to, const NodePtr &node, const core::GlobalState &gs,
                             int tabs) const;
};

struct NodeDeleter {
    void operator()(Node *node) {
        node->deleteTagged();
    }
};

class NodePtr : public std::unique_ptr<Node, NodeDeleter> {
    using Base = std::unique_ptr<Node, NodeDeleter>;

public:
    NodePtr() = default;
    NodePtr(std::nullptr_t) : Base(nullptr) {}
    explicit NodePtr(Node *node) : Base(node) {}

    NodePtr(NodePtr &&p) = default;
    NodePtr &operator=(NodePtr &&p) = default;

    // typecase support
    template <class To> static bool isa(const NodePtr &node);
    template <class To> static const To &cast(const NodePtr &node);
    template <class To> static To &cast(NodePtr &node) {
        return const_cast<To &>(cast<To>(static_cast<const NodePtr &>(node)));
    }

    NodeTag tag() const {
        return get()->tag;
    }
};

template <class N, typename... Args>
NodePtr make_node(Args &&... args) {
    return NodePtr(new N(std::forward<Args>(args)...));
}

template <class To> bool isa_node(const NodePtr &what) {
    return what != nullptr && what.tag() == NodeToTag<To>::value;
}

template <class To> bool isa_node(const Node *what) {
    return what != nullptr && what->tag == NodeToTag<To>::value;
 }

// We disallow casting on temporary values because the lifetime of the returned value is
// tied to the temporary, but it is possible for the temporary to be destroyed at the end
// of the current statement, leading to use-after-free bugs.
template <class To> To *cast_node(NodePtr &&what) = delete;

template <class To> To *cast_node(Node *what) {
    if (isa_node<To>(what)) {
        return static_cast<To *>(what);
    } else {
        return nullptr;
    }
}

template <class To> To *cast_node(NodePtr &what) {
    return cast_node<To>(what.get());
}

template <class To> const To *cast_node(const Node *what) {
    if (isa_node<To>(what)) {
        return static_cast<To *>(what);
    } else {
        return nullptr;
    }
}

template <class To> const To *cast_node(const NodePtr &what) {
    return cast_node<To>(what.get());
}

// We disallow casting on temporary values because the lifetime of the returned value is
// tied to the temporary, but it is possible for the temporary to be destroyed at the end
// of the current statement, leading to use-after-free bugs.
template <class To> To &cast_node_nonnull(NodePtr &&what) = delete;

template <class To> To &cast_node_nonnull(NodePtr &what) {
    ENFORCE(isa_node<To>(what), "cast_node_nonnull failed!");
    return *static_cast<To *>(what.get());
}

template <class To> const To &cast_node_nonnull(const NodePtr &what) {
    ENFORCE(isa_node<To>(what), "cast_node_nonnull failed!");
    return *static_cast<To *>(what.get());
}

template <class To> inline bool NodePtr::isa(const NodePtr &what) {
    return isa_node<To>(what);
}

template <> inline bool NodePtr::isa<NodePtr>(const NodePtr &what) {
    return true;
}

template <class To> inline const To &NodePtr::cast(const NodePtr &what) {
    return cast_node_nonnull<To>(what);
}

template <> inline const NodePtr &NodePtr::cast<NodePtr>(const NodePtr &what) {
    return what;
}

using NodeVec = InlinedVector<NodePtr, 4>;

#include "parser/Node_gen.h"
}; // namespace sorbet::parser

#endif
