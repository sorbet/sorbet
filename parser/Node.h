#include "ast/ast.h"
#include "common/common.h"

#include <memory>
#include <string>
#include <vector>

namespace ruby_typer {
namespace parser {

using ast::Loc;
using ast::NameRef;
using std::move;
using std::unique_ptr;
using std::vector;

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
