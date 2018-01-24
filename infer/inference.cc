#include "inference.h"
#include "cfg/CFG.h"
#include "core/Context.h"
#include "core/Names/infer.h"
#include "core/Symbols.h"
#include "core/errors/infer.h"
#include "core/errors/internal.h"
#include <algorithm> // find, remove_if

using namespace std;
using namespace ruby_typer;
using namespace infer;
template struct std::pair<core::LocalVariable, shared_ptr<core::Type>>;

class Environment;

// storing all the knowledge is slow
// it only makes sense for us to store it if we are going to use it
// wallk all the instructions and collect knowledge that we may ever need
class KnowledgeFilter {
    unordered_set<core::LocalVariable> used_vars;
    KnowledgeFilter() = default;

public:
    KnowledgeFilter(KnowledgeFilter &) = delete;
    KnowledgeFilter(KnowledgeFilter &&) = delete;

    bool isNeeded(core::LocalVariable var) {
        return used_vars.find(var) != used_vars.end();
    }
    static unique_ptr<KnowledgeFilter> buildFor(core::Context ctx, unique_ptr<cfg::CFG> &cfg) {
        unique_ptr<KnowledgeFilter> result(new KnowledgeFilter());
        for (auto &bb : cfg->basicBlocks) {
            if (bb->bexit.cond.name != core::NameRef::noName() && bb->bexit.cond.name != core::Names::blockCall()) {
                result->used_vars.insert(bb->bexit.cond);
            }
            for (auto &bind : bb->exprs) {
                if (cfg::Send *send = dynamic_cast<cfg::Send *>(bind.value.get())) {
                    if (send->fun == core::Names::hardAssert()) {
                        if (send->args.size() >= 1) {
                            result->used_vars.insert(send->args[0]);
                        }
                    }
                }
            }
        }
        bool changed = true;
        while (changed) {
            changed = false;
            for (auto it = cfg->backwardsTopoSort.rbegin(); it != cfg->backwardsTopoSort.rend(); ++it) {
                auto &bb = *it;
                for (auto &bind : bb->exprs) {
                    if (cfg::Ident *id = dynamic_cast<cfg::Ident *>(bind.value.get())) {
                        if (result->isNeeded(bind.bind) && !result->isNeeded(id->what)) {
                            result->used_vars.insert(id->what);
                            changed = true;
                        }
                    } else if (cfg::Send *send = dynamic_cast<cfg::Send *>(bind.value.get())) {
                        if (send->fun == core::Names::bang()) {
                            if (send->args.size() == 0) {
                                if (result->isNeeded(bind.bind) && !result->isNeeded(send->recv)) {
                                    result->used_vars.insert(send->recv);
                                    changed = true;
                                }
                            }
                        } else if (send->fun == core::Names::eqeq()) {
                            if (send->args.size() == 1) {
                                if (result->isNeeded(send->args[0]) && !result->isNeeded(send->recv)) {
                                    result->used_vars.insert(send->recv);
                                    changed = true;
                                } else if (result->isNeeded(send->recv) && !result->isNeeded(send->args[0])) {
                                    result->used_vars.insert(send->args[0]);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        return result;
    }
};

/**
 * Encode things that we know hold and don't hold
 */
struct KnowledgeFact {
    bool isDead = false;
    /* the following type tests are known to be true */
    InlinedVector<std::pair<core::LocalVariable, shared_ptr<core::Type>>, 1> yesTypeTests;
    /* the following type tests are known to be false */
    InlinedVector<std::pair<core::LocalVariable, shared_ptr<core::Type>>, 1> noTypeTests;

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

    /** Computes all possible implications of this knowledge holding as an exit from environment env in block bb
     */
    KnowledgeFact under(core::Context ctx, Environment env, core::Loc loc, cfg::CFG &inWhat, cfg::BasicBlock *bb,
                        bool isNeeded);

    void sanityCheck() {
        if (!debug_mode) {
            return;
        }
        for (auto &a : yesTypeTests) {
            ENFORCE(a.second.get() != nullptr);
        }
        for (auto &a : noTypeTests) {
            ENFORCE(a.second.get() != nullptr);
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

std::shared_ptr<core::Type> allocateBySymbol(core::Context ctx, core::SymbolRef symbol) {
    ENFORCE(symbol.info(ctx).isClass());
    if (symbol.info(ctx).typeParams.size() == 0) {
        return make_shared<core::ClassType>(symbol);
    } else {
        vector<std::shared_ptr<core::Type>> targs;
        targs.reserve(symbol.info(ctx).typeParams.size());
        for (auto UNUSED(x) : symbol.info(ctx).typeParams) {
            targs.emplace_back(core::Types::dynamic());
        }
        return make_shared<core::AppliedType>(symbol, move(targs));
    }
}

shared_ptr<core::Type> dropLiteral(shared_ptr<core::Type> tp) {
    if (auto *a = core::cast_type<core::LiteralType>(tp.get())) {
        return a->underlying;
    }
    return tp;
}

shared_ptr<core::Type> runTypeConstructor(core::Context ctx, core::TypeConstructor *typeConstructor) {
    ENFORCE(typeConstructor->protoType.info(ctx).isClass());
    if (typeConstructor->protoType == core::GlobalState::defn_T_all()) {
        int i = 1;
        shared_ptr<core::Type> result = typeConstructor->targs[0];
        while (i < typeConstructor->targs.size()) {
            result = core::Types::buildAnd(ctx, result, typeConstructor->targs[i]);
            i++;
        }
        return result;
    }
    if (typeConstructor->protoType == core::GlobalState::defn_T_any()) {
        int i = 1;
        shared_ptr<core::Type> result = typeConstructor->targs[0];
        while (i < typeConstructor->targs.size()) {
            result = core::Types::buildOr(ctx, result, typeConstructor->targs[i]);
            i++;
        }
        return result;
    }
    if (typeConstructor->protoType == core::GlobalState::defn_T_Array()) {
        return make_shared<core::AppliedType>(core::GlobalState::defn_Array(), typeConstructor->targs);
    }

    if (typeConstructor->protoType == core::GlobalState::defn_T_Hash()) {
        return make_shared<core::AppliedType>(core::GlobalState::defn_Hash(), typeConstructor->targs);
    }
    return make_shared<core::AppliedType>(typeConstructor->protoType, typeConstructor->targs);
}

bool isTypeConstructor(core::Context ctx, std::shared_ptr<core::Type> recv) {
    if (dynamic_cast<core::TypeConstructor *>(recv.get()) != nullptr) {
        return true;
    }
    auto asClass = dynamic_cast<core::ClassType *>(recv.get());
    if (asClass == nullptr) {
        return false;
    }
    core::SymbolRef klass = asClass->symbol;
    // Consider using a flag for this?
    auto attached = klass.info(ctx).attachedClass(ctx);
    return attached.exists() && !attached.info(ctx).typeParams.empty() &&
           !klass.info(ctx).findMemberTransitive(ctx, core::Names::squareBrackets()).exists();
}

shared_ptr<core::Type> unwrapConstructor(core::Context ctx, core::Loc loc, shared_ptr<core::Type> &tp) {
    if (auto *tc = dynamic_cast<core::TypeConstructor *>(tp.get())) {
        if (!tc->targs.empty()) {
            return runTypeConstructor(ctx, tc);
        } else {
            if (tc->protoType != core::GlobalState::defn_untyped()) {
                ctx.state.error(loc, core::errors::Infer::BareTypeUsage, "Unsupported usage of bare type");
            }
            return core::Types::dynamic();
        }
    }
    if (core::ClassType *classType = dynamic_cast<core::ClassType *>(tp.get())) {
        core::SymbolRef attachedClass = classType->symbol.info(ctx).attachedClass(ctx);
        if (!attachedClass.exists()) {
            ctx.state.error(loc, core::errors::Infer::BareTypeUsage, "Unsupported usage of bare type");
            return core::Types::dynamic();
        }

        return make_shared<core::ClassType>(attachedClass);
    }
    return tp;
}

shared_ptr<core::Type> dropConstructor(core::Context ctx, core::Loc loc, shared_ptr<core::Type> &tp) {
    if (auto *tc = dynamic_cast<core::TypeConstructor *>(tp.get())) {
        if (tc->protoType != core::GlobalState::defn_untyped()) {
            ctx.state.error(loc, core::errors::Infer::BareTypeUsage, "Unsupported usage of bare type");
        }
        return core::Types::dynamic();
    }
    return tp;
}

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
        ENFORCE(TestedKnowledge::empty.falsy.yesTypeTests.empty());
        ENFORCE(TestedKnowledge::empty.falsy.noTypeTests.empty());
        ENFORCE(TestedKnowledge::empty.truthy.noTypeTests.empty());
        ENFORCE(TestedKnowledge::empty.truthy.yesTypeTests.empty());
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
        if (isDead) {
            buf << "dead=" << isDead << endl;
        }
        int i = -1;
        for (auto var : vars) {
            i++;
            auto &name = var.name.name(ctx);
            if (name.kind == core::NameKind::UNIQUE && name.unique.original == core::Names::debugEnvironmentTemp()) {
                continue;
            }
            buf << var.name.toString(ctx) << ": " << types[i].type->toString(ctx, 0) << endl;
            buf << knowledge[i].toString(ctx) << endl;
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
        ENFORCE(types[fnd - vars.begin()].type.get() != nullptr);
        return types[fnd - vars.begin()];
    }

    TestedKnowledge &getKnowledge(core::LocalVariable symbol, bool shouldFail = true) {
        auto fnd = find(vars.begin(), vars.end(), symbol);
        if (fnd == vars.end()) {
            ENFORCE(!shouldFail, "Missing knowledge?");
            return TestedKnowledge::empty;
        }
        auto &r = knowledge[fnd - vars.begin()];
        r.sanityCheck();
        return r;
    }

    /* propagate knowledge on `to = from` */
    void propagateKnowledge(core::Context ctx, core::LocalVariable to, core::LocalVariable from,
                            unique_ptr<KnowledgeFilter> &knowledgeFilter) {
        if (knowledgeFilter->isNeeded(to) && knowledgeFilter->isNeeded(from)) {
            getKnowledge(to) = getKnowledge(from);
            getKnowledge(to).truthy.noTypeTests.emplace_back(from, core::Types::falsyTypes());
            getKnowledge(to).falsy.yesTypeTests.emplace_back(from, core::Types::falsyTypes());
            getKnowledge(from).truthy.noTypeTests.emplace_back(to, core::Types::falsyTypes());
            getKnowledge(from).falsy.yesTypeTests.emplace_back(to, core::Types::falsyTypes());
            getKnowledge(from).sanityCheck();
            getKnowledge(to).sanityCheck();
        }
    }

    /* variable was reasigned. Forget everything about previous value */
    void clearKnowledge(core::Context ctx, core::LocalVariable reassigned,
                        unique_ptr<KnowledgeFilter> &knowledgeFilter) {
        int i = -1;
        for (auto &k : knowledge) {
            i++;
            if (knowledgeFilter->isNeeded(this->vars[i])) {
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
                k.falsy.noTypeTests.erase(
                    remove_if(k.falsy.noTypeTests.begin(), k.falsy.noTypeTests.end(),
                              [&](auto const &c) -> bool { return c.first.name == reassigned.name; }),
                    k.falsy.noTypeTests.end());
                k.sanityCheck();
            }
        }
    }

    /* Special case sources of knowledge */
    void updateKnowledge(core::Context ctx, core::LocalVariable local, core::Loc loc, cfg::Send *send,
                         unique_ptr<KnowledgeFilter> &knowledgeFilter) {
        if (send->fun == core::Names::bang()) {
            if (!knowledgeFilter->isNeeded(local))
                return;
            auto &whoKnows = getKnowledge(local);
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
            if (!knowledgeFilter->isNeeded(local))
                return;
            auto &whoKnows = getKnowledge(local);
            whoKnows.truthy.yesTypeTests.emplace_back(send->recv, core::Types::nil());
            whoKnows.falsy.noTypeTests.emplace_back(send->recv, core::Types::nil());
            whoKnows.sanityCheck();
        }

        if (send->args.empty()) {
            return;
        }
        if (send->fun == core::Names::kind_of() || send->fun == core::Names::is_a_p()) {
            if (!knowledgeFilter->isNeeded(local))
                return;
            auto &whoKnows = getKnowledge(local);
            core::TypeAndOrigins klass = getTypeAndOrigin(ctx, send->args[0]);
            if (klass.type->derivesFrom(ctx, core::GlobalState::defn_Class())) {
                auto *s = core::cast_type<core::ClassType>(klass.type.get());
                if (s == nullptr) {
                    return;
                }
                core::SymbolRef attachedClass = s->symbol.info(ctx).attachedClass(ctx);
                if (attachedClass.exists()) {
                    whoKnows.truthy.yesTypeTests.emplace_back(send->recv, allocateBySymbol(ctx, attachedClass));
                    whoKnows.falsy.noTypeTests.emplace_back(send->recv, allocateBySymbol(ctx, attachedClass));
                }
                whoKnows.sanityCheck();
            }
        } else if (send->fun == core::Names::eqeq()) {
            if (!knowledgeFilter->isNeeded(local))
                return;
            auto &whoKnows = getKnowledge(local);
            core::TypeAndOrigins tp1 = getTypeAndOrigin(ctx, send->args[0]);
            core::TypeAndOrigins tp2 = getTypeAndOrigin(ctx, send->recv);
            if (tp1.type->isDynamic() && tp2.type->isDynamic()) {
                return;
            }

            ENFORCE(tp1.type.get() != nullptr);
            ENFORCE(tp2.type.get() != nullptr);
            whoKnows.truthy.yesTypeTests.emplace_back(send->recv, tp1.type);
            whoKnows.truthy.yesTypeTests.emplace_back(send->args[0], tp2.type);
            whoKnows.sanityCheck();
        } else if (send->fun == core::Names::tripleEq()) {
            if (!knowledgeFilter->isNeeded(local))
                return;
            auto &whoKnows = getKnowledge(local);
            core::TypeAndOrigins recvKlass = getTypeAndOrigin(ctx, send->recv);
            if (!recvKlass.type->derivesFrom(ctx, core::GlobalState::defn_Class())) {
                return;
            }

            auto *s = core::cast_type<core::ClassType>(recvKlass.type.get());
            if (s == nullptr) {
                return;
            }

            core::SymbolRef attachedClass = s->symbol.info(ctx).attachedClass(ctx);
            if (attachedClass.exists()) {
                whoKnows.truthy.yesTypeTests.emplace_back(send->args[0], allocateBySymbol(ctx, attachedClass));
                whoKnows.falsy.noTypeTests.emplace_back(send->args[0], allocateBySymbol(ctx, attachedClass));
            }
            whoKnows.sanityCheck();

        } else if (send->fun == core::Names::hardAssert()) {
            core::TypeAndOrigins recvKlass = getTypeAndOrigin(ctx, send->recv);
            if (!recvKlass.type->derivesFrom(ctx, core::GlobalState::defn_Kernel())) {
                return;
            }

            assumeKnowledge(ctx, true, send->args[0], loc, vars);
            if (this->isDead) {
                ctx.state.error(loc, core::errors::Infer::DeadBranchInferencer, "Hard assert is always false");
            }
        }
    }

    void setTypeAndOrigin(core::LocalVariable symbol, core::TypeAndOrigins typeAndOrigins) {
        ENFORCE(typeAndOrigins.type.get() != nullptr);

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
        Environment copy = *this;
        copy.assumeKnowledge(ctx, isTrue, bb->bexit.cond, bb->bexit.loc, filter);
        return copy;
    }

    void assumeKnowledge(core::Context ctx, bool isTrue, core::LocalVariable cond, core::Loc loc,
                         const vector<core::LocalVariable> &filter) {
        auto fnd = find(vars.begin(), vars.end(), cond);
        if (fnd == vars.end()) {
            // Can't use getKnowledge because of loops
            return;
        }
        auto &thisKnowledge = this->knowledge[fnd - vars.begin()];
        thisKnowledge.sanityCheck();
        if (!isTrue) {
            core::TypeAndOrigins tp = getTypeAndOrigin(ctx, cond);
            tp.origins.emplace_back(loc);
            if (tp.type->isDynamic()) {
                tp.type = core::Types::falsyTypes();
            } else {
                tp.type = core::Types::glb(ctx, tp.type, core::Types::falsyTypes());
                if (tp.type->isBottom()) {
                    isDead = true;
                    return;
                }
            }
            setTypeAndOrigin(cond, tp);
        } else {
            core::TypeAndOrigins tp = getTypeAndOrigin(ctx, cond);
            tp.origins.emplace_back(loc);
            tp.type = core::Types::dropSubtypesOf(
                ctx, core::Types::dropSubtypesOf(ctx, tp.type, core::GlobalState::defn_NilClass()),
                core::GlobalState::defn_FalseClass());
            setTypeAndOrigin(cond, tp);
        }

        auto &knowledgeToChoose = isTrue ? thisKnowledge.truthy : thisKnowledge.falsy;

        if (isDead) {
            return;
        }
        for (auto &typeTested : knowledgeToChoose.yesTypeTests) {
            if (find(filter.begin(), filter.end(), typeTested.first) == filter.end()) {
                continue;
            }
            core::TypeAndOrigins tp = getTypeAndOrigin(ctx, typeTested.first);
            tp.origins.emplace_back(loc);
            if (tp.type->isDynamic()) { // this is actually incorrect, as it may be some more exact type, but this rule
                // makes it easier to migrate code
                tp.type = typeTested.second;
            } else {
                tp.type = core::Types::glb(ctx, tp.type, typeTested.second);
                if (tp.type->isBottom()) {
                    isDead = true;
                    setTypeAndOrigin(typeTested.first, tp);
                    return;
                }
            }
            setTypeAndOrigin(typeTested.first, tp);
        }

        for (auto &typeTested : knowledgeToChoose.noTypeTests) {
            if (find(filter.begin(), filter.end(), typeTested.first) == filter.end()) {
                continue;
            }
            core::TypeAndOrigins tp = getTypeAndOrigin(ctx, typeTested.first);
            tp.origins.emplace_back(loc);
            if (!tp.type->isDynamic()) {
                tp.type = core::Types::approximateSubtract(ctx, tp.type, typeTested.second);
                setTypeAndOrigin(typeTested.first, tp);
                if (tp.type->isBottom()) {
                    isDead = true;
                    return;
                }
            }
        }
    }

    void mergeWith(core::Context ctx, Environment &other, core::Loc loc, cfg::CFG &inWhat, cfg::BasicBlock *bb,
                   unique_ptr<KnowledgeFilter> &knowledgeFilter) {
        int i = -1;
        this->isDead |= other.isDead;
        for (core::LocalVariable var : vars) {
            i++;
            auto otherTO = other.getTypeAndOrigin(ctx, var);
            auto otherType = dropConstructor(ctx, loc, otherTO.type);
            auto &thisTO = types[i];
            if (thisTO.type.get() != nullptr) {
                thisTO.type = core::Types::lub(ctx, thisTO.type, otherType);
                thisTO.type->sanityCheck(ctx);
            } else {
                types[i].type = otherType;
            }
            for (auto origin : otherTO.origins) {
                if (find(thisTO.origins.begin(), thisTO.origins.end(), origin) == thisTO.origins.end()) {
                    thisTO.origins.push_back(origin);
                }
            }

            if (((bb->flags & cfg::CFG::LOOP_HEADER) != 0) && bb->outerLoops <= inWhat.maxLoopWrite[var]) {
                continue;
            }
            bool canBeFalsy = core::Types::canBeFalsy(ctx, otherType);
            bool canBeTruthy = core::Types::canBeTruthy(ctx, otherType);

            if (canBeTruthy) {
                auto &thisKnowledge = getKnowledge(var);
                auto otherTruthy = other.getKnowledge(var, false)
                                       .truthy.under(ctx, other, loc, inWhat, bb, knowledgeFilter->isNeeded(var));
                if (!otherTruthy.isDead) {
                    if (!thisKnowledge.seenTruthyOption) {
                        thisKnowledge.seenTruthyOption = true;
                        thisKnowledge.truthy = otherTruthy;
                    } else {
                        thisKnowledge.truthy.min(ctx, otherTruthy);
                    }
                }
            }

            if (canBeFalsy) {
                auto &thisKnowledge = getKnowledge(var);
                auto otherFalsy = other.getKnowledge(var, false)
                                      .falsy.under(ctx, other, loc, inWhat, bb, knowledgeFilter->isNeeded(var));
                if (!otherFalsy.isDead) {
                    if (!thisKnowledge.seenFalsyOption) {
                        thisKnowledge.seenFalsyOption = true;
                        thisKnowledge.falsy = otherFalsy;
                    } else {
                        thisKnowledge.falsy.min(ctx, otherFalsy);
                    }
                }
            }
        }
    }

    void populateFrom(core::Context ctx, Environment &other) {
        int i = -1;
        this->isDead = other.isDead;
        for (core::LocalVariable var : vars) {
            i++;
            auto otherTO = other.getTypeAndOrigin(ctx, var);
            types[i].type = dropConstructor(ctx, otherTO.origins[0], otherTO.type);
            types[i].origins = otherTO.origins;
            auto &thisKnowledge = getKnowledge(var);
            auto &otherKnowledge = other.getKnowledge(var, false);
            thisKnowledge = otherKnowledge;
        }
    }
    shared_ptr<core::Type> dispatchNew(core::Context ctx, core::TypeAndOrigins recvType, cfg::Send *send,
                                       vector<core::TypeAndOrigins> &args, cfg::Binding &bind) {
        core::ClassType *classType = core::cast_type<core::ClassType>(recvType.type.get());
        if (classType == nullptr) {
            core::TypeConstructor *typeConstructor = dynamic_cast<core::TypeConstructor *>(recvType.type.get());
            if (typeConstructor != nullptr) {
                ENFORCE(!typeConstructor->targs.empty());
                return runTypeConstructor(ctx, typeConstructor);
            }
            return recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type, send->hasBlock);
        }

        if (classType->symbol == core::GlobalState::defn_untyped()) {
            return recvType.type;
        }

        core::SymbolRef newSymbol = classType->symbol.info(ctx).findMemberTransitive(ctx, core::Names::new_());
        if (newSymbol.exists() && newSymbol.info(ctx).owner != core::GlobalState::defn_BasicObject()) {
            // custom `new` was defined
            return recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type, send->hasBlock);
        }

        core::SymbolRef attachedClass = classType->symbol.info(ctx).attachedClass(ctx);
        if (!attachedClass.exists()) {
            // `foo`.new() but `foo` isn't a Class
            return recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type, send->hasBlock);
        }

        auto type = allocateBySymbol(ctx, attachedClass);

        // call constructor
        newSymbol = attachedClass.info(ctx).findMemberTransitive(ctx, core::Names::initialize());
        if (newSymbol.exists()) {
            auto initializeResult =
                type->dispatchCall(ctx, core::Names::initialize(), bind.loc, args, recvType.type, send->hasBlock);
        } else {
            if (!args.empty()) {
                ctx.state.error(bind.loc, core::errors::Infer::MethodArgumentCountMismatch,
                                "Wrong number of arguments for constructor.\n Expected: 0, found: {}", args.size());
            }
        }
        return type;
    }

    shared_ptr<core::Type> dropLiteral(shared_ptr<core::Type> tp) {
        if (auto *a = core::cast_type<core::LiteralType>(tp.get())) {
            return a->underlying;
        }
        return tp;
    }

    shared_ptr<core::Type> processBinding(core::Context ctx, cfg::Binding &bind, int loopCount, int bindMinLoops,
                                          unique_ptr<KnowledgeFilter> &knowledgeFilter) {
        try {
            core::TypeAndOrigins tp;
            bool noLoopChecking = cfg::isa_instruction<cfg::Alias>(bind.value.get()) ||
                                  cfg::isa_instruction<cfg::LoadArg>(bind.value.get());
            typecase(
                bind.value.get(),
                [&](cfg::Send *send) {
                    vector<core::TypeAndOrigins> args;

                    args.reserve(send->args.size());
                    for (core::LocalVariable arg : send->args) {
                        args.emplace_back(getTypeAndOrigin(ctx, arg));
                    }

                    auto recvType = getTypeAndOrigin(ctx, send->recv);

                    switch (send->fun.id()) {
                        case core::Names::untyped()._id: {
                            if (!recvType.type->derivesFrom(ctx, core::GlobalState::defn_T()) ||
                                recvType.type->isDynamic()) {
                                break;
                            }
                            vector<shared_ptr<core::Type>> empty;
                            tp.origins.push_back(bind.loc);
                            tp.type = make_shared<core::TypeConstructor>(core::GlobalState::defn_untyped(), empty);
                            return;
                        }
                        case core::Names::any()._id:
                        case core::Names::all()._id: {
                            if (!recvType.type->derivesFrom(ctx, core::GlobalState::defn_T()) ||
                                recvType.type->isDynamic()) {
                                break;
                            }
                            tp.origins.push_back(bind.loc);
                            ENFORCE(!send->args.empty());
                            vector<shared_ptr<core::Type>> targs;
                            for (auto &arg : send->args) {
                                auto tp = getTypeAndOrigin(ctx, arg);
                                targs.push_back(unwrapConstructor(ctx, tp.origins[0], tp.type));
                            }
                            tp.origins.push_back(bind.loc);

                            tp.type = make_shared<core::TypeConstructor>(
                                core::GlobalState::defn_T().info(ctx).findMember(ctx, send->fun), targs);
                            return;
                        }
                        default:
                            break;
                    }
                    if (send->fun == core::Names::new_()) {
                        tp.type = dispatchNew(ctx, recvType, send, args, bind);
                    } else if (send->fun == core::Names::squareBrackets() && isTypeConstructor(ctx, recvType.type)) {
                        core::SymbolRef attached;
                        auto *asClass = dynamic_cast<core::ClassType *>(recvType.type.get());
                        if (asClass != nullptr) {
                            core::SymbolRef klass = asClass->symbol;
                            // Consider using a flag for this?
                            attached = klass.info(ctx).attachedClass(ctx);
                        } else {
                            auto *tc = dynamic_cast<core::TypeConstructor *>(recvType.type.get());
                            ENFORCE(tc != nullptr);
                            ENFORCE(tc->targs.empty());
                            attached = tc->protoType;
                        }
                        vector<shared_ptr<core::Type>> targs;
                        // TODO match arguments
                        for (auto arg : args) {
                            targs.emplace_back(unwrapConstructor(ctx, arg.origins[0], arg.type));
                        }
                        tp.type = make_shared<core::TypeConstructor>(attached, targs);
                    } else if (send->fun == core::Names::super()) {
                        // TODO
                        tp.type = core::Types::dynamic();
                    } else {
                        tp.type =
                            recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type, send->hasBlock);
                    }
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::Ident *i) {
                    auto typeAndOrigin = getTypeAndOrigin(ctx, i->what, true);
                    tp.type = typeAndOrigin.type;
                    tp.origins = typeAndOrigin.origins;
                    ENFORCE(!tp.origins.empty(), "Inferencer did not assign location");
                },
                [&](cfg::Alias *a) {
                    core::SymbolRef symbol = a->what;
                    core::Symbol &info = symbol.info(ctx);
                    if (a->what == core::GlobalState::defn_T_Array()) {
                        vector<shared_ptr<core::Type>> empty;
                        tp.type = make_shared<core::TypeConstructor>(core::GlobalState::defn_Array(), empty);
                        tp.origins.push_back(symbol.info(ctx).definitionLoc);

                    } else if (a->what == core::GlobalState::defn_T_Hash()) {
                        vector<shared_ptr<core::Type>> empty;
                        tp.type = make_shared<core::TypeConstructor>(core::GlobalState::defn_Hash(), empty);
                        tp.origins.push_back(symbol.info(ctx).definitionLoc);
                    } else if (info.isClass()) {
                        ENFORCE(info.resultType.get(), "Type should have been filled in by the namer");
                        tp.type = info.resultType;
                        tp.origins.push_back(symbol.info(ctx).definitionLoc);
                    } else if (info.isField() || info.isStaticField() || info.isMethodArgument()) {
                        if (info.resultType.get() != nullptr) {
                            tp.type = info.resultType;
                            tp.origins.push_back(info.definitionLoc);
                        } else {
                            tp.origins.push_back(core::Loc::none());
                            tp.type = core::Types::dynamic();
                        }
                    } else {
                        Error::notImplemented();
                    }
                },
                [&](cfg::Self *i) {
                    tp.type = make_shared<core::ClassType>(i->klass);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::SymbolLit *i) {
                    tp.type = make_shared<core::LiteralType>(ctx.state.defn_Symbol(), i->value);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::StringLit *i) {
                    tp.type = make_shared<core::LiteralType>(ctx.state.defn_String(), i->value);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::LoadArg *i) {
                    /* read type from info filled by define_method */
                    tp.type = getTypeAndOrigin(ctx, i->receiver).type->getCallArgumentType(ctx, i->method, i->arg);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::Return *i) {
                    auto expectedType = ctx.owner.info(ctx).resultType;
                    if (!expectedType) {
                        expectedType = core::Types::dynamic();
                    }
                    auto typeAndOrigin = getTypeAndOrigin(ctx, i->what);
                    if (!core::Types::isSubType(ctx, typeAndOrigin.type, expectedType)) {
                        ctx.state.error(core::ComplexError(
                            bind.loc, core::errors::Infer::ReturnTypeMismatch,
                            "Returning value that does not conform to method result type",
                            {core::ErrorSection("Expected " + expectedType->toString(ctx),
                                                {
                                                    core::ErrorLine::from(ctx.owner.info(ctx).definitionLoc,
                                                                          "Method `{}` has return type `{}`",
                                                                          ctx.owner.info(ctx).name.toString(ctx),
                                                                          expectedType->toString(ctx)),
                                                }),
                             core::ErrorSection("Got " + typeAndOrigin.type->toString(ctx) + " originating from:",
                                                typeAndOrigin.origins2Explanations(ctx))}));
                    }
                    tp.type = core::Types::bottom();
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::IntLit *i) {
                    tp.type = make_shared<core::LiteralType>(i->value);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::BoolLit *i) {
                    tp.type = make_shared<core::LiteralType>(i->value);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::FloatLit *i) {
                    tp.type = make_shared<core::LiteralType>(i->value);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::Unanalyzable *i) {
                    tp.type = core::Types::dynamic();
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::Cast *c) {
                    tp.type = c->type;
                    tp.origins.push_back(bind.loc);

                    if (c->assertType) {
                        auto ty = getTypeAndOrigin(ctx, c->value);
                        if (ty.type->isDynamic()) {
                            ctx.state.error(core::ComplexError(
                                bind.loc, core::errors::Infer::CastTypeMismatch,
                                "The typechecker was unable to infer the type of the argument to "
                                "assert_type!.",
                                {core::ErrorSection("Value originated from:", ty.origins2Explanations(ctx)),
                                 core::ErrorSection("You may need to add additional `sig` "
                                                    "annotations.")}));
                        } else if (!core::Types::isSubType(ctx, ty.type, c->type)) {
                            ctx.state.error(core::ComplexError(
                                bind.loc, core::errors::Infer::CastTypeMismatch,
                                "assert_type!: argument does not have asserted type",
                                {core::ErrorSection("Expected " + c->type->toString(ctx), {}),
                                 core::ErrorSection("Got " + ty.type->toString(ctx) + " originating from:",
                                                    ty.origins2Explanations(ctx))}));
                        }
                    }
                },
                [&](cfg::DebugEnvironment *d) {
                    d->str = toString(ctx);
                    tp.type = core::Types::bottom();
                    tp.origins.push_back(bind.loc);
                });

            ENFORCE(tp.type.get() != nullptr, "Inferencer did not assign type");
            ENFORCE(!tp.origins.empty(), "Inferencer did not assign location");

            core::TypeAndOrigins cur = getTypeAndOrigin(ctx, bind.bind, true);

            if (noLoopChecking || loopCount == bindMinLoops) {
                clearKnowledge(ctx, bind.bind, knowledgeFilter);
                if (auto *send = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
                    updateKnowledge(ctx, bind.bind, bind.loc, send, knowledgeFilter);
                } else if (auto *i = cfg::cast_instruction<cfg::Ident>(bind.value.get())) {
                    propagateKnowledge(ctx, bind.bind, i->what, knowledgeFilter);
                }
                setTypeAndOrigin(bind.bind, tp);
            } else {
                if (!core::Types::isSubType(ctx, dropLiteral(tp.type), dropLiteral(cur.type))) {
                    ctx.state.error(bind.loc, core::errors::Infer::PinnedVariableMismatch,
                                    "Changing type of pinned argument, {} is not a subtype of {}",
                                    tp.type->toString(ctx), cur.type->toString(ctx));
                    tp.type = core::Types::dynamic();
                }
                clearKnowledge(ctx, bind.bind, knowledgeFilter);
                if (auto *send = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
                    updateKnowledge(ctx, bind.bind, bind.loc, send, knowledgeFilter);
                } else if (auto *i = cfg::cast_instruction<cfg::Ident>(bind.value.get())) {
                    propagateKnowledge(ctx, bind.bind, i->what, knowledgeFilter);
                }
                setTypeAndOrigin(bind.bind, tp);
            }

            return tp.type;
        } catch (...) {
            ctx.state.error(bind.loc, core::errors::Internal::InternalError, "Failed to type (backtrace is above)");
            throw;
        }
    }

    void ensureGoodCondition(core::Context ctx, core::LocalVariable cond) {}
    void ensureGoodAssignTarget(core::Context ctx, core::LocalVariable target) {}
};

KnowledgeFact KnowledgeFact::under(core::Context ctx, Environment env, core::Loc loc, cfg::CFG &inWhat,
                                   cfg::BasicBlock *bb, bool isNeeded) {
    KnowledgeFact copy = *this;
    if (env.isDead) {
        copy.isDead = true;
        return copy;
    }
    int i = -1;
    bool enteringLoop = (bb->flags & cfg::CFG::LOOP_HEADER) != 0;
    for (auto local : env.vars) {
        i++;
        if (enteringLoop && bb->outerLoops <= inWhat.maxLoopWrite[local]) {
            continue;
        }
        auto fnd = find_if(copy.yesTypeTests.begin(), copy.yesTypeTests.end(),
                           [&](auto const &e) -> bool { return e.first == local; });
        if (fnd == copy.yesTypeTests.end()) {
            // add info from env to knowledge
            ENFORCE(env.types[i].type.get() != nullptr);
            // This handles code snippets such as
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
            // or simply
            //
            //   a = T.cast(a, T.any(Integer, NilClass))
            //   hard_assert(a.nil? && 1==1)
            //   T.assert_type(a, Integer)
            //
            // Enabling this also incures a big perf slowdown. But we might be able to engineer it away if need to.
            //
            if (isNeeded) {
                copy.yesTypeTests.emplace_back(local, env.types[i].type);
            }
        } else {
            auto &second = fnd->second;
            auto &typeAndOrigin = env.types[i];
            auto combinedType =
                core::Types::glb(ctx, dropConstructor(ctx, typeAndOrigin.origins[0], typeAndOrigin.type), second);
            if (combinedType->isBottom()) {
                copy.isDead = true;
                break;
            }
        }
    }
    return copy;
}
bool isSyntheticReturn(core::Context ctx, cfg::Binding &bind) {
    if (bind.bind.name == core::Names::finalReturn()) {
        if (auto *ret = dynamic_cast<cfg::Return *>(bind.value.get())) {
            return ret->what.isSyntheticTemporary(ctx);
        }
    }
    return false;
}

unique_ptr<cfg::CFG> ruby_typer::infer::Inference::run(core::Context ctx, unique_ptr<cfg::CFG> cfg) {
    counterInc("infer.methods_typechecked");
    const int startErrorCount = ctx.state.totalErrors();
    vector<Environment> outEnvironments;
    outEnvironments.resize(cfg->maxBasicBlockId);
    for (int i = 0; i < cfg->basicBlocks.size(); i++) {
        outEnvironments[cfg->backwardsTopoSort[i]->id].bb = cfg->backwardsTopoSort[i];
    }
    vector<bool> visited;
    visited.resize(cfg->maxBasicBlockId);
    unique_ptr<KnowledgeFilter> knowledgeFilter = KnowledgeFilter::buildFor(ctx, cfg);
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
                    current.mergeWith(ctx, envAsSeenFromBranch, parent->bexit.loc, *cfg.get(), bb, knowledgeFilter);
                }
            }
        }
        int i = -1;
        for (auto &uninitialized : current.types) {
            i++;
            if (uninitialized.type.get() == nullptr) {
                uninitialized.type = core::Types::nil();
                uninitialized.origins.push_back(ctx.owner.info(ctx).definitionLoc);
            } else {
                uninitialized.type->sanityCheck(ctx);
            }
        }

        visited[bb->id] = true;
        if (current.isDead) {
            // this block is unreachable.
            if (!bb->exprs.empty() &&
                !(bb->exprs.size() == 1 && isSyntheticReturn(ctx, bb->exprs[0])) // synthetic final return
            ) {
                ctx.state.error(bb->exprs[0].loc, core::errors::Infer::DeadBranchInferencer,
                                "This code is unreachable");
            }
            continue;
        }

        for (cfg::Binding &bind : bb->exprs) {
            current.ensureGoodAssignTarget(ctx, bind.bind);
            bind.tpe = current.processBinding(ctx, bind, bb->outerLoops, cfg->minLoops[bind.bind], knowledgeFilter);
            bind.tpe->sanityCheck(ctx);
            if (current.isDead) {
                break;
            }
        }
        if (!current.isDead) {
            current.ensureGoodCondition(ctx, bb->bexit.cond);
        }
        histogramInc("infer.environment.size", current.vars.size());
        for (auto &k : current.knowledge) {
            histogramInc("infer.knowledge.truthy.yes.size", k.truthy.yesTypeTests.size());
            histogramInc("infer.knowledge.truthy.no.size", k.truthy.noTypeTests.size());
            histogramInc("infer.knowledge.falsy.yes.size", k.falsy.yesTypeTests.size());
            histogramInc("infer.knowledge.falsy.no.size", k.falsy.noTypeTests.size());
        }
    }
    if (startErrorCount == ctx.state.totalErrors()) {
        counterInc("infer.methods_typechecked.no_errors");
    }
    return cfg;
}
