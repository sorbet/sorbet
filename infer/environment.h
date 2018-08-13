#ifndef SORBET_ENVIRONMENT_H
#define SORBET_ENVIRONMENT_H

#include "../common/common.h"
#include "cfg/CFG.h"
#include "core/Context.h"
#include "core/Errors.h"
#include "core/Names/infer.h"
#include "core/Symbols.h"
#include "core/errors/infer.h"
#include "core/errors/internal.h"
#include "core/lsp/QueryResponse.h"
#include "inference.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace sorbet {
namespace infer {

class Environment;

// storing all the knowledge is slow
// it only makes sense for us to store it if we are going to use it
// wallk all the instructions and collect knowledge that we may ever need
class KnowledgeFilter {
    std::unordered_set<core::LocalVariable> used_vars;

public:
    KnowledgeFilter(core::Context ctx, std::unique_ptr<cfg::CFG> &cfg);

    KnowledgeFilter(KnowledgeFilter &) = delete;
    KnowledgeFilter(KnowledgeFilter &&) = delete;

    bool isNeeded(core::LocalVariable var);
};

class KnowledgeRef;
/**
 * Encode things that we know hold and don't hold
 */
struct KnowledgeFact {
    bool isDead = false;
    /* the following type tests are known to be true */
    InlinedVector<std::pair<core::LocalVariable, std::shared_ptr<core::Type>>, 1> yesTypeTests;
    /* the following type tests are known to be false */
    InlinedVector<std::pair<core::LocalVariable, std::shared_ptr<core::Type>>, 1> noTypeTests;

    /* this is a "merge" of two knowledges - computes a "lub" of knowledges */
    void min(core::Context ctx, const KnowledgeFact &other);

    /** Computes all possible implications of this knowledge holding as an exit from environment env in block bb
     */
    static KnowledgeRef under(core::Context ctx, const KnowledgeRef &what, const Environment &env, core::Loc loc,
                              cfg::CFG &inWhat, cfg::BasicBlock *bb, bool isNeeded);

    void sanityCheck() const;

    std::string toString(core::Context ctx) const;
};

// KnowledgeRef wraps a `KnowledgeFact` with copy-on-write semantics
class KnowledgeRef {
public:
    KnowledgeRef() : knowledge(std::make_shared<KnowledgeFact>()) {}
    KnowledgeRef(const KnowledgeRef &) = default;
    KnowledgeRef &operator=(const KnowledgeRef &) = default;
    KnowledgeRef(KnowledgeRef &&) = default;
    KnowledgeRef &operator=(KnowledgeRef &&) = default;

    const KnowledgeFact &operator*() const;
    const KnowledgeFact *operator->() const;

    KnowledgeFact &mutate();

private:
    std::shared_ptr<KnowledgeFact> knowledge;
};

/** Almost a named pair of two KnowledgeFact-s. One holds knowledge that is true when a variable is falsy,
 * the other holds knowledge which is true if the same variable is falsy->
 */
class TestedKnowledge {
public:
    KnowledgeRef truthy, falsy;
    bool seenTruthyOption; // Only used during environment merge. Used to indicate "all-knowing" truthy option.
    bool seenFalsyOption;  // Same for falsy

    std::string toString(core::Context ctx) const;

    static TestedKnowledge empty; // optimization

    void sanityCheck() const;
};

class Environment {
public:
    Environment() = default;
    Environment(const Environment &rhs) = delete;
    Environment(Environment &&rhs) = default;

    bool isDead = false;
    cfg::BasicBlock *bb;

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
    UnorderedMap<core::LocalVariable, VariableState> vars;

    UnorderedMap<core::SymbolRef, std::shared_ptr<core::Type>> blockTypes;
    UnorderedMap<core::LocalVariable, core::TypeAndOrigins> pinnedTypes;

    std::string toString(core::Context ctx) const;

    bool hasType(core::Context ctx, core::LocalVariable symbol) const;

    // NB: you can't call this function on vars in the first basic block since
    // their type will be nullptr
    core::TypeAndOrigins getTypeAndOrigin(core::Context ctx, core::LocalVariable symbol) const;

    core::TypeAndOrigins getOrCreateTypeAndOrigin(core::Context ctx, core::LocalVariable symbol);

    const TestedKnowledge &getKnowledge(core::LocalVariable symbol, bool shouldFail = true) const;
    bool getKnownTruthy(core::LocalVariable var) const;

    TestedKnowledge &getKnowledge(core::LocalVariable symbol, bool shouldFail = true) {
        return const_cast<TestedKnowledge &>(const_cast<const Environment *>(this)->getKnowledge(symbol, shouldFail));
    }

    /* propagate knowledge on `to = from` */
    void propagateKnowledge(core::Context ctx, core::LocalVariable to, core::LocalVariable from,
                            KnowledgeFilter &knowledgeFilter);

    /* variable was reasigned. Forget everything about previous value */
    void clearKnowledge(core::Context ctx, core::LocalVariable reassigned, KnowledgeFilter &knowledgeFilter);

    /* Special case sources of knowledge */
    void updateKnowledge(core::Context ctx, core::LocalVariable local, core::Loc loc, cfg::Send *send,
                         KnowledgeFilter &knowledgeFilter);

    void setTypeAndOrigin(core::LocalVariable symbol, core::TypeAndOrigins typeAndOrigins);

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
                                       const UnorderedMap<core::LocalVariable, VariableState> &filter);

    void assumeKnowledge(core::Context ctx, bool isTrue, core::LocalVariable cond, core::Loc loc,
                         const UnorderedMap<core::LocalVariable, VariableState> &filter);

    core::TypeAndOrigins getTypeAndOriginFromOtherEnv(core::Context ctx, core::LocalVariable var,
                                                      const Environment &other);

    void mergeWith(core::Context ctx, const Environment &other, core::Loc loc, cfg::CFG &inWhat, cfg::BasicBlock *bb,
                   KnowledgeFilter &knowledgeFilter);

    void computePins(core::Context ctx, const std::vector<Environment> &envs, const cfg::CFG &inWhat,
                     const cfg::BasicBlock *bb);

    void populateFrom(core::Context ctx, const Environment &other);

    // Extract the return value type from a proc. This should potentially be a
    // method on `Type` or otherwise handled there.
    std::shared_ptr<core::Type> getReturnType(core::Context ctx, std::shared_ptr<core::Type> procType);

    std::shared_ptr<core::Type> processBinding(core::Context ctx, cfg::Binding &bind, int loopCount, int bindMinLoops,
                                               KnowledgeFilter &knowledgeFilter, core::TypeConstraint &constr);

    void ensureGoodCondition(core::Context ctx, core::LocalVariable cond) {}
    void ensureGoodAssignTarget(core::Context ctx, core::LocalVariable target) {}

    void cloneFrom(const Environment &rhs);

private:
    Environment &operator=(const Environment &rhs) = default;
};

} // namespace infer
} // namespace sorbet

#endif // SORBET_ENVIRONMENT_H
