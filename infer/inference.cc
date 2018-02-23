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

public:
    KnowledgeFilter(core::Context ctx, unique_ptr<cfg::CFG> &cfg) {
        for (auto &bb : cfg->basicBlocks) {
            if (bb->bexit.cond != core::LocalVariable::noVariable() &&
                bb->bexit.cond != core::LocalVariable::blockCall()) {
                used_vars.insert(bb->bexit.cond);
            }
            for (auto &bind : bb->exprs) {
                if (cfg::Send *send = dynamic_cast<cfg::Send *>(bind.value.get())) {
                    if (send->fun == core::Names::hardAssert()) {
                        if (!send->args.empty()) {
                            used_vars.insert(send->args[0]);
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
                        if (isNeeded(bind.bind) && !isNeeded(id->what)) {
                            used_vars.insert(id->what);
                            changed = true;
                        }
                    } else if (cfg::Send *send = dynamic_cast<cfg::Send *>(bind.value.get())) {
                        if (send->fun == core::Names::bang()) {
                            if (send->args.empty()) {
                                if (isNeeded(bind.bind) && !isNeeded(send->recv)) {
                                    used_vars.insert(send->recv);
                                    changed = true;
                                }
                            }
                        } else if (send->fun == core::Names::eqeq()) {
                            if (send->args.size() == 1) {
                                if (isNeeded(send->args[0]) && !isNeeded(send->recv)) {
                                    used_vars.insert(send->recv);
                                    changed = true;
                                } else if (isNeeded(send->recv) && !isNeeded(send->args[0])) {
                                    used_vars.insert(send->args[0]);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    KnowledgeFilter(KnowledgeFilter &) = delete;
    KnowledgeFilter(KnowledgeFilter &&) = delete;

    bool isNeeded(core::LocalVariable var) {
        return used_vars.find(var) != used_vars.end();
    }
};

class KnowledgeRef;
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
    void min(core::Context ctx, const KnowledgeFact &other) {
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
    static KnowledgeRef under(core::Context ctx, const KnowledgeRef &what, const Environment &env, core::Loc loc,
                              cfg::CFG &inWhat, cfg::BasicBlock *bb, bool isNeeded);

    void sanityCheck() const {
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

    string toString(core::Context ctx) const {
        stringstream buf;

        for (auto &el : yesTypeTests) {
            buf << "    " << el.first.toString(ctx) << " to be " << el.second->toString(ctx, 0) << endl;
        }
        for (auto &el : noTypeTests) {
            buf << "    " << el.first.toString(ctx) << " NOT to be " << el.second->toString(ctx, 0) << endl;
        }
        return buf.str();
    }
};

shared_ptr<core::Type> dropLiteral(shared_ptr<core::Type> tp) {
    if (auto *a = core::cast_type<core::LiteralType>(tp.get())) {
        return a->underlying;
    }
    return tp;
}

shared_ptr<core::Type> dropConstructor(core::Context ctx, core::Loc loc, shared_ptr<core::Type> tp) {
    if (auto *mt = core::cast_type<core::MetaType>(tp.get())) {
        if (!mt->wrapped->isDynamic()) {
            ctx.state.error(loc, core::errors::Infer::BareTypeUsage, "Unsupported usage of bare type");
        }
        return core::Types::dynamic();
    }
    return tp;
}

// KnowledgeRef wraps a `KnowledgeFact` with copy-on-write semantics
class KnowledgeRef {
public:
    KnowledgeRef() : knowledge(make_shared<KnowledgeFact>()) {}
    KnowledgeRef(const KnowledgeRef &) = default;
    KnowledgeRef &operator=(const KnowledgeRef &) = default;
    KnowledgeRef(KnowledgeRef &&) = default;
    KnowledgeRef &operator=(KnowledgeRef &&) = default;

    const KnowledgeFact &operator*() const {
        return *knowledge.get();
    }
    const KnowledgeFact *operator->() const {
        return knowledge.get();
    }

    KnowledgeFact &mutate() {
        if (knowledge.use_count() > 1) {
            knowledge = make_shared<KnowledgeFact>(*knowledge);
        }
        ENFORCE(knowledge.use_count() == 1);
        return *knowledge.get();
    }

private:
    shared_ptr<KnowledgeFact> knowledge;
};

/** Almost a named pair of two KnowledgeFact-s. One holds knowledge that is true when a variable is falsy,
 * the other holds knowledge which is true if the same variable is falsy->
 */
class TestedKnowledge {
public:
    KnowledgeRef truthy, falsy;
    bool seenTruthyOption; // Only used during environment merge. Used to indicate "all-knowing" truthy option.
    bool seenFalsyOption;  // Same for falsy

    string toString(core::Context ctx) {
        stringstream buf;
        if (!truthy->noTypeTests.empty() || !truthy->yesTypeTests.empty()) {
            buf << "  Being truthy entails:" << endl;
        }
        buf << truthy->toString(ctx);
        if (!falsy->noTypeTests.empty() || !falsy->yesTypeTests.empty()) {
            buf << "  Being falsy entails:" << endl;
        }
        buf << falsy->toString(ctx);
        return buf.str();
    }

    static TestedKnowledge empty; // optimization

    void sanityCheck() const {
        if (!debug_mode) {
            return;
        }
        truthy->sanityCheck();
        falsy->sanityCheck();
        ENFORCE(TestedKnowledge::empty.falsy->yesTypeTests.empty());
        ENFORCE(TestedKnowledge::empty.falsy->noTypeTests.empty());
        ENFORCE(TestedKnowledge::empty.truthy->noTypeTests.empty());
        ENFORCE(TestedKnowledge::empty.truthy->yesTypeTests.empty());
    };
};

TestedKnowledge TestedKnowledge::empty;

class Environment {
public:
    Environment() = default;
    Environment(const Environment &rhs) = delete;
    Environment(Environment &&rhs) = default;

    bool isDead = false;
    cfg::BasicBlock *bb;
    vector<core::LocalVariable> vars;
    vector<core::TypeAndOrigins> types;
    vector<TestedKnowledge> knowledge;
    unordered_map<core::SymbolRef, shared_ptr<core::Type>> blockTypes;

    string toString(core::Context ctx) {
        stringstream buf;
        if (isDead) {
            buf << "dead=" << isDead << endl;
        }
        int i = -1;
        for (auto var : vars) {
            i++;
            if (var._name == core::Names::debugEnvironmentTemp()) {
                continue;
            }
            buf << var.toString(ctx) << ": " << types[i].type->toString(ctx, 0) << endl;
            buf << knowledge[i].toString(ctx) << endl;
        }
        return buf.str();
    }

    core::TypeAndOrigins getTypeAndOrigin(core::Context ctx, core::LocalVariable symbol) const {
        auto fnd = find(vars.begin(), vars.end(), symbol);
        if (fnd == vars.end()) {
            core::TypeAndOrigins ret;
            ret.type = core::Types::nil();
            ret.origins.push_back(ctx.owner.data(ctx).definitionLoc);
            return ret;
        }
        ENFORCE(types[fnd - vars.begin()].type.get() != nullptr);
        return types[fnd - vars.begin()];
    }

    core::TypeAndOrigins getOrCreateTypeAndOrigin(core::Context ctx, core::LocalVariable symbol) {
        auto fnd = find(vars.begin(), vars.end(), symbol);
        if (fnd == vars.end()) {
            core::TypeAndOrigins ret;
            ret.type = core::Types::nil();
            ret.origins.push_back(ctx.owner.data(ctx).definitionLoc);
            vars.emplace_back(symbol);
            types.push_back(ret);
            knowledge.emplace_back();
            return ret;
        }
        ENFORCE(types[fnd - vars.begin()].type.get() != nullptr);
        return types[fnd - vars.begin()];
    }

    const TestedKnowledge &getKnowledge(core::LocalVariable symbol, bool shouldFail = true) const {
        auto fnd = find(vars.begin(), vars.end(), symbol);
        if (fnd == vars.end()) {
            ENFORCE(!shouldFail, "Missing knowledge?");
            return TestedKnowledge::empty;
        }
        auto &r = knowledge[fnd - vars.begin()];
        r.sanityCheck();
        return r;
    }

    TestedKnowledge &getKnowledge(core::LocalVariable symbol, bool shouldFail = true) {
        return const_cast<TestedKnowledge &>(const_cast<const Environment *>(this)->getKnowledge(symbol, shouldFail));
    }

    /* propagate knowledge on `to = from` */
    void propagateKnowledge(core::Context ctx, core::LocalVariable to, core::LocalVariable from,
                            KnowledgeFilter &knowledgeFilter) {
        if (knowledgeFilter.isNeeded(to) && knowledgeFilter.isNeeded(from)) {
            getKnowledge(to) = getKnowledge(from);
            getKnowledge(to).truthy.mutate().noTypeTests.emplace_back(from, core::Types::falsyTypes());
            getKnowledge(to).falsy.mutate().yesTypeTests.emplace_back(from, core::Types::falsyTypes());
            getKnowledge(from).truthy.mutate().noTypeTests.emplace_back(to, core::Types::falsyTypes());
            getKnowledge(from).falsy.mutate().yesTypeTests.emplace_back(to, core::Types::falsyTypes());
            getKnowledge(from).sanityCheck();
            getKnowledge(to).sanityCheck();
        }
    }

    /* variable was reasigned. Forget everything about previous value */
    void clearKnowledge(core::Context ctx, core::LocalVariable reassigned, KnowledgeFilter &knowledgeFilter) {
        int i = -1;
        for (auto &k : knowledge) {
            i++;
            if (knowledgeFilter.isNeeded(this->vars[i])) {
                auto &truthy = k.truthy.mutate();
                auto &falsy = k.falsy.mutate();
                truthy.yesTypeTests.erase(remove_if(truthy.yesTypeTests.begin(), truthy.yesTypeTests.end(),
                                                    [&](auto const &c) -> bool { return c.first == reassigned; }),
                                          truthy.yesTypeTests.end());
                falsy.yesTypeTests.erase(remove_if(falsy.yesTypeTests.begin(), falsy.yesTypeTests.end(),
                                                   [&](auto const &c) -> bool { return c.first == reassigned; }),
                                         falsy.yesTypeTests.end());
                truthy.noTypeTests.erase(remove_if(truthy.noTypeTests.begin(), truthy.noTypeTests.end(),
                                                   [&](auto const &c) -> bool { return c.first == reassigned; }),
                                         truthy.noTypeTests.end());
                falsy.noTypeTests.erase(remove_if(falsy.noTypeTests.begin(), falsy.noTypeTests.end(),
                                                  [&](auto const &c) -> bool { return c.first == reassigned; }),
                                        falsy.noTypeTests.end());
                k.sanityCheck();
            }
        }
    }

    /* Special case sources of knowledge */
    void updateKnowledge(core::Context ctx, core::LocalVariable local, core::Loc loc, cfg::Send *send,
                         KnowledgeFilter &knowledgeFilter) {
        if (send->fun == core::Names::bang()) {
            if (!knowledgeFilter.isNeeded(local)) {
                return;
            }
            auto &whoKnows = getKnowledge(local);
            auto other = find(vars.begin(), vars.end(), send->recv) - vars.begin();
            if (other != vars.size()) {
                whoKnows.truthy = this->knowledge[other].falsy;
                whoKnows.falsy = this->knowledge[other].truthy;
                this->knowledge[other].truthy.mutate().noTypeTests.emplace_back(local, core::Types::falsyTypes());
                this->knowledge[other].falsy.mutate().yesTypeTests.emplace_back(local, core::Types::falsyTypes());
            }
            whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv, core::Types::falsyTypes());
            whoKnows.falsy.mutate().noTypeTests.emplace_back(send->recv, core::Types::falsyTypes());

            whoKnows.sanityCheck();
        } else if (send->fun == core::Names::nil_p()) {
            if (!knowledgeFilter.isNeeded(local)) {
                return;
            }
            auto &whoKnows = getKnowledge(local);
            whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv, core::Types::nil());
            whoKnows.falsy.mutate().noTypeTests.emplace_back(send->recv, core::Types::nil());
            whoKnows.sanityCheck();
        }

        if (send->args.empty()) {
            return;
        }
        if (send->fun == core::Names::kind_of() || send->fun == core::Names::is_a_p()) {
            if (!knowledgeFilter.isNeeded(local)) {
                return;
            }
            auto &whoKnows = getKnowledge(local);
            core::TypeAndOrigins klass = getTypeAndOrigin(ctx, send->args[0]);
            if (klass.type->derivesFrom(ctx, core::Symbols::Module())) {
                auto *s = core::cast_type<core::ClassType>(klass.type.get());
                if (s == nullptr) {
                    return;
                }
                core::SymbolRef attachedClass = s->symbol.data(ctx).attachedClass(ctx);
                if (attachedClass.exists()) {
                    auto ty = attachedClass.data(ctx).externalType(ctx);
                    whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv, ty);
                    whoKnows.falsy.mutate().noTypeTests.emplace_back(send->recv, ty);
                }
                whoKnows.sanityCheck();
            }
        } else if (send->fun == core::Names::eqeq()) {
            if (!knowledgeFilter.isNeeded(local)) {
                return;
            }
            auto &whoKnows = getKnowledge(local);
            core::TypeAndOrigins tp1 = getTypeAndOrigin(ctx, send->args[0]);
            core::TypeAndOrigins tp2 = getTypeAndOrigin(ctx, send->recv);
            if (tp1.type->isDynamic() && tp2.type->isDynamic()) {
                return;
            }

            ENFORCE(tp1.type.get() != nullptr);
            ENFORCE(tp2.type.get() != nullptr);
            whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv, tp1.type);
            whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->args[0], tp2.type);
            whoKnows.sanityCheck();
        } else if (send->fun == core::Names::tripleEq()) {
            if (!knowledgeFilter.isNeeded(local)) {
                return;
            }
            auto &whoKnows = getKnowledge(local);
            core::TypeAndOrigins recvKlass = getTypeAndOrigin(ctx, send->recv);
            if (!recvKlass.type->derivesFrom(ctx, core::Symbols::Module())) {
                return;
            }

            auto *s = core::cast_type<core::ClassType>(recvKlass.type.get());
            if (s == nullptr) {
                return;
            }

            core::SymbolRef attachedClass = s->symbol.data(ctx).attachedClass(ctx);
            if (attachedClass.exists()) {
                auto ty = attachedClass.data(ctx).externalType(ctx);
                whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->args[0], ty);
                whoKnows.falsy.mutate().noTypeTests.emplace_back(send->args[0], ty);
            }
            whoKnows.sanityCheck();

        } else if (send->fun == core::Names::hardAssert()) {
            core::TypeAndOrigins recvKlass = getTypeAndOrigin(ctx, send->recv);
            if (!recvKlass.type->derivesFrom(ctx, core::Symbols::Kernel())) {
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

    /*
     * Create an Environment out of this one that holds if final condition in
     * this environment was isTrue
     *
     * Either returns a reference to `env` unchanged, or populates `copy` and
     * returns a reference to that. This odd calling convention is used to avoid
     * copies, and because all callers of this immediately use the result and
     * then discard it, so the mixed lifetimes are not a problem in practice.
     */
    static const Environment &withCond(core::Context ctx, const Environment &env, Environment &copy, bool isTrue,
                                       const vector<core::LocalVariable> &filter) {
        if (!env.bb->bexit.cond.exists() || env.bb->bexit.cond == core::LocalVariable::blockCall()) {
            return env;
        }
        copy.cloneFrom(env);
        copy.assumeKnowledge(ctx, isTrue, env.bb->bexit.cond, env.bb->bexit.loc, filter);
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
                ctx, core::Types::dropSubtypesOf(ctx, tp.type, core::Symbols::NilClass()), core::Symbols::FalseClass());
            setTypeAndOrigin(cond, tp);
        }

        auto &knowledgeToChoose = isTrue ? thisKnowledge.truthy : thisKnowledge.falsy;

        if (isDead) {
            return;
        }

        for (auto &typeTested : knowledgeToChoose->yesTypeTests) {
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

        for (auto &typeTested : knowledgeToChoose->noTypeTests) {
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

    void mergeWith(core::Context ctx, const Environment &other, core::Loc loc, cfg::CFG &inWhat, cfg::BasicBlock *bb,
                   KnowledgeFilter &knowledgeFilter) {
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
                auto otherTruthy = KnowledgeFact::under(ctx, other.getKnowledge(var, false).truthy, other, loc, inWhat,
                                                        bb, knowledgeFilter.isNeeded(var));
                if (!otherTruthy->isDead) {
                    if (!thisKnowledge.seenTruthyOption) {
                        thisKnowledge.seenTruthyOption = true;
                        thisKnowledge.truthy = otherTruthy;
                    } else {
                        thisKnowledge.truthy.mutate().min(ctx, *otherTruthy);
                    }
                }
            }

            if (canBeFalsy) {
                auto &thisKnowledge = getKnowledge(var);
                auto otherFalsy = KnowledgeFact::under(ctx, other.getKnowledge(var, false).falsy, other, loc, inWhat,
                                                       bb, knowledgeFilter.isNeeded(var));
                if (!otherFalsy->isDead) {
                    if (!thisKnowledge.seenFalsyOption) {
                        thisKnowledge.seenFalsyOption = true;
                        thisKnowledge.falsy = otherFalsy;
                    } else {
                        thisKnowledge.falsy.mutate().min(ctx, *otherFalsy);
                    }
                }
            }
        }

        for (auto &blk : other.blockTypes) {
            auto mine = this->blockTypes.find(blk.first);
            if (mine == this->blockTypes.end()) {
                this->blockTypes[blk.first] = blk.second;
            } else {
                ENFORCE(mine->second == blk.second);
            }
        }
    }

    void populateFrom(core::Context ctx, const Environment &other) {
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

        this->blockTypes = other.blockTypes;
    }

    shared_ptr<core::Type> dropLiteral(shared_ptr<core::Type> tp) {
        if (auto *a = core::cast_type<core::LiteralType>(tp.get())) {
            return a->underlying;
        }
        return tp;
    }

    // Extract the return value type from a proc. This should potentially be a
    // method on `Type` or otherwise handled there.
    shared_ptr<core::Type> getReturnType(core::Context ctx, shared_ptr<core::Type> procType) {
        if (!procType->derivesFrom(ctx, core::Symbols::Proc())) {
            return core::Types::dynamic();
        }
        auto *applied = core::cast_type<core::AppliedType>(procType.get());
        if (applied == nullptr || applied->targs.empty()) {
            return core::Types::dynamic();
        }
        // Proc types have their return type as the first targ
        return applied->targs.front();
    }

    shared_ptr<core::Type> processBinding(core::Context ctx, cfg::Binding &bind, int loopCount, int bindMinLoops,
                                          KnowledgeFilter &knowledgeFilter) {
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

                    if (send->fun == core::Names::super()) {
                        // TODO
                        if (send->block.exists()) {
                            this->blockTypes[send->block] = core::Types::dynamic();
                        }
                        tp.type = core::Types::dynamic();
                    } else {
                        shared_ptr<core::Type> *blockTy = nullptr;
                        if (send->block.exists()) {
                            blockTy = &this->blockTypes[send->block];
                        }
                        tp.type = recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type,
                                                              recvType.type, blockTy);
                    }
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::Ident *i) {
                    auto typeAndOrigin = getOrCreateTypeAndOrigin(ctx, i->what);
                    tp.type = typeAndOrigin.type;
                    tp.origins = typeAndOrigin.origins;
                    ENFORCE(!tp.origins.empty(), "Inferencer did not assign location");
                },
                [&](cfg::Alias *a) {
                    core::SymbolRef symbol = a->what.data(ctx).dealias(ctx);
                    const core::Symbol &data = symbol.data(ctx);
                    if (data.isClass()) {
                        ENFORCE(data.resultType.get(), "Type should have been filled in by the namer");
                        tp.type = data.resultType;
                        tp.origins.push_back(symbol.data(ctx).definitionLoc);
                    } else if (data.isField() || data.isStaticField() || data.isMethodArgument() ||
                               data.isTypeMember()) {
                        if (data.resultType.get() != nullptr) {
                            if (data.isField()) {
                                tp.type = core::Types::resultTypeAsSeenFrom(
                                    ctx, symbol, ctx.owner.data(ctx).enclosingClass(ctx),
                                    ctx.owner.data(ctx).enclosingClass(ctx).data(ctx).selfTypeArgs(ctx));
                            } else {
                                tp.type = data.resultType;
                            }
                            tp.origins.push_back(data.definitionLoc);
                        } else {
                            tp.origins.push_back(core::Loc::none());
                            tp.type = core::Types::dynamic();
                        }
                    } else {
                        Error::notImplemented();
                    }
                },
                [&](cfg::Self *i) {
                    tp.type = i->klass.data(ctx).selfType(ctx);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::SymbolLit *i) {
                    tp.type = make_shared<core::LiteralType>(core::Symbols::Symbol(), i->value);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::StringLit *i) {
                    tp.type = make_shared<core::LiteralType>(core::Symbols::String(), i->value);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::LoadArg *i) {
                    /* read type from info filled by define_method */
                    tp.type = getTypeAndOrigin(ctx, i->receiver).type->getCallArgumentType(ctx, i->method, i->arg);
                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::LoadYieldParam *i) {
                    auto it = this->blockTypes.find(i->block);
                    ENFORCE(it != this->blockTypes.end(), "Inside a block we never entered!");
                    auto &procType = it->second;
                    if (procType == nullptr) {
                        tp.type = core::Types::dynamic();
                    } else {
                        tp.type = procType->getCallArgumentType(ctx, core::Names::call(), i->arg);
                    }

                    tp.origins.push_back(bind.loc);
                },
                [&](cfg::Return *i) {
                    auto expectedType = ctx.owner.data(ctx).resultType;
                    if (!expectedType) {
                        expectedType = core::Types::dynamic();
                    } else {
                        expectedType = core::Types::resultTypeAsSeenFrom(
                            ctx, ctx.owner, ctx.owner.data(ctx).enclosingClass(ctx),
                            ctx.owner.data(ctx).enclosingClass(ctx).data(ctx).selfTypeArgs(ctx));
                    }
                    tp.type = core::Types::bottom();
                    tp.origins.push_back(bind.loc);

                    auto typeAndOrigin = getTypeAndOrigin(ctx, i->what);
                    if (!core::Types::isSubType(ctx, typeAndOrigin.type, expectedType)) {
                        ctx.state.error(core::ComplexError(
                            bind.loc, core::errors::Infer::ReturnTypeMismatch,
                            "Returning value that does not conform to method result type",
                            {core::ErrorSection("Expected " + expectedType->show(ctx),
                                                {
                                                    core::ErrorLine::from(ctx.owner.data(ctx).definitionLoc,
                                                                          "Method `{}` has return type `{}`",
                                                                          ctx.owner.data(ctx).name.toString(ctx),
                                                                          expectedType->show(ctx)),
                                                }),
                             core::ErrorSection("Got " + typeAndOrigin.type->show(ctx) + " originating from:",
                                                typeAndOrigin.origins2Explanations(ctx))}));
                    }
                },
                [&](cfg::BlockReturn *i) {
                    auto it = this->blockTypes.find(i->block);
                    ENFORCE(it != this->blockTypes.end(), "Returning from a block we never entered!");
                    auto &procType = it->second;
                    if (procType != nullptr) {
                        auto typeAndOrigin = getTypeAndOrigin(ctx, i->what);
                        auto expectedType = getReturnType(ctx, procType);
                        if (!core::Types::isSubType(ctx, typeAndOrigin.type, expectedType)) {
                            // TODO(nelhage): We should somehow report location
                            // information about the `send` and/or the
                            // definition of the block type
                            ctx.state.error(core::ComplexError(
                                bind.loc, core::errors::Infer::ReturnTypeMismatch,
                                "Returning value that does not conform to block result type",
                                {core::ErrorSection("Expected " + expectedType->toString(ctx)),
                                 core::ErrorSection("Got " + typeAndOrigin.type->toString(ctx) + " originating from:",
                                                    typeAndOrigin.origins2Explanations(ctx))}));
                        }
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
                                "assert_type!: argument does not have asserted type " + c->type->show(ctx),
                                {core::ErrorSection("Got " + ty.type->show(ctx) + " originating from:",
                                                    ty.origins2Explanations(ctx))}));
                        }
                    }
                },
                [&](cfg::DebugEnvironment *d) {
                    d->str = toString(ctx);
                    tp.type = core::Types::bottom();
                    tp.origins.push_back(bind.loc);
                });

            ENFORCE(tp.type.get() != nullptr, "Inferencer did not assign type: ", bind.value->toString(ctx));
            tp.type->sanityCheck(ctx);

            if (!tp.type->isFullyDefined()) {
                ctx.state.error(
                    bind.loc, core::errors::Infer::IncompleteType,
                    "Expression does not have a fully-defined type (Did you reference another class's type members?)");
                tp.type = core::Types::dynamic();
            }
            ENFORCE(!tp.origins.empty(), "Inferencer did not assign location");
            if (auto *send = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
                if (send->block.exists()) {
                    ENFORCE(this->blockTypes.find(send->block) != this->blockTypes.end());
                }
            }

            core::TypeAndOrigins cur = getOrCreateTypeAndOrigin(ctx, bind.bind);

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
                    switch (bindMinLoops) {
                        case cfg::CFG::MIN_LOOP_FIELD:
                            ctx.state.error(bind.loc, core::errors::Infer::FieldReassignmentTypeMismatch,
                                            "Reassigning field with a value of wrong type: {} is not a subtype of {}",
                                            tp.type->show(ctx), cur.type->show(ctx));
                            break;
                        case cfg::CFG::MIN_LOOP_GLOBAL:
                            ctx.state.error(bind.loc, core::errors::Infer::GlobalReassignmentTypeMismatch,
                                            "Reassigning global with a value of wrong type: {} is not a subtype of {}",
                                            tp.type->show(ctx), cur.type->show(ctx));
                            break;
                        default:
                            ctx.state.error(bind.loc, core::errors::Infer::PinnedVariableMismatch,
                                            "Changing type of pinned argument, {} is not a subtype of {}",
                                            tp.type->show(ctx), cur.type->show(ctx));
                            break;
                    }
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

    void cloneFrom(const Environment &rhs) {
        *this = rhs;
    }

private:
    Environment &operator=(const Environment &rhs) = default;
};

KnowledgeRef KnowledgeFact::under(core::Context ctx, const KnowledgeRef &what, const Environment &env, core::Loc loc,
                                  cfg::CFG &inWhat, cfg::BasicBlock *bb, bool isNeeded) {
    if (what->yesTypeTests.empty() && !isNeeded) {
        return what;
    }
    KnowledgeRef copy = what;
    if (env.isDead) {
        copy.mutate().isDead = true;
        return copy;
    }
    int i = -1;
    bool enteringLoop = (bb->flags & cfg::CFG::LOOP_HEADER) != 0;
    for (auto local : env.vars) {
        i++;
        if (enteringLoop && bb->outerLoops <= inWhat.maxLoopWrite[local]) {
            continue;
        }
        auto fnd = find_if(copy->yesTypeTests.begin(), copy->yesTypeTests.end(),
                           [&](auto const &e) -> bool { return e.first == local; });
        if (fnd == copy->yesTypeTests.end()) {
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
                copy.mutate().yesTypeTests.emplace_back(local, env.types[i].type);
            }
        } else {
            auto &second = fnd->second;
            auto &typeAndOrigin = env.types[i];
            auto combinedType =
                core::Types::glb(ctx, dropConstructor(ctx, typeAndOrigin.origins[0], typeAndOrigin.type), second);
            if (combinedType->isBottom()) {
                copy.mutate().isDead = true;
                break;
            }
        }
    }
    return copy;
}
bool isSyntheticReturn(core::Context ctx, cfg::Binding &bind) {
    if (bind.bind._name == core::Names::finalReturn()) {
        if (auto *ret = dynamic_cast<cfg::Return *>(bind.value.get())) {
            return ret->what.isSyntheticTemporary(ctx) ||
                   (ret->what._name.data(ctx).kind == core::NameKind::CONSTANT &&
                    ret->what._name.data(ctx).cnst.original == core::Names::nil());
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
    KnowledgeFilter knowledgeFilter(ctx, cfg);
    if (!cfg->basicBlocks.empty()) {
        ENFORCE(!cfg->symbol.data(ctx).isAbstract());
    }
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
            Environment tempEnv;
            auto &envAsSeenFromBranch =
                Environment::withCond(ctx, outEnvironments[parent->id], tempEnv, isTrueBranch, current.vars);
            current.populateFrom(ctx, envAsSeenFromBranch);
        } else {
            current.isDead = (bb != cfg->entry());
            for (cfg::BasicBlock *parent : bb->backEdges) {
                if (!visited[parent->id]) {
                    continue;
                }
                bool isTrueBranch = parent->bexit.thenb == bb;
                Environment tempEnv;
                auto &envAsSeenFromBranch =
                    Environment::withCond(ctx, outEnvironments[parent->id], tempEnv, isTrueBranch, current.vars);
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
                uninitialized.origins.push_back(ctx.owner.data(ctx).definitionLoc);
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
            histogramInc("infer.knowledge.truthy.yes.size", k.truthy->yesTypeTests.size());
            histogramInc("infer.knowledge.truthy.no.size", k.truthy->noTypeTests.size());
            histogramInc("infer.knowledge.falsy.yes.size", k.falsy->yesTypeTests.size());
            histogramInc("infer.knowledge.falsy.no.size", k.falsy->noTypeTests.size());
        }
    }
    if (startErrorCount == ctx.state.totalErrors()) {
        counterInc("infer.methods_typechecked.no_errors");
    }
    return cfg;
}
