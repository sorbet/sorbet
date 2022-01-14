#ifndef SORBET_PARSER_BUILDER_H
#define SORBET_PARSER_BUILDER_H

#include "ruby_parser/builder.hh"
#include "ruby_parser/driver.hh"

#include "core/core.h"

#include <memory>

namespace sorbet::parser {

class Node;

class Builder final {
public:
    Builder(sorbet::core::GlobalState &gs, sorbet::core::FileRef file);
    ~Builder();

    static ruby_parser::builder interface;

    struct BuildResult {
        std::unique_ptr<Node> ast;
        ruby_parser::diagnostics_t diagnostics;
    };
    BuildResult build(const std::vector<std::string> &initialLocals, bool trace);

private:
    core::GlobalState &gs_;
    core::FileRef file_;
};
}; // namespace sorbet::parser

#endif
