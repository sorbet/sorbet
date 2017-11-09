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

        if (symbol.info(ctx).isClass()) {
            ast::TypeAndOrigins instanceTypeAndOrigin;
            ast::SymbolRef sym = symbol.info(ctx).singletonClass(ctx);
            instanceTypeAndOrigin.type = make_shared<ast::ClassType>(sym);
            instanceTypeAndOrigin.origins.push_back(sym.info(ctx).definitionLoc);
            return instanceTypeAndOrigin;
        }

        if (symbol.info(ctx).resultType != nullptr) {
            ast::TypeAndOrigins typeAndOrigin;
            typeAndOrigin.type = symbol.info(ctx).resultType;
            typeAndOrigin.origins.push_back(symbol.info(ctx).definitionLoc);
            return typeAndOrigin;
        }

        auto fnd = find(vars.begin(), vars.end(), symbol);
        if (fnd == vars.end()) {
            ast::TypeAndOrigins ret;
            ret.type = ast::Types::nil();
            ret.origins.push_back(symbol.info(ctx).owner.info(ctx).definitionLoc);
            return ret;
        }
        Error::check(types[fnd - vars.begin()].type.get() != nullptr);
        return types[fnd - vars.begin()];
    }

    void setTypeAndOrigin(ast::SymbolRef symbol, ast::TypeAndOrigins typeAndOrigins) {
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

    ast::TypeAndOrigins dispatchNew(ast::Context ctx, ast::TypeAndOrigins recvType, cfg::Send *send,
                                    vector<ast::TypeAndOrigins> &args, cfg::Binding &bind) {
        ast::TypeAndOrigins result;
        if (ast::ClassType *classType = dynamic_cast<ast::ClassType *>(recvType.type.get())) {
            ast::SymbolRef newSymbol = classType->symbol.info(ctx).findMember(ast::Names::new_());
            if (!newSymbol.exists() || newSymbol.info(ctx).owner == ast::GlobalState::defn_Basic_Object()) {
                ast::SymbolRef attachedClass = classType->symbol.info(ctx).attachedClass(ctx);
                Error::check(attachedClass.exists());
                result.type = make_shared<ast::ClassType>(attachedClass);
                result.origins.push_back(bind.loc);

                // call constructor
                newSymbol = attachedClass.info(ctx).findMember(ast::Names::initialize());
                if (newSymbol.exists()) {
                    auto initilizeResult =
                        result.type->dispatchCall(ctx, ast::Names::initialize(), bind.loc, args, recvType.type);
                } else {
                    if (args.size() > 0) {
                        ctx.state.errors.error(bind.loc, ast::ErrorClass::MethodArgumentCountMismatch,
                                               "Wrong number of arguments for constructor.\n Expected: 0, found: {}",
                                               args.size());
                    }
                }
            } else { // custom new was defined
                result.type = recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type);
                result.origins.push_back(bind.loc);
            }
        } else {
            result.type = recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type);
            result.origins.push_back(bind.loc);
        }
        return result;
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
                vector<ast::TypeAndOrigins> args;

                args.reserve(send->args.size());
                for (ast::SymbolRef arg : send->args) {
                    args.emplace_back(getTypeAndOrigin(ctx, arg));
                }

                auto recvType = getTypeAndOrigin(ctx, send->recv);
                if (send->fun == ast::Names::new_()) {
                    tp = dispatchNew(ctx, recvType, send, args, bind);
                } else {
                    tp.type = recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type);
                    tp.origins.push_back(bind.loc);
                }
            },
            [&](cfg::Super *i) { Error::notImplemented(); },
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
    vector<bool> visited;
    visited.resize(cfg->basicBlocks.size());
    for (cfg::BasicBlock *bb : cfg->backwardsTopoSort) {
        if (bb == cfg->deadBlock())
            continue;
        Environment &current = outEnvironments[bb->id];
        for (ast::SymbolRef arg : bb->args) {
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
                uninitialized.type = ast::Types::nil();
                uninitialized.origins.push_back(current.vars[i].info(ctx).owner.info(ctx).definitionLoc);
            }
            i++;
        }
        for (cfg::Binding &bind : bb->exprs) {
            current.ensureGoodAssignTarget(ctx, bind.bind);
            bind.tpe = current.processBinding(ctx, bind, bb->outerLoops);
        }
        current.ensureGoodCondition(ctx, bb->bexit.cond);
        visited[bb->id] = true;
    }
}
