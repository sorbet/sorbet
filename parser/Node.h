#ifndef SORBET_PARSER_NODE_H
#define SORBET_PARSER_NODE_H

#include "ast/Trees.h"
#include "common/common.h"
#include "core/core.h"
#include <memory>
#include <string>

namespace sorbet::parser {

class Node {
public:
    Node(core::LocOffsets loc) : loc(loc) {
        ENFORCE(loc.exists(), "Location of parser node is none");
    }
    virtual ~Node() = default;
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const = 0;
    std::string toString(const core::GlobalState &gs) const {
        return toStringWithTabs(gs);
    }
    virtual std::string toJSON(const core::GlobalState &gs, int tabs = 0) = 0;
    virtual std::string toJSONWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) = 0;
    virtual std::string toWhitequark(const core::GlobalState &gs, int tabs = 0) = 0;
    virtual std::string nodeName() const = 0;
    core::LocOffsets loc;
    std::unique_ptr<Node> deepCopy() const;

    virtual ast::ExpressionPtr takeDesugaredExpr() {
        return nullptr;
    }

    virtual bool hasDesugaredExpr() {
        return false;
    }

    virtual const ast::ExpressionPtr &peekDesugaredExpr() const {
        static const ast::ExpressionPtr nullExpr = nullptr;
        return nullExpr;
    }

protected:
    void printTabs(fmt::memory_buffer &to, int count) const;
    void printNode(fmt::memory_buffer &to, const std::unique_ptr<Node> &node, const core::GlobalState &gs,
                   int tabs) const;
    void printNodeJSON(fmt::memory_buffer &to, const std::unique_ptr<Node> &node, const core::GlobalState &gs,
                       int tabs) const;
    void printNodeJSONWithLocs(fmt::memory_buffer &to, const std::unique_ptr<Node> &node, const core::GlobalState &gs,
                               core::FileRef file, int tabs) const;
    void printNodeWhitequark(fmt::memory_buffer &to, const std::unique_ptr<Node> &node, const core::GlobalState &gs,
                             int tabs) const;
};

template <class To> To *cast_node(Node *what) {
    static_assert(!std::is_pointer_v<To>, "To has to be a pointer");
    static_assert(std::is_assignable_v<Node *&, To *>, "Ill Formed To, has to be a subclass of Expression");
    static_assert(std::is_final_v<To>, "To is not final");
    return fast_cast<Node, To>(what);
}

template <class To> bool isa_node(Node *what) {
    return cast_node<To>(what) != nullptr;
}

using NodeVec = InlinedVector<std::unique_ptr<Node>, 4>;

inline NodeVec NodeVec1(std::unique_ptr<Node> node) {
    NodeVec result;
    result.emplace_back(std::move(node));
    return result;
}

#include "parser/Node_gen.h"
}; // namespace sorbet::parser

#endif
