#ifndef SRUBY_PARSER_PARSER_H
#define SRUBY_PARSER_PARSER_H

#include "Node.h"

namespace ruby_typer {
namespace parser {

class Parser final {
public:
    static std::unique_ptr<Node> run(ruby_typer::core::GlobalState &gs, ruby_typer::core::FileRef file);
    static std::unique_ptr<Node> run(ruby_typer::core::GlobalState &gs, const std::string &path,
                                     const std::string &src);
};

} // namespace parser
} // namespace ruby_typer

#endif // SRUBY_PARSER_PARSER_H
