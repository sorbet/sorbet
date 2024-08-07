#ifndef SORBET_PARSER_PRISM_TRANSLATOR_H
#define SORBET_PARSER_PRISM_TRANSLATOR_H

#include <memory>

#include "../Node.h" // To clarify: these are Sorbet Parser nodes, not Prism ones.

extern "C" {
#include "prism.h"
}

namespace sorbet::parser::Prism {

class Translator final {
public:
    std::unique_ptr<parser::Node> convertPrismToSorbet(pm_node_t *node, pm_parser_t *parser, core::GlobalState &gs);
};

} // namespace sorbet::parser::Prism
#endif // SORBET_PARSER_PRISM_TRANSLATOR_H
