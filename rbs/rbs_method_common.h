#ifndef SORBET_RBS_RBS_METHOD_COMMON_H
#define SORBET_RBS_RBS_METHOD_COMMON_H

#include "core/LocOffsets.h"
#include <string>

extern "C" {
#include "include/rbs.h"
}

namespace sorbet::rbs {

struct RBSArg {
    core::LocOffsets loc;
    core::LocOffsets nameLoc;
    rbs_ast_symbol_t *name;
    rbs_node_t *type;

    enum class Kind {
        Positional,
        OptionalPositional,
        RestPositional,
        Keyword,
        OptionalKeyword,
        RestKeyword,
        Block,
    };

    Kind kind;
};

inline std::string argKindToString(RBSArg::Kind kind) {
    switch (kind) {
        case RBSArg::Kind::Positional:
            return "positional";
        case RBSArg::Kind::OptionalPositional:
            return "optional positional";
        case RBSArg::Kind::RestPositional:
            return "rest positional";
        case RBSArg::Kind::Keyword:
            return "keyword";
        case RBSArg::Kind::OptionalKeyword:
            return "optional keyword";
        case RBSArg::Kind::RestKeyword:
            return "rest keyword";
        case RBSArg::Kind::Block:
            return "block";
    }
}

} // namespace sorbet::rbs

#endif // SORBET_RBS_RBS_METHOD_COMMON_H
