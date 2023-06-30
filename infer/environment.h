#ifndef SORBET_ENVIRONMENT_H
#define SORBET_ENVIRONMENT_H

#include "cfg/CFG.h"
#include "common/common.h"
#include "core/Context.h"
#include "core/Error.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/errors/infer.h"
#include "core/errors/internal.h"
#include "core/lsp/QueryResponse.h"
#include "inference.h"
#include <memory>
#include <utility>
#include <vector>

namespace sorbet::infer {

class TypeTestReverseIndex final {
    // Note: vectors are sorted and are treated as sets.
    UnorderedMap<cfg::LocalRef, InlinedVector<cfg::LocalRef, 1>> index;
    static const InlinedVector<cfg::LocalRef, 1> empty;

public:
    TypeTestReverseIndex() = default;
    TypeTestReverseIndex(const TypeTestReverseIndex &rhs) = delete;
    TypeTestReverseIndex(TypeTestReverseIndex &&rhs) = default;

    TypeTestReverseIndex &operator=(const TypeTestReverseIndex &rhs) = delete;
    TypeTestReverseIndex &operator=(TypeTestReverseIndex &&rhs) = delete;

    void addToIndex(cfg::LocalRef from, cfg::LocalRef to);
    const InlinedVector<cfg::LocalRef, 1> &get(cfg::LocalRef from) const;
    void replace(cfg::LocalRef from, InlinedVector<cfg::LocalRef, 1> &&list);
    void cloneFrom(const TypeTestReverseIndex &index);
};

class Environment;
struct KnowledgeFact;

// storing all the knowledge is slow
// it only makes sense for us to store it if we are going to use it
// wallk all the instructions and collect knowledge that we may ever need
class KnowledgeFilter {
    std::vector<bool> used_vars;

public:
    KnowledgeFilter(core::Context ctx, std::unique_ptr<cfg::CFG> &cfg);

    KnowledgeFilter(KnowledgeFilter &) = delete;
    KnowledgeFilter(KnowledgeFilter &&) = delete;

    bool isNeeded(cfg::LocalRef var);
};

// KnowledgeRef wraps a `KnowledgeFact` with copy-on-write semantics
class KnowledgeRef {
    // Is private to ensure that yes/no type test updates go through trusted paths that keep TypeTestReverseIndex
    // updated.
    KnowledgeFact &mutate();
    std::shared_ptr<KnowledgeFact> knowledge;

public:
    KnowledgeRef();
    KnowledgeRef(const KnowledgeRef &) = default;
    KnowledgeRef &operator=(const KnowledgeRef &) = default;
    KnowledgeRef(KnowledgeRef &&) = default;
    KnowledgeRef &operator=(KnowledgeRef &&) = default;

    const KnowledgeFact &operator*() const;
    const KnowledgeFact *operator->() const;

    void addYesTypeTest(cfg::LocalRef of, TypeTestReverseIndex &index, cfg::LocalRef ref, core::TypePtr type);
    void addNoTypeTest(cfg::LocalRef of, TypeTestReverseIndex &index, cfg::LocalRef ref, core::TypePtr type);
    void markDead();
    void min(core::Context ctx, const KnowledgeFact &other);

    /**
     * Computes all possible implications of this knowledge holding as an exit from environment env in block bb
     */
    KnowledgeRef under(core::Context ctx, const Environment &env, cfg::CFG &inWhat, cfg::BasicBlock *bb,
                       bool isNeeded) const;

    void removeReferencesToVar(cfg::LocalRef ref);
};

/** Almost a named pair of two KnowledgeFact-s. One holds knowledge that is true when a variable is falsy,
 * the other holds knowledge which is true if the same variable is falsy->
 */
class TestedKnowledge {
    // Hide to prevent direct assignment so that all mutations go thru methods that keep TypeTestReverseIndex updated.
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

    void replaceTruthy(cfg::LocalRef of, TypeTestReverseIndex &index, const KnowledgeRef &newTruthy);
    void replaceFalsy(cfg::LocalRef of, TypeTestReverseIndex &index, const KnowledgeRef &newFalsy);
    void replace(cfg::LocalRef of, TypeTestReverseIndex &index, const TestedKnowledge &knowledge);

    std::string toString(const core::GlobalState &gs, const cfg::CFG &cfg) const;

    static TestedKnowledge empty; // optimization

    void removeReferencesToVar(cfg::LocalRef ref);
    void sanityCheck() const;
    void emitKnowledgeSizeMetric() const;
};

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

    // Map from LocalRef to LocalRefs that _may_ contain it in yes/no type tests (overapproximation).
    TypeTestReverseIndex typeTestsWithVar;

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

    void setUninitializedVarsToNil(const core::Context &ctx, core::Loc origin);

    std::string toString(const core::GlobalState &gs, const cfg::CFG &cfg) const;

    // NB: you can't call this function on vars in the first basic block since
    // their type will be nullptr
    const core::TypeAndOrigins &getTypeAndOrigin(core::Context ctx, cfg::LocalRef symbol) const;

    const core::TypeAndOrigins &getAndFillTypeAndOrigin(core::Context ctx, cfg::VariableUseSite &symbol) const;

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
                   KnowledgeFilter &knowledgeFilter);

    void computePins(core::Context ctx, const std::vector<Environment> &envs, const cfg::CFG &inWhat,
                     const cfg::BasicBlock *bb);

    void populateFrom(core::Context ctx, const Environment &other);

    core::TypePtr
    processBinding(core::Context ctx, const cfg::CFG &inWhat, cfg::Binding &bind, int loopCount, int bindMinLoops,
                   KnowledgeFilter &knowledgeFilter, core::TypeConstraint &constr, core::TypePtr &methodReturnType,
                   const std::optional<cfg::BasicBlock::BlockExitCondInfo> &parentUpdateKnowledgeReceiver);

    core::Loc locForUninitialized() const {
        return ownerLoc;
    }
};

} // namespace sorbet::infer

#endif // SORBET_ENVIRONMENT_H
