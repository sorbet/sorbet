#include "rbs/prism/SignatureTranslatorPrism.h"
#include "core/errors/rewriter.h"
#include "rbs/prism/MethodTypeToParserNodePrism.h"
#include "rbs/prism/TypeParamsToParserNodesPrism.h"
#include "rbs/prism/TypeToParserNodePrism.h"
#include "rbs/rbs_common.h"

using namespace std;
using namespace sorbet::parser::Prism;

namespace sorbet::rbs {

namespace {
Parser makeParser(string_view str) {
    return Parser{makeRBSString(str), RBS_ENCODING_UTF_8_ENTRY};
}
} // namespace

pm_node_t *
SignatureTranslatorPrism::translateAssertionType(absl::Span<pair<core::LocOffsets, core::NameRef>> typeParams,
                                                 const rbs::RBSDeclaration &assertion) {
    Parser rbsParser = makeParser(assertion.string);
    rbs_node_t *rbsType = rbsParser.parseType();

    if (rbsParser.hasError()) {
        auto loc = assertion.typeLocFromRange(rbsParser.getError()->token.range);
        if (auto e = ctx.beginIndexerError(loc, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS type ({})", rbsParser.getError()->message);
        }
        return nullptr;
    }

    auto typeToParserNodePrism = TypeToParserNodePrism{ctx, typeParams, move(rbsParser), *prismParser};
    return typeToParserNodePrism.toPrismNode(rbsType, assertion);
}

pm_node_t *SignatureTranslatorPrism::translateType(const RBSDeclaration &declaration) {
    Parser rbsParser = makeParser(declaration.string);
    rbs_node_t *rbsType = rbsParser.parseType();

    if (rbsParser.hasError()) {
        auto offset = declaration.typeLocFromRange(rbsParser.getError()->token.range);
        if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS type ({})", rbsParser.getError()->message);
        }
        return nullptr;
    }

    auto typeTranslator = TypeToParserNodePrism{ctx, {}, move(rbsParser), *prismParser};
    return typeTranslator.toPrismNode(rbsType, declaration);
}

pm_node_t *SignatureTranslatorPrism::translateAttrSignature(pm_call_node_t *call, const RBSDeclaration &declaration,
                                                            absl::Span<const Comment> annotations) {
    Parser rbsParser = makeParser(declaration.string);
    rbs_node_t *rbsType = rbsParser.parseType();

    if (rbsParser.hasError()) {
        auto offset = declaration.typeLocFromRange(rbsParser.getError()->token.range);
        // First parse failed, let's check if the user mistakenly used a method signature on an accessor
        auto methodParser = makeParser(declaration.string);
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

    auto methodTypeToParserNode = MethodTypeToParserNodePrism{ctx, move(rbsParser), *prismParser};
    return methodTypeToParserNode.attrSignature(call, rbsType, declaration, annotations);
}

pm_node_t *SignatureTranslatorPrism::translateMethodSignature(pm_node_t *methodDef, const RBSDeclaration &declaration,
                                                              absl::Span<const Comment> annotations) {
    Parser rbsParser = makeParser(declaration.string);
    rbs_method_type_t *rbsMethodType = rbsParser.parseMethodType();

    if (rbsParser.hasError()) {
        auto offset = declaration.typeLocFromRange(rbsParser.getError()->token.range);
        if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS signature ({})", rbsParser.getError()->message);
        }
        return nullptr;
    }

    auto methodTypeToParserNodePrism = MethodTypeToParserNodePrism{ctx, move(rbsParser), *prismParser};
    return methodTypeToParserNodePrism.methodSignature(methodDef, rbsMethodType, declaration, annotations);
}

vector<pm_node_t *> SignatureTranslatorPrism::translateTypeParams(const RBSDeclaration &declaration) {
    Parser rbsParser = makeParser(declaration.string);
    rbs_node_list_t *rbsTypeParams = rbsParser.parseTypeParams();

    if (rbsParser.hasError()) {
        auto offset = declaration.typeLocFromRange(rbsParser.getError()->token.range);
        if (auto e = ctx.beginIndexerError(offset, core::errors::Rewriter::RBSSyntaxError)) {
            e.setHeader("Failed to parse RBS type parameters ({})", rbsParser.getError()->message);
        }
        return {};
    }

    TypeParamsToParserNodesPrism typeParamsTranslator{ctx, rbsParser, *prismParser};
    return typeParamsTranslator.typeParams(rbsTypeParams, declaration);
}

} // namespace sorbet::rbs
