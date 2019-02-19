#include "dsl/dsl.h"
#include "ast/treemap/treemap.h"
#include "ast/verifier/verifier.h"
#include "dsl/ChalkODMProp.h"
#include "dsl/Command.h"
#include "dsl/DSLBuilder.h"
#include "dsl/InterfaceWrapper.h"
#include "dsl/MixinEncryptedProp.h"
#include "dsl/Sinatra.h"
#include "dsl/Struct.h"
#include "dsl/attr_reader.h"
#include "dsl/custom/CustomReplace.h"

using namespace std;

namespace sorbet::dsl {

class DSLReplacer {
    friend class DSL;

public:
    unique_ptr<ast::ClassDef> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> classDef) {
        Command::patchDSL(ctx, classDef.get());

        ast::Expression *prevStat = nullptr;
        UnorderedMap<ast::Expression *, vector<unique_ptr<ast::Expression>>> replaceNodes;
        for (auto &stat : classDef->rhs) {
            typecase(stat.get(),
                     [&](ast::Assign *assign) {
                         auto nodes = Struct::replaceDSL(ctx, assign);
                         if (!nodes.empty()) {
                             replaceNodes[stat.get()] = std::move(nodes);
                             return;
                         }
                     },

                     [&](ast::Send *send) {
                         auto nodes = ChalkODMProp::replaceDSL(ctx, send);
                         if (!nodes.empty()) {
                             replaceNodes[stat.get()] = std::move(nodes);
                             return;
                         }

                         nodes = MixinEncryptedProp::replaceDSL(ctx, send);
                         if (!nodes.empty()) {
                             replaceNodes[stat.get()] = std::move(nodes);
                             return;
                         }

                         nodes = DSLBuilder::replaceDSL(ctx, send);
                         if (!nodes.empty()) {
                             replaceNodes[stat.get()] = std::move(nodes);
                             return;
                         }

                         // This one is different: it gets an extra prevStat argument.
                         nodes = AttrReader::replaceDSL(ctx, send, prevStat);
                         if (!nodes.empty()) {
                             replaceNodes[stat.get()] = std::move(nodes);
                             return;
                         }
                     },

                     [&](ast::MethodDef *mdef) {
                         auto nodes = Sinatra::replaceDSL(ctx, mdef);
                         if (!nodes.empty()) {
                             replaceNodes[stat.get()] = std::move(nodes);
                             return;
                         }
                     },

                     [&](ast::Expression *e) {});

            for (auto &dsl : customDSLs) {
                auto nodes = dsl.matchAndReplace(ctx, stat.get());
                if (!nodes.empty()) {
                    replaceNodes[stat.get()] = std::move(nodes);
                    break; // first match wins
                }
            }

            prevStat = stat.get();
        }
        if (replaceNodes.empty()) {
            return classDef;
        }

        auto oldRHS = std::move(classDef->rhs);
        classDef->rhs.clear();
        classDef->rhs.reserve(oldRHS.size());

        for (auto &stat : oldRHS) {
            if (auto it = replaceNodes.find(stat.get()); it != replaceNodes.end()) {
                for (auto &newNode : it->second) {
                    classDef->rhs.emplace_back(std::move(newNode));
                }
            } else {
                classDef->rhs.emplace_back(std::move(stat));
            }
        }
        return classDef;
    }

    unique_ptr<ast::Expression> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> send) {
        return InterfaceWrapper::replaceDSL(ctx, std::move(send));
    }

private:
    vector<custom::CustomReplace> &customDSLs;
    DSLReplacer(vector<custom::CustomReplace> &customDSLs) : customDSLs(customDSLs){};
};

unique_ptr<ast::Expression> DSL::run(core::MutableContext ctx, unique_ptr<ast::Expression> tree,
                                     vector<custom::CustomReplace> &customDSLs) {
    auto ast = std::move(tree);

    DSLReplacer dslReplacer(customDSLs);
    ast = ast::TreeMap::apply(ctx, dslReplacer, std::move(ast));
    auto verifiedResult = ast::Verifier::run(ctx, std::move(ast));
    return verifiedResult;
}

}; // namespace sorbet::dsl
