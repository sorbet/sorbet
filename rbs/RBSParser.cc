#include "rbs/RBSParser.h"

using namespace std;

namespace sorbet::rbs {

rbs_string_t makeRBSString(const string_view &str) {
    return {
        .start = str.data(),
        .end = str.data() + str.size(),
        .type = rbs_string_t::RBS_STRING_SHARED,
    };
}

pair<optional<MethodType>, optional<ParseError>> RBSParser::parseSignature(core::Context ctx, Comment comment) {
    rbs_string_t rbsString = makeRBSString(comment.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    auto lexer = unique_ptr<lexstate, void (*)(lexstate *)>(alloc_lexer(rbsString, encoding, 0, comment.string.size()),
                                                            [](lexstate *p) { free(p); });

    auto parser = unique_ptr<parserstate, void (*)(parserstate *)>(alloc_parser(lexer.get(), 0, comment.string.size()),
                                                                   [](parserstate *p) { free(p); });

    rbs_methodtype_t *rbsMethodType = nullptr;
    parse_method_type(parser.get(), &rbsMethodType);

    if (parser->error) {
        core::LocOffsets offset = locFromRange(comment.loc, parser->error->token.range);

        return make_pair(nullopt, ParseError{offset, parser->error->message});
    }

    return make_pair(MethodType{comment.loc, unique_ptr<rbs_methodtype_t>(rbsMethodType)}, nullopt);
}

pair<optional<Type>, optional<ParseError>> RBSParser::parseType(core::Context ctx, Comment comment) {
    rbs_string_t rbsString = makeRBSString(comment.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    auto lexer = unique_ptr<lexstate, void (*)(lexstate *)>(alloc_lexer(rbsString, encoding, 0, comment.string.size()),
                                                            [](lexstate *p) { free(p); });

    auto parser = unique_ptr<parserstate, void (*)(parserstate *)>(alloc_parser(lexer.get(), 0, comment.string.size()),
                                                                   [](parserstate *p) { free(p); });

    rbs_node_t *rbsType = nullptr;
    parse_type(parser.get(), &rbsType);

    if (parser->error) {
        core::LocOffsets offset = locFromRange(comment.loc, parser->error->token.range);

        return make_pair(nullopt, ParseError{offset, parser->error->message});
    }

    return make_pair(Type{comment.loc, unique_ptr<rbs_node_t>(rbsType)}, nullopt);
}

} // namespace sorbet::rbs
