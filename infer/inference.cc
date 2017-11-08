#include "inference.h"
#include "../cfg/CFG.h"
#include <algorithm> // find
#include <unordered_map>

using namespace std;
using namespace ruby_typer;
using namespace infer;

class Environment {
public:
    vector<ast::SymbolRef> vars;
    vector<ast::TypeAndOrigins> types;

    ast::TypeAndOrigins getTypeAndOrigin(ast::Context ctx, ast::SymbolRef symbol) {
        if (symbol == ast::GlobalState::defn_dynamic()) {
            ast::TypeAndOrigins dynamicTypeAndOrigin;
            dynamicTypeAndOrigin.type = ast::Types::dynamic();
            dynamicTypeAndOrigin.origins.push_back(ast::Loc::none(0));
            return dynamicTypeAndOrigin;
        }
        auto fnd = find(vars.begin(), vars.end(), symbol);
        if (fnd == vars.end()) {
            vars.emplace_back(symbol);
            types.emplace_back();
            ast::TypeAndOrigins &ret = types[types.size() - 1];
            if (ret.type.get() == nullptr) {
                ret.type = ast::Types::nil();
                ret.origins.push_back(symbol.info(ctx).owner.info(ctx).definitionLoc);
            }
            return ret;
        }
        return types[fnd - vars.begin()];
    }

    void setTypeAndOrigin(ast::SymbolRef symbol, ast::TypeAndOrigins typeAndOrigins) {
        static ast::TypeAndOrigins dynamicTypeAndOrigin;

        auto fnd = find(vars.begin(), vars.end(), symbol);
        if (fnd == vars.end()) {
            vars.emplace_back(symbol);
            types.emplace_back();
            types[types.size() - 1] = typeAndOrigins;
            return;
        }
        types[fnd - vars.begin()] = typeAndOrigins;
        return;
    }

    void mergeWith(ast::Context ctx, Environment &other) {
        int i = 0;
        for (ast::SymbolRef var : vars) {
            auto otherTO = other.getTypeAndOrigin(ctx, var);
            if (types[i].type.get() != nullptr) {
                types[i].type = ast::Types::lub(ctx, types[i].type, otherTO.type);
            } else {
                types[i].type = otherTO.type;
            }
            types[i].origins.insert(types[i].origins.end(), otherTO.origins.begin(), otherTO.origins.end());
            i++;
        }
    }

    shared_ptr<ast::Type> processBinding(ast::Context ctx, cfg::Binding &bind, int loopCount) {
        ast::TypeAndOrigins tp;
        typecase(
            bind.value.get(),
            [&](cfg::Ident *i) {
                auto typeAndOrigin = getTypeAndOrigin(ctx, i->what);
                tp.type = typeAndOrigin.type;
                tp.origins = typeAndOrigin.origins;
                if (i->what == ctx.state.defn_todo()) {
                    tp.origins.push_back(ast::Loc::none(0));
                    tp.type = ast::Types::dynamic();
                } else {
                    Error::check(tp.origins.size() > 0, "Inferencer did not assign location");
                }
            },
            [&](cfg::Send *send) {
                vector<ast::TypeAndOrigins>
                    args; // Warning: do not make this be a reference vector. See comment on getTypeAndOrigin

                args.reserve(send->args.size());
                for (ast::SymbolRef arg : send->args) {
                    args.emplace_back(getTypeAndOrigin(ctx, arg));
                }

                auto recvType = getTypeAndOrigin(ctx, send->recv);
                tp.type = recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::New *i) { Error::notImplemented(); }, [&](cfg::Super *i) { Error::notImplemented(); },
            [&](cfg::FloatLit *i) {
                tp.type = make_shared<ast::Literal>(i->value);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::IntLit *i) {
                tp.type = make_shared<ast::Literal>(i->value);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::StringLit *i) {
                tp.type = make_shared<ast::Literal>(i->value);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::BoolLit *i) {
                tp.type = make_shared<ast::Literal>(i->value);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Nil *i) {
                tp.type = ast::Types::nil();
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Self *i) {
                tp.type = make_shared<ast::ClassType>(i->klass);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::NotSupported *i) {
                tp.type = ast::Types::dynamic();
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Return *i) {
                auto expectedType = ctx.owner.info(ctx).resultType;
                if (!expectedType) {
                    expectedType = ast::Types::dynamic();
                }
                auto typeAndOrigin = getTypeAndOrigin(ctx, i->what);
                if (!ast::Types::isSubType(ctx, typeAndOrigin.type, expectedType)) {
                    ctx.state.errors.error(ast::Reporter::ComplexError(
                        bind.loc, ast::ErrorClass::ReturnTypeMismatch,
                        "Returning value that does not conform to method result type",
                        {ast::Reporter::ErrorSection(
                             "Expected " + expectedType->toString(ctx),
                             {
                                 ast::Reporter::ErrorLine::from(
                                     ctx.owner.info(ctx).definitionLoc, "Method {} has return type is defined as {}",
                                     ctx.owner.info(ctx).name.toString(ctx), expectedType->toString(ctx)),
                             }),
                         ast::Reporter::ErrorSection("Got " + typeAndOrigin.type->toString(ctx) + " originating from:",
                                                     typeAndOrigin.origins2Explanations(ctx))}));
                }
                tp.type = ast::Types::bottom();
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::LoadArg *i) {
                /* read type from info filled by define_method */
                tp.type = getTypeAndOrigin(ctx, i->receiver).type->getCallArgumentType(ctx, i->method, i->arg);
                tp.origins.push_back(bind.loc);
            });
        Error::check(tp.type.get() != nullptr, "Inferencer did not assign type");
        Error::check(tp.origins.size() > 0, "Inferencer did not assign location");

        ast::TypeAndOrigins cur = getTypeAndOrigin(ctx, bind.bind);

        if (loopCount >= bind.bind.info(ctx).minLoops) {
            setTypeAndOrigin(bind.bind, tp);
        } else {
            if (!ast::Types::isSubType(ctx, tp.type, cur.type)) {
                ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::PinnedVariableMismatch,
                                       "Changing type of pinned argument, {} is not a subtype of {}",
                                       tp.type->toString(ctx), cur.type->toString(ctx));
            }
            tp.type = ast::Types::dynamic();
            setTypeAndOrigin(bind.bind, tp);
        }
        return tp.type;
    }

    void ensureGoodCondition(ast::Context ctx, ast::SymbolRef cond) {}
    void ensureGoodAssignTarget(ast::Context ctx, ast::SymbolRef target) {}
};

void ruby_typer::infer::Inference::run(ast::Context ctx, unique_ptr<cfg::CFG> &cfg) {
    vector<Environment> outEnvironments;
    outEnvironments.resize(cfg->basicBlocks.size());
    for (cfg::BasicBlock *bb : cfg->backwardsTopoSort) {
        if (bb == cfg->deadBlock())
            continue;
        Environment &current = outEnvironments[bb->id];
        for (ast::SymbolRef arg : bb->args) {
            current.vars.push_back(arg);
            current.types.emplace_back();
        }
        for (cfg::BasicBlock *parent : bb->backEdges) {
            current.mergeWith(ctx, outEnvironments[parent->id]);
        }
        for (cfg::Binding &bind : bb->exprs) {
            current.ensureGoodAssignTarget(ctx, bind.bind);
            bind.tpe = current.processBinding(ctx, bind, bb->outerLoops);
        }
        current.ensureGoodCondition(ctx, bb->bexit.cond);
    }
}
