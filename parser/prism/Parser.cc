#include "parser/prism/Parser.h"

namespace sorbet::parser::Prism {

pm_parser_t *Parser::tmp_public_get_raw_parser_pointer() {
    return get_raw_parser_pointer();
}

pm_parser_t *Parser::get_raw_parser_pointer() {
    return parser.get();
}

pm_node_t *Parser::parse_root() {
    return pm_parse(parser.get());
};

}; // namespace sorbet::parser::Prism
