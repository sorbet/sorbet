#include "ast/Names.h"
#include "common/common.h"

#include <memory>
#include <vector>

namespace ruby_typer {
namespace parser {

using ruby_typer::ast::NameRef;
using std::move;
using std::unique_ptr;
using std::vector;

struct Loc {
    u4 begin_pos, end_pos;

    static Loc none() {
        return Loc{(u4)-1, (u4)-1};
    }

    bool is_none() {
        return begin_pos == (u4)-1 && end_pos == (u4)-1;
    }
};

class Node {
public:
    Node(Loc loc) : loc(loc) {}
    virtual ~Node() = default;

    Loc loc;
};

#include "parser/Node_gen.h"
}; // namespace parser
}; // namespace ruby_typer
