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

class Ident : public Node {
public:
    Ident(Loc loc, NameRef name) : Node(loc), name(name) {}

    NameRef name;
};

class Class : public Node {
public:
    Class(Loc loc, unique_ptr<Node> &&name, unique_ptr<Node> &&superclass, unique_ptr<Node> &&body)
        : Node(loc), name(move(name)), superclass(move(superclass)), body(move(body)) {}

    unique_ptr<Node> name;
    unique_ptr<Node> superclass;
    unique_ptr<Node> body;
};

class Module : public Node {
public:
    Module(Loc loc, unique_ptr<Node> name, unique_ptr<Node> &&body) : Node(loc), name(move(name)), body(move(body)) {}

    unique_ptr<Node> name;
    unique_ptr<Node> body;
};

class DefMethod : public Node {
public:
    DefMethod(Loc loc, NameRef name, unique_ptr<Node> &&args, unique_ptr<Node> &&body)
        : Node(loc), name(name), args(move(args)), body(move(body)) {}

    ast::NameRef name;
    unique_ptr<Node> args;
    unique_ptr<Node> body;
};
};
};
