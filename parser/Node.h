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

class NodeWithExpr final : public Node {
    ast::ExpressionPtr desugaredExpr;

public:
    const std::unique_ptr<Node> wrappedNode;

    NodeWithExpr(std::unique_ptr<Node> wrappedNode, ast::ExpressionPtr desugaredExpr)
        : Node(wrappedNode->loc), desugaredExpr(std::move(desugaredExpr)), wrappedNode(std::move(wrappedNode)) {
        ENFORCE(this->wrappedNode != nullptr, "Can't create NodeWithExpr with a null wrappedNode");
        ENFORCE(this->desugaredExpr != nullptr, "Can't create NodeWithExpr with a null desugaredExpr for node: {}",
                wrappedNode->nodeName());
    }
    virtual ~NodeWithExpr() = default;

    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const final {
        return wrappedNode->toStringWithTabs(gs, tabs);
    }

    virtual std::string toJSON(const core::GlobalState &gs, int tabs = 0) final {
        return wrappedNode->toJSON(gs, tabs);
    }

    virtual std::string toJSONWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) final {
        return wrappedNode->toJSONWithLocs(gs, file, tabs);
    }

    virtual std::string toWhitequark(const core::GlobalState &gs, int tabs = 0) final {
        return wrappedNode->toWhitequark(gs, tabs);
    }

    virtual std::string nodeName() const final {
        return wrappedNode->nodeName();
    }

    virtual ast::ExpressionPtr takeDesugaredExpr() final {
        ENFORCE(this->desugaredExpr != nullptr,
                "Tried to call make a second call to `takeDesugaredExpr()` on a NodeWithExpr");

        // We know each `NodeAndExpr` object's `takeDesugaredExpr()` will be called at most once, either:
        // 1. When its parent node is being translated in `prism/Translator.cc`,
        //    and this value is used to create that parent's expr.
        // 2. When this node is visted by `node2TreeImpl` in `PrismDesugar.cc`,
        //    and this value is called from the `NodeWithExpr` case
        //
        // Because of this, we don't need to make any copies here. Just move this value out,
        // and hand exclusive ownership to the caller.
        return std::move(this->desugaredExpr);
    }

    virtual bool hasDesugaredExpr() final {
        return this->desugaredExpr != nullptr;
    }

    virtual const ast::ExpressionPtr &peekDesugaredExpr() const final {
        return this->desugaredExpr;
    }

    // Like `parser::cast_node`, but can cast a `NodeWithExpr` *as if* it was its wrapped node.
    template <class To> static To *cast_node(parser::Node *what) {
        if (auto casted = parser::cast_node<To>(what)) {
            categoryCounterInc("prism_cast_node", "correct");
            return casted;
        }

        if (auto wrapper = parser::cast_node<parser::NodeWithExpr>(what)) {
            categoryCounterInc("prism_cast_node", "delegated");
            return parser::cast_node<To>(wrapper->wrappedNode.get());
        } else {
            categoryCounterInc("prism_cast_node", "miss");
            return nullptr;
        }
    }

    // Like `parser::isa_node`, but can check a `NodeWithExpr` *as if* it was its wrapped node.
    template <class To> static bool isa_node(parser::Node *what) {
        return NodeWithExpr::cast_node<To>(what) != nullptr;
    }
};

using NodeVec = InlinedVector<std::unique_ptr<Node>, 4>;

inline NodeVec NodeVec1(std::unique_ptr<Node> node) {
    NodeVec result;
    result.emplace_back(std::move(node));
    return result;
}

#include "parser/Node_gen.h"
}; // namespace sorbet::parser

#endif
