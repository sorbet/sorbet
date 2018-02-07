#include "dsl/dsl.h"
#include "ast/treemap/treemap.h"
#include "dsl/ChalkODMProp.h"
#include "dsl/Struct.h"
#include "dsl/attr_reader.h"

using namespace std;

namespace ruby_typer {
namespace dsl {

class DSLReplacer {
    friend class DSL;

public:
    ast::ClassDef *postTransformClassDef(core::Context ctx, ast::ClassDef *classDef) {
        unordered_map<ast::Expression *, vector<unique_ptr<ast::Expression>>> replaceNodes;
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

                         nodes = AttrReader::replaceDSL(ctx, send);
                         if (!nodes.empty()) {
                             replaceNodes[stat.get()] = move(nodes);
                             return;
                         }
                     },

                     [&](ast::Expression *e) {});
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

private:
    DSLReplacer() {}
};

unique_ptr<ast::Expression> DSL::run(core::Context ctx, unique_ptr<ast::Expression> tree) {
    auto ast = move(tree);

    DSLReplacer dslReplacer;
    ast = ast::TreeMap<DSLReplacer>::apply(ctx, dslReplacer, move(ast));

    return ast;
}

} // namespace dsl
}; // namespace ruby_typer
