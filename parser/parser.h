#ifndef SORBET_PARSER_PARSER_H
#define SORBET_PARSER_PARSER_H

#include "Node.h"

namespace sorbet::parser {

class Parser final {
public:
    static std::unique_ptr<Node> run(core::GlobalState &gs, core::FileRef file,
                                     std::vector<std::string> initialLocals = {});
    static std::unique_ptr<Node> run(core::GlobalState &gs, std::string_view path, std::string_view src,
                                     std::vector<std::string> initialLocals = {});
};

} // namespace sorbet::parser

#endif // SORBET_PARSER_PARSER_H
