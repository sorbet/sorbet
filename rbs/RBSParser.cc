#include "rbs/RBSParser.h"
#include "core/errors/rewriter.h"

using namespace std;

namespace sorbet::rbs {

rbs_string_t makeRBSString(const string_view &str) {
    return {
        .start = str.data(),
        .end = str.data() + str.size(),
        .type = rbs_string_t::RBS_STRING_SHARED,
    };
}

optional<MethodType> RBSParser::parseSignature(core::Context ctx, Comment comment) {
    rbs_string_t rbsString = makeRBSString(comment.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    auto lexer = unique_ptr<lexstate, void (*)(lexstate *)>(alloc_lexer(rbsString, encoding, 0, comment.string.size()),
                                                            [](lexstate *p) { free(p); });

    auto parser = unique_ptr<parserstate, void (*)(parserstate *)>(alloc_parser(lexer.get(), 0, comment.string.size()),
                                                                   [](parserstate *p) { free(p); });

    rbs_methodtype_t *rbsMethodType = nullptr;
    parse_method_type(parser.get(), &rbsMethodType);

    if (parser->error) {
        core::LocOffsets offset{
            comment.loc.beginPos() + parser->error->token.range.start.char_pos + 2,
            comment.loc.beginPos() + parser->error->token.range.end.char_pos + 2,
        };

        if (auto e = ctx.beginError(offset, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS signature ({})", parser->error->message);
        }

        return nullopt;
    }

    return MethodType{comment.loc, unique_ptr<rbs_methodtype_t>(rbsMethodType)};
}

optional<Type> RBSParser::parseType(core::Context ctx, Comment comment) {
    rbs_string_t rbsString = makeRBSString(comment.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    auto lexer = unique_ptr<lexstate, void (*)(lexstate *)>(alloc_lexer(rbsString, encoding, 0, comment.string.size()),
                                                            [](lexstate *p) { free(p); });

    auto parser = unique_ptr<parserstate, void (*)(parserstate *)>(alloc_parser(lexer.get(), 0, comment.string.size()),
                                                                   [](parserstate *p) { free(p); });

    rbs_node_t *rbsType = nullptr;
    parse_type(parser.get(), &rbsType);

    if (parser->error) {
        core::LocOffsets offset{
            comment.loc.beginPos() + parser->error->token.range.start.char_pos + 2,
            comment.loc.beginPos() + parser->error->token.range.end.char_pos + 2,
        };

        if (auto e = ctx.beginError(offset, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS type ({})", parser->error->message);
        }

        return nullopt;
    }

    return Type{comment.loc, unique_ptr<rbs_node_t>(rbsType)};
}

} // namespace sorbet::rbs
