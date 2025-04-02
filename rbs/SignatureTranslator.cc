#include "rbs/SignatureTranslator.h"
#include "core/errors/rewriter.h"
#include "rbs/MethodTypeToParserNode.h"
#include "rbs/TypeToParserNode.h"
#include "rbs/rbs_common.h"

using namespace std;

namespace sorbet::rbs {

namespace {

rbs_string_t makeRBSString(const string_view &str) {
    return rbs_string_new(str.data(), str.data() + str.size());
}

} // namespace

unique_ptr<parser::Node>
SignatureTranslator::translateAssertionType(vector<std::pair<core::LocOffsets, core::NameRef>> typeParams,
                                            const rbs::Comment &assertion) {
    rbs_string_t rbsString = makeRBSString(assertion.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    Parser parser(rbsString, encoding);
    rbs_node_t *rbsType = parser.parseType();

    if (parser.hasError()) {
        core::LocOffsets offset = locFromRange(assertion.typeLoc, parser.getError()->token.range);
        if (auto e = ctx.beginError(offset, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS type ({})", parser.getError()->message);
        }
        return nullptr;
    }

    auto typeToParserNode = TypeToParserNode(ctx, typeParams, std::move(parser));
    return typeToParserNode.toParserNode(rbsType, assertion.typeLoc);
}

unique_ptr<parser::Node> SignatureTranslator::translateType(const parser::Send *send, const rbs::Comment &signature,
                                                            const vector<Comment> &annotations) {
    rbs_string_t rbsString = makeRBSString(signature.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    Parser parser(rbsString, encoding);
    rbs_node_t *rbsType = parser.parseType();

    if (parser.hasError()) {
        core::LocOffsets offset = locFromRange(signature.typeLoc, parser.getError()->token.range);
        // First parse failed, let's check if the user mistakenly used a method signature on an accessor
        auto methodParser = Parser(rbsString, encoding);
        methodParser.parseMethodType();

        if (!methodParser.hasError()) {
            if (auto e = ctx.beginError(offset, core::errors::Rewriter::RBSSyntaxError)) {
                e.setHeader("Using a method signature on an accessor is not allowed, use a bare type instead");
            }
        } else {
            if (auto e = ctx.beginError(offset, core::errors::Rewriter::RBSSyntaxError)) {
                e.setHeader("Failed to parse RBS type ({})", methodParser.getError()->message);
            }
        }

        return nullptr;
    }

    auto methodTypeToParserNode = MethodTypeToParserNode(ctx, std::move(parser));
    return methodTypeToParserNode.attrSignature(send, rbsType, signature.typeLoc, signature.commentLoc, annotations);
}

unique_ptr<parser::Node> SignatureTranslator::translateSignature(const parser::Node *methodDef,
                                                                 const rbs::Comment &signature,
                                                                 const vector<Comment> &annotations) {
    rbs_string_t rbsString = makeRBSString(signature.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    Parser parser(rbsString, encoding);
    rbs_method_type_t *rbsMethodType = parser.parseMethodType();

    if (parser.hasError()) {
        core::LocOffsets offset = locFromRange(signature.typeLoc, parser.getError()->token.range);

        if (auto e = ctx.beginError(offset, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS signature ({})", parser.getError()->message);
        }

        return nullptr;
    }

    auto methodTypeToParserNode = MethodTypeToParserNode(ctx, std::move(parser));
    return methodTypeToParserNode.methodSignature(methodDef, rbsMethodType, signature.typeLoc, signature.commentLoc,
                                                  annotations);
}

} // namespace sorbet::rbs
