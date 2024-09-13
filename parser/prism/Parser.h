#ifndef SORBET_PARSER_PRISM_PARSER_H
#define SORBET_PARSER_PRISM_PARSER_H

#include <memory>
#include <string>

extern "C" {
#include "prism.h"
}

#include "core/LocOffsets.h"

namespace sorbet::parser::Prism {

class Node;

// A backing implemenation detail of `Parser`, which stores a Prism parser and its options in a single allocation.
struct ParserStorage {
    // The version of Ruby syntax that we're parsing with Prism. This determines what syntax is supported or not.
    static constexpr std::string_view ParsedRubyVersion = "3.3.0";
    pm_parser_t parser;
    pm_options_t options;

    ParserStorage(std::string_view source_code) : parser{}, options{} {
        pm_options_version_set(&options, ParsedRubyVersion.data(), ParsedRubyVersion.size());

        pm_parser_init(&parser, reinterpret_cast<const uint8_t *>(source_code.data()), source_code.size(), &options);
    }

    ~ParserStorage() {
        pm_parser_free(&parser);
        pm_options_free(&options);
    }

    ParserStorage(const ParserStorage &) = delete;
    ParserStorage &operator=(const ParserStorage &) = delete;
    ParserStorage(ParserStorage &&) = delete;
    ParserStorage &operator=(ParserStorage &&) = delete;
};

class Parser final {
    friend class Node;
    friend struct NodeDeleter;

    std::shared_ptr<ParserStorage> storage;

public:
    Parser(std::string_view source_code) : storage(std::make_shared<ParserStorage>(source_code)) {}

    Parser(const Parser &) = default;
    Parser &operator=(const Parser &) = default;

    Node parse_root();
    core::LocOffsets translateLocation(pm_location_t *location);
    std::string_view resolveConstant(pm_constant_id_t constant_id);
    std::string_view extractString(pm_string_t *string);

private:
    pm_parser_t *get_raw_parser_pointer();
};

class Node final {
    struct NodeDeleter {
        Parser parser;

        void operator()(pm_node_t *node) {
            pm_node_destroy(parser.get_raw_parser_pointer(), node);
        }
    };

    friend class Parser;
    friend class Translator;

    Parser parser;
    std::unique_ptr<pm_node_t, NodeDeleter> node;

    Node(Parser parser, pm_node_t *node) : parser{parser}, node{node, NodeDeleter{parser}} {}

    Node(const Node &) = delete;            // Copy constructor
    Node &operator=(const Node &) = delete; // Copy assignment

    pm_node_t *get_raw_node_pointer() const {
        return node.get();
    }
};

} // namespace sorbet::parser::Prism
#endif // SORBET_PARSER_PRISM_PARSER_H
