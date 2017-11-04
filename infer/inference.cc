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

    ast::TypeAndOrigins &getTypeAndOrigin(ast::SymbolRef symbol) {
        auto fnd = find(vars.begin(), vars.end(), symbol);
        if (fnd == vars.end()) {
            vars.emplace_back(symbol);
            types.emplace_back();
            ast::TypeAndOrigins &ret = types[types.size() - 1];
            if (ret.type.get() == nullptr) {
                ret.type = ast::Types::nil();
            }
            return types[types.size() - 1];
        }
        return types[fnd - vars.begin()];
    }

    void mergeWith(ast::Context ctx, Environment &other) {
        int i = 0;
        for (ast::SymbolRef var : vars) {
            auto &otherTO = other.getTypeAndOrigin(var);
            if (types[i].type.get() != nullptr) {
                types[i].type = ast::Types::lub(ctx, types[i].type, otherTO.type);
            } else {
                types[i].type = otherTO.type;
            }
            types[i].origins.insert(types[i].origins.end(), otherTO.origins.begin(), otherTO.origins.end());
            i++;
        }
    }

    void processBinding(ast::Context ctx, cfg::Binding &bind, int loopCount) {
        shared_ptr<ast::Type> tp;
        vector<ast::Loc> loc; // todo: use tiny vector
        typecase(
            bind.value.get(),
            [&](cfg::Ident *i) {
                auto &typeAndOrigin = getTypeAndOrigin(i->what);
                tp = typeAndOrigin.type;
                loc = typeAndOrigin.origins;
                if (i->what.isPlaceHolder()) {
                    loc.push_back(ast::Loc::none(0));
                    tp = ast::Types::dynamic();
                } else {
                    Error::check(loc.size() > 0, "Inferencer did not assign location");
                }
            },
            [&](cfg::Send *send) {
                auto &recvType = getTypeAndOrigin(send->recv);
                vector<ast::TypeAndOrigins> args;

                args.reserve(send->args.size());
                for (ast::SymbolRef arg : send->args) {
                    args.emplace_back(getTypeAndOrigin(arg));
                }
                tp = recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type);
                loc.push_back(bind.loc);
            },
            [&](cfg::New *i) { Error::notImplemented(); }, [&](cfg::Super *i) { Error::notImplemented(); },
            [&](cfg::FloatLit *i) {
                tp = make_shared<ast::Literal>(i->value);
                loc.push_back(bind.loc);
            },
            [&](cfg::IntLit *i) {
                tp = make_shared<ast::Literal>(i->value);
                loc.push_back(bind.loc);
            },
            [&](cfg::StringLit *i) {
                tp = make_shared<ast::Literal>(i->value);
                loc.push_back(bind.loc);
            },
            [&](cfg::BoolLit *i) {
                tp = make_shared<ast::Literal>(i->value);
                loc.push_back(bind.loc);
            },
            [&](cfg::Nil *i) {
                tp = ast::Types::nil();
                loc.push_back(bind.loc);
            },
            [&](cfg::Self *i) {
                tp = make_shared<ast::ClassType>(i->klass);
                loc.push_back(bind.loc);
            },
            [&](cfg::NotSupported *i) {
                tp = ast::Types::dynamic();
                loc.push_back(bind.loc);
            },
            [&](cfg::Return *i) {
                auto expectedType = ctx.owner.info(ctx).resultType;
                if (!expectedType) {
                    expectedType = ast::Types::dynamic();
                }
                auto &typeAndOrigin = getTypeAndOrigin(i->what);
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
                tp = ast::Types::bottom();
                loc.push_back(bind.loc);
            },
            [&](cfg::LoadArg *i) {
                /* read type from info filled by define_method */
                tp = getTypeAndOrigin(i->receiver).type->getCallArgumentType(ctx, i->method, i->arg);
                loc.push_back(bind.loc);
            });
        Error::check(tp.get() != nullptr, "Inferencer did not assign type");
        Error::check(loc.size() > 0, "Inferencer did not assign location");

        ast::TypeAndOrigins &cur = getTypeAndOrigin(bind.bind);

        if (loopCount >= bind.bind.info(ctx).minLoops) {
            cur.type = tp;
            cur.origins = loc;
        } else {
            if (!ast::Types::isSubType(ctx, tp, cur.type)) {
                ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::PinnedVariableMismatch,
                                       "Changing type of pinned argument, {} is not a subtype of {}", tp->toString(ctx),
                                       cur.type->toString(ctx));
            }
            cur.origins = loc;
        }
    }

    void ensureGoodCondition(ast::Context ctx, ast::SymbolRef) {}
};

void ruby_typer::infer::Inference::run(ast::Context ctx, std::unique_ptr<cfg::CFG> &cfg) {
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
            current.processBinding(ctx, bind, bb->outerLoops);
        }
        current.ensureGoodCondition(ctx, bb->bexit.cond);
    }
}
