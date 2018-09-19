#include "dsl/dsl.h"
#include "ast/treemap/treemap.h"
#include "dsl/ChalkODMProp.h"
#include "dsl/Command.h"
#include "dsl/DSLBuilder.h"
#include "dsl/InterfaceWrapper.h"
#include "dsl/Sinatra.h"
#include "dsl/Struct.h"
#include "dsl/attr_reader.h"

using namespace std;

namespace sorbet {
namespace dsl {

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
                             replaceNodes[stat.get()] = move(nodes);
                             return;
                         }
                     },

                     [&](ast::Send *send) {
                         auto nodes = ChalkODMProp::replaceDSL(ctx, send);
                         if (!nodes.empty()) {
                             replaceNodes[stat.get()] = move(nodes);
                             return;
                         }

                         nodes = DSLBuilder::replaceDSL(ctx, send);
                         if (!nodes.empty()) {
                             replaceNodes[stat.get()] = move(nodes);
                             return;
                         }

                         // This one is different: it gets an extra prevStat argument.
                         nodes = AttrReader::replaceDSL(ctx, send, prevStat);
                         if (!nodes.empty()) {
                             replaceNodes[stat.get()] = move(nodes);
                             return;
                         }
                     },

                     [&](ast::MethodDef *mdef) {
                         auto nodes = Sinatra::replaceDSL(ctx, mdef);
                         if (!nodes.empty()) {
                             replaceNodes[stat.get()] = move(nodes);
                             return;
                         }
                     },

                     [&](ast::Expression *e) {});

            prevStat = stat.get();
        }
        if (replaceNodes.empty()) {
            return classDef;
        }

        auto oldRHS = move(classDef->rhs);
        classDef->rhs.clear();

        for (auto &stat : oldRHS) {
            if (replaceNodes.find(stat.get()) != replaceNodes.end()) {
                for (auto &newNode : replaceNodes.at(stat.get())) {
                    classDef->rhs.emplace_back(move(newNode));
                }
            } else {
                classDef->rhs.emplace_back(move(stat));
            }
        }
        return classDef;
    }

    unique_ptr<ast::Expression> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> send) {
        return InterfaceWrapper::replaceDSL(ctx, move(send));
    }

private:
    DSLReplacer() = default;
};

unique_ptr<ast::Expression> DSL::run(core::MutableContext ctx, unique_ptr<ast::Expression> tree) {
    auto ast = move(tree);

    DSLReplacer dslReplacer;
    ast = ast::TreeMap::apply(ctx, dslReplacer, move(ast));

    return ast;
}

} // namespace dsl
}; // namespace sorbet
