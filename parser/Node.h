#include "ast/Names.h"
#include "common/common.h"

#include <memory>

namespace sruby {
namespace parser {

using sruby::ast::NameRef;
using std::unique_ptr;
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

#include "Node_gen.h"
};
};
