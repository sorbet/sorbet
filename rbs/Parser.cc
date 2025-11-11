#include "rbs/Parser.h"

using namespace std;

namespace sorbet::rbs {

namespace {

// RBS's parser needs to initialize a global constant pool to host around 26 unique strings
// that are shared across all parsers.
// When the process exits, we need to free the constant pool too.
// This class and the static rbsLibraryInitializer variable leverage C++'s static initialization
// and destruction semantics to automatically initialize the constant pool at program startup
// and clean it up at program termination.
class RBSLibraryInitializer {
public:
    RBSLibraryInitializer() {
        const size_t num_uniquely_interned_strings = 26;
        rbs_constant_pool_init(RBS_GLOBAL_CONSTANT_POOL, num_uniquely_interned_strings);
    }

    ~RBSLibraryInitializer() {
        rbs_constant_pool_free(RBS_GLOBAL_CONSTANT_POOL);
    }
};

RBSLibraryInitializer rbsLibraryInitializer;
} // namespace

Parser::Parser(rbs_string_t rbsString, const rbs_encoding_t *encoding)
    : parser(rbs_parser_new(rbsString, encoding, 0, rbsString.end - rbsString.start), rbs_parser_free) {}

string_view Parser::resolveConstant(const rbs_ast_symbol_t *symbol) const {
    auto constant = rbs_constant_pool_id_to_constant(&parser->constant_pool, symbol->constant_id);
    return string_view(reinterpret_cast<const char *>(constant->start), constant->length);
}

string_view Parser::resolveKeyword(const rbs_keyword_t *keyword) const {
    auto constant = rbs_constant_pool_id_to_constant(RBS_GLOBAL_CONSTANT_POOL, keyword->constant_id);
    return string_view(reinterpret_cast<const char *>(constant->start), constant->length);
}

rbs_node_list_t *Parser::parseTypeParams() {
    rbs_node_list_t *typeParams;
    auto moduleParams = true;
    rbs_parse_type_params(parser.get(), moduleParams, &typeParams);
    return typeParams;
}

rbs_method_type_t *Parser::parseMethodType() {
    rbs_method_type_t *methodType = nullptr;
    rbs_parse_method_type(parser.get(), &methodType, true);
    return methodType;
}

rbs_node_t *Parser::parseType() {
    rbs_node_t *type = nullptr;
    rbs_parse_type(parser.get(), &type, true, true);
    return type;
}

bool Parser::hasError() const {
    return parser->error != nullptr;
}

const rbs_error_t *Parser::getError() const {
    return parser->error;
}

} // namespace sorbet::rbs
