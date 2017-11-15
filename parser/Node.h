#include "absl/container/inlined_vector.h"
#include "common/common.h"
#include "core/core.h"
#include <memory>
#include <string>
#include <vector>

namespace ruby_typer {
namespace parser {

using core::Loc;
using core::NameRef;
using std::move;
using std::unique_ptr;
using std::vector;

class Node {
public:
    Node(Loc loc) : loc(loc) {
        DEBUG_ONLY(Error::check(!loc.is_none(), "Location of parser node is none"));
    }
    virtual ~Node() = default;
    virtual std::string toString(core::GlobalState &gs, int tabs = 0) = 0;
    virtual std::string nodeName() = 0;
    Loc loc;

protected:
    void printTabs(std::stringstream &to, int count);
    void printNode(std::stringstream &to, unique_ptr<Node> &node, core::GlobalState &gs, int tabs);
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

typedef InlinedVector<std::unique_ptr<Node>, 4> NodeVec;

#include "parser/Node_gen.h"
}; // namespace parser
}; // namespace ruby_typer
