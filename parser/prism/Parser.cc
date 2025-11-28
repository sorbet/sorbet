#include "parser/prism/Parser.h"
#include "absl/types/span.h"
#include "parser/prism/Helpers.h"
#include "parser/prism/Translator.h"

using namespace std;

namespace sorbet::parser::Prism {

parser::ParseResult Parser::run(core::MutableContext ctx, bool directlyDesugar, bool preserveConcreteSyntax) {
    auto file = ctx.file;
    auto source = file.data(ctx).source();
    Prism::Parser parser{source};
    bool collectComments = ctx.state.cacheSensitiveOptions.rbsEnabled;
    Prism::ParseResult parseResult = parser.parseWithoutTranslation(collectComments);

    auto translatedTree =
        Prism::Translator(parser, ctx, parseResult.parseErrors, directlyDesugar, preserveConcreteSyntax)
            .translate(parseResult.getRawNodePointer());
    return parser::ParseResult{move(translatedTree), move(parseResult.commentLocations)};
}

pm_parser_t *Parser::getRawParserPointer() {
    return &parser;
}

// Parses without translating and returns raw Prism nodes for intermediate processing (e.g., RBS rewriting)
// Caller must keep Parser alive for later translation, unlike run() which parses + translates in one step
ParseResult Parser::parseWithoutTranslation(bool collectComments) {
    pm_node_t *root = pm_parse(&parser);
    auto comments = collectComments ? collectCommentLocations() : vector<core::LocOffsets>{};
    return ParseResult{*this, root, collectErrors(), move(comments)};
};

core::LocOffsets Parser::translateLocation(pm_location_t location) const {
    return translateLocation(location.start, location.end);
}

core::LocOffsets Parser::translateLocation(const uint8_t *start, const uint8_t *end) const {
    uint32_t startPos = static_cast<uint32_t>(start - parser.start);
    uint32_t endPos = static_cast<uint32_t>(end - parser.start);

    return core::LocOffsets{startPos, endPos};
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

        parseErrors.emplace_back(error->diag_id, string(error->message), error->location, level);
    }

    return parseErrors;
}

vector<core::LocOffsets> Parser::collectCommentLocations() {
    auto commentList = parser.comment_list;
    vector<core::LocOffsets> commentLocations;
    commentLocations.reserve(parser.comment_list.size);

    for (auto *node = commentList.head; node != nullptr; node = node->next) {
        auto *comment = reinterpret_cast<pm_comment_t *>(node);
        ENFORCE(comment != nullptr);

        core::LocOffsets location = translateLocation(comment->location);

        commentLocations.push_back(location);
    }

    return commentLocations;
}

string Parser::prettyPrint(pm_node_t *node) const {
    pm_buffer_t buffer{};
    pm_prettyprint(&buffer, const_cast<pm_parser_t *>(&parser), node);
    string result(buffer.value, buffer.length);
    pm_buffer_free(&buffer);
    return result;
}
}; // namespace sorbet::parser::Prism
