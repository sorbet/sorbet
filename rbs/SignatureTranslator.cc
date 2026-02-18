#include "rbs/SignatureTranslator.h"
#include "core/errors/rewriter.h"
#include "rbs/MethodTypeToParserNode.h"
#include "rbs/TypeParamsToParserNodes.h"
#include "rbs/TypeToParserNode.h"
#include "rbs/rbs_common.h"

using namespace std;

namespace sorbet::rbs {

namespace {

rbs_string_t makeRBSString(const string &str) {
    return rbs_string_new(str.data(), str.data() + str.size());
}

} // namespace

unique_ptr<parser::Node>
SignatureTranslator::translateAssertionType(vector<pair<core::LocOffsets, core::NameRef>> typeParams,
                                            const rbs::RBSDeclaration &assertion) {
    rbs_string_t rbsString = makeRBSString(assertion.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    Parser parser(rbsString, encoding);
    rbs_node_t *rbsType = parser.parseType();

    if (parser.hasError()) {
        core::LocOffsets loc = assertion.typeLocFromTokenRange(parser.getError()->token.range);
        if (auto e = ctx.beginIndexerError(loc, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS type ({})", parser.getError()->message);
        }
        return nullptr;
    }

    auto typeToParserNode = TypeToParserNode(ctx, typeParams, move(parser));
    return typeToParserNode.toParserNode(rbsType, assertion);
}

unique_ptr<parser::Node> SignatureTranslator::translateAttrSignature(const parser::Send *send,
                                                                     const RBSDeclaration &declaration,
                                                                     const vector<Comment> &annotations) {
    rbs_string_t rbsString = makeRBSString(declaration.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    Parser parser(rbsString, encoding);
    rbs_node_t *rbsType = parser.parseType();

    if (parser.hasError()) {
        core::LocOffsets offset = declaration.typeLocFromTokenRange(parser.getError()->token.range);
        // First parse failed, let's check if the user mistakenly used a method signature on an accessor
        auto methodParser = Parser(rbsString, encoding);
        methodParser.parseMethodType();

        if (!methodParser.hasError()) {
            if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
                e.setHeader("Using a method signature on an accessor is not allowed, use a bare type instead");
            }
        } else {
            if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
                e.setHeader("Failed to parse RBS type ({})", methodParser.getError()->message);
            }
        }

        return nullptr;
    }

    auto methodTypeToParserNode = MethodTypeToParserNode(ctx, move(parser));
    return methodTypeToParserNode.attrSignature(send, rbsType, declaration, annotations);
}

unique_ptr<parser::Node> SignatureTranslator::translateMethodSignature(const parser::Node *methodDef,
                                                                       const RBSDeclaration &declaration,
                                                                       const vector<Comment> &annotations) {
    rbs_string_t rbsString = makeRBSString(declaration.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    Parser parser(rbsString, encoding);
    rbs_method_type_t *rbsMethodType = parser.parseMethodType();

    if (parser.hasError()) {
        core::LocOffsets offset = declaration.typeLocFromTokenRange(parser.getError()->token.range);

        if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS signature ({})", parser.getError()->message);
        }

        return nullptr;
    }

    auto methodTypeToParserNode = MethodTypeToParserNode(ctx, move(parser));
    return methodTypeToParserNode.methodSignature(methodDef, rbsMethodType, declaration, annotations);
}

unique_ptr<parser::Node> SignatureTranslator::translateType(const RBSDeclaration &declaration) {
    rbs_string_t rbsString = makeRBSString(declaration.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    Parser parser(rbsString, encoding);
    rbs_node_t *rbsType = parser.parseType();

    if (parser.hasError()) {
        core::LocOffsets offset = declaration.typeLocFromTokenRange(parser.getError()->token.range);
        if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS type ({})", parser.getError()->message);
        }

        return nullptr;
    }

    auto typeTranslator = TypeToParserNode(ctx, vector<pair<core::LocOffsets, core::NameRef>>(), parser);
    return typeTranslator.toParserNode(rbsType, declaration);
}

parser::NodeVec SignatureTranslator::translateTypeParams(const RBSDeclaration &declaration) {
    rbs_string_t rbsString = makeRBSString(declaration.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    Parser parser(rbsString, encoding);
    rbs_node_list_t *rbsTypeParams = parser.parseTypeParams();

    if (parser.hasError()) {
        core::LocOffsets offset = declaration.typeLocFromTokenRange(parser.getError()->token.range);

        if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS type parameters ({})", parser.getError()->message);
        }

        return parser::NodeVec();
    }

    auto typeParamsToParserNode = TypeParamsToParserNode(ctx, move(parser));
    return typeParamsToParserNode.typeParams(rbsTypeParams, declaration);
}

} // namespace sorbet::rbs
