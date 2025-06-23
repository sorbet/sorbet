#include "parser/prism/Parser.h"

using namespace std;

namespace sorbet::parser::Prism {

pm_parser_t *Parser::getRawParserPointer() {
    return &storage->parser;
}

ParseResult Parser::parse_root() {
    pm_node_t *root = pm_parse(getRawParserPointer());
    return ParseResult{*this, root, collectErrors()};
};

core::LocOffsets Parser::translateLocation(pm_location_t location) {
    uint32_t start = static_cast<uint32_t>(location.start - storage->parser.start);
    uint32_t end = static_cast<uint32_t>(location.end - storage->parser.start);

    return core::LocOffsets{start, end};
}

string_view Parser::resolveConstant(pm_constant_id_t constant_id) {
    pm_constant_t *constant = pm_constant_pool_id_to_constant(&storage->parser.constant_pool, constant_id);

    return string_view(reinterpret_cast<const char *>(constant->start), constant->length);
}

string_view Parser::extractString(pm_string_t *string) {
    return string_view(reinterpret_cast<const char *>(pm_string_source(string)), pm_string_length(string));
}

vector<ParseError> Parser::collectErrors() {
    vector<ParseError> parseErrors;
    parseErrors.reserve(storage->parser.error_list.size);

    auto error_list = storage->parser.error_list;

    for (auto *node = error_list.head; node != nullptr; node = node->next) {
        auto *error = reinterpret_cast<pm_diagnostic_t *>(node);
        auto level = static_cast<pm_error_level_t>(error->level);

        ParseError parseError(error->diag_id, string(reinterpret_cast<const char *>(error->message)), error->location,
                              level);

        parseErrors.push_back(parseError);
    }

    return parseErrors;
}
}; // namespace sorbet::parser::Prism
