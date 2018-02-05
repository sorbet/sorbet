#include "dsl/dsl.h"
#include "ast/treemap/treemap.h"
#include "dsl/ChalkODMProp.h"

using namespace std;

namespace ruby_typer {
namespace dsl {

class DSLReplacer {
    friend class DSL;

public:
    ast::ClassDef *postTransformClassDef(core::Context ctx, ast::ClassDef *classDef) {
        vector<unique_ptr<ast::Expression>> newNodes;
        for (auto &stat : classDef->rhs) {
            typecase(stat.get(),
                     [&](ast::Send *send) {

                         auto nodes = ChalkODMProp::replaceDSL(ctx, send);

                         if (!nodes.empty()) {
                             for (auto &node : nodes) {
                                 newNodes.emplace_back(move(node));
                             }
                             stat.reset(nullptr);
                         }
                     },

                     [&](ast::Expression *e) {});
        }

        for (auto &node : newNodes) {
            classDef->rhs.emplace_back(move(node));
        }

        auto toRemove = remove_if(classDef->rhs.begin(), classDef->rhs.end(),
                                  [](unique_ptr<ast::Expression> &stat) -> bool { return stat.get() == nullptr; });

        classDef->rhs.erase(toRemove, classDef->rhs.end());
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
