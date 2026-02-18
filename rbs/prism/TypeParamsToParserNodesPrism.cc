#include "rbs/prism/TypeParamsToParserNodesPrism.h"

#include "core/errors/rewriter.h"
#include "rbs/prism/TypeToParserNodePrism.h"

using namespace std;

namespace sorbet::rbs {

vector<pm_node_t *> TypeParamsToParserNodesPrism::typeParams(const rbs_node_list_t *rbsTypeParams,
                                                             const RBSDeclaration &declaration) {
    vector<pm_node_t *> result{};
    result.reserve(rbsTypeParams->length);

    for (auto *listNode = rbsTypeParams->head; listNode != nullptr; listNode = listNode->next) {
        auto *rbsTypeParam = rbs_down_cast<rbs_ast_type_param_t>(listNode->node);
        auto loc = declaration.typeLocFromRange(listNode->node->location->rg);

        if (rbsTypeParam->unchecked) {
            if (auto e = ctx.beginIndexerError(loc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("`{}` type parameters are not supported by Sorbet", "unchecked");
            }
        }

        auto nameStr = parser.resolveConstant(rbsTypeParam->name);
        auto nameConstant = ctx.state.enterNameConstant(nameStr);

        absl::InlinedVector<pm_node_t *, 1> args{};
        if (rbsTypeParam->variance) {
            auto variance = rbsTypeParam->variance;
            if (variance == RBS_TYPE_PARAM_VARIANCE_COVARIANT) {
                args.push_back(prism.Symbol(loc, core::Names::covariant().show(ctx.state)));
            } else if (variance == RBS_TYPE_PARAM_VARIANCE_CONTRAVARIANT) {
                args.push_back(prism.Symbol(loc, core::Names::contravariant().show(ctx.state)));
            }
        }

        auto defaultType = rbsTypeParam->default_type;
        auto upperBound = rbsTypeParam->upper_bound;
        auto lowerBound = rbsTypeParam->lower_bound;

        pm_node_t *block = nullptr;
        if (defaultType || upperBound || lowerBound) {
            auto typeTranslator = TypeToParserNodePrism{ctx, {}, parser, prismParser};
            absl::InlinedVector<pm_node_t *, 3> pairs{};

            if (defaultType) {
                auto key = prism.Symbol(loc, core::Names::fixed().show(ctx.state));
                auto value = typeTranslator.toPrismNode(defaultType, declaration);
                pairs.push_back(prism.AssocNode(loc, key, value));
            }

            if (upperBound) {
                auto key = prism.Symbol(loc, core::Names::upper().show(ctx.state));
                auto value = typeTranslator.toPrismNode(upperBound, declaration);
                pairs.push_back(prism.AssocNode(loc, key, value));
            }

            if (lowerBound) {
                auto key = prism.Symbol(loc, core::Names::lower().show(ctx.state));
                auto value = typeTranslator.toPrismNode(lowerBound, declaration);
                pairs.push_back(prism.AssocNode(loc, key, value));
            }

            block = prism.Block(loc, prism.Hash(loc, absl::MakeSpan(pairs)));
        }

        auto typeCall = prism.Call(loc, prism.SorbetPrivateStatic(loc), "type_member"sv, absl::MakeSpan(args), block);
        auto assign = prism.ConstantWriteNode(loc, prism.addConstantToPool(nameConstant.show(ctx.state)), typeCall);
        result.push_back(assign);
    }

    return result;
}

} // namespace sorbet::rbs
