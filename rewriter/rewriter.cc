#include "rewriter/rewriter.h"
#include "ast/treemap/treemap.h"
#include "ast/verifier/verifier.h"
#include "common/typecase.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include "rewriter/AttrReader.h"
#include "rewriter/ClassNew.h"
#include "rewriter/Cleanup.h"
#include "rewriter/Command.h"
#include "rewriter/DSLBuilder.h"
#include "rewriter/DefaultArgs.h"
#include "rewriter/Delegate.h"
#include "rewriter/Flatfiles.h"
#include "rewriter/Flatten.h"
#include "rewriter/Initializer.h"
#include "rewriter/InterfaceWrapper.h"
#include "rewriter/Mattr.h"
#include "rewriter/Minitest.h"
#include "rewriter/MixinEncryptedProp.h"
#include "rewriter/ModuleFunction.h"
#include "rewriter/Private.h"
#include "rewriter/Prop.h"
#include "rewriter/Rails.h"
#include "rewriter/Regexp.h"
#include "rewriter/SelfNew.h"
#include "rewriter/SigRewriter.h"
#include "rewriter/Struct.h"
#include "rewriter/TEnum.h"
#include "rewriter/TypeMembers.h"

using namespace std;

namespace sorbet::rewriter {

class Rewriterer {
    friend class Rewriter;

public:
    ast::TreePtr postTransformClassDef(core::MutableContext ctx, ast::TreePtr tree) {
        auto *classDef = ast::cast_tree<ast::ClassDef>(tree);

        Command::run(ctx, classDef);
        Rails::run(ctx, classDef);
        TEnum::run(ctx, classDef);
        Flatfiles::run(ctx, classDef);
        Prop::run(ctx, classDef);
        TypeMembers::run(ctx, classDef);
        DefaultArgs::run(ctx, classDef);

        for (auto &extension : ctx.state.semanticExtensions) {
            extension->run(ctx, classDef);
        }

        ast::TreePtr *prevStat = nullptr;
        UnorderedMap<void *, vector<ast::TreePtr>> replaceNodes;
        for (auto &stat : classDef->rhs) {
            typecase(
                stat.get(),
                [&](ast::Assign *assign) {
                    vector<ast::TreePtr> nodes;

                    nodes = Struct::run(ctx, assign);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }

                    nodes = ClassNew::run(ctx, assign);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }

                    nodes = Regexp::run(ctx, assign);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }
                },

                [&](ast::Send *send) {
                    vector<ast::TreePtr> nodes;

                    nodes = MixinEncryptedProp::run(ctx, send);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }

                    nodes = Minitest::run(ctx, send);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = move(nodes);
                        return;
                    }

                    nodes = DSLBuilder::run(ctx, send);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }

                    nodes = Private::run(ctx, send);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }

                    nodes = Delegate::run(ctx, send);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }

                    // This one is different: it gets an extra prevStat argument.
                    nodes = AttrReader::run(ctx, send, prevStat);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }

                    // This one is also a little different: it gets the ClassDef kind
                    nodes = Mattr::run(ctx, send, classDef->kind);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }
                },

                [&](ast::MethodDef *mdef) { Initializer::run(ctx, mdef, prevStat); },

                [&](ast::Expression *e) {});

            prevStat = &stat;
        }
        if (replaceNodes.empty()) {
            ModuleFunction::run(ctx, classDef);
            return tree;
        }

        auto oldRHS = std::move(classDef->rhs);
        classDef->rhs.clear();
        classDef->rhs.reserve(oldRHS.size());

        for (auto &stat : oldRHS) {
            auto replacement = replaceNodes.find(stat.get());
            if (replacement == replaceNodes.end()) {
                classDef->rhs.emplace_back(std::move(stat));
            } else {
                for (auto &newNode : replacement->second) {
                    classDef->rhs.emplace_back(std::move(newNode));
                }
            }
        }
        ModuleFunction::run(ctx, classDef);

        return tree;
    }

    // NOTE: this case differs from the `Send` typecase branch in `postTransformClassDef` above, as it will apply to all
    // sends, not just those that are present in the RHS of a `ClassDef`.
    ast::TreePtr postTransformSend(core::MutableContext ctx, ast::TreePtr tree) {
        auto *send = ast::cast_tree<ast::Send>(tree);

        if (auto expr = InterfaceWrapper::run(ctx, send)) {
            return expr;
        }

        if (auto expr = SelfNew::run(ctx, send)) {
            return expr;
        }

        if (SigRewriter::run(ctx, send)) {
            return tree;
        }

        return tree;
    }

private:
    Rewriterer() = default;
};

ast::TreePtr Rewriter::run(core::MutableContext ctx, ast::TreePtr tree) {
    auto ast = std::move(tree);

    Rewriterer rewriter;
    ast = ast::TreeMap::apply(ctx, rewriter, std::move(ast));
    // This AST flattening pass requires that we mutate the AST in a way that our previous DSL passes were not designed
    // around, which is why it runs all at once and is not expressed as a `patch` method like the other DSL passes. This
    // is a rare case: in general, we should *not* add new DSL passes here.
    auto flattened = Flatten::run(ctx, std::move(ast));
    auto cleaned = Cleanup::run(ctx, std::move(flattened));
    auto verifiedResult = ast::Verifier::run(ctx, std::move(cleaned));
    return verifiedResult;
}

}; // namespace sorbet::rewriter
