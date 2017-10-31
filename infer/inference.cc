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
    vector<shared_ptr<Type>> types;

    shared_ptr<Type> &getType(ast::SymbolRef symbol) {
        auto fnd = find(vars.begin(), vars.end(), symbol);
        if (fnd == vars.end()) {
            Error::raise("Should never happen");
        }
        return types[fnd - vars.begin()];
    }

    void mergeWith(ast::Context ctx, Environment &other) {
        int i = 0;
        for (ast::SymbolRef var : vars) {
            types[i] = Types::lub(ctx, types[i], other.getType(var));
            i++;
        }
    }

    void processBinding(ast::Context ctx, cfg::Binding &bind, int loopCount) {
        shared_ptr<Type> tp;
        typecase(
            bind.value.get(), [&](cfg::Ident *i) { tp = getType(i->what); },
            [&](cfg::Send *send) {
                auto &recvType = getType(send->recv);
                vector<shared_ptr<Type>> args(send->args.size());
                for (ast::SymbolRef arg : send->args) {
                    args.emplace_back(getType(arg));
                }
                tp = recvType->dispatchCall(ctx, send->fun, args);
            },
            [&](cfg::New *i) { Error::notImplemented(); }, [&](cfg::Super *i) { Error::notImplemented(); },
            [&](cfg::FloatLit *i) { tp = make_shared<Literal>(i->value); },
            [&](cfg::IntLit *i) { tp = make_shared<Literal>(i->value); },
            [&](cfg::StringLit *i) { tp = make_shared<Literal>(i->value); }, [&](cfg::Nil *i) { tp = Types::nil(); },
            [&](cfg::Self *i) { tp = make_shared<ClassType>(i->klass); },
            [&](cfg::NotSupported *i) { tp = Types::dynamic(); },
            [&](cfg::Return *i) {
                auto expectedType =
                    make_shared<ClassType>(ctx.owner.info(ctx).result().orElse(ast::GlobalState::defn_dynamic()));
                if (!Types::isSubType(ctx, getType(i->what), expectedType)) {
                    ctx.state.errors.error(
                        ast::Loc::none(0), ast::ErrorClass::ReturnTypeMismatch,
                        "Returning value that does not conform to method {} result type. Expected: {},\n found: {}",
                        ctx.owner.toString(ctx), expectedType->toString(ctx), getType(i->what)->toString(ctx));
                }
                tp = Types::bottom();
            },
            [&](cfg::LoadArg *i) {
                /* read type from info filled by define_method */
                tp = Types::top();
            });

        shared_ptr<Type> &cur = getType(bind.bind);

        if (loopCount == 0) {
            cur = tp;
        } else {
            if (!Types::isSubType(ctx, tp, cur)) {
                ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::PinnedVariableMismatch,
                                       "Changing type of pinner argument, {} is not a subtype of {}", tp->toString(ctx),
                                       cur->toString(ctx));
            }
        }
    }

    void ensureGoodCondition(ast::Context ctx, ast::SymbolRef) {}
};

void ruby_typer::infer::Inference::run(ast::Context ctx, cfg::CFG &cfg) {
    vector<Environment> outEnvironments;
    outEnvironments.resize(cfg.basicBlocks.size());
    for (cfg::BasicBlock *bb : cfg.backwardsTopoSort) {
        if (bb == cfg.deadBlock())
            continue;
        Environment &current = outEnvironments[bb->id];
        for (ast::SymbolRef arg : bb->args) {
            current.vars.push_back(arg);
            current.types.emplace_back(Types::bottom());
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