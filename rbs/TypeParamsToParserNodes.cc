#include "rbs/TypeParamsToParserNodes.h"

#include "core/errors/rewriter.h"
#include "parser/helper.h"
#include "parser/parser.h"
#include "rbs/TypeToParserNode.h"
#include "rbs/rbs_common.h"

using namespace std;

namespace sorbet::rbs {

parser::NodeVec TypeParamsToParserNode::typeParams(const rbs_node_list_t *rbsTypeParams,
                                                   const RBSDeclaration &declaration) {
    parser::NodeVec result;

    for (rbs_node_list_node_t *list_node = rbsTypeParams->head; list_node != nullptr; list_node = list_node->next) {
        ENFORCE(list_node->node->type == RBS_AST_TYPE_PARAM,
                "Unexpected node type `{}` in type parameter list, expected `{}`", rbs_node_type_name(list_node->node),
                "TypeParam");

        auto rbsTypeParam = (rbs_ast_type_param_t *)list_node->node;
        auto loc = declaration.typeLocFromRange(list_node->node->location->rg);

        if (rbsTypeParam->unchecked) {
            if (auto e = ctx.beginError(loc, core::errors::Rewriter::RBSUnsupported)) {
                e.setHeader("`{}` type parameters are not supported by Sorbet", "unchecked");
            }
        }

        auto nameStr = parser.resolveConstant(rbsTypeParam->name);
        auto nameUTF8 = ctx.state.enterNameUTF8(nameStr);
        auto nameConstant = ctx.state.enterNameConstant(nameUTF8);

        auto args = parser::NodeVec();
        if (rbsTypeParam->variance) {
            auto variance = parser.resolveKeyword(rbsTypeParam->variance);
            if (variance == "covariant") {
                args.emplace_back(parser::MK::Symbol(loc, core::Names::covariant()));
            } else if (variance == "contravariant") {
                args.emplace_back(parser::MK::Symbol(loc, core::Names::contravariant()));
            }
        }

        auto typeConst = parser::MK::Const(loc, nullptr, nameConstant);
        auto typeSend = parser::MK::Send(loc, parser::MK::Self(loc), core::Names::syntheticTypeMember(), loc, move(args));

        if (rbsTypeParam->default_type || rbsTypeParam->upper_bound) {
            auto typeTranslator = TypeToParserNode(ctx, vector<pair<core::LocOffsets, core::NameRef>>(), parser);
            auto pairs = parser::NodeVec();

            if (rbsTypeParam->default_type) {
                pairs.emplace_back(
                    make_unique<parser::Pair>(loc, parser::MK::Symbol(loc, core::Names::fixed()),
                                              typeTranslator.toParserNode(rbsTypeParam->default_type, declaration)));
            }

            if (rbsTypeParam->upper_bound) {
                pairs.emplace_back(
                    make_unique<parser::Pair>(loc, parser::MK::Symbol(loc, core::Names::upper()),
                                              typeTranslator.toParserNode(rbsTypeParam->upper_bound, declaration)));
            }

            auto body = parser::MK::Hash(loc, false, move(pairs));
            typeSend = make_unique<parser::Block>(loc, move(typeSend), nullptr, move(body));
        }

        auto assign = make_unique<parser::Assign>(loc, move(typeConst), move(typeSend));
        result.emplace_back(move(assign));
    }

    return result;
}

} // namespace sorbet::rbs
