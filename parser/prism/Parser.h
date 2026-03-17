#ifndef SORBET_PARSER_PRISM_PARSER_H
#define SORBET_PARSER_PRISM_PARSER_H

#include <memory>
#include <string>
#include <vector>

extern "C" {
#include "prism.h"
}

#include "core/LocOffsets.h"
#include "parser/Node.h" // To clarify: these are Sorbet Parser nodes, not Prism ones.

namespace sorbet::parser::Prism {

class Factory;
class ParseResult;

class ParseError {
public:
    ParseError(pm_diagnostic_id_t id, const std::string &message, pm_location_t location, pm_error_level_t level)
        : id(id), message(message), location(location), level(level) {}

    pm_diagnostic_id_t id;
    std::string message;
    pm_location_t location;
    pm_error_level_t level;
};

class Parser final {
    // The version of Ruby syntax that we're parsing with Prism. This determines what syntax is supported or not.
    static constexpr std::string_view ParsedRubyVersion = "3.4.0";

    pm_parser_t parser;
    pm_options_t options;

    friend class ParseResult;
    friend class Factory;

public:
    Parser(std::string_view sourceCode) : parser{}, options{} {
        pm_options_version_set(&options, ParsedRubyVersion.data(), ParsedRubyVersion.size());

        pm_parser_init(&parser, reinterpret_cast<const uint8_t *>(sourceCode.data()), sourceCode.size(), &options);
    }

    ~Parser() {
        pm_parser_free(&parser);
        pm_options_free(&options);
    }

    Parser(const Parser &) = delete;
    Parser &operator=(const Parser &) = delete;
    Parser(Parser &&) = delete;
    Parser &operator=(Parser &&) = delete;

    static Prism::ParseResult run(core::MutableContext ctx, bool preserveConcreteSyntax = false);

    static ParseResult parseWithoutTranslation(std::string_view source, bool collectComments = false);
    core::LocOffsets translateLocation(pm_location_t location) const;
    core::LocOffsets translateLocation(const uint8_t *start, const uint8_t *end) const;
    std::string_view resolveConstant(pm_constant_id_t constantId) const;
    std::string_view extractString(pm_string_t *string) const;
    std::string prettyPrint(pm_node_t *node) const;

    pm_location_t getZeroWidthLocation() const;
    pm_location_t convertLocOffsets(core::LocOffsets loc) const;

    bool isTUntyped(pm_node_t *node) const;
    bool isT(pm_node_t *node) const;
    bool isSetterCall(pm_node_t *node) const;
    bool isSafeNavigationCall(pm_node_t *node) const;
    bool isVisibilityCall(pm_node_t *node) const;
    bool isAttrAccessorCall(pm_node_t *node) const;

    void destroyNode(pm_node_t *node);

private:
    std::vector<ParseError> collectErrors();
    std::vector<core::LocOffsets> collectCommentLocations();
    pm_parser_t *getRawParserPointer();
};

class ParseResult final {
    friend class Parser;
    friend class Translator;

    std::unique_ptr<Parser> parser;
    pm_node_t *node;
    std::vector<ParseError> parseErrors;
    std::vector<core::LocOffsets> commentLocations;

public:
    ParseResult() = default;
    ParseResult(std::unique_ptr<Parser> parser, pm_node_t *node, std::vector<ParseError> parseErrors,
                std::vector<core::LocOffsets> commentLocations)
        : parser{std::move(parser)}, node{node}, parseErrors{std::move(parseErrors)}, commentLocations{std::move(
                                                                                          commentLocations)} {}

    ~ParseResult() {
        if (node != nullptr && parser != nullptr) {
            parser->destroyNode(node);
        }
    }

    ParseResult(ParseResult &&other) noexcept
        : parser{std::move(other.parser)}, node{other.node}, parseErrors{std::move(other.parseErrors)},
          commentLocations{std::move(other.commentLocations)} {
        other.node = nullptr;
    }

    ParseResult &operator=(ParseResult &&other) noexcept {
        this->parser = std::move(other.parser);
        this->node = std::move(other.node);
        other.node = nullptr;
        this->parseErrors = std::move(other.parseErrors);
        this->commentLocations = std::move(other.commentLocations);
        return *this;
    }

    ParseResult(const ParseResult &) = delete;            // Copy constructor
    ParseResult &operator=(const ParseResult &) = delete; // Copy assignment

    pm_node_t *getRawNodePointer() const {
        return node;
    }

    // Replace the root node, e.g. after RBS rewriting.
    void replaceRootNode(pm_node_t *newNode) {
        // Does not destroy the old node, since the rewriter mutates the tree in-place.
        node = newNode;
    }

    std::string prettyPrint() const {
        return parser->prettyPrint(node);
    }

    const std::vector<core::LocOffsets> &getCommentLocations() const {
        return commentLocations;
    }

    const std::vector<ParseError> &getParseErrors() const {
        return parseErrors;
    }

    Parser &getParser() {
        return *parser;
    }
};

} // namespace sorbet::parser::Prism
#endif // SORBET_PARSER_PRISM_PARSER_H
