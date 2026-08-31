#ifndef SORBET_ENVIRONMENT_H
#define SORBET_ENVIRONMENT_H

#include "cfg/CFG.h"
#include "common/common.h"
#include "core/Context.h"
#include "core/Error.h"
#include "core/Names.h"
#include "core/Refcounting.h"
#include "core/Symbols.h"
#include "core/errors/infer.h"
#include "core/errors/internal.h"
#include "core/lsp/QueryResponse.h"
#include "inference.h"
#include <memory>
#include <utility>
#include <vector>

namespace sorbet::infer {

class Environment;

// Per-local scratch marks for one inference run, so that merging knowledge neither allocates nor sorts:
// `KnowledgeRef::under` marks the locals that already have a test, `KnowledgeFact::min` records the first position of
// each local among the other fact's tests. Users clear their marks when done.
class LocalMarks final {
    size_t numLocals;
    // Allocated on first use: most methods never merge knowledge.
    std::vector<uint32_t> values;
    std::vector<uint32_t> touched;

public:
    static constexpr uint32_t UNSET = UINT32_MAX;

    explicit LocalMarks(size_t numLocals) : numLocals(numLocals) {}
    LocalMarks(const LocalMarks &) = delete;
    LocalMarks &operator=(const LocalMarks &) = delete;

    uint32_t get(cfg::LocalRef ref) const {
        ENFORCE(ref.id() < numLocals);
        return values.empty() ? UNSET : values[ref.id()];
    }

    // Marks `ref` with `value` unless it is marked already; returns whether it was unmarked.
    bool setIfUnset(cfg::LocalRef ref, uint32_t value) {
        ENFORCE(ref.id() < numLocals);
        if (values.empty()) {
            values.assign(numLocals, UNSET);
        }
        auto &slot = values[ref.id()];
        if (slot != UNSET) {
            return false;
        }
        slot = value;
        touched.emplace_back(ref.id());
        return true;
    }

    void clear() {
        for (auto id : touched) {
            values[id] = UNSET;
        }
        touched.clear();
    }

    // Clears the marks when it goes out of scope.
    struct Clearer {
        LocalMarks &marks;
        explicit Clearer(LocalMarks &marks) : marks(marks) {}
        ~Clearer() {
            marks.clear();
        }
    };
};

/**
 * Encode things that we know hold and don't hold.
 */
struct KnowledgeFact : public core::RefCounted<KnowledgeFact> {
    KnowledgeFact() = default;

    bool isDead = false;
    /* the following type tests are known to be true */
    InlinedVector<std::pair<cfg::LocalRef, core::TypePtr>, 1> yesTypeTests;
    /* the following type tests are known to be false */
    InlinedVector<std::pair<cfg::LocalRef, core::TypePtr>, 1> noTypeTests;

    /* this is a "merge" of two knowledges - computes a "lub" of knowledges */
    void min(core::Context ctx, const KnowledgeFact &other, LocalMarks &marks);

    void sanityCheck() const;

    std::string toString(const core::GlobalState &gs, const cfg::CFG &cfg) const;

    // Can't use the regular copy constructor because `core::RefCounted` isn't copyable.
    core::RefPtr<KnowledgeFact> freshCopy();

    KnowledgeFact(bool isDead, const InlinedVector<std::pair<cfg::LocalRef, core::TypePtr>, 1> &yesTypeTests,
                  const InlinedVector<std::pair<cfg::LocalRef, core::TypePtr>, 1> &noTypeTests);
};
CheckSize(KnowledgeFact, 56, 8);

// storing all the knowledge is slow
// it only makes sense for us to store it if we are going to use it
// wallk all the instructions and collect knowledge that we may ever need
class KnowledgeFilter {
    std::vector<bool> used_vars;

public:
    KnowledgeFilter(core::Context ctx, cfg::CFG &cfg);

    KnowledgeFilter(KnowledgeFilter &) = delete;
    KnowledgeFilter(KnowledgeFilter &&) = delete;

    bool isNeeded(cfg::LocalRef var);
};

// KnowledgeRef wraps a `KnowledgeFact` with copy-on-write semantics
class KnowledgeRef {
    // Is private to ensure that yes/no type test updates go through trusted paths.
    KnowledgeFact &mutate();
    // `nullptr` stands for the empty fact (no type tests, not dead). The overwhelming majority of variables never
    // accumulate any knowledge, so representing "empty" without an allocation avoids creating (and refcounting, and
    // destroying) a KnowledgeFact per variable per Environment.
    core::RefPtr<KnowledgeFact> knowledge;
    static const KnowledgeFact emptyFact;

public:
    KnowledgeRef() = default;
    KnowledgeRef(const KnowledgeRef &) = default;
    KnowledgeRef &operator=(const KnowledgeRef &) = default;
    KnowledgeRef(KnowledgeRef &&) = default;
    KnowledgeRef &operator=(KnowledgeRef &&) = default;

    const KnowledgeFact &operator*() const;
    const KnowledgeFact *operator->() const;

    void addYesTypeTest(cfg::LocalRef ref, core::TypePtr type);
    void addNoTypeTest(cfg::LocalRef ref, core::TypePtr type);
    void markDead();
    void min(core::Context ctx, const KnowledgeFact &other, LocalMarks &marks);

    /**
     * Computes all possible implications of this knowledge holding as an exit from environment env in block bb
     */
    KnowledgeRef under(core::Context ctx, const Environment &env, cfg::CFG &inWhat, cfg::BasicBlock *bb, bool isNeeded,
                       LocalMarks &marks) const;

    void removeReferencesToVar(cfg::LocalRef ref);
};
CheckSize(KnowledgeRef, 8, 8);

/** Almost a named pair of two KnowledgeFact-s. One holds knowledge that is true when a variable is falsy,
 * the other holds knowledge which is true if the same variable is falsy->
 */
class TestedKnowledge {
    // Hide to prevent direct assignment so that all mutations go thru methods.
    KnowledgeRef _truthy, _falsy;

public:
    bool seenTruthyOption; // Only used during environment merge. Used to indicate "all-knowing" truthy option.
    bool seenFalsyOption;  // Same for falsy

    const KnowledgeRef &truthy() const {
        return _truthy;
    }

    const KnowledgeRef &falsy() const {
        return _falsy;
    }

    KnowledgeRef &truthy() {
        return _truthy;
    }
    KnowledgeRef &falsy() {
        return _falsy;
    }

    void replaceTruthy(const KnowledgeRef &newTruthy);
    void replaceFalsy(const KnowledgeRef &newFalsy);
    void replace(const TestedKnowledge &knowledge);

    std::string toString(const core::GlobalState &gs, const cfg::CFG &cfg) const;

    static thread_local TestedKnowledge empty; // optimization

    void removeReferencesToVar(cfg::LocalRef ref);
    void sanityCheck() const;
    void emitKnowledgeSizeMetric() const;

    bool isEmpty() const {
        return _truthy->yesTypeTests.empty() && _truthy->noTypeTests.empty() && _falsy->yesTypeTests.empty() &&
               _falsy->noTypeTests.empty();
    }
};
CheckSize(TestedKnowledge, 24, 8);

class Environment {
    const core::TypeAndOrigins uninitialized;

    const core::Loc ownerLoc;

    /*
     * These four vectors represent the core state store of the environment,
     * modeling a map from local variables to (type, knowledge, known-truthy)
     * tuples.
     *
     * As we learn flow-dependent information through the CFG, we normally
     * represent that information in the type of a variable; For instance, "v is
     * falsy" is equivalent to "v : T.any(NilClass, FalseClass)", and "v is
     * truthy" can strip `NilClass` from an OrType.
     *
     * However, some types, such as Object, are inhabited by both truthy and
     * falsy value but are not explicit unions. For these types, there is no
     * type that represents (e.g.) "Object \ T.any(NilClass,
     * FalseClass)". Instead of augmenting the type system with some hack to
     * represent such types, we carry an extra bit of information in the
     * inferencer. There's no need for the inverse "known falsy" bit because a
     * simple subtyping check suffices to represent that one.
     */

    struct VariableState {
        core::TypeAndOrigins typeAndOrigins;
        TestedKnowledge knowledge;
        bool knownTruthy;
    };
    // TODO(jvilk): Use vectors.
    UnorderedMap<cfg::LocalRef, VariableState> _vars;

    UnorderedMap<cfg::LocalRef, core::TypeAndOrigins> pinnedTypes;

    // Variables whose knowledge may hold type tests (an overapproximation: entries are never removed), so that
    // `clearKnowledge` visits only them rather than every variable of a block with thousands of bindings.
    InlinedVector<cfg::LocalRef, 8> knowledgeOwners;
    void noteKnowledgeOwner(cfg::LocalRef var);

    bool hasType(core::Context ctx, cfg::LocalRef symbol) const;

    TestedKnowledge &getKnowledge(cfg::LocalRef symbol, bool shouldFail = true) {
        return const_cast<TestedKnowledge &>(const_cast<const Environment *>(this)->getKnowledge(symbol, shouldFail));
    }

    const TestedKnowledge &getKnowledge(cfg::LocalRef symbol, bool shouldFail = true) const;

    bool getKnownTruthy(cfg::LocalRef var) const;

    /* propagate knowledge on `to = from` */
    void propagateKnowledge(core::Context ctx, cfg::LocalRef to, cfg::LocalRef from, KnowledgeFilter &knowledgeFilter);

    /* variable was reasigned. Forget everything about previous value */
    void clearKnowledge(core::Context ctx, cfg::LocalRef reassigned, KnowledgeFilter &knowledgeFilter);

    // Handles updateKnowledge for methods that behave like Kernel#is_a?, Module#===, etc.
    void updateKnowledgeKindOf(core::Context ctx, cfg::LocalRef local, core::Loc loc, const core::TypePtr &klassType,
                               cfg::LocalRef ref, KnowledgeFilter &knowledgeFilter, core::NameRef fun);

    /* Special case sources of knowledge */
    void updateKnowledge(core::Context ctx, cfg::LocalRef local, core::Loc loc, const cfg::Send *send,
                         KnowledgeFilter &knowledgeFilter);

    void setTypeAndOrigin(cfg::LocalRef symbol, const core::TypeAndOrigins &typeAndOrigins);

    void assumeKnowledge(core::Context ctx, bool isTrue, cfg::LocalRef cond, core::Loc loc,
                         const UnorderedMap<cfg::LocalRef, VariableState> &filter);

    void cloneFrom(const Environment &rhs);

    core::TypeAndOrigins getTypeFromRebind(core::Context ctx, const core::DispatchComponent &main,
                                           cfg::LocalRef fallback);

public:
    Environment(core::Loc ownerLoc);
    Environment(const Environment &rhs) = delete;
    Environment(Environment &&rhs) = default;

    bool isDead = false;
    cfg::BasicBlock *bb;

    const UnorderedMap<cfg::LocalRef, VariableState> &vars() const {
        return _vars;
    }

    void initializeBasicBlockArgs(const cfg::BasicBlock &bb);

    void setUninitializedVarsToNil(core::Context ctx, core::Loc origin);

    std::string toString(const core::GlobalState &gs, const cfg::CFG &cfg) const;

    // NB: you can't call this function on vars in the first basic block since
    // their type will be nullptr
    const core::TypeAndOrigins &getTypeAndOrigin(cfg::LocalRef symbol) const;

    const core::TypeAndOrigins &getAndFillTypeAndOrigin(cfg::VariableUseSite &symbol) const;
    const core::TypeAndOrigins &getAndFillTypeAndOrigin(cfg::LocalRef symbol, core::TypePtr &ty) const;

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
                                       const UnorderedMap<cfg::LocalRef, VariableState> &filter);

    void mergeWith(core::Context ctx, const Environment &other, cfg::CFG &inWhat, cfg::BasicBlock *bb,
                   KnowledgeFilter &knowledgeFilter, LocalMarks &marks);

    void computePins(core::Context ctx, const std::vector<Environment> &envs, const cfg::CFG &inWhat,
                     const cfg::BasicBlock *bb);

    void populateFrom(core::Context ctx, const Environment &other);

    core::TypePtr
    processBinding(core::Context ctx, const cfg::CFG &inWhat, cfg::Binding &bind, int loopCount, int bindMinLoops,
                   KnowledgeFilter &knowledgeFilter, core::TypeConstraint &constr,
                   const core::TypePtr &methodReturnType,
                   const std::optional<cfg::BasicBlock::BlockExitCondInfo> &parentUpdateKnowledgeReceiver);

    core::Loc locForUninitialized() const {
        return ownerLoc;
    }
};

} // namespace sorbet::infer

#endif // SORBET_ENVIRONMENT_H
