#include "inference.h"
#include "../cfg/CFG.h"
#include "../core/Context.h"
#include "../core/Symbols.h"
#include <algorithm> // find
#include <unordered_map>

using namespace std;
using namespace ruby_typer;
using namespace infer;

class Environment {
public:
    vector<core::LocalVariable> vars;
    vector<core::TypeAndOrigins> types;

    core::TypeAndOrigins getTypeAndOrigin(core::Context ctx, core::LocalVariable symbol) {
        auto fnd = find(vars.begin(), vars.end(), symbol);
        if (fnd == vars.end()) {
            core::TypeAndOrigins ret;
            ret.type = core::Types::nil();
            ret.origins.push_back(ctx.owner.info(ctx).definitionLoc);
            return ret;
        }
        Error::check(types[fnd - vars.begin()].type.get() != nullptr);
        return types[fnd - vars.begin()];
    }

    void setTypeAndOrigin(core::LocalVariable symbol, core::TypeAndOrigins typeAndOrigins) {
        Error::check(typeAndOrigins.type.get() != nullptr);

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

    void mergeWith(core::Context ctx, Environment &other) {
        int i = 0;
        for (core::LocalVariable var : vars) {
            auto otherTO = other.getTypeAndOrigin(ctx, var);
            auto &thisTO = types[i];
            if (thisTO.type.get() != nullptr) {
                thisTO.type = core::Types::lub(ctx, thisTO.type, otherTO.type);
            } else {
                types[i].type = otherTO.type;
            }
            for (auto origin : otherTO.origins) {
                if (find(thisTO.origins.begin(), thisTO.origins.end(), origin) == thisTO.origins.end()) {
                    thisTO.origins.push_back(origin);
                }
            }

            i++;
        }
    }

    shared_ptr<core::Type> dispatchNew(core::Context ctx, core::TypeAndOrigins recvType, cfg::Send *send,
                                       vector<core::TypeAndOrigins> &args, cfg::Binding &bind) {
        core::ClassType *classType = dynamic_cast<core::ClassType *>(recvType.type.get());
        if (classType == nullptr) {
            return recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type);
        }

        if (classType->symbol == core::GlobalState::defn_untyped()) {
            return recvType.type;
        }

        core::SymbolRef newSymbol = classType->symbol.info(ctx).findMember(core::Names::new_());
        if (newSymbol.exists() && newSymbol.info(ctx).owner != core::GlobalState::defn_Basic_Object()) {
            // custom `new` was defined
            return recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type);
        }

        core::SymbolRef attachedClass = classType->symbol.info(ctx).attachedClass(ctx);
        if (!attachedClass.exists()) {
            // `foo`.new() but `foo` isn't a Class
            return recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type);
        }

        auto type = make_shared<core::ClassType>(attachedClass);

        // call constructor
        newSymbol = attachedClass.info(ctx).findMember(core::Names::initialize());
        if (newSymbol.exists()) {
            auto initializeResult = type->dispatchCall(ctx, core::Names::initialize(), bind.loc, args, recvType.type);
        } else {
            if (args.size() > 0) {
                ctx.state.errors.error(bind.loc, core::ErrorClass::MethodArgumentCountMismatch,
                                       "Wrong number of arguments for constructor.\n Expected: 0, found: {}",
                                       args.size());
            }
        }
        return type;
    }

    shared_ptr<core::Type> dropLiteral(shared_ptr<core::Type> tp) {
        if (auto *a = dynamic_cast<core::Literal *>(tp.get())) {
            return a->underlying;
        }
        return tp;
    }

    shared_ptr<core::Type> processBinding(core::Context ctx, cfg::Binding &bind, int loopCount, int bindMinLoops) {
        core::TypeAndOrigins tp;
        bool noLoopChecking = dynamic_cast<cfg::Alias *>(bind.value.get()) != nullptr;
        typecase(
            bind.value.get(),
            [&](cfg::Alias *a) {
                core::SymbolRef symbol = a->what;
                core::Symbol &info = symbol.info(ctx);
                if (info.isClass()) {
                    core::SymbolRef sym = info.singletonClass(ctx);
                    tp.type = make_shared<core::ClassType>(sym);
                    tp.origins.push_back(info.definitionLoc);
                } else if (info.isField() || info.isStaticField() || info.isMethodArgument()) {
                    if (info.resultType.get() != nullptr) {
                        tp.type = info.resultType;
                        tp.origins.push_back(info.definitionLoc);
                    } else {
                        tp.origins.push_back(core::Loc::none(0));
                        tp.type = core::Types::dynamic();
                    }
                } else {
                    Error::notImplemented();
                }
            },
            [&](cfg::Ident *i) {
                auto typeAndOrigin = getTypeAndOrigin(ctx, i->what);
                tp.type = typeAndOrigin.type;
                tp.origins = typeAndOrigin.origins;
                Error::check(tp.origins.size() > 0, "Inferencer did not assign location");
            },
            [&](cfg::Send *send) {
                vector<core::TypeAndOrigins> args;

                args.reserve(send->args.size());
                for (core::LocalVariable arg : send->args) {
                    args.emplace_back(getTypeAndOrigin(ctx, arg));
                }

                auto recvType = getTypeAndOrigin(ctx, send->recv);
                if (send->fun == core::Names::new_()) {
                    tp.type = dispatchNew(ctx, recvType, send, args, bind);
                } else if (send->fun == core::Names::super()) {
                    // TODO
                    tp.type = core::Types::dynamic();
                    tp.origins.push_back(bind.loc);
                } else {
                    tp.type = recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type);
                }
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::FloatLit *i) {
                tp.type = make_shared<core::Literal>(i->value);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::IntLit *i) {
                tp.type = make_shared<core::Literal>(i->value);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::StringLit *i) {
                tp.type = make_shared<core::Literal>(i->value);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::BoolLit *i) {
                tp.type = make_shared<core::Literal>(i->value);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Nil *i) {
                tp.type = core::Types::nil();
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Self *i) {
                tp.type = make_shared<core::ClassType>(i->klass);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::NotSupported *i) {
                tp.type = core::Types::dynamic();
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Return *i) {
                auto expectedType = ctx.owner.info(ctx).resultType;
                if (!expectedType) {
                    expectedType = core::Types::dynamic();
                }
                auto typeAndOrigin = getTypeAndOrigin(ctx, i->what);
                if (!core::Types::isSubType(ctx, typeAndOrigin.type, expectedType)) {
                    ctx.state.errors.error(core::Reporter::ComplexError(
                        bind.loc, core::ErrorClass::ReturnTypeMismatch,
                        "Returning value that does not conform to method result type",
                        {core::Reporter::ErrorSection(
                             "Expected " + expectedType->toString(ctx),
                             {
                                 core::Reporter::ErrorLine::from(
                                     ctx.owner.info(ctx).definitionLoc, "Method {} has return type is defined as {}",
                                     ctx.owner.info(ctx).name.toString(ctx), expectedType->toString(ctx)),
                             }),
                         core::Reporter::ErrorSection("Got " + typeAndOrigin.type->toString(ctx) + " originating from:",
                                                      typeAndOrigin.origins2Explanations(ctx))}));
                }
                tp.type = core::Types::bottom();
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::LoadArg *i) {
                /* read type from info filled by define_method */
                tp.type = getTypeAndOrigin(ctx, i->receiver).type->getCallArgumentType(ctx, i->method, i->arg);
                tp.origins.push_back(bind.loc);
            });
        Error::check(tp.type.get() != nullptr, "Inferencer did not assign type");
        Error::check(tp.origins.size() > 0, "Inferencer did not assign location");

        core::TypeAndOrigins cur = getTypeAndOrigin(ctx, bind.bind);

        if (noLoopChecking || loopCount == bindMinLoops) {
            setTypeAndOrigin(bind.bind, tp);
        } else {
            if (!core::Types::isSubType(ctx, dropLiteral(tp.type), dropLiteral(cur.type))) {
                ctx.state.errors.error(bind.loc, core::ErrorClass::PinnedVariableMismatch,
                                       "Changing type of pinned argument, {} is not a subtype of {}",
                                       tp.type->toString(ctx), cur.type->toString(ctx));
                tp.type = core::Types::dynamic();
            }
            setTypeAndOrigin(bind.bind, tp);
        }
        return tp.type;
    }

    void ensureGoodCondition(core::Context ctx, core::LocalVariable cond) {}
    void ensureGoodAssignTarget(core::Context ctx, core::LocalVariable target) {}
};

void ruby_typer::infer::Inference::run(core::Context ctx, unique_ptr<cfg::CFG> &cfg) {
    vector<Environment> outEnvironments;
    outEnvironments.resize(cfg->basicBlocks.size());
    vector<bool> visited;
    visited.resize(cfg->basicBlocks.size());
    for (cfg::BasicBlock *bb : cfg->backwardsTopoSort) {
        if (bb == cfg->deadBlock())
            continue;
        Environment &current = outEnvironments[bb->id];
        current.vars.reserve(bb->args.size());
        current.types.reserve(bb->args.size());
        for (core::LocalVariable arg : bb->args) {
            current.vars.push_back(arg);
            current.types.emplace_back();
        }
        for (cfg::BasicBlock *parent : bb->backEdges) {
            if (!visited[parent->id])
                continue;
            current.mergeWith(ctx, outEnvironments[parent->id]);
        }
        int i = 0;
        for (auto &uninitialized : current.types) {
            if (uninitialized.type.get() == nullptr) {
                uninitialized.type = core::Types::nil();
                uninitialized.origins.push_back(ctx.owner.info(ctx).definitionLoc);
            }
            i++;
        }
        for (cfg::Binding &bind : bb->exprs) {
            current.ensureGoodAssignTarget(ctx, bind.bind);
            bind.tpe = current.processBinding(ctx, bind, bb->outerLoops, cfg->minLoops[bind.bind]);
        }
        current.ensureGoodCondition(ctx, bb->bexit.cond);
        visited[bb->id] = true;
    }
}
