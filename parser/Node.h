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
    virtual std::string toWhitequark(const core::GlobalState &gs, int tabs = 0) = 0;
    virtual std::string nodeName() = 0;
    core::LocOffsets loc;

protected:
    void printTabs(fmt::memory_buffer &to, int count) const;
    void printNode(fmt::memory_buffer &to, const std::unique_ptr<Node> &node, const core::GlobalState &gs,
                   int tabs) const;
    void printNodeJSON(fmt::memory_buffer &to, const std::unique_ptr<Node> &node, const core::GlobalState &gs,
                       int tabs) const;
    void printNodeWhitequark(fmt::memory_buffer &to, const std::unique_ptr<Node> &node, const core::GlobalState &gs,
                             int tabs) const;
};

template <class To> To *cast_node(Node *what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Node *&, To *>::value, "Ill Formed To, has to be a subclass of Expression");
#if __cplusplus >= 201402L
    static_assert(std::is_final<To>::value, "To is not final");
#elif __has_feature(is_final)
    static_assert(__is_final(To), "To is not final");
#else
    static_assert(false);
#endif
    return fast_cast<Node, To>(what);
}

template <class To> bool isa_node(Node *what) {
    return cast_node<To>(what) != nullptr;
}

using NodeVec = InlinedVector<std::unique_ptr<Node>, 4>;

#include "parser/Node_gen.h"
}; // namespace sorbet::parser
