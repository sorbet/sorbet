#include "ast/Names.h"
#include "common/common.h"

#include <memory>

namespace sruby {
namespace parser {

using sruby::ast::NameRef;
using std::unique_ptr;

struct Loc {
    u4 begin_pos, end_pos;
};

class Node {
public:
    Node(Loc loc) : loc(loc) {}
    virtual ~Node() = default;

    Loc loc;
};

class Ident : public Node {
public:
    Ident(Loc loc, NameRef name) : Node(loc), name(name) {}

    NameRef name;
};

class Class : public Node {
public:
    ast::NameRef name;
    unique_ptr<Node> superclass;
    unique_ptr<Node> body;
};

class Module : public Node {
public:
    ast::NameRef name;
    unique_ptr<Node> body;
};

class Def : public Node {
public:
    ast::NameRef name;
    unique_ptr<Node> args;
    unique_ptr<Node> body;
};
};
};
