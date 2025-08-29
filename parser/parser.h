#ifndef SORBET_PARSER_PARSER_H
#define SORBET_PARSER_PARSER_H

#include "Node.h"
#include "ParseResult.h"

namespace sorbet::parser {

class Parser final {
public:
    struct Settings {
        bool traceLexer : 1;
        bool traceParser : 1;
        bool indentationAware : 1;
        bool collectComments : 1;

        Settings() : traceLexer(false), traceParser(false), indentationAware(false), collectComments(false) {}
        Settings(bool traceLexer, bool traceParser, bool indentationAware, bool collectComments = false)
            : traceLexer(traceLexer), traceParser(traceParser), indentationAware(indentationAware),
              collectComments(collectComments) {}

        Settings withIndentationAware() {
            return Settings{this->traceLexer, this->traceParser, true, this->collectComments};
        }
    };

    static ParseResult run(core::GlobalState &gs, core::FileRef file, Settings settings,
                           std::vector<std::string> initialLocals = {});
};

} // namespace sorbet::parser

#endif // SORBET_PARSER_PARSER_H
