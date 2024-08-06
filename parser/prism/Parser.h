#ifndef SORBET_PARSER_PRISM_PARSER_H
#define SORBET_PARSER_PRISM_PARSER_H

#include <memory>
#include <string>

extern "C" {
#include "prism.h"
}

namespace sorbet::parser::Prism {

class Node;

class Parser final {
    friend class Node;
    friend struct NodeDeleter;

    std::shared_ptr<pm_parser_t> parser;

public:
    Parser(std::string_view source_code)
        : parser(new pm_parser_t, [](auto p) {
              pm_parser_free(p);
              delete (p);
          }) {
        const pm_options_t *options = nullptr;
        pm_parser_init(parser.get(), reinterpret_cast<const uint8_t *>(source_code.data()), source_code.size(),
                       options);
    }

    Parser(const Parser &) = default;
    Parser &operator=(const Parser &) = delete;

    // Expose the raw parser pointer temporarily, until we build wrappers for the other APIs
    pm_parser_t *tmp_public_get_raw_parser_pointer();

    Node parse_root();

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

    Parser parser;
    std::unique_ptr<pm_node_t, NodeDeleter> node;

    Node(Parser parser, pm_node_t *node) : parser{parser}, node{node, NodeDeleter{parser}} {}

    Node(const Node &) = delete;            // Copy constructor
    Node &operator=(const Node &) = delete; // Copy assignment

public:
    // Expose the raw parser pointer temporarily, until we build wrappers for the other APIs
    pm_node_t *tmp_public_get_raw_node_pointer() {
        return node.get();
    }
};

} // namespace sorbet::parser::Prism
#endif // SORBET_PARSER_PRISM_PARSER_H
