#include "parser/prism/Parser.h"

namespace sorbet::parser::Prism {

pm_parser_t *Parser::tmp_public_get_raw_parser_pointer() {
    return get_raw_parser_pointer();
}

pm_parser_t *Parser::get_raw_parser_pointer() {
    return parser.get();
}

Node Parser::parse_root() {
    pm_node_t *root = pm_parse(parser.get());
    return Node{*this, root};
};

}; // namespace sorbet::parser::Prism
