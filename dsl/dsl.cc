#include "dsl/dsl.h"
#include "ast/treemap/treemap.h"
#include "ast/verifier/verifier.h"
#include "common/typecase.h"
#include "dsl/ClassNew.h"
#include "dsl/Command.h"
#include "dsl/DSLBuilder.h"
#include "dsl/Delegate.h"
#include "dsl/InterfaceWrapper.h"
#include "dsl/Mattr.h"
#include "dsl/Minitest.h"
#include "dsl/MixinEncryptedProp.h"
#include "dsl/OpusEnum.h"
#include "dsl/Private.h"
#include "dsl/Prop.h"
#include "dsl/ProtobufDescriptorPool.h"
#include "dsl/Rails.h"
#include "dsl/Regexp.h"
#include "dsl/Struct.h"
#include "dsl/TypeMembers.h"
#include "dsl/attr_reader.h"
#include "dsl/module_function.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"

using namespace std;

namespace sorbet::dsl {

class DSLReplacer {
    friend class DSL;

public:
    unique_ptr<ast::ClassDef> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> classDef) {
        Command::patchDSL(ctx, classDef.get());
        Rails::patchDSL(ctx, classDef.get());
        OpusEnum::patchDSL(ctx, classDef.get());
        Prop::patchDSL(ctx, classDef.get());
        TypeMembers::patchDSL(ctx, classDef.get());

        for (auto &extension : ctx.state.semanticExtensions) {
            extension->patchDSL(ctx, classDef.get());
        }

        ast::Expression *prevStat = nullptr;
        UnorderedMap<ast::Expression *, vector<unique_ptr<ast::Expression>>> replaceNodes;
        for (auto &stat : classDef->rhs) {
            typecase(
                stat.get(),
                [&](ast::Assign *assign) {
                    auto nodes = Struct::replaceDSL(ctx, assign);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }

                    nodes = ClassNew::replaceDSL(ctx, assign);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }

                    nodes = ProtobufDescriptorPool::replaceDSL(ctx, assign);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }

                    nodes = Regexp::replaceDSL(ctx, assign);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }
                },

                [&](ast::Send *send) {
                    vector<unique_ptr<ast::Expression>> nodes;

                    nodes = MixinEncryptedProp::replaceDSL(ctx, send);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }

                    nodes = Minitest::replaceDSL(ctx, send);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = move(nodes);
                        return;
                    }

                    nodes = DSLBuilder::replaceDSL(ctx, send);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }

                    nodes = Private::replaceDSL(ctx, send);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }

                    nodes = Delegate::replaceDSL(ctx, send);
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

                    // This one is also a little different: it gets the ClassDef kind
                    nodes = Mattr::replaceDSL(ctx, send, classDef->kind);
                    if (!nodes.empty()) {
                        replaceNodes[stat.get()] = std::move(nodes);
                        return;
                    }
                },

                [&](ast::Expression *e) {});

            prevStat = stat.get();
        }
        if (replaceNodes.empty()) {
            ModuleFunction::patchDSL(ctx, classDef.get());
            return classDef;
        }

        auto oldRHS = std::move(classDef->rhs);
        classDef->rhs.clear();
        classDef->rhs.reserve(oldRHS.size());

        for (auto &stat : oldRHS) {
            if (replaceNodes.find(stat.get()) == replaceNodes.end()) {
                classDef->rhs.emplace_back(std::move(stat));
            } else {
                for (auto &newNode : replaceNodes.at(stat.get())) {
                    classDef->rhs.emplace_back(std::move(newNode));
                }
            }
        }
        ModuleFunction::patchDSL(ctx, classDef.get());

        return classDef;
    }

    unique_ptr<ast::Expression> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> send) {
        return InterfaceWrapper::replaceDSL(ctx, std::move(send));
    }

private:
    DSLReplacer() = default;
};

unique_ptr<ast::Expression> DSL::run(core::MutableContext ctx, unique_ptr<ast::Expression> tree) {
    auto ast = std::move(tree);

    DSLReplacer dslReplacer;
    ast = ast::TreeMap::apply(ctx, dslReplacer, std::move(ast));
    auto verifiedResult = ast::Verifier::run(ctx, std::move(ast));
    return verifiedResult;
}

}; // namespace sorbet::dsl
