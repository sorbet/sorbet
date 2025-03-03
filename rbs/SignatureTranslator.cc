#include "rbs/SignatureTranslator.h"
#include "ast/ast.h"
#include "core/errors/rewriter.h"
#include "rbs/MethodTypeTranslator.h"
#include "rbs/rbs_common.h"

using namespace std;

namespace sorbet::rbs {

rbs_string_t SignatureTranslator::makeRBSString(const string_view &str) {
    return {
        .start = str.data(),
        .end = str.data() + str.size(),
        .type = rbs_string_t::RBS_STRING_SHARED,
    };
}

ast::ExpressionPtr SignatureTranslator::translateType(core::MutableContext ctx, const ast::Send *send,
                                                      const std::vector<Comment> &annotations) {
    rbs_string_t rbsString = makeRBSString(this->signature.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    auto parser = std::shared_ptr<parserstate>(alloc_parser(rbsString, encoding, 0, this->signature.string.size()),
                                               [](parserstate *p) { free_parser(p); });

    rbs_node_t *rbsType = nullptr;
    parse_type(parser.get(), &rbsType);

    if (parser->error) {
        core::LocOffsets offset = locFromRange(this->signature.loc, parser->error->token.range);
        // First parse failed, let's check if the user mistakenly used a method signature on an accessor
        auto methodParser =
            std::shared_ptr<parserstate>(alloc_parser(rbsString, encoding, 0, this->signature.string.size()),
                                         [](parserstate *p) { free_parser(p); });

        rbs_methodtype_t *methodRbsType = nullptr;
        parse_method_type(methodParser.get(), &methodRbsType);

        if (!methodParser->error) {
            if (auto e = ctx.beginError(offset, core::errors::Rewriter::RBSSyntaxError)) {
                e.setHeader("Using a method signature on an accessor is not allowed, use a bare type instead");
            }
        } else {
            if (auto e = ctx.beginError(offset, core::errors::Rewriter::RBSSyntaxError)) {
                e.setHeader("Failed to parse RBS type ({})", methodParser->error->message);
            }
        }

        return nullptr;
    }

    auto methodTypeTranslator = MethodTypeTranslator(ctx, parser);
    return methodTypeTranslator.attrSignature(send, rbsType, this->signature.loc, annotations);
}

ast::ExpressionPtr SignatureTranslator::translateSignature(core::MutableContext ctx, const ast::MethodDef *methodDef,
                                                           const std::vector<rbs::Comment> &annotations) {
    rbs_string_t rbsString = makeRBSString(this->signature.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    auto parser = std::shared_ptr<parserstate>(alloc_parser(rbsString, encoding, 0, this->signature.string.size()),
                                               [](parserstate *p) { free_parser(p); });

    rbs_methodtype_t *rbsMethodType = nullptr;
    parse_method_type(parser.get(), &rbsMethodType);

    if (parser->error) {
        core::LocOffsets offset = locFromRange(this->signature.loc, parser->error->token.range);

        if (auto e = ctx.beginError(offset, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS signature ({})", parser->error->message);
        }

        return nullptr;
    }

    auto methodTypeTranslator = MethodTypeTranslator(ctx, parser);
    return methodTypeTranslator.methodSignature(methodDef, rbsMethodType, this->signature.loc, annotations);
}

} // namespace sorbet::rbs
