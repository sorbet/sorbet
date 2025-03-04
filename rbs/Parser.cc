#include "rbs/Parser.h"

namespace sorbet::rbs {

Parser::Parser(rbs_string_t rbsString, const rbs_encoding_t *encoding)
    : parser(alloc_parser(rbsString, encoding, 0, rbsString.end - rbsString.start), free_parser) {}

std::string_view Parser::resolveConstant(const rbs_ast_symbol_t *symbol) const {
    auto constant = rbs_constant_pool_id_to_constant(&parser->constant_pool, symbol->constant_id);
    return std::string_view(reinterpret_cast<const char *>(constant->start), constant->length);
}

rbs_methodtype_t *Parser::parseMethodType() {
    rbs_methodtype_t *methodType = nullptr;
    parse_method_type(parser.get(), &methodType);
    return methodType;
}

rbs_node_t *Parser::parseType() {
    rbs_node_t *type = nullptr;
    parse_type(parser.get(), &type);
    return type;
}

bool Parser::hasError() const {
    return parser->error != nullptr;
}

const error *Parser::getError() const {
    return parser->error;
}
} // namespace sorbet::rbs
