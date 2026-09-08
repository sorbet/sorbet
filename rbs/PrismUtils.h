#ifndef SORBET_RBS_PRISM_UTILS_H
#define SORBET_RBS_PRISM_UTILS_H

#include "parser/prism/Helpers.h"
#include "parser/prism/Parser.h"

namespace sorbet::rbs {

inline bool isSelfOrKernel(pm_node_t *node, const parser::Prism::Parser &parser) {
    using namespace parser::Prism;

    if (isa_node<pm_self_node_t>(node)) {
        return true;
    }

    if (auto *constant = down_cast<pm_constant_read_node_t>(node)) {
        return parser.resolveConstant(constant->name) == "Kernel";
    }

    if (auto *constantPath = down_cast<pm_constant_path_node_t>(node)) {
        return constantPath->parent == nullptr && parser.resolveConstant(constantPath->name) == "Kernel";
    }

    return false;
}

inline bool isRaiseCall(pm_node_t *node, const parser::Prism::Parser &parser) {
    using namespace parser::Prism;

    auto *call = down_cast<pm_call_node_t>(node);
    return call != nullptr && parser.resolveConstant(call->name) == "raise" &&
           (call->receiver == nullptr || isSelfOrKernel(call->receiver, parser));
}

} // namespace sorbet::rbs

#endif // SORBET_RBS_PRISM_UTILS_H
