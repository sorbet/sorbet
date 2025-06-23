#include "parser/prism/Parser.h"
#include "parser/prism/Helpers.h"
#include "parser/prism/Translator.h"

using namespace std;

namespace sorbet::parser::Prism {

unique_ptr<parser::Node> Parser::run(core::MutableContext &ctx, core::FileRef file) {
    auto source = file.data(ctx).source();
    Prism::Parser parser{source};
    Prism::ParseResult parseResult = parser.parse();

    return Prism::Translator(parser, ctx, file).translate(move(parseResult));
}

pm_parser_t *Parser::getRawParserPointer() {
    return &parser;
}

ParseResult Parser::parse() {
    pm_node_t *root = pm_parse(&parser);
    return ParseResult{*this, root, collectErrors()};
};

core::LocOffsets Parser::translateLocation(pm_location_t location) const {
    uint32_t start = static_cast<uint32_t>(location.start - parser.start);
    uint32_t end = static_cast<uint32_t>(location.end - parser.start);

    return core::LocOffsets{start, end};
}

string_view Parser::resolveConstant(pm_constant_id_t constantId) const {
    pm_constant_t *constant = pm_constant_pool_id_to_constant(&parser.constant_pool, constantId);

    return cast_prism_string(constant->start, constant->length);
}

string_view Parser::extractString(pm_string_t *string) const {
    return cast_prism_string(pm_string_source(string), pm_string_length(string));
}

vector<ParseError> Parser::collectErrors() {
    vector<ParseError> parseErrors;
    parseErrors.reserve(parser.error_list.size);

    auto errorList = parser.error_list;

    for (auto *node = errorList.head; node != nullptr; node = node->next) {
        auto *error = reinterpret_cast<pm_diagnostic_t *>(node);
        auto level = static_cast<pm_error_level_t>(error->level);

        ParseError parseError(error->diag_id, string(error->message), error->location, level);

        parseErrors.push_back(parseError);
    }

    return parseErrors;
}
}; // namespace sorbet::parser::Prism
