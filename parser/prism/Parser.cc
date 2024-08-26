#include "parser/prism/Parser.h"

namespace sorbet::parser::Prism {

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

std::string_view Parser::resolveConstant(pm_constant_id_t constant_id) {
    pm_constant_t *constant = pm_constant_pool_id_to_constant(&parser->constant_pool, constant_id);

    return std::string_view(reinterpret_cast<const char *>(constant->start), constant->length);
}

std::string_view Parser::extractString(pm_string_t *string) {
    if (string->type != pm_string_t::PM_STRING_CONSTANT && string->type != pm_string_t::PM_STRING_SHARED) {
        Exception::raise(
            "This string needs to be freed eventually with `pm_string_free`, but that isn't implemented yet");
    }

    return std::string_view(reinterpret_cast<const char *>(pm_string_source(string)), pm_string_length(string));
}

}; // namespace sorbet::parser::Prism
