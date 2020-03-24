#ifndef RBS_PARSER_DRIVER_HH
#define RBS_PARSER_DRIVER_HH

#include "File.hh"
#include "Parser.hh"
#include "ast.hh"

#include <string>
#include <vector>

namespace rbs_parser {
class Driver {
public:
    File *file;

    Driver(File *file) : file(file){};

    Loc loc(Parser::location_type begin, Parser::location_type end) {
        return Loc(Pos(begin.begin.line, begin.begin.column), Pos(end.end.line, end.end.column));
    }

    NodeList *list() { return new NodeList(); }

    NodeList *list(Node *node) { return new NodeList(node); }

    NodeList *merge(Node *node1, Node *node2) {
        NodeList *list = new NodeList();
        list->emplace_back(node1);
        list->emplace_back(node2);
        return list;
    }

    NodeList *merge(Node *node, NodeList *nodes) {
        NodeList *list = new NodeList();
        list->emplace_back(node);
        list->concat(nodes);
        return list;
    }

    NodeList *merge(NodeList *nodes, Node *node) {
        NodeList *list = new NodeList();
        list->concat(nodes);
        list->emplace_back(node);
        return list;
    }
};
} // namespace rbs_parser

#endif
