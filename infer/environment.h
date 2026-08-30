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

// A pinned type is only ever consulted when a variable is assigned at a loop depth other than the one it was introduced
// at (see the loop checking in `Environment::processBinding`). Most pins are never consulted: every constant reference
// pins a fresh temporary, and every field is pinned by the alias that introduces it. Recording only the pins that some
// binding can consult keeps the pin table that flows from block to block small.
class PinFilter {
    std::vector<bool> consulted;

public:
    explicit PinFilter(const cfg::CFG &cfg);

    PinFilter(const PinFilter &) = delete;
    PinFilter(PinFilter &&) = delete;

    bool isConsulted(cfg::LocalRef var) const;
};

// The pinned types of an environment, a table shared by the environments of consecutive blocks until one of them pins
// a variable (copy-on-write): most blocks pin nothing, and a method that pins hundreds of fields would otherwise copy
// the table into every one of its blocks.
class PinnedTypes {
    std::shared_ptr<UnorderedMap<cfg::LocalRef, core::TypeAndOrigins>> table;

public:
    bool empty() const {
        return table == nullptr || table->empty();
    }

    // The pin of `var`, or `nullptr`.
    const core::TypeAndOrigins *find(cfg::LocalRef var) const {
        if (table == nullptr) {
            return nullptr;
        }
        auto fnd = table->find(var);
        return fnd == table->end() ? nullptr : &fnd->second;
    }

    void set(cfg::LocalRef var, const core::TypeAndOrigins &tp) {
        if (table == nullptr) {
            table = std::make_shared<UnorderedMap<cfg::LocalRef, core::TypeAndOrigins>>();
        } else if (table.use_count() > 1) {
            table = std::make_shared<UnorderedMap<cfg::LocalRef, core::TypeAndOrigins>>(*table);
        }
        (*table)[var] = tp;
    }

    // Whether `f` holds for every pin.
    template <class F> bool allOf(F f) const {
        if (table == nullptr) {
            return true;
        }
        return absl::c_all_of(*table, [&f](const auto &pair) { return f(pair.first, pair.second); });
    }

    void clear() {
        table = nullptr;
    }
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

    // Empty and alive on both options: knowledge that says nothing at all.
    bool isTrivial() const {
        return isEmpty() && !_truthy->isDead && !_falsy->isDead;
    }
};
CheckSize(TestedKnowledge, 24, 8);

struct VariableState {
    core::TypeAndOrigins typeAndOrigins;
    TestedKnowledge knowledge;
    bool knownTruthy;
};

// The variables an environment holds, with their states. A block's environment holds the variables live into the block
// and those assigned in it, looked up by local; a table indexed by local id with the present locals listed replaces a
// hash map: no hashing, and the per-block passes over the variables walk a contiguous list.
class VariableTable {
    // The states of the present locals, in insertion order, and the locals themselves.
    std::vector<VariableState> states;
    std::vector<cfg::LocalRef> locals;
    // One entry per local of the method: 1 + the index in `states` of a present local, 0 for an absent one.
    std::vector<uint32_t> positions;

public:
    // A present local and its state, as iteration yields them.
    template <class State> struct Entry {
        cfg::LocalRef local;
        State &state;
    };

    template <class Table, class State> class Iterator {
        Table &table;
        size_t index;

    public:
        Iterator(Table &table, size_t index) : table(table), index(index) {}
        bool operator!=(const Iterator &other) const {
            return index != other.index;
        }
        Iterator &operator++() {
            index++;
            return *this;
        }
        Entry<State> operator*() const {
            return Entry<State>{table.locals[index], table.states[index]};
        }
    };

private:
    // References into `states` are handed out and held across insertions, so `states` must never reallocate: it is
    // reserved for every local of the method up front (the memory is not touched until used). A table can hold at
    // most every local once.
    void reserveAll() {
        states.reserve(positions.size());
        locals.reserve(positions.size());
    }

public:
    VariableTable() = default;
    VariableTable(const VariableTable &other) : states(other.states), locals(other.locals), positions(other.positions) {
        reserveAll();
    }
    VariableTable &operator=(const VariableTable &other) {
        states = other.states;
        locals = other.locals;
        positions = other.positions;
        reserveAll();
        return *this;
    }
    VariableTable(VariableTable &&) = default;
    VariableTable &operator=(VariableTable &&) = default;

    // Empties the table and sizes it for the `numLocals` locals of the method.
    void init(size_t numLocals) {
        states.clear();
        locals.clear();
        positions.assign(numLocals, 0);
        reserveAll();
    }

    // Frees the table's storage.
    void release() {
        states = std::vector<VariableState>();
        locals = std::vector<cfg::LocalRef>();
        positions = std::vector<uint32_t>();
    }

    size_t size() const {
        return states.size();
    }

    bool contains(cfg::LocalRef local) const {
        return local.id() < positions.size() && positions[local.id()] != 0;
    }

    VariableState *find(cfg::LocalRef local) {
        return contains(local) ? &states[positions[local.id()] - 1] : nullptr;
    }

    const VariableState *find(cfg::LocalRef local) const {
        return contains(local) ? &states[positions[local.id()] - 1] : nullptr;
    }

    // The state of `local`, inserted (value-initialized) if absent.
    VariableState &operator[](cfg::LocalRef local) {
        ENFORCE(local.id() < positions.size());
        auto &position = positions[local.id()];
        if (position == 0) {
            ENFORCE(states.size() < states.capacity(), "Environment holds more variables than it was sized for");
            states.emplace_back();
            locals.emplace_back(local);
            position = states.size();
        }
        return states[position - 1];
    }

    auto begin() {
        return Iterator<VariableTable, VariableState>(*this, 0);
    }
    auto end() {
        return Iterator<VariableTable, VariableState>(*this, states.size());
    }
    auto begin() const {
        return Iterator<const VariableTable, const VariableState>(*this, 0);
    }
    auto end() const {
        return Iterator<const VariableTable, const VariableState>(*this, states.size());
    }
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

    VariableTable _vars;

    PinnedTypes pinnedTypes;

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

    void cloneFrom(const Environment &rhs);

    core::TypeAndOrigins getTypeFromRebind(core::Context ctx, const core::DispatchComponent &main,
                                           cfg::LocalRef fallback);

public:
    Environment(core::Loc ownerLoc);
    Environment(const Environment &rhs) = delete;
    Environment(Environment &&rhs) = default;

    bool isDead = false;
    cfg::BasicBlock *bb;

    const VariableTable &vars() const {
        return _vars;
    }

    void initializeBasicBlockArgs(const cfg::BasicBlock &bb, size_t numLocals);

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
                                       const VariableTable &filter);

    void mergeWith(core::Context ctx, const Environment &other, cfg::CFG &inWhat, cfg::BasicBlock *bb,
                   KnowledgeFilter &knowledgeFilter, LocalMarks &marks);

    void computePins(core::Context ctx, const std::vector<Environment> &envs, const cfg::CFG &inWhat,
                     const cfg::BasicBlock *bb);

    void populateFrom(core::Context ctx, const Environment &other);

    // Frees the variable and pin tables. Once every successor block has been processed nothing reads them again; only
    // `isDead` is consulted afterwards.
    void release();

    // Narrows the variables of `filter`, and the condition itself if this environment holds it, under the exit
    // condition of `source`'s block being `isTrue`. The condition's type, truthiness and knowledge are read from
    // `source`; the variables narrowed are this environment's, which must hold the same types as `source` for the
    // variables of `filter`: a clone of `source` (see `withCond`), or a block's own environment just populated from
    // its only predecessor.
    void assumeKnowledge(core::Context ctx, const Environment &source, bool isTrue, const VariableTable &filter);

    core::TypePtr
    processBinding(core::Context ctx, const cfg::CFG &inWhat, cfg::Binding &bind, int loopCount, int bindMinLoops,
                   KnowledgeFilter &knowledgeFilter, const PinFilter &pinFilter, core::TypeConstraint &constr,
                   const core::TypePtr &methodReturnType,
                   const std::optional<cfg::BasicBlock::BlockExitCondInfo> &parentUpdateKnowledgeReceiver);

    core::Loc locForUninitialized() const {
        return ownerLoc;
    }
};

} // namespace sorbet::infer

#endif // SORBET_ENVIRONMENT_H
