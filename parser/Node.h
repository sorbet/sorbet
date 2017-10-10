#include "ast/Names.h"
#include "common/common.h"

#include <memory>
#include <vector>

namespace ruby_typer {
namespace parser {

using ruby_typer::ast::NameRef;
using std::unique_ptr;
using std::vector;
using std::move;

struct Loc {
    u4 begin_pos, end_pos;
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
