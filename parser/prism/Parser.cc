#include "parser/prism/Parser.h"

namespace sorbet::parser::Prism {

pm_parser_t *Parser::get_raw_parser_pointer() {
    return &storage->parser;
}

Node Parser::parse_root() {
    pm_node_t *root = pm_parse(get_raw_parser_pointer());
    return Node{*this, root};
};

core::LocOffsets Parser::translateLocation(pm_location_t location) {
    uint32_t start = static_cast<uint32_t>(location.start - storage->parser.start);
    uint32_t end = static_cast<uint32_t>(location.end - storage->parser.start);

    return core::LocOffsets{start, end};
}

std::string_view Parser::resolveConstant(pm_constant_id_t constant_id) {
    pm_constant_t *constant = pm_constant_pool_id_to_constant(&storage->parser.constant_pool, constant_id);

    return std::string_view(reinterpret_cast<const char *>(constant->start), constant->length);
}

std::string_view Parser::extractString(pm_string_t *string) {
    return std::string_view(reinterpret_cast<const char *>(pm_string_source(string)), pm_string_length(string));
}

void Parser::collectErrors() {
    parseErrors.reserve(storage->parser.error_list.size);

    auto error_list = storage->parser.error_list;

    for (auto *node = error_list.head; node != nullptr; node = node->next) {
        auto *error = reinterpret_cast<pm_diagnostic_t *>(node);
        auto level = static_cast<pm_error_level_t>(error->level);

        ParseError parseError(pm_diagnostic_id_human(error->diag_id),
                              std::string(reinterpret_cast<const char *>(error->message)), error->location, level);

        parseErrors.push_back(parseError);
    }
}
}; // namespace sorbet::parser::Prism
