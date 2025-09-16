#include "rbs/prism/SignatureTranslatorPrism.h"
#include "core/errors/rewriter.h"
#include "parser/prism/Helpers.h"
#include "rbs/MethodTypeToParserNode.h"
#include "rbs/TypeParamsToParserNodes.h"
#include "rbs/TypeToParserNode.h"
#include "rbs/prism/MethodTypeToParserNodePrism.h"
#include "rbs/rbs_common.h"

using namespace std;
using namespace sorbet::parser::Prism;

namespace sorbet::rbs {

namespace {

rbs_string_t makeRBSString(const string &str) {
    return rbs_string_new(str.data(), str.data() + str.size());
}

} // namespace

// unique_ptr<parser::Node> SignatureTranslatorPrism::translateAssertionType(vector<pair<core::LocOffsets,
// core::NameRef>> typeParams,
//                                                  const rbs::RBSDeclaration &assertion) {
//     rbs_string_t rbsString = makeRBSString(assertion.string);
//     const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

//     Parser parser(rbsString, encoding);
//     rbs_node_t *rbsType = parser.parseType();

//     if (parser.hasError()) {
//         core::LocOffsets loc = assertion.typeLocFromRange(parser.getError()->token.range);
//         if (auto e = ctx.beginIndexerError(loc, core::errors::Rewriter::RBSSyntaxError)) {
//             e.setHeader("Failed to parse RBS type ({})", parser.getError()->message);
//         }
//         return nullptr;
//     }

//     auto typeToParserNode = TypeToParserNode(ctx, typeParams, move(parser));
//     return typeToParserNode.toParserNode(rbsType, assertion);
// }

// unique_ptr<parser::Node> SignatureTranslatorPrism::translateAttrSignature(const pm_call_node_t *call,
//                                                                           const RBSDeclaration &declaration,
//                                                                           const vector<Comment> &annotations) {
//     rbs_string_t rbsString = makeRBSString(declaration.string);
//     const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

//     Parser parser(rbsString, encoding);
//     rbs_node_t *rbsType = parser.parseType();

//     if (parser.hasError()) {
//         core::LocOffsets offset = declaration.typeLocFromRange(parser.getError()->token.range);
//         // First parse failed, let's check if the user mistakenly used a method signature on an accessor
//         auto methodParser = Parser(rbsString, encoding);
//         methodParser.parseMethodType();

//         if (!methodParser.hasError()) {
//             if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
//                 e.setHeader("Using a method signature on an accessor is not allowed, use a bare type instead");
//             }
//         } else {
//             if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
//                 e.setHeader("Failed to parse RBS type ({})", methodParser.getError()->message);
//             }
//         }

//         return nullptr;
//     }

//     // TODO: Need to implement MethodTypeToParserNode.attrSignature to work with Prism nodes
//     // For now, we'll need to convert pm_call_node_t to parser::Send for compatibility
//     (void)call; // Suppress unused variable warning
//     (void)rbsType; // Suppress unused variable warning
//     (void)annotations; // Suppress unused variable warning

//     // Temporary stub - needs proper implementation
//     // auto methodTypeToParserNode = MethodTypeToParserNode(ctx, move(parser));
//     // return methodTypeToParserNode.attrSignature(prismCallToParserSend(call), rbsType, declaration, annotations);
//     return nullptr;
// }

unique_ptr<parser::Node> SignatureTranslatorPrism::translateMethodSignature(const pm_node_t *methodDef,
                                                                            const RBSDeclaration &declaration,
                                                                            const vector<Comment> &annotations) {
    rbs_string_t rbsString = makeRBSString(declaration.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    Parser parser(rbsString, encoding);
    rbs_method_type_t *rbsMethodType = parser.parseMethodType();

    if (parser.hasError()) {
        rbs_range_t tokenRange = parser.getError()->token.range;
        core::LocOffsets offset = declaration.typeLocFromRange(tokenRange);

        if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS signature ({})", parser.getError()->message);
        }

        return nullptr;
    }

    auto methodTypeToParserNodePrism = MethodTypeToParserNodePrism(ctx, move(parser));
    return methodTypeToParserNodePrism.methodSignature(methodDef, rbsMethodType, declaration, annotations);
}

// New Prism node creation methods
pm_node_t* SignatureTranslatorPrism::createPrismMethodSignature(const pm_node_t *methodDef,
                                                                const RBSDeclaration &declaration,
                                                                const std::vector<Comment> &annotations) {
    if (!parser) {
        return nullptr; // Need parser for Prism node creation
    }

    // If there's no RBS signature to parse, create a placeholder signature
    if (declaration.string.empty()) {
        auto methodTypeToParserNodePrism = MethodTypeToParserNodePrism(ctx, Parser(rbs_string_new("", ""), &rbs_encodings[RBS_ENCODING_UTF_8]), *parser);
        return methodTypeToParserNodePrism.createSigCallPlaceholder();
    }

    // Parse the RBS declaration to get the method type, following the same pattern as translateMethodSignature
    rbs_string_t rbsString = makeRBSString(declaration.string);
    const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

    Parser rbsParser(rbsString, encoding);
    rbs_method_type_t *rbsMethodType = rbsParser.parseMethodType();

    if (rbsParser.hasError()) {
        // If parsing fails, fall back to placeholder
        auto methodTypeToParserNodePrism = MethodTypeToParserNodePrism(ctx, move(rbsParser), *parser);
        return methodTypeToParserNodePrism.createSigCallPlaceholder();
    }

    // Delegate to MethodTypeToParserNodePrism for actual Prism node creation
    auto methodTypeToParserNodePrism = MethodTypeToParserNodePrism(ctx, move(rbsParser), *parser);
    return methodTypeToParserNodePrism.createPrismMethodSignature(methodDef, rbsMethodType, declaration, annotations);
}

// unique_ptr<parser::Node> SignatureTranslatorPrism::translateType(const RBSDeclaration &declaration) {
//     rbs_string_t rbsString = makeRBSString(declaration.string);
//     const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

//     Parser parser(rbsString, encoding);
//     rbs_node_t *rbsType = parser.parseType();

//     if (parser.hasError()) {
//         core::LocOffsets offset = declaration.typeLocFromRange(parser.getError()->token.range);
//         if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
//             e.setHeader("Failed to parse RBS type ({})", parser.getError()->message);
//         }

//         return nullptr;
//     }

//     auto typeTranslator = TypeToParserNode(ctx, vector<pair<core::LocOffsets, core::NameRef>>(), parser);
//     return typeTranslator.toParserNode(rbsType, declaration);
// }

// parser::NodeVec SignatureTranslatorPrism::translateTypeParams(const RBSDeclaration &declaration) {
//     rbs_string_t rbsString = makeRBSString(declaration.string);
//     const rbs_encoding_t *encoding = &rbs_encodings[RBS_ENCODING_UTF_8];

//     Parser parser(rbsString, encoding);
//     rbs_node_list_t *rbsTypeParams = parser.parseTypeParams();

//     if (parser.hasError()) {
//         rbs_range_t tokenRange = parser.getError()->token.range;
//         core::LocOffsets offset = declaration.typeLocFromRange(tokenRange);

//         if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
//             e.setHeader("Failed to parse RBS type parameters ({})", parser.getError()->message);
//         }

//         return parser::NodeVec();
//     }

//     auto typeParamsToParserNode = TypeParamsToParserNode(ctx, move(parser));
//     return typeParamsToParserNode.typeParams(rbsTypeParams, declaration);
// }


} // namespace sorbet::rbs
