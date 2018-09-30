#ifndef SORBET_PARSER_PARSER_H
#define SORBET_PARSER_PARSER_H

#include "Node.h"

namespace sorbet::parser {

class Parser final {
public:
    static std::unique_ptr<Node> run(sorbet::core::GlobalState &gs, sorbet::core::FileRef file);
    static std::unique_ptr<Node> run(sorbet::core::GlobalState &gs, const std::string &path, const std::string &src);
};

} // namespace sorbet::parser

#endif // SORBET_PARSER_PARSER_H
