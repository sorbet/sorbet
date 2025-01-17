#include "RBSParser.h"
#include "core/errors/rewriter.h"

namespace sorbet::rbs {

std::unique_ptr<MethodType> RBSParser::parseSignature(core::Context ctx, Comment comment) {
    rbs_string_t rbsString = {
        .start = comment.string.data(),
        .end = comment.string.data() + comment.string.size(),
        .type = rbs_string_t::RBS_STRING_SHARED,
    };

    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    auto lexer = std::unique_ptr<lexstate, void (*)(lexstate *)>(
        alloc_lexer(rbsString, encoding, 0, comment.string.size()), [](lexstate *p) { free(p); });

    auto parser = std::unique_ptr<parserstate, void (*)(parserstate *)>(
        alloc_parser(lexer.get(), 0, comment.string.size()), [](parserstate *p) { free(p); });

    rbs_methodtype_t *rbsMethodType = nullptr;
    parse_method_type(parser.get(), &rbsMethodType);

    if (parser->error) {
        core::LocOffsets offset{
            comment.loc.beginPos() + parser->error->token.range.start.char_pos + 2,
            comment.loc.beginPos() + parser->error->token.range.end.char_pos + 2,
        };

        if (auto e = ctx.beginError(offset, core::errors::Rewriter::RBSError)) {
            e.setHeader("Failed to parse RBS signature ({})", parser->error->message);
        }

        return nullptr;
    }

    return std::make_unique<MethodType>(comment.loc, rbsMethodType);
}

std::unique_ptr<Type> RBSParser::parseType(core::Context ctx, Comment comment) {
    rbs_string_t rbsString = {
        .start = comment.string.data(),
        .end = comment.string.data() + comment.string.size(),
        .type = rbs_string_t::RBS_STRING_SHARED,
    };

    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    auto lexer = std::unique_ptr<lexstate, void (*)(lexstate *)>(
        alloc_lexer(rbsString, encoding, 0, comment.string.size()), [](lexstate *p) { free(p); });

    auto parser = std::unique_ptr<parserstate, void (*)(parserstate *)>(
        alloc_parser(lexer.get(), 0, comment.string.size()), [](parserstate *p) { free(p); });

    rbs_node_t *rbsType = nullptr;
    parse_type(parser.get(), &rbsType);

    if (parser->error) {
        core::LocOffsets offset{
            comment.loc.beginPos() + parser->error->token.range.start.char_pos + 2,
            comment.loc.beginPos() + parser->error->token.range.end.char_pos + 2,
        };

        if (auto e = ctx.beginError(offset, core::errors::Rewriter::RBSError)) {
            e.setHeader("Failed to parse RBS type ({})", parser->error->message);
        }

        return nullptr;
    }

    return std::make_unique<Type>(comment.loc, rbsType);
}

} // namespace sorbet::rbs
