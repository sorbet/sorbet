#ifndef SORBET_RBS_PARSER_H
#define SORBET_RBS_PARSER_H

#include "rbs/rbs_common.h"
#include <string_view>

namespace sorbet::rbs {

class Parser {
private:
    std::shared_ptr<parserstate> parser;

public:
    Parser(rbs_string_t rbsString, const rbs_encoding_t *encoding);

    std::string_view resolveConstant(const rbs_ast_symbol_t *symbol) const;

    rbs_methodtype_t *parseMethodType();
    rbs_node_t *parseType();

    bool hasError() const;
    const error *getError() const;
};

} // namespace sorbet::rbs

#endif // SORBET_RBS_PARSER_H
