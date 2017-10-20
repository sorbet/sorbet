#include "ast/ast.h"
#include "common/common.h"

#include <memory>
#include <string>
#include <vector>

namespace ruby_typer {
namespace parser {

using ruby_typer::ast::FileRef;
using ruby_typer::ast::NameRef;
using std::move;
using std::unique_ptr;
using std::vector;

class Loc {
public:
    FileRef file;
    u4 begin_pos, end_pos;

    static Loc none(FileRef file) {
        return Loc{file, (u4)-1, (u4)-1};
    }

    bool is_none() {
        return begin_pos == (u4)-1 && end_pos == (u4)-1;
    }

    Loc() : file(0), begin_pos(-1), end_pos(-1){};
    Loc(FileRef file, u4 begin, u4 end) : file(file), begin_pos(begin), end_pos(end){};

    Loc &operator=(const Loc &rhs) = default;
    Loc &operator=(Loc &&rhs) = default;
    Loc(const Loc &rhs) = default;
    Loc(Loc &&rhs) = default;
};

class Node {
public:
    Node(Loc loc) : loc(loc) {}
    virtual ~Node() = default;
    virtual std::string toString(ast::ContextBase &ctx, int tabs = 0) = 0;
    virtual std::string nodeName() = 0;
    Loc loc;

protected:
    void printTabs(std::stringstream &to, int count);
    void printNode(std::stringstream &to, unique_ptr<Node> &node, ast::ContextBase &ctx, int tabs);
};

#include "parser/Node_gen.h"
}; // namespace parser
}; // namespace ruby_typer
