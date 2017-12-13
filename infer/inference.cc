#include "inference.h"
#include "../cfg/CFG.h"
#include "../core/Context.h"
#include "../core/Symbols.h"
#include <algorithm> // find, remove_if
#include <unordered_map>

using namespace std;
using namespace ruby_typer;
using namespace infer;
template struct std::pair<core::LocalVariable, shared_ptr<core::Type>>;

class Environment;

/**
 * Encode things that we know hold and don't hold
 */
struct KnowledgeFact {
    /* the following type tests are known to be true */
    std::vector<std::pair<core::LocalVariable, shared_ptr<core::Type>>> yesTypeTests;
    /* he following type tests are known to be false */
    std::vector<std::pair<core::LocalVariable, shared_ptr<core::Type>>> noTypeTests;

    /* this is a "merge" of two knowledges - computes a "lub" of knowledges */
    void min(core::Context ctx, KnowledgeFact other) {
        for (auto it = yesTypeTests.begin(); it != yesTypeTests.end(); /* nothing */) {
            auto &entry = *it;
            core::LocalVariable local = entry.first;
            auto fnd = find_if(other.yesTypeTests.begin(), other.yesTypeTests.end(),
                               [&](auto const &elem) -> bool { return elem.first == local; });
            if (fnd == other.yesTypeTests.end()) {
                it = yesTypeTests.erase(it);
            } else {
                entry.second = core::Types::lub(ctx, fnd->second, entry.second);
                it++;
            }
        }
        for (auto it = noTypeTests.begin(); it != noTypeTests.end(); /* nothing */) {
            auto &entry = *it;
            core::LocalVariable local = entry.first;
            auto fnd = find_if(other.noTypeTests.begin(), other.noTypeTests.end(),
                               [&](auto const &elem) -> bool { return elem.first == local; });
            if (fnd == other.noTypeTests.end()) {
                it = noTypeTests.erase(it);
            } else {
                entry.second = core::Types::glb(ctx, fnd->second, entry.second);
                it++;
            }
        }
    }

    /** Computes all possible implications of this knowledge holding as an exit from environment env in block forBlock
     */
    KnowledgeFact under(core::Context ctx, Environment env, core::Loc loc, cfg::CFG &inWhat, cfg::BasicBlock &forBlock);
    void sanityCheck() {
        if (!debug_mode) {
            return;
        }
        for (auto &a : yesTypeTests) {
            Error::check(a.second.get() != nullptr);
        }
        for (auto &a : noTypeTests) {
            Error::check(a.second.get() != nullptr);
        }
    }

    string toString(core::Context ctx) {
        stringstream buf;

        for (auto &el : yesTypeTests) {
            buf << "    " << el.first.name.toString(ctx) << " to be " << el.second->toString(ctx, 0) << endl;
        }
        for (auto &el : noTypeTests) {
            buf << "    " << el.first.name.toString(ctx) << " NOT to be " << el.second->toString(ctx, 0) << endl;
        }
        return buf.str();
    }
};

/** Almost a named pair of two KnowledgeFact-s. One holds knowledge that is true when a variable is falsy,
 * the other holds knowledge which is true if the same variable is falsy.
 */
class TestedKnowledge {
public:
    KnowledgeFact truthy, falsy;
    bool seenTruthyOption; // Only used during environment merge. Used to indicate "all-knowing" truthy option.
    bool seenFalsyOption;  // Same for falsy

    string toString(core::Context ctx) {
        stringstream buf;
        if (!truthy.noTypeTests.empty() || !truthy.yesTypeTests.empty()) {
            buf << "  Being truthy entails:" << endl;
        }
        buf << truthy.toString(ctx);
        if (!falsy.noTypeTests.empty() || !falsy.yesTypeTests.empty()) {
            buf << "  Being falsy entails:" << endl;
        }
        buf << falsy.toString(ctx);
        return buf.str();
    }

    static TestedKnowledge empty; // optimization

    void sanityCheck() {
        if (!debug_mode) {
            return;
        }
        truthy.sanityCheck();
        falsy.sanityCheck();
        Error::check(TestedKnowledge::empty.falsy.yesTypeTests.empty());
        Error::check(TestedKnowledge::empty.falsy.noTypeTests.empty());
        Error::check(TestedKnowledge::empty.truthy.noTypeTests.empty());
        Error::check(TestedKnowledge::empty.truthy.yesTypeTests.empty());
    };
};

TestedKnowledge TestedKnowledge::empty;

class Environment {
public:
    bool isDead = false;
    cfg::BasicBlock *bb;
    vector<core::LocalVariable> vars;
    vector<core::TypeAndOrigins> types;
    vector<TestedKnowledge> knowledge;

    string toString(core::Context ctx) {
        stringstream buf;
        buf << "dead=" << isDead << endl;
        int i = 0;
        for (auto var : vars) {
            buf << var.name.toString(ctx) << ": " << types[i].type->toString(ctx, 0) << endl;
            buf << knowledge[i].toString(ctx) << endl;

            i++;
        }
        return buf.str();
    }

    core::TypeAndOrigins getTypeAndOrigin(core::Context ctx, core::LocalVariable symbol,
                                          bool createIfDoesNotExist = false) {
        auto fnd = find(vars.begin(), vars.end(), symbol);
        if (fnd == vars.end()) {
            core::TypeAndOrigins ret;
            ret.type = core::Types::nil();
            ret.origins.push_back(ctx.owner.info(ctx).definitionLoc);
            if (createIfDoesNotExist) {
                vars.emplace_back(symbol);
                types.push_back(ret);
                knowledge.emplace_back();
            }
            return ret;
        }
        Error::check(types[fnd - vars.begin()].type.get() != nullptr);
        return types[fnd - vars.begin()];
    }

    TestedKnowledge &getKnowledge(core::LocalVariable symbol, bool shouldFail = true) {
        auto fnd = find(vars.begin(), vars.end(), symbol);
        if (fnd == vars.end()) {
            Error::check(!shouldFail, "Missing knowledge?");
            return TestedKnowledge::empty;
        }
        auto &r = knowledge[fnd - vars.begin()];
        r.sanityCheck();
        return r;
    }

    /* propagate knowledge on `to = from` */
    void propagateKnowledge(core::Context ctx, core::LocalVariable to, core::LocalVariable from) {
        getKnowledge(to) = getKnowledge(from);
        getKnowledge(to).truthy.noTypeTests.emplace_back(from, core::Types::falsyTypes());
        getKnowledge(to).falsy.yesTypeTests.emplace_back(from, core::Types::falsyTypes());
        getKnowledge(from).truthy.noTypeTests.emplace_back(to, core::Types::falsyTypes());
        getKnowledge(from).falsy.yesTypeTests.emplace_back(to, core::Types::falsyTypes());
        getKnowledge(from).sanityCheck();
        getKnowledge(to).sanityCheck();
    }

    /* variable was reasigned. Forget everything about previous value */
    void clearKnowledge(core::Context ctx, core::LocalVariable reassigned) {
        for (auto &k : knowledge) {
            k.truthy.yesTypeTests.erase(
                remove_if(k.truthy.yesTypeTests.begin(), k.truthy.yesTypeTests.end(),
                          [&](auto const &c) -> bool { return c.first.name == reassigned.name; }),
                k.truthy.yesTypeTests.end());
            k.falsy.yesTypeTests.erase(
                remove_if(k.falsy.yesTypeTests.begin(), k.falsy.yesTypeTests.end(),
                          [&](auto const &c) -> bool { return c.first.name == reassigned.name; }),
                k.falsy.yesTypeTests.end());
            k.truthy.noTypeTests.erase(
                remove_if(k.truthy.noTypeTests.begin(), k.truthy.noTypeTests.end(),
                          [&](auto const &c) -> bool { return c.first.name == reassigned.name; }),
                k.truthy.noTypeTests.end());
            k.falsy.noTypeTests.erase(remove_if(k.falsy.noTypeTests.begin(), k.falsy.noTypeTests.end(),
                                                [&](auto const &c) -> bool { return c.first.name == reassigned.name; }),
                                      k.falsy.noTypeTests.end());
            k.sanityCheck();
        }
    }

    /* Special case sources of knowledge */
    void updateKnowledge(core::Context ctx, core::LocalVariable local, cfg::Send *send) {
        auto &whoKnows = getKnowledge(local);

        if (send->fun == core::Names::bang()) {
            auto other = find(vars.begin(), vars.end(), send->recv) - vars.begin();
            if (other != vars.size()) {
                whoKnows.truthy = this->knowledge[other].falsy;
                whoKnows.falsy = this->knowledge[other].truthy;
                this->knowledge[other].truthy.noTypeTests.emplace_back(local, core::Types::falsyTypes());
                this->knowledge[other].falsy.yesTypeTests.emplace_back(local, core::Types::falsyTypes());
            }
            whoKnows.truthy.yesTypeTests.emplace_back(send->recv, core::Types::falsyTypes());
            whoKnows.falsy.noTypeTests.emplace_back(send->recv, core::Types::falsyTypes());

            whoKnows.sanityCheck();
        } else if (send->fun == core::Names::nil_p()) {
            whoKnows.truthy.yesTypeTests.emplace_back(send->recv, core::Types::nil());
            whoKnows.falsy.noTypeTests.emplace_back(send->recv, core::Types::nil());
            whoKnows.sanityCheck();
        }

        if (send->args.empty()) {
            return;
        }
        if (send->fun == core::Names::kind_of() || send->fun == core::Names::is_a_p()) {
            core::TypeAndOrigins klass = getTypeAndOrigin(ctx, send->args[0]);
            if (klass.type->derivesFrom(ctx, core::GlobalState::defn_Class())) {
                auto *s = dynamic_cast<core::ClassType *>(klass.type.get());
                Error::check(s != nullptr);
                core::SymbolRef attachedClass = s->symbol.info(ctx).attachedClass(ctx);
                if (attachedClass.exists()) {
                    whoKnows.truthy.yesTypeTests.emplace_back(send->recv, make_shared<core::ClassType>(attachedClass));
                    whoKnows.falsy.noTypeTests.emplace_back(send->recv, make_shared<core::ClassType>(attachedClass));
                }
                whoKnows.sanityCheck();
            }
        } else if (send->fun == core::Names::eqeq()) {
            core::TypeAndOrigins tp1 = getTypeAndOrigin(ctx, send->args[0]);
            core::TypeAndOrigins tp2 = getTypeAndOrigin(ctx, send->recv);
            if (tp1.type->isDynamic() && tp2.type->isDynamic()) {
                return;
            }

            Error::check(tp1.type.get() != nullptr);
            Error::check(tp2.type.get() != nullptr);
            whoKnows.truthy.yesTypeTests.emplace_back(send->recv, tp1.type);
            whoKnows.truthy.yesTypeTests.emplace_back(send->args[0], tp2.type);
            whoKnows.sanityCheck();
        }
    }

    void setTypeAndOrigin(core::LocalVariable symbol, core::TypeAndOrigins typeAndOrigins) {
        Error::check(typeAndOrigins.type.get() != nullptr);

        auto fnd = find(vars.begin(), vars.end(), symbol);
        if (fnd == vars.end()) {
            vars.emplace_back(symbol);
            types.push_back(typeAndOrigins);
            knowledge.emplace_back();
            return;
        }
        types[fnd - vars.begin()] = typeAndOrigins;
        return;
    }

    /* Create an Environment out of this one that holds if final condition in this environment was isTrue */
    Environment
    withCond(core::Context ctx, bool isTrue,
             const vector<core::LocalVariable> &filter) { // todo: copying environments here is slow. And commonly this
        // returns a simple copy. Add an external fast path
        if (!bb->bexit.cond.exists() || bb->bexit.cond.name == core::Names::blockCall()) {
            return *this; // copy
        }
        auto fnd = find(vars.begin(), vars.end(), bb->bexit.cond);
        if (fnd == vars.end()) {
            return *this; // copy
        }
        auto &thisKnowledge = this->knowledge[fnd - vars.begin()];
        thisKnowledge.sanityCheck();
        Environment copy = *this;
        if (!isTrue) {
            core::TypeAndOrigins tp = copy.getTypeAndOrigin(ctx, bb->bexit.cond);
            tp.origins.emplace_back(this->bb->bexit.loc);
            if (tp.type->isDynamic()) {
                tp.type = core::Types::falsyTypes();
            } else {
                tp.type = core::Types::glb(ctx, tp.type, core::Types::falsyTypes());
            }
            copy.setTypeAndOrigin(bb->bexit.cond, tp);
        } else {
            core::TypeAndOrigins tp = copy.getTypeAndOrigin(ctx, bb->bexit.cond);
            tp.origins.emplace_back(this->bb->bexit.loc);
            tp.type = core::Types::dropSubtypesOf(
                ctx, core::Types::dropSubtypesOf(ctx, tp.type, core::GlobalState::defn_NilClass()),
                core::GlobalState::defn_FalseClass());
            copy.setTypeAndOrigin(bb->bexit.cond, tp);
        }

        auto &knowledgeToChoose = isTrue ? thisKnowledge.truthy : thisKnowledge.falsy;

        for (auto &typeTested : knowledgeToChoose.yesTypeTests) {
            if (find(filter.begin(), filter.end(), typeTested.first) == filter.end()) {
                continue;
            }
            core::TypeAndOrigins tp = copy.getTypeAndOrigin(ctx, typeTested.first);
            if (tp.type->isDynamic()) { // this is actually incorrect, as it may be some more exact type, but this rule
                // makes it easier to migrate code
                tp.type = typeTested.second;
            } else {
                tp.type = core::Types::glb(ctx, tp.type, typeTested.second);
                if (tp.type->isBottom()) {
                    copy.isDead = true;
                }
            }
            copy.setTypeAndOrigin(typeTested.first, tp);
        }

        for (auto &typeTested : knowledgeToChoose.noTypeTests) {
            if (find(filter.begin(), filter.end(), typeTested.first) == filter.end()) {
                continue;
            }
            core::TypeAndOrigins tp = copy.getTypeAndOrigin(ctx, typeTested.first);
            if (!tp.type->isDynamic()) {
                tp.type = core::Types::approximateSubtract(ctx, tp.type, typeTested.second);
                if (tp.type->isBottom()) {
                    copy.isDead = true;
                }
                copy.setTypeAndOrigin(typeTested.first, tp);
            }
        }
        return copy;
    }

    void mergeWith(core::Context ctx, Environment &other, core::Loc loc, cfg::CFG &inWhat, cfg::BasicBlock &forBlock) {
        int i = 0;
        this->isDead |= other.isDead;
        for (core::LocalVariable var : vars) {
            auto otherTO = other.getTypeAndOrigin(ctx, var);
            auto &thisTO = types[i];
            if (thisTO.type.get() != nullptr) {
                thisTO.type = core::Types::lub(ctx, thisTO.type, otherTO.type);
                thisTO.type->sanityCheck(ctx);
            } else {
                types[i].type = otherTO.type;
            }
            for (auto origin : otherTO.origins) {
                if (find(thisTO.origins.begin(), thisTO.origins.end(), origin) == thisTO.origins.end()) {
                    thisTO.origins.push_back(origin);
                }
            }

            if (((forBlock.flags & cfg::CFG::LOOP_HEADER) != 0) && forBlock.outerLoops <= inWhat.maxLoopWrite[var]) {
                i++;
                continue;
            }
            bool canBeFalsy = core::Types::canBeFalsy(ctx, otherTO.type);
            bool canBeTruthy = core::Types::canBeTruthy(ctx, otherTO.type);

            if (canBeTruthy) {
                auto &thisKnowledge = getKnowledge(var);
                auto &otherKnowledge = other.getKnowledge(var, false);
                if (!thisKnowledge.seenTruthyOption) {
                    thisKnowledge.seenTruthyOption = true;
                    thisKnowledge.truthy = otherKnowledge.truthy.under(ctx, other, loc, inWhat, forBlock);
                } else {
                    thisKnowledge.truthy.min(ctx, otherKnowledge.truthy.under(ctx, other, loc, inWhat, forBlock));
                }
            }

            if (canBeFalsy) {
                auto &thisKnowledge = getKnowledge(var);
                auto &otherKnowledge = other.getKnowledge(var, false);
                if (!thisKnowledge.seenFalsyOption) {
                    thisKnowledge.seenFalsyOption = true;
                    thisKnowledge.falsy = otherKnowledge.falsy.under(ctx, other, loc, inWhat, forBlock);
                } else {
                    thisKnowledge.falsy.min(ctx, otherKnowledge.falsy.under(ctx, other, loc, inWhat, forBlock));
                }
            }

            i++;
        }
    }

    void populateFrom(core::Context ctx, Environment &other) {
        int i = 0;
        this->isDead = other.isDead;
        for (core::LocalVariable var : vars) {
            auto otherTO = other.getTypeAndOrigin(ctx, var);
            types[i].type = otherTO.type;
            types[i].origins = otherTO.origins;
            auto &thisKnowledge = getKnowledge(var);
            auto &otherKnowledge = other.getKnowledge(var, false);
            thisKnowledge = otherKnowledge;
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

        core::SymbolRef newSymbol = classType->symbol.info(ctx).findMemberTransitive(ctx, core::Names::new_());
        if (newSymbol.exists() && newSymbol.info(ctx).owner != core::GlobalState::defn_BasicObject()) {
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
        newSymbol = attachedClass.info(ctx).findMemberTransitive(ctx, core::Names::initialize());
        if (newSymbol.exists()) {
            auto initializeResult = type->dispatchCall(ctx, core::Names::initialize(), bind.loc, args, recvType.type);
        } else {
            if (!args.empty()) {
                ctx.state.errors.error(bind.loc, core::ErrorClass::MethodArgumentCountMismatch,
                                       "Wrong number of arguments for constructor.\n Expected: 0, found: {}",
                                       args.size());
            }
        }
        return type;
    }

    shared_ptr<core::Type> dropLiteral(shared_ptr<core::Type> tp) {
        if (auto *a = dynamic_cast<core::LiteralType *>(tp.get())) {
            return a->underlying;
        }
        return tp;
    }

    shared_ptr<core::Type> processBinding(core::Context ctx, cfg::Binding &bind, int loopCount, int bindMinLoops) {
        try {
            core::TypeAndOrigins tp;
            bool noLoopChecking = dynamic_cast<cfg::Alias *>(bind.value.get()) != nullptr ||
                                  dynamic_cast<cfg::LoadArg *>(bind.value.get()) != nullptr;
            typecase(
                bind.value.get(),
                [&](cfg::Alias *a) {
                    core::SymbolRef symbol = a->what;
                    core::Symbol &info = symbol.info(ctx);
                    if (info.isClass()) {
                        if (info.resultType.get() == nullptr) {
                            core::SymbolRef sym = info.singletonClass(ctx);
                            tp.type = make_shared<core::ClassType>(sym);
                        } else {
                            tp.type = info.resultType;
                        }
                        tp.origins.push_back(symbol.info(ctx).definitionLoc);
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
                    auto typeAndOrigin = getTypeAndOrigin(ctx, i->what, true);
                    tp.type = typeAndOrigin.type;
                    tp.origins = typeAndOrigin.origins;
                    Error::check(!tp.origins.empty(), "Inferencer did not assign location");
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
                    tp.type = make_shared<core::LiteralType>(i->value);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::IntLit *i) {
                    tp.type = make_shared<core::LiteralType>(i->value);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::StringLit *i) {
                    tp.type = make_shared<core::LiteralType>(ctx.state.defn_String(), i->value);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::SymbolLit *i) {
                    tp.type = make_shared<core::LiteralType>(ctx.state.defn_Symbol(), i->value);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::BoolLit *i) {
                    tp.type = make_shared<core::LiteralType>(i->value);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::Self *i) {
                    tp.type = make_shared<core::ClassType>(i->klass);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::Unanalyzable *i) {
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
                                     core::Reporter::ErrorLine::from(ctx.owner.info(ctx).definitionLoc,
                                                                     "Method {} has return type is defined as {}",
                                                                     ctx.owner.info(ctx).name.toString(ctx),
                                                                     expectedType->toString(ctx)),
                                 }),
                             core::Reporter::ErrorSection("Got " + typeAndOrigin.type->toString(ctx) +
                                                              " originating from:",
                                                          typeAndOrigin.origins2Explanations(ctx))}));
                    }
                    tp.type = core::Types::bottom();
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::LoadArg *i) {
                    /* read type from info filled by define_method */
                    tp.type = getTypeAndOrigin(ctx, i->receiver).type->getCallArgumentType(ctx, i->method, i->arg);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::Cast *c) {
                    tp.type = c->type;
                    tp.origins.push_back(bind.loc);

                    if (c->assertType) {
                        auto ty = getTypeAndOrigin(ctx, c->value);
                        if (ty.type->isDynamic()) {
                            ctx.state.errors.error(core::Reporter::ComplexError(
                                bind.loc, core::ErrorClass::CastTypeMismatch,
                                "The typechecker was unable to infer the type of the argument to "
                                "assert_type!.",
                                {core::Reporter::ErrorSection("Value originated from:", ty.origins2Explanations(ctx)),
                                 core::Reporter::ErrorSection("You may need to add additional `standard_method` "
                                                              "annotations.")}));
                        } else if (!core::Types::isSubType(ctx, ty.type, c->type)) {
                            ctx.state.errors.error(core::Reporter::ComplexError(
                                bind.loc, core::ErrorClass::CastTypeMismatch,
                                "assert_type!: argument does not have asserted type",
                                {core::Reporter::ErrorSection("Expected " + c->type->toString(ctx), {}),
                                 core::Reporter::ErrorSection("Got " + ty.type->toString(ctx) + " originating from:",
                                                              ty.origins2Explanations(ctx))}));
                        }
                    }
                });
            Error::check(tp.type.get() != nullptr, "Inferencer did not assign type");
            Error::check(!tp.origins.empty(), "Inferencer did not assign location");

            core::TypeAndOrigins cur = getTypeAndOrigin(ctx, bind.bind, true);

            if (noLoopChecking || loopCount == bindMinLoops) {
                clearKnowledge(ctx, bind.bind);
                if (auto *send = dynamic_cast<cfg::Send *>(bind.value.get())) {
                    updateKnowledge(ctx, bind.bind, send);
                } else if (auto *i = dynamic_cast<cfg::Ident *>(bind.value.get())) {
                    propagateKnowledge(ctx, bind.bind, i->what);
                }
                setTypeAndOrigin(bind.bind, tp);
            } else {
                if (!core::Types::isSubType(ctx, dropLiteral(tp.type), dropLiteral(cur.type))) {
                    ctx.state.errors.error(bind.loc, core::ErrorClass::PinnedVariableMismatch,
                                           "Changing type of pinned argument, {} is not a subtype of {}",
                                           tp.type->toString(ctx), cur.type->toString(ctx));
                    tp.type = core::Types::dynamic();
                }
                clearKnowledge(ctx, bind.bind);
                if (auto *send = dynamic_cast<cfg::Send *>(bind.value.get())) {
                    updateKnowledge(ctx, bind.bind, send);
                } else if (auto *i = dynamic_cast<cfg::Ident *>(bind.value.get())) {
                    propagateKnowledge(ctx, bind.bind, i->what);
                }
                setTypeAndOrigin(bind.bind, tp);
            }

            return tp.type;
        } catch (...) {
            ctx.state.errors.error(bind.loc, core::ErrorClass::Internal, "Failed to type (backtrace is above)");
            throw;
        }
    }

    void ensureGoodCondition(core::Context ctx, core::LocalVariable cond) {}
    void ensureGoodAssignTarget(core::Context ctx, core::LocalVariable target) {}
};

KnowledgeFact KnowledgeFact::under(core::Context ctx, Environment env, core::Loc loc, cfg::CFG &cfg,
                                   cfg::BasicBlock &bb) {
    KnowledgeFact copy = *this;
    int i = 0;
    bool enteringLoop = (bb.flags & cfg::CFG::LOOP_HEADER) != 0;
    for (auto local : env.vars) {
        if (enteringLoop && bb.outerLoops <= cfg.maxLoopWrite[local]) {
            i++;
            continue;
        }
        auto fnd = find_if(copy.yesTypeTests.begin(), copy.yesTypeTests.end(),
                           [&](auto const &e) -> bool { return e.first == local; });
        if (fnd == copy.yesTypeTests.end()) {
            // add info from env to knowledge
            Error::check(env.types[i].type.get() != nullptr);
            // This is intentionally disabled to dumb the inference down.
            // It was intended to handle code snippets such as
            //
            //    if (...)
            //      v = true
            //      s = 1
            //    else
            //      v = false
            //    end
            //
            //    if (v)
            //      puts(s + 1)
            //    end
            //
            //    but we've decided that it's "too smart".
            //    keeping it here so that we can re-enable it if needed.
            //    Enabling this also incures a big perf slowdown. But we might be able to engineer it away if need to.
            //
            // copy.yesTypeTests.emplace_back(local, env.types[i].type);
        } else {
            auto &second = fnd->second;
            auto &typeAndOrigin = env.types[i];
            bool saneCondition = core::Types::isSubType(ctx, typeAndOrigin.type, second);
            if (!saneCondition) {
                ctx.state.errors.error(core::Reporter::ComplexError(
                    loc, core::ErrorClass::DeadBranchInferencer,
                    "This branch is unreachable. It has conflicting type requirements. This has to be a " +
                        second->toString(ctx),
                    {core::Reporter::ErrorSection("Got " + typeAndOrigin.type->toString(ctx) + " originating from:",
                                                  typeAndOrigin.origins2Explanations(ctx))}));
                fnd->second = core::Types::bottom();
            }
        }
        i++;
    }
    return copy;
}

void ruby_typer::infer::Inference::run(core::Context ctx, unique_ptr<cfg::CFG> &cfg) {
    vector<Environment> outEnvironments;
    outEnvironments.resize(cfg->maxBasicBlockId);
    for (int i = 0; i < cfg->basicBlocks.size(); i++) {
        outEnvironments[cfg->backwardsTopoSort[i]->id].bb = cfg->backwardsTopoSort[i];
    }
    vector<bool> visited;
    visited.resize(cfg->maxBasicBlockId);
    for (cfg::BasicBlock *bb : cfg->backwardsTopoSort) {
        if (bb == cfg->deadBlock()) {
            continue;
        }
        Environment &current = outEnvironments[bb->id];
        current.vars.reserve(bb->args.size());
        current.types.resize(bb->args.size());
        current.knowledge.resize(bb->args.size());
        for (core::LocalVariable arg : bb->args) {
            current.vars.push_back(arg);
        }
        if (bb->backEdges.size() == 1) {
            auto *parent = bb->backEdges[0];
            bool isTrueBranch = parent->bexit.thenb == bb;
            auto envAsSeenFromBranch = outEnvironments[parent->id].withCond(ctx, isTrueBranch, current.vars); // copy
            current.populateFrom(ctx, envAsSeenFromBranch);
        } else {
            current.isDead = (bb != cfg->entry());
            for (cfg::BasicBlock *parent : bb->backEdges) {
                if (!visited[parent->id]) {
                    continue;
                }
                bool isTrueBranch = parent->bexit.thenb == bb;
                auto envAsSeenFromBranch =
                    outEnvironments[parent->id].withCond(ctx, isTrueBranch, current.vars); // copy
                if (!envAsSeenFromBranch.isDead) {
                    current.isDead = false;
                    current.mergeWith(ctx, envAsSeenFromBranch, parent->bexit.loc, *cfg.get(), *bb);
                }
            }
        }
        int i = 0;
        for (auto &uninitialized : current.types) {
            if (uninitialized.type.get() == nullptr) {
                uninitialized.type = core::Types::nil();
                uninitialized.origins.push_back(ctx.owner.info(ctx).definitionLoc);
            } else {
                uninitialized.type->sanityCheck(ctx);
            }
            i++;
        }
        //        printf("Entry into basic block #%i with body \n%s\n, context:\n%s", bb->id, bb->toString(ctx).c_str(),
        //               current.toString(ctx).c_str());

        visited[bb->id] = true;
        if (current.isDead) {
            // this block is unreachable.
            ctx.state.errors.error(bb->loc(), core::ErrorClass::DeadBranchInferencer, "This code is unreachable");
            continue;
        }

        for (cfg::Binding &bind : bb->exprs) {
            current.ensureGoodAssignTarget(ctx, bind.bind);
            bind.tpe = current.processBinding(ctx, bind, bb->outerLoops, cfg->minLoops[bind.bind]);
            bind.tpe->sanityCheck(ctx);
        }
        current.ensureGoodCondition(ctx, bb->bexit.cond);
    }
}
