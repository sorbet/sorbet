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

core::LocOffsets Parser::translateLocation(pm_location_t *location) {
    uint32_t start = static_cast<uint32_t>(location->start - parser->start);
    uint32_t end = static_cast<uint32_t>(location->end - parser->start);

    return core::LocOffsets{start, end};
}

}; // namespace sorbet::parser::Prism
