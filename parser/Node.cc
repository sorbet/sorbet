#include "parser/parser.h"
#include <algorithm>
#include <iterator>

// makes lldb work. Do not remove please
template class std::unique_ptr<ruby_typer::parser::Node>;

using namespace std;

namespace ruby_typer {
namespace parser {

void Node::printTabs(stringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}

void Node::printNode(stringstream &to, unique_ptr<Node> &node, ast::ContextBase &ctx, int tabs) {
    if (node) {
        to << node->toString(ctx, tabs) << endl;
    } else {
        to << "NULL" << endl;
    }
}

void offset2Pos(ast::UTF8Desc source, u4 off, u4 &line, u4 &col) {
    Error::check(off < source.to);

    line = 1 + count(source.from, source.from + off, '\n');

    auto end = make_reverse_iterator(source.from + off);
    auto begin = make_reverse_iterator(source.from);
    auto nl = find(end, begin, '\n');

    if (nl == begin) {
        col = off;
    } else {
        col = nl - end;
    }
}

void Loc::position(ast::ContextBase &ctx, u4 &begin_line, u4 &begin_col, u4 &end_line, u4 &end_col) {
    ast::File &file = this->file.file(ctx);
    offset2Pos(file.source(), begin_pos, begin_line, begin_col);
    offset2Pos(file.source(), end_pos, end_line, end_col);
}

}; // namespace parser
} // namespace ruby_typer
