#include "parser/prism/Parser.h"
#include "absl/types/span.h"
#include "parser/prism/Helpers.h"
#include "parser/prism/Translator.h"

using namespace std;

namespace sorbet::parser::Prism {

using namespace std::literals::string_view_literals;

parser::ParseResult Parser::run(core::MutableContext ctx, bool preserveConcreteSyntax) {
    auto file = ctx.file;
    auto source = file.data(ctx).source();
    Prism::Parser parser{source};
    bool collectComments = ctx.state.cacheSensitiveOptions.rbsEnabled;
    Prism::ParseResult parseResult = parser.parseWithoutTranslation(collectComments);

    auto enclosingBlockParamLoc = core::LocOffsets::none();
    auto enclosingBlockParamName = core::NameRef::noName();
    auto translatedTree = Prism::Translator(parser, ctx, parseResult.parseErrors, preserveConcreteSyntax,
                                            enclosingBlockParamLoc, enclosingBlockParamName)
                              .translate_TODO(parseResult.getRawNodePointer());

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

pm_location_t Parser::getZeroWidthLocation() const {
    return {.start = parser.start, .end = parser.start};
}

pm_location_t Parser::convertLocOffsets(core::LocOffsets loc) const {
    return {.start = parser.start + loc.beginPos(), .end = parser.start + loc.endPos()};
}

vector<ParseError> Parser::collectErrors() {
    vector<ParseError> parseErrors{};
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
    vector<core::LocOffsets> commentLocations{};
    commentLocations.reserve(parser.comment_list.size);

    for (auto *node = commentList.head; node != nullptr; node = node->next) {
        auto *comment = reinterpret_cast<pm_comment_t *>(node);
        ENFORCE(comment != nullptr);

        core::LocOffsets location = translateLocation(comment->location);

        commentLocations.push_back(location);
    }

    return commentLocations;
}

bool Parser::isTUntyped(pm_node_t *node) const {
    if (!node || !PM_NODE_TYPE_P(node, PM_CALL_NODE)) {
        return false;
    }

    pm_call_node_t *call = down_cast<pm_call_node_t>(node);
    auto methodName = resolveConstant(call->name);

    return methodName == "untyped"sv && isT(call->receiver);
}

bool Parser::isT(pm_node_t *node) const {
    if (!node) {
        return false;
    }

    if (PM_NODE_TYPE_P(node, PM_CONSTANT_READ_NODE)) { // T
        auto *constNode = down_cast<pm_constant_read_node_t>(node);
        auto name = resolveConstant(constNode->name);
        return name == "T";
    } else if (PM_NODE_TYPE_P(node, PM_CONSTANT_PATH_NODE)) { // ::T
        auto *pathNode = down_cast<pm_constant_path_node_t>(node);
        auto name = resolveConstant(pathNode->name);
        return name == "T" && pathNode->parent == nullptr;
    }

    return false;
}

bool Parser::isSetterCall(pm_node_t *node) const {
    if (!PM_NODE_TYPE_P(node, PM_CALL_NODE)) {
        return false;
    }

    auto *call = down_cast<pm_call_node_t>(node);
    auto methodName = resolveConstant(call->name);
    return !methodName.empty() && methodName.back() == '=';
}

bool Parser::isSafeNavigationCall(pm_node_t *node) const {
    return PM_NODE_TYPE_P(node, PM_CALL_NODE) && PM_NODE_FLAG_P(node, PM_CALL_NODE_FLAGS_SAFE_NAVIGATION);
}

bool Parser::isVisibilityCall(pm_node_t *node) const {
    if (!PM_NODE_TYPE_P(node, PM_CALL_NODE)) {
        return false;
    }

    auto *call = down_cast<pm_call_node_t>(node);

    // Must have no receiver (implicit self)
    if (call->receiver != nullptr) {
        return false;
    }

    // Must have exactly one argument
    if (call->arguments == nullptr || call->arguments->arguments.size != 1) {
        return false;
    }

    // That argument must be a method definition
    pm_node_t *arg = call->arguments->arguments.nodes[0];
    if (!PM_NODE_TYPE_P(arg, PM_DEF_NODE)) {
        return false;
    }

    // Check if the method name is a visibility modifier
    auto methodName = resolveConstant(call->name);
    return methodName == "private"sv || methodName == "protected"sv || methodName == "public"sv ||
           methodName == "private_class_method"sv || methodName == "public_class_method"sv ||
           methodName == "package_private"sv || methodName == "package_private_class_method"sv;
}

bool Parser::isAttrAccessorCall(pm_node_t *node) const {
    if (!PM_NODE_TYPE_P(node, PM_CALL_NODE)) {
        return false;
    }

    auto *call = down_cast<pm_call_node_t>(node);

    // Must have no receiver or self receiver
    if (call->receiver != nullptr && !PM_NODE_TYPE_P(call->receiver, PM_SELF_NODE)) {
        return false;
    }

    // Check if the method name is attr_reader, attr_writer, or attr_accessor
    auto methodName = resolveConstant(call->name);
    return methodName == "attr_reader"sv || methodName == "attr_writer"sv || methodName == "attr_accessor"sv;
}

string Parser::prettyPrint(pm_node_t *node) const {
    pm_buffer_t buffer{};
    pm_prettyprint(&buffer, const_cast<pm_parser_t *>(&parser), node);
    string result(buffer.value, buffer.length);
    pm_buffer_free(&buffer);
    return result;
}
}; // namespace sorbet::parser::Prism
