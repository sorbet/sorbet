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
#include "parser/ParseResult.h"

namespace sorbet::parser::Prism {

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

struct SimpleParseResult {
    std::vector<ParseError> parseErrors;
    std::vector<core::LocOffsets> commentLocations;

    SimpleParseResult(std::vector<ParseError> parseErrors, std::vector<core::LocOffsets> commentLocations)
        : parseErrors(std::move(parseErrors)), commentLocations(std::move(commentLocations)) {}
};

class Parser final {
    // The version of Ruby syntax that we're parsing with Prism. This determines what syntax is supported or not.
    static constexpr std::string_view ParsedRubyVersion = "3.3.0";

    pm_parser_t parser;
    pm_options_t options;

    friend class ParseResult;
    friend struct NodeDeleter;

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

    static parser::ParseResult run(core::MutableContext ctx, bool directlyDesugar = true,
                                   bool preserveConcreteSyntax = false);
    static ParseResult parseOnly(core::MutableContext &ctx);
    static parser::ParseResult translateOnly(core::MutableContext &ctx, const Parser &parser, pm_node_t *node,
                                             const std::vector<ParseError> &parseErrors,
                                             const std::vector<core::LocOffsets> &commentLocations,
                                             bool preserveConcreteSyntax);

    ParseResult parse(bool collectComments = false);
    core::LocOffsets translateLocation(pm_location_t location) const;
    std::string_view resolveConstant(pm_constant_id_t constantId) const;
    std::string_view extractString(pm_string_t *string) const;

private:
    std::vector<ParseError> collectErrors();
    std::vector<core::LocOffsets> collectCommentLocations();
    pm_parser_t *getRawParserPointer();
};

class ParseResult final {
    struct NodeDeleter {
        Parser &parser;

        void operator()(pm_node_t *node) {
            pm_node_destroy(parser.getRawParserPointer(), node);
        }
    };

    friend class Parser;
    friend class Translator;

    const Parser &parser;
    const std::unique_ptr<pm_node_t, NodeDeleter> node;
    const std::vector<ParseError> parseErrors;
    std::vector<core::LocOffsets> commentLocations;

    ParseResult(Parser &parser, pm_node_t *node, std::vector<ParseError> parseErrors,
                std::vector<core::LocOffsets> commentLocations)
        : parser{parser}, node{node, NodeDeleter{parser}}, parseErrors{parseErrors}, commentLocations{
                                                                                         commentLocations} {}

    ParseResult(const ParseResult &) = delete;            // Copy constructor
    ParseResult &operator=(const ParseResult &) = delete; // Copy assignment
    ParseResult(ParseResult &&) = delete;                 // Move constructor
    ParseResult &operator=(ParseResult &&) = delete;      // Move assignment

public:
    pm_node_t *getRawNodePointer() const {
        return node.get();
    }

    const std::vector<core::LocOffsets> &getCommentLocations() const {
        return commentLocations;
    }

    const std::vector<ParseError> &getParseErrors() const {
        return parseErrors;
    }

    const Parser &getParser() const {
        return parser;
    }
};

} // namespace sorbet::parser::Prism
#endif // SORBET_PARSER_PRISM_PARSER_H
