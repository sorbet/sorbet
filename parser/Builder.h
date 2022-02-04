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

    // Marked `const` so that `Parser::run` can confidently reuse one `Builder` object across
    // multiple parses.
    std::unique_ptr<Node> build(ruby_parser::base_driver *driver, bool trace) const;

    class Impl;

private:
    std::unique_ptr<Impl> impl_;
};
}; // namespace sorbet::parser

#endif
