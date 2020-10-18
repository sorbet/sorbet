#include "environment.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "core/GlobalState.h"
#include "core/TypeConstraint.h"
#include <algorithm> // find, remove_if

template struct std::pair<sorbet::core::LocalVariable, std::shared_ptr<sorbet::core::Type>>;

using namespace std;

namespace sorbet::infer {

namespace {
core::TypePtr dropConstructor(core::Context ctx, core::Loc loc, core::TypePtr tp) {
    if (auto *mt = core::cast_type<core::MetaType>(tp.get())) {
        if (!mt->wrapped->isUntyped()) {
            if (auto e = ctx.state.beginError(loc, core::errors::Infer::BareTypeUsage)) {
                e.setHeader("Unsupported usage of bare type");
            }
        }
        return core::Types::untypedUntracked();
    }
    return tp;
}

bool typeTestReferencesVar(const InlinedVector<std::pair<cfg::LocalRef, core::TypePtr>, 1> &typeTest,
                           cfg::LocalRef var) {
    return absl::c_any_of(typeTest, [var](auto &test) { return test.first == var; });
}
} // namespace

cfg::LocalRef LocalRefRef::data(const Environment &env) const {
    ENFORCE_NO_TIMER(this->exists());
    return env.vars[_id];
}

const TestedKnowledge &LocalRefRef::knowledge(const Environment &env) const {
    const auto &state = env._varState[_id];
    return state.knowledge;
}

const core::TypeAndOrigins &LocalRefRef::typeAndOrigin(const Environment &env) const {
    const auto &state = env._varState[_id];
    auto &origin = state.typeAndOrigins;
    if (origin.type.get() == nullptr) {
        return env.uninitialized;
    }
    return origin;
}

bool LocalRefRef::hasType(const Environment &env) const {
    const auto &state = env._varState[_id];
    return state.typeAndOrigins.type.get() != nullptr;
}

bool LocalRefRef::knownTruthy(const Environment &env) const {
    const auto &state = env._varState[_id];
    return state.knownTruthy;
}

bool LocalRefRef::exists() const {
    return _id > 0;
}
bool LocalRefRef::operator==(const LocalRefRef &rhs) const {
    return this->_id == rhs._id;
}

bool LocalRefRef::operator!=(const LocalRefRef &rhs) const {
    return this->_id != rhs._id;
}

void TypeTestReverseIndex::addToIndex(cfg::LocalRef from, LocalRefRef to) {
    auto &list = index[from];
    // Maintain invariant: lists are sorted sets (no duplicate entries)
    // lower_bound returns the first location that is >= to's ID, which is where `to` should be inserted if *it != to
    // (uses binary search, O(log n))
    auto it = absl::c_lower_bound(list, to, [](const auto &a, const auto &b) -> bool { return a.id() < b.id(); });
    if (it == list.end() || *it != to) {
        list.insert(it, to);
    }
}

const InlinedVector<LocalRefRef, 1> &TypeTestReverseIndex::get(cfg::LocalRef from) const {
    auto it = index.find(from);
    if (it == index.end()) {
        return empty;
    }
    return it->second;
}

void TypeTestReverseIndex::replace(cfg::LocalRef from, InlinedVector<LocalRefRef, 1> &&list) {
    index[from] = std::move(list);
}

void TypeTestReverseIndex::cloneFrom(const TypeTestReverseIndex &index) {
    this->index = index.index;
}

/**
 * Encode things that we know hold and don't hold.
 */
struct KnowledgeFact {
    bool isDead = false;
    /* the following type tests are known to be true */
    InlinedVector<std::pair<cfg::LocalRef, core::TypePtr>, 1> yesTypeTests;
    /* the following type tests are known to be false */
    InlinedVector<std::pair<cfg::LocalRef, core::TypePtr>, 1> noTypeTests;

    /* this is a "merge" of two knowledges - computes a "lub" of knowledges */
    void min(core::Context ctx, const KnowledgeFact &other);

    void sanityCheck() const;

    std::string toString(const core::GlobalState &gs, const cfg::CFG &cfg) const;
};

KnowledgeFilter::KnowledgeFilter(core::Context ctx, unique_ptr<cfg::CFG> &cfg) {
    used_vars.resize(cfg->numLocalVariables());
    for (auto &bb : cfg->basicBlocks) {
        ENFORCE(bb->bexit.cond.variable.exists());
        if (bb->bexit.cond.variable != cfg::LocalRef::unconditional() &&
            bb->bexit.cond.variable != cfg::LocalRef::blockCall()) {
            used_vars[bb->bexit.cond.variable.id()] = true;
        }
    }
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto &bb : cfg->forwardsTopoSort) {
            for (auto &bind : bb->exprs) {
                if (auto *id = cfg::cast_instruction<cfg::Ident>(bind.value.get())) {
                    if (isNeeded(bind.bind.variable) && !isNeeded(id->what)) {
                        used_vars[id->what.id()] = true;
                        changed = true;
                    }
                } else if (auto *send = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
                    if (send->fun == core::Names::bang()) {
                        if (send->args.empty()) {
                            if (isNeeded(bind.bind.variable) && !isNeeded(send->recv.variable)) {
                                used_vars[send->recv.variable.id()] = true;
                                changed = true;
                            }
                        }
                    } else if (send->fun == core::Names::eqeq()) {
                        if (send->args.size() == 1) {
                            if (isNeeded(send->args[0].variable) && !isNeeded(send->recv.variable)) {
                                used_vars[send->recv.variable.id()] = true;
                                changed = true;
                            } else if (isNeeded(send->recv.variable) && !isNeeded(send->args[0].variable)) {
                                used_vars[send->args[0].variable.id()] = true;
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }

    if (debug_mode) {
        int count = 0;
        for (auto used_var : used_vars) {
            if (used_var) {
                count++;
            }
        }
        histogramInc("knowledgefilter.used_vars", count);
    }
}

bool KnowledgeFilter::isNeeded(cfg::LocalRef var) {
    return used_vars[var.id()];
}

KnowledgeRef KnowledgeRef::under(core::Context ctx, const Environment &env, core::Loc loc, cfg::CFG &inWhat,
                                 cfg::BasicBlock *bb, bool isNeeded) const {
    if (knowledge->yesTypeTests.empty() && !isNeeded) {
        return *this;
    }
    KnowledgeRef copy = *this;
    if (env.isDead) {
        copy.markDead();
        return copy;
    }
    bool enteringLoop = (bb->flags & cfg::CFG::LOOP_HEADER) != 0;
    int i = 0;
    for (auto &state : env.varState()) {
        auto local = LocalRefRef(i++);
        if (!local.exists()) {
            continue;
        }

        auto localRef = local.data(env);
        if (enteringLoop && bb->outerLoops <= localRef.maxLoopWrite(inWhat)) {
            continue;
        }

        auto fnd = absl::c_find_if(copy->yesTypeTests, [&](auto const &e) -> bool { return e.first == localRef; });
        if (fnd == copy->yesTypeTests.end()) {
            // add info from env to knowledge
            ENFORCE(state.typeAndOrigins.type.get() != nullptr);
            // This handles code snippets such as
            //
            //    if (...)
            //      v = true
            //      s = 1
            //    else
            //      v = false
            //      s = nil
            //    end
            //
            //    if (v)
            //      puts(s + 1)
            //    end
            //
            // This pattern is relatively rare in user code, but it ends up
            // being important inside the desugaring of `||` and `&&` as well as
            // in inferring results from a `hard_assert`
            //
            // Adding this information makes environments much larger than they
            // would be otherwise; Many of the performance optimizations in this
            // file effectively exist to support this feature.

            auto type = state.typeAndOrigins.type;
            if (isNeeded && !type->isUntyped() && !core::isa_type<core::MetaType>(type.get())) {
                // Direct mutation of `yesTypeTests` rather than going through `addYesTypeTest`.
                // This is fine since `copy` is unmoored from a particular environment.
                copy.mutate().yesTypeTests.emplace_back(localRef, type);
            }
        } else {
            auto &second = fnd->second;
            auto &typeAndOrigin = state.typeAndOrigins;
            auto combinedType = core::Types::all(ctx, typeAndOrigin.type, second);
            if (combinedType->isBottom()) {
                copy.markDead();
                break;
            }
        }
    }
    return copy;
}

void KnowledgeFact::min(core::Context ctx, const KnowledgeFact &other) {
    for (auto it = yesTypeTests.begin(); it != yesTypeTests.end(); /* nothing */) {
        auto &entry = *it;
        cfg::LocalRef local = entry.first;
        auto fnd = absl::c_find_if(other.yesTypeTests, [&](auto const &elem) -> bool { return elem.first == local; });
        if (fnd == other.yesTypeTests.end()) {
            // Note: No need to update Environment::typeTestsWithVar since it is an overapproximation
            it = yesTypeTests.erase(it);
        } else {
            entry.second = core::Types::any(ctx, fnd->second, entry.second);
            it++;
        }
    }
    for (auto it = noTypeTests.begin(); it != noTypeTests.end(); /* nothing */) {
        auto &entry = *it;
        cfg::LocalRef local = entry.first;
        auto fnd = absl::c_find_if(other.noTypeTests, [&](auto const &elem) -> bool { return elem.first == local; });
        if (fnd == other.noTypeTests.end()) {
            // Note: No need to update Environment::typeTestsWithVar since it is an overapproximation
            it = noTypeTests.erase(it);
        } else {
            entry.second = core::Types::all(ctx, fnd->second, entry.second);
            it++;
        }
    }
}

void KnowledgeFact::sanityCheck() const {
    if (!debug_mode) {
        return;
    }
    for (auto &a : yesTypeTests) {
        ENFORCE(a.second.get() != nullptr);
        ENFORCE(!a.second->isUntyped());
    }
    for (auto &a : noTypeTests) {
        ENFORCE(a.second.get() != nullptr);
        ENFORCE(!a.second->isUntyped());
    }
}

string KnowledgeFact::toString(const core::GlobalState &gs, const cfg::CFG &cfg) const {
    vector<string> buf1, buf2;

    for (auto &el : yesTypeTests) {
        buf1.emplace_back(
            fmt::format("    {} to be {}\n", el.first.showRaw(gs, cfg), el.second->toStringWithTabs(gs, 0)));
    }
    for (auto &el : noTypeTests) {
        buf2.emplace_back(
            fmt::format("    {} NOT to be {}\n", el.first.showRaw(gs, cfg), el.second->toStringWithTabs(gs, 0)));
    }
    fast_sort(buf1);
    fast_sort(buf2);

    return fmt::format("{}{}", fmt::join(buf1, ""), fmt::join(buf2, ""));
}

const shared_ptr<KnowledgeFact> KnowledgeRef::empty = make_shared<KnowledgeFact>();

KnowledgeRef::KnowledgeRef() : knowledge(KnowledgeRef::empty) {}

const KnowledgeFact &KnowledgeRef::operator*() const {
    return *knowledge.get();
}

const KnowledgeFact *KnowledgeRef::operator->() const {
    return knowledge.get();
}

KnowledgeFact &KnowledgeRef::mutate() {
    if (knowledge.use_count() > 1) {
        knowledge = make_shared<KnowledgeFact>(*knowledge);
    }
    ENFORCE(knowledge.use_count() == 1);
    return *knowledge.get();
}

void KnowledgeRef::addYesTypeTest(LocalRefRef of, TypeTestReverseIndex &index, cfg::LocalRef ref, core::TypePtr type) {
    index.addToIndex(ref, of);
    this->mutate().yesTypeTests.emplace_back(ref, type);
}

void KnowledgeRef::addNoTypeTest(LocalRefRef of, TypeTestReverseIndex &index, cfg::LocalRef ref, core::TypePtr type) {
    index.addToIndex(ref, of);
    this->mutate().noTypeTests.emplace_back(ref, type);
}

void KnowledgeRef::markDead() {
    this->mutate().isDead = true;
}

void KnowledgeRef::min(core::Context ctx, const KnowledgeFact &other) {
    this->mutate().min(ctx, other);
}

void KnowledgeRef::removeReferencesToVar(cfg::LocalRef var) {
    if (typeTestReferencesVar(knowledge->yesTypeTests, var) || typeTestReferencesVar(knowledge->noTypeTests, var)) {
        auto &typeTests = this->mutate();
        // No requirement to update Environment::typeTestsWithVar, which is an overapproximation.
        typeTests.yesTypeTests.erase(remove_if(typeTests.yesTypeTests.begin(), typeTests.yesTypeTests.end(),
                                               [&](auto const &c) -> bool { return c.first == var; }),
                                     typeTests.yesTypeTests.end());
        typeTests.noTypeTests.erase(remove_if(typeTests.noTypeTests.begin(), typeTests.noTypeTests.end(),
                                              [&](auto const &c) -> bool { return c.first == var; }),
                                    typeTests.noTypeTests.end());
    }
}

string TestedKnowledge::toString(const core::GlobalState &gs, const cfg::CFG &cfg) const {
    fmt::memory_buffer buf;
    if (!_truthy->noTypeTests.empty() || !_truthy->yesTypeTests.empty()) {
        fmt::format_to(buf, "  Being truthy entails:\n{}", _truthy->toString(gs, cfg));
    }
    if (!_falsy->noTypeTests.empty() || !_falsy->yesTypeTests.empty()) {
        fmt::format_to(buf, "  Being falsy entails:\n{}", _falsy->toString(gs, cfg));
    }
    return to_string(buf);
}

void TestedKnowledge::replaceTruthy(LocalRefRef of, TypeTestReverseIndex &index, const KnowledgeRef &newTruthy) {
    // Note: No need to remove old items from Environment::typeTestsWithVar since it is an overapproximation
    for (auto &entry : newTruthy->yesTypeTests) {
        index.addToIndex(entry.first, of);
    }

    for (auto &entry : newTruthy->noTypeTests) {
        index.addToIndex(entry.first, of);
    }

    _truthy = newTruthy;
}

void TestedKnowledge::replaceFalsy(LocalRefRef of, TypeTestReverseIndex &index, const KnowledgeRef &newFalsy) {
    // Note: No need to remove old items from Environment::typeTestsWithVar since it is an overapproximation
    for (auto &entry : newFalsy->yesTypeTests) {
        index.addToIndex(entry.first, of);
    }

    for (auto &entry : newFalsy->noTypeTests) {
        index.addToIndex(entry.first, of);
    }

    _falsy = newFalsy;
}

void TestedKnowledge::replace(LocalRefRef of, TypeTestReverseIndex &index, const TestedKnowledge &knowledge) {
    this->seenTruthyOption = knowledge.seenTruthyOption;
    this->seenFalsyOption = knowledge.seenFalsyOption;
    replaceFalsy(of, index, knowledge.falsy());
    replaceTruthy(of, index, knowledge.truthy());
}

void TestedKnowledge::removeReferencesToVar(cfg::LocalRef var) {
    this->_truthy.removeReferencesToVar(var);
    this->_falsy.removeReferencesToVar(var);
}

void TestedKnowledge::emitKnowledgeSizeMetric() const {
    histogramInc("infer.knowledge.truthy.yes.size", _truthy->yesTypeTests.size());
    histogramInc("infer.knowledge.truthy.no.size", _truthy->noTypeTests.size());
    histogramInc("infer.knowledge.falsy.yes.size", _falsy->yesTypeTests.size());
    histogramInc("infer.knowledge.falsy.no.size", _falsy->noTypeTests.size());
}

void TestedKnowledge::sanityCheck() const {
    if (!debug_mode) {
        return;
    }
    _truthy->sanityCheck();
    _falsy->sanityCheck();
    ENFORCE(TestedKnowledge::empty._falsy->yesTypeTests.empty());
    ENFORCE(TestedKnowledge::empty._falsy->noTypeTests.empty());
    ENFORCE(TestedKnowledge::empty._truthy->noTypeTests.empty());
    ENFORCE(TestedKnowledge::empty._truthy->yesTypeTests.empty());
}

void Environment::enterLocalInternal(cfg::LocalRef ref, LocalRefRef &refRef) {
    // TODO: Enforce not frozen?
    int id = this->vars.size();
    this->vars.emplace_back(ref);
    this->_varState.emplace_back();

    ENFORCE(this->vars.size() == this->_varState.size());
    refRef = LocalRefRef(id);
}

LocalRefRef Environment::enterLocal(cfg::LocalRef ref) {
    auto &refRef = this->definedVars[ref.id()];
    if (!refRef.exists() && ref.exists()) {
        // ref is an out parameter.
        enterLocalInternal(ref, refRef);
    }
    return refRef;
}

LocalRefRef Environment::lookupLocal(cfg::LocalRef ref) const {
    // Will be the non-existant LocalRefRef if not defined.
    return this->definedVars[ref.id()];
}

string Environment::toString(const core::GlobalState &gs, const cfg::CFG &cfg) const {
    fmt::memory_buffer buf;
    if (isDead) {
        fmt::format_to(buf, "dead={:d}\n", isDead);
    }
    vector<pair<LocalRefRef, VariableState>> sorted;
    int i = 0;
    for (const auto &state : _varState) {
        LocalRefRef ref(i++);
        if (ref.exists()) {
            sorted.emplace_back(ref, state);
        }
    }
    fast_sort(sorted, [&cfg, this](const auto &lhs, const auto &rhs) -> bool {
        return lhs.first.data(*this).data(cfg)._name.id() < rhs.first.data(*this).data(cfg)._name.id() ||
               (lhs.first.data(*this).data(cfg)._name.id() == rhs.first.data(*this).data(cfg)._name.id() &&
                lhs.first.data(*this).data(cfg).unique < rhs.first.data(*this).data(cfg).unique);
    });
    for (const auto &pair : sorted) {
        auto var = pair.first;
        auto &state = pair.second;
        if (var.data(*this).data(cfg)._name == core::Names::debugEnvironmentTemp()) {
            continue;
        }
        fmt::format_to(buf, "{}: {}{}\n{}\n", var.data(*this).showRaw(gs, cfg),
                       state.typeAndOrigins.type->toStringWithTabs(gs, 0), state.knownTruthy ? " (and truthy)\n" : "",
                       state.knowledge.toString(gs, cfg));
    }
    return to_string(buf);
}

const core::TypeAndOrigins &Environment::getAndFillTypeAndOrigin(core::Context ctx,
                                                                 cfg::VariableUseSite &symbol) const {
    auto ref = lookupLocal(symbol.variable);
    auto &tp = ref.typeAndOrigin(*this);
    symbol.type = tp.type;
    return tp;
}

void Environment::propagateKnowledge(core::Context ctx, LocalRefRef to, cfg::LocalRef fromLocalRef,
                                     KnowledgeFilter &knowledgeFilter) {
    auto toLocalRef = to.data(*this);
    if (knowledgeFilter.isNeeded(to.data(*this)) && knowledgeFilter.isNeeded(fromLocalRef)) {
        auto from = enterLocal(fromLocalRef);
        auto &fromState = _varState[from.id()];
        auto &toState = _varState[to.id()];
        toState.knownTruthy = fromState.knownTruthy;
        auto &toKnowledge = toState.knowledge;
        auto &fromKnowledge = fromState.knowledge;
        if (fromState.typeAndOrigins.type.get() == nullptr) {
            fromState.typeAndOrigins.type = core::Types::nilClass();
        }
        if (toState.typeAndOrigins.type.get() == nullptr) {
            toState.typeAndOrigins.type = core::Types::nilClass();
        }

        // Copy properties from fromKnowledge to toKnowledge
        toKnowledge.replace(to, typeTestsWithVar, fromKnowledge);

        toKnowledge.truthy().addNoTypeTest(to, typeTestsWithVar, fromLocalRef, core::Types::falsyTypes());
        toKnowledge.falsy().addYesTypeTest(to, typeTestsWithVar, fromLocalRef, core::Types::falsyTypes());
        fromKnowledge.truthy().addNoTypeTest(from, typeTestsWithVar, toLocalRef, core::Types::falsyTypes());
        fromKnowledge.falsy().addYesTypeTest(from, typeTestsWithVar, toLocalRef, core::Types::falsyTypes());
        fromKnowledge.sanityCheck();
        toKnowledge.sanityCheck();
    }
}

void Environment::clearKnowledge(core::Context ctx, LocalRefRef reassigned, KnowledgeFilter &knowledgeFilter) {
    auto &typesWithReassigned = typeTestsWithVar.get(reassigned.data(*this));
    if (!typesWithReassigned.empty()) {
        InlinedVector<LocalRefRef, 1> replacement;
        for (auto &var : typesWithReassigned) {
            if (knowledgeFilter.isNeeded(var.data(*this))) {
                auto &k = _varState[var.id()].knowledge;
                k.removeReferencesToVar(reassigned.data(*this));
                k.sanityCheck();
            } else {
                replacement.emplace_back(var);
            }
        }
        typeTestsWithVar.replace(reassigned.data(*this), std::move(replacement));
    }

    ENFORCE_NO_TIMER(reassigned.id() < _varState.size());
    auto &state = _varState[reassigned.id()];
    state.knownTruthy = false;
}

namespace {

// A singleton is a class which is inhabited by a single value.
//
// If a variable != a value whose type is a singleton, then that variable's type must not be that value's type.
//
// This is not the case for most other equality tests. e.g., x != 0 does not imply ¬ x : Integer.
//
// This powers (among other things) exhaustiveness checking for T::Enum.
bool isSingleton(core::Context ctx, core::SymbolRef sym) {
    // Singletons that are built into the Ruby VM
    if (sym == core::Symbols::NilClass() || sym == core::Symbols::FalseClass() || sym == core::Symbols::TrueClass()) {
        return true;
    }

    // T::Enum values are modeled as singletons of their own fake class.
    if (sym.data(ctx)->name.data(ctx)->isTEnumName(ctx)) {
        return true;
    }

    // The Ruby stdlib has a Singleton module which lets people invent their own singletons.
    return (sym.data(ctx)->derivesFrom(ctx, core::Symbols::Singleton()) && sym.data(ctx)->isClassOrModuleFinal());
}

} // namespace

void Environment::updateKnowledge(core::Context ctx, LocalRefRef ref, core::Loc loc, const cfg::Send *send,
                                  KnowledgeFilter &knowledgeFilter) {
    ENFORCE_NO_TIMER(ref.exists());
    if (!knowledgeFilter.isNeeded(ref.data(*this))) {
        return;
    }

    if (send->fun == core::Names::bang()) {
        auto &whoKnows = _varState[ref.id()].knowledge;
        auto recvVar = lookupLocal(send->recv.variable);
        if (recvVar.exists()) {
            auto &k = _varState[recvVar.id()].knowledge;
            whoKnows.replaceTruthy(ref, typeTestsWithVar, k.falsy());
            whoKnows.replaceFalsy(ref, typeTestsWithVar, k.truthy());
            k.truthy().addYesTypeTest(recvVar, typeTestsWithVar, ref.data(*this), core::Types::falsyTypes());
            k.falsy().addNoTypeTest(recvVar, typeTestsWithVar, ref.data(*this), core::Types::falsyTypes());
        }
        whoKnows.truthy().addYesTypeTest(ref, typeTestsWithVar, send->recv.variable, core::Types::falsyTypes());
        whoKnows.falsy().addNoTypeTest(ref, typeTestsWithVar, send->recv.variable, core::Types::falsyTypes());

        whoKnows.sanityCheck();
    } else if (send->fun == core::Names::nil_p()) {
        auto &whoKnows = _varState[ref.id()].knowledge;
        whoKnows.truthy().addYesTypeTest(ref, typeTestsWithVar, send->recv.variable, core::Types::nilClass());
        whoKnows.falsy().addNoTypeTest(ref, typeTestsWithVar, send->recv.variable, core::Types::nilClass());
        whoKnows.sanityCheck();
    } else if (send->fun == core::Names::blank_p()) {
        // Note that this assumes that .blank? is a rails-compatible monkey patch.
        // In other cases this flow analysis might make incorrect assumptions.
        auto &originalType = send->recv.type;
        auto knowledgeTypeWithoutFalsy = core::Types::approximateSubtract(ctx, originalType, core::Types::falsyTypes());

        if (!core::Types::equiv(ctx, knowledgeTypeWithoutFalsy, originalType)) {
            auto &whoKnows = _varState[ref.id()].knowledge;
            whoKnows.falsy().addYesTypeTest(ref, typeTestsWithVar, send->recv.variable, knowledgeTypeWithoutFalsy);
            whoKnows.sanityCheck();
        }
    } else if (send->fun == core::Names::present_p()) {
        // Note that this assumes that .present? is a rails-compatible monkey patch.
        // In other cases this flow analysis might make incorrect assumptions.
        auto &originalType = send->recv.type;
        auto knowledgeTypeWithoutFalsy = core::Types::approximateSubtract(ctx, originalType, core::Types::falsyTypes());

        if (!core::Types::equiv(ctx, knowledgeTypeWithoutFalsy, originalType)) {
            auto &whoKnows = _varState[ref.id()].knowledge;
            whoKnows.truthy().addYesTypeTest(ref, typeTestsWithVar, send->recv.variable, knowledgeTypeWithoutFalsy);
            whoKnows.sanityCheck();
        }
    }

    if (send->args.empty()) {
        return;
    }
    // TODO(jez) We should probably update this to be aware of T::NonForcingConstants.non_forcing_is_a?
    if (send->fun == core::Names::kindOf_p() || send->fun == core::Names::isA_p()) {
        auto &whoKnows = _varState[ref.id()].knowledge;
        auto &klassType = send->args[0].type;
        core::SymbolRef klass = core::Types::getRepresentedClass(ctx, klassType.get());
        if (klass.exists()) {
            auto ty = klass.data(ctx)->externalType(ctx);
            if (!ty->isUntyped()) {
                whoKnows.truthy().addYesTypeTest(ref, typeTestsWithVar, send->recv.variable, ty);
                whoKnows.falsy().addNoTypeTest(ref, typeTestsWithVar, send->recv.variable, ty);
            }
            whoKnows.sanityCheck();
        }
    } else if (send->fun == core::Names::eqeq() || send->fun == core::Names::neq()) {
        auto &whoKnows = _varState[ref.id()].knowledge;
        const auto &argType = send->args[0].type;
        const auto &recvType = send->recv.type;

        auto &truthy = send->fun == core::Names::eqeq() ? whoKnows.truthy() : whoKnows.falsy();
        auto &falsy = send->fun == core::Names::eqeq() ? whoKnows.falsy() : whoKnows.truthy();

        ENFORCE(argType.get() != nullptr);
        ENFORCE(recvType.get() != nullptr);
        if (!argType->isUntyped()) {
            truthy.addYesTypeTest(ref, typeTestsWithVar, send->recv.variable, argType);
        }
        if (!recvType->isUntyped()) {
            truthy.addYesTypeTest(ref, typeTestsWithVar, send->args[0].variable, recvType);
        }
        if (auto s = core::cast_type<core::ClassType>(argType.get())) {
            // check if s is a singleton. in this case we can learn that
            // a failed comparison means that type test would also fail
            if (isSingleton(ctx, s->symbol)) {
                falsy.addNoTypeTest(ref, typeTestsWithVar, send->recv.variable, argType);
            }
        }
        if (auto s = core::cast_type<core::ClassType>(recvType.get())) {
            // check if s is a singleton. in this case we can learn that
            // a failed comparison means that type test would also fail
            if (isSingleton(ctx, s->symbol)) {
                falsy.addNoTypeTest(ref, typeTestsWithVar, send->args[0].variable, recvType);
            }
        }
        whoKnows.sanityCheck();
    } else if (send->fun == core::Names::tripleEq()) {
        auto &whoKnows = _varState[ref.id()].knowledge;
        const auto &recvType = send->recv.type;

        // `when` against class literal
        core::SymbolRef representedClass = core::Types::getRepresentedClass(ctx, recvType.get());
        if (representedClass.exists()) {
            auto representedType = representedClass.data(ctx)->externalType(ctx);
            if (!representedType->isUntyped()) {
                whoKnows.truthy().addYesTypeTest(ref, typeTestsWithVar, send->args[0].variable, representedType);
                whoKnows.falsy().addNoTypeTest(ref, typeTestsWithVar, send->args[0].variable, representedType);
            }
        }

        // `when` against singleton
        if (auto s = core::cast_type<core::ClassType>(recvType.get())) {
            // check if s is a singleton. in this case we can learn that
            // a failed comparison means that type test would also fail
            if (isSingleton(ctx, s->symbol)) {
                whoKnows.truthy().addYesTypeTest(ref, typeTestsWithVar, send->args[0].variable, recvType);
                whoKnows.falsy().addNoTypeTest(ref, typeTestsWithVar, send->args[0].variable, recvType);
            }
        }
        whoKnows.sanityCheck();

    } else if (send->fun == core::Names::lessThan()) {
        const auto &recvKlass = send->recv.type;
        const auto &argType = send->args[0].type;

        if (auto *argClass = core::cast_type<core::ClassType>(argType.get())) {
            if (!recvKlass->derivesFrom(ctx, core::Symbols::Class()) ||
                !argClass->symbol.data(ctx)->derivesFrom(ctx, core::Symbols::Class())) {
                return;
            }
        } else if (auto *argClass = core::cast_type<core::AppliedType>(argType.get())) {
            if (!recvKlass->derivesFrom(ctx, core::Symbols::Class()) ||
                !argClass->klass.data(ctx)->derivesFrom(ctx, core::Symbols::Class())) {
                return;
            }
        } else {
            return;
        }

        auto &whoKnows = _varState[ref.id()].knowledge;
        whoKnows.truthy().addYesTypeTest(ref, typeTestsWithVar, send->recv.variable, argType);
        whoKnows.falsy().addNoTypeTest(ref, typeTestsWithVar, send->recv.variable, argType);
        whoKnows.sanityCheck();
    }
}

void Environment::setTypeAndOrigin(LocalRefRef symbol, const core::TypeAndOrigins &typeAndOrigins) {
    ENFORCE_NO_TIMER(symbol.exists());
    ENFORCE(typeAndOrigins.type.get() != nullptr);
    _varState[symbol.id()].typeAndOrigins = typeAndOrigins;
}

const Environment &Environment::withCond(core::Context ctx, const Environment &env, Environment &copy, bool isTrue,
                                         const Environment &filter) {
    ENFORCE(env.bb->bexit.cond.variable.exists());
    if (env.bb->bexit.cond.variable == cfg::LocalRef::unconditional() ||
        env.bb->bexit.cond.variable == cfg::LocalRef::blockCall()) {
        return env;
    }
    copy.cloneFrom(env);
    copy.assumeKnowledge(ctx, isTrue, env.bb->bexit.cond.variable, core::Loc(ctx.file, env.bb->bexit.loc), filter);
    return copy;
}

void Environment::assumeKnowledge(core::Context ctx, bool isTrue, cfg::LocalRef cond, core::Loc loc,
                                  const Environment &filter) {
    // Note: We don't _enterLocal_ unless we are definitively setting its type and origin
    auto ref = lookupLocal(cond);
    ref.knowledge(*this).sanityCheck();

    {
        core::TypeAndOrigins tp = ref.typeAndOrigin(*this);

        if (!isTrue) {
            if (ref.knownTruthy(*this)) {
                isDead = true;
                return;
            }

            tp.origins.emplace_back(loc);
            if (tp.type->isUntyped()) {
                tp.type = core::Types::falsyTypes();
            } else {
                tp.type = core::Types::all(ctx, tp.type, core::Types::falsyTypes());
                if (tp.type->isBottom()) {
                    isDead = true;
                    return;
                }
            }
            if (!ref.exists()) {
                ref = enterLocal(cond);
            }
            setTypeAndOrigin(ref, tp);
        } else {
            tp.origins.emplace_back(loc);
            tp.type = core::Types::dropSubtypesOf(
                ctx, core::Types::dropSubtypesOf(ctx, tp.type, core::Symbols::NilClass()), core::Symbols::FalseClass());
            if (tp.type->isBottom()) {
                isDead = true;
                return;
            }
            if (!ref.exists()) {
                ref = enterLocal(cond);
            }
            setTypeAndOrigin(ref, tp);
            _varState[ref.id()].knownTruthy = true;
        }
    }

    if (isDead) {
        return;
    }

    auto &knowledgeToChoose = isTrue ? ref.knowledge(*this).truthy() : ref.knowledge(*this).falsy();
    auto &yesTests = knowledgeToChoose->yesTypeTests;
    auto &noTests = knowledgeToChoose->noTypeTests;

    for (auto &typeTested : yesTests) {
        if (!filter.lookupLocal(typeTested.first).exists()) {
            continue;
        }

        auto ref = enterLocal(typeTested.first);
        core::TypeAndOrigins tp = ref.typeAndOrigin(*this);
        auto glbbed = core::Types::all(ctx, tp.type, typeTested.second);
        if (tp.type != glbbed) {
            tp.origins.emplace_back(loc);
            tp.type = glbbed;
        }

        setTypeAndOrigin(ref, tp);
        if (tp.type->isBottom()) {
            isDead = true;
            return;
        }
    }

    for (auto &typeTested : noTests) {
        if (!filter.lookupLocal(typeTested.first).exists()) {
            continue;
        }
        auto ref = lookupLocal(typeTested.first);
        core::TypeAndOrigins tp = ref.typeAndOrigin(*this);
        tp.origins.emplace_back(loc);

        if (!tp.type->isUntyped()) {
            tp.type = core::Types::approximateSubtract(ctx, tp.type, typeTested.second);
            if (!ref.exists()) {
                ref = enterLocal(typeTested.first);
            }
            setTypeAndOrigin(ref, tp);
            if (tp.type->isBottom()) {
                isDead = true;
                return;
            }
        }
    }
}

void Environment::mergeWith(core::Context ctx, const Environment &other, core::Loc loc, cfg::CFG &inWhat,
                            cfg::BasicBlock *bb, KnowledgeFilter &knowledgeFilter) {
    this->isDead |= other.isDead;
    int i = 0;
    for (auto &state : _varState) {
        auto thisRef = LocalRefRef(i++);
        if (!thisRef.exists()) {
            continue;
        }
        auto varLocalRef = thisRef.data(*this);
        auto otherRef = other.lookupLocal(varLocalRef);
        const auto &otherTO = otherRef.typeAndOrigin(other);
        auto &thisTO = state.typeAndOrigins;
        if (thisTO.type.get() != nullptr) {
            thisTO.type = core::Types::any(ctx, thisTO.type, otherTO.type);
            thisTO.type->sanityCheck(ctx);
            for (auto origin : otherTO.origins) {
                if (!absl::c_linear_search(thisTO.origins, origin)) {
                    thisTO.origins.emplace_back(origin);
                }
            }
            state.knownTruthy = state.knownTruthy && otherRef.knownTruthy(other);
        } else {
            thisTO = otherTO;
            state.knownTruthy = otherRef.knownTruthy(other);
        }

        if (((bb->flags & cfg::CFG::LOOP_HEADER) != 0) && bb->outerLoops <= varLocalRef.maxLoopWrite(inWhat)) {
            continue;
        }
        bool canBeFalsy = !otherRef.knownTruthy(other) && core::Types::canBeFalsy(ctx, otherTO.type);
        bool canBeTruthy = core::Types::canBeTruthy(ctx, otherTO.type);

        if (canBeTruthy) {
            auto &thisKnowledge = _varState[thisRef.id()].knowledge;
            const auto &otherTruthy = otherRef.knowledge(other).truthy().under(ctx, other, loc, inWhat, bb,
                                                                               knowledgeFilter.isNeeded(varLocalRef));
            if (!otherTruthy->isDead) {
                if (!thisKnowledge.seenTruthyOption) {
                    thisKnowledge.seenTruthyOption = true;
                    thisKnowledge.replaceTruthy(thisRef, typeTestsWithVar, otherTruthy);
                } else {
                    thisKnowledge.truthy().min(ctx, *otherTruthy);
                }
            }
        }

        if (canBeFalsy) {
            auto &thisKnowledge = _varState[thisRef.id()].knowledge;
            const auto &otherFalsy = otherRef.knowledge(other).falsy().under(ctx, other, loc, inWhat, bb,
                                                                             knowledgeFilter.isNeeded(varLocalRef));
            if (!otherFalsy->isDead) {
                if (!thisKnowledge.seenFalsyOption) {
                    thisKnowledge.seenFalsyOption = true;
                    thisKnowledge.replaceFalsy(thisRef, typeTestsWithVar, otherFalsy);
                } else {
                    thisKnowledge.falsy().min(ctx, *otherFalsy);
                }
            }
        }
    }
}

void Environment::computePins(core::Context ctx, const vector<Environment> &envs, const cfg::CFG &inWhat,
                              const cfg::BasicBlock *bb) {
    if (bb->backEdges.empty()) {
        return;
    }

    for (int i = 1; i < _varState.size(); i++) {
        auto var = LocalRefRef(i);
        core::TypeAndOrigins tp;

        auto bindMinLoops = var.data(*this).minLoops(inWhat);
        if (bb->outerLoops == bindMinLoops || bindMinLoops == var.data(*this).maxLoopWrite(inWhat)) {
            continue;
        }

        for (cfg::BasicBlock *parent : bb->backEdges) {
            auto &other = envs[parent->id];
            auto otherPin = other.pinnedTypes.find(var.data(*this));
            if (var.data(*this).exists() && otherPin != other.pinnedTypes.end()) {
                if (tp.type != nullptr) {
                    tp.type = core::Types::any(ctx, tp.type, otherPin->second.type);
                    for (auto origin : otherPin->second.origins) {
                        if (!absl::c_linear_search(tp.origins, origin)) {
                            tp.origins.emplace_back(origin);
                        }
                    }
                    tp.type->sanityCheck(ctx);
                } else {
                    tp = otherPin->second;
                }
            }
        }

        if (tp.type != nullptr) {
            pinnedTypes[var.data(*this)] = tp;
        }
    }
}

void Environment::populateFrom(core::Context ctx, const Environment &other) {
    this->isDead = other.isDead;
    int i = 0;
    for (auto &state : _varState) {
        LocalRefRef var(i++);
        if (!var.exists()) {
            continue;
        }
        auto otherVar = other.lookupLocal(var.data(*this));

        state.typeAndOrigins = otherVar.typeAndOrigin(other);
        state.knowledge.replace(var, typeTestsWithVar, otherVar.knowledge(other));
        state.knownTruthy = otherVar.knownTruthy(other);
    }

    this->pinnedTypes = other.pinnedTypes;
}

core::TypePtr Environment::getReturnType(core::Context ctx, core::TypePtr procType) {
    if (!procType->derivesFrom(ctx, core::Symbols::Proc())) {
        return core::Types::untypedUntracked();
    }
    auto *applied = core::cast_type<core::AppliedType>(procType.get());
    if (applied == nullptr || applied->targs.empty()) {
        return core::Types::untypedUntracked();
    }
    // Proc types have their return type as the first targ
    return applied->targs.front();
}

core::TypePtr flattenArrays(core::Context ctx, core::TypePtr type) {
    core::TypePtr result;

    typecase(
        type.get(),

        [&](core::OrType *o) {
            result = core::Types::any(ctx, flattenArrays(ctx, o->left), flattenArrays(ctx, o->right));
        },

        [&](core::AppliedType *a) {
            if (a->klass != core::Symbols::Array()) {
                result = type;
                return;
            }
            ENFORCE(a->targs.size() == 1);
            result = a->targs.front();
        },

        [&](core::TupleType *t) { result = t->elementType(); },

        [&](core::Type *t) { result = std::move(type); });
    return result;
}

core::TypePtr flatmapHack(core::Context ctx, core::TypePtr receiver, core::TypePtr returnType, core::NameRef fun) {
    if (fun != core::Names::flatMap()) {
        return returnType;
    }
    if (!receiver->derivesFrom(ctx, core::Symbols::Enumerable())) {
        return returnType;
    }

    if (!receiver->isUntyped() && receiver->derivesFrom(ctx, core::Symbols::Enumerator_Lazy())) {
        return returnType;
    }

    return core::Types::arrayOf(ctx, flattenArrays(ctx, returnType));
}

core::TypePtr Environment::processBinding(core::Context ctx, const cfg::CFG &inWhat, cfg::Binding &bind, int loopCount,
                                          int bindMinLoops, KnowledgeFilter &knowledgeFilter,
                                          core::TypeConstraint &constr, core::TypePtr &methodReturnType) {
    try {
        core::TypeAndOrigins tp;
        bool noLoopChecking = cfg::isa_instruction<cfg::Alias>(bind.value.get()) ||
                              cfg::isa_instruction<cfg::LoadArg>(bind.value.get()) ||
                              cfg::isa_instruction<cfg::LoadSelf>(bind.value.get());

        bool checkFullyDefined = true;
        const core::lsp::Query &lspQuery = ctx.state.lspQuery;
        bool lspQueryMatch = lspQuery.matchesLoc(core::Loc(ctx.file, bind.loc));

        // Will get its type set at end of function.
        auto bindVarRef = enterLocal(bind.bind.variable);

        typecase(
            bind.value.get(),
            [&](cfg::Send *send) {
                InlinedVector<const core::TypeAndOrigins *, 2> args;

                args.reserve(send->args.size());
                for (cfg::VariableUseSite &arg : send->args) {
                    args.emplace_back(&getAndFillTypeAndOrigin(ctx, arg));
                }

                const core::TypeAndOrigins &recvType = getAndFillTypeAndOrigin(ctx, send->recv);
                if (send->link) {
                    checkFullyDefined = false;
                }
                core::CallLocs locs{
                    ctx.file,
                    bind.loc,
                    send->receiverLoc,
                    send->argLocs,
                };
                core::DispatchArgs dispatchArgs{send->fun, locs, args, recvType.type, recvType.type, send->link};
                auto dispatched = recvType.type->dispatchCall(ctx, dispatchArgs);

                auto it = &dispatched;
                while (it != nullptr) {
                    for (auto &err : it->main.errors) {
                        ctx.state._error(std::move(err));
                    }
                    lspQueryMatch = lspQueryMatch || lspQuery.matchesSymbol(it->main.method);
                    it = it->secondary.get();
                }
                shared_ptr<core::DispatchResult> retainedResult;
                if (send->link) {
                    // this type should never be used, thus we put a useless type
                    tp.type = core::Types::void_();
                } else {
                    tp.type = dispatched.returnType;
                }
                if (send->link || lspQueryMatch) {
                    retainedResult = make_shared<core::DispatchResult>(std::move(dispatched));
                }
                if (lspQueryMatch) {
                    core::lsp::QueryResponse::pushQueryResponse(
                        ctx, core::lsp::SendResponse(core::Loc(ctx.file, bind.loc), retainedResult, send->fun,
                                                     send->isPrivateOk, ctx.owner));
                }
                if (send->link) {
                    // This should eventually become ENFORCEs but currently they are wrong
                    if (!retainedResult->main.blockReturnType) {
                        retainedResult->main.blockReturnType = core::Types::untyped(ctx, retainedResult->main.method);
                    }
                    if (!retainedResult->main.blockPreType) {
                        retainedResult->main.blockPreType = core::Types::untyped(ctx, retainedResult->main.method);
                    }
                    ENFORCE(retainedResult->main.sendTp);

                    send->link->result = move(retainedResult);
                }
                tp.origins.emplace_back(core::Loc(ctx.file, bind.loc));
            },
            [&](cfg::Ident *i) {
                auto ref = lookupLocal(i->what);
                const core::TypeAndOrigins &typeAndOrigin = ref.typeAndOrigin(*this);
                tp.type = typeAndOrigin.type;
                tp.origins = typeAndOrigin.origins;

                if (lspQueryMatch && !bind.value->isSynthetic) {
                    core::lsp::QueryResponse::pushQueryResponse(
                        ctx,
                        core::lsp::IdentResponse(core::Loc(ctx.file, bind.loc), i->what.data(inWhat), tp, ctx.owner));
                }

                ENFORCE(ctx.file.data(ctx).hasParseErrors || !tp.origins.empty(), "Inferencer did not assign location");
            },
            [&](cfg::Alias *a) {
                core::SymbolRef symbol = a->what.data(ctx)->dealias(ctx);
                const auto &data = symbol.data(ctx);
                if (data->isClassOrModule()) {
                    auto singletonClass = data->lookupSingletonClass(ctx);
                    ENFORCE(singletonClass.exists(), "Every class should have a singleton class by now.");
                    tp.type = singletonClass.data(ctx)->externalType(ctx);
                    tp.origins.emplace_back(symbol.data(ctx)->loc());
                } else if (data->isField() || (data->isStaticField() && !data->isTypeAlias()) || data->isTypeMember()) {
                    if (data->resultType.get() != nullptr) {
                        if (data->isTypeMember()) {
                            if (data->isFixed()) {
                                // pick the upper bound here, as
                                // isFixed() => lowerBound == upperBound.
                                auto lambdaParam = core::cast_type<core::LambdaParam>(data->resultType.get());
                                ENFORCE(lambdaParam != nullptr);
                                tp.type = core::make_type<core::MetaType>(lambdaParam->upperBound);
                            } else {
                                tp.type = core::make_type<core::MetaType>(core::make_type<core::SelfTypeParam>(symbol));
                            }
                        } else if (data->isField()) {
                            tp.type = core::Types::resultTypeAsSeenFrom(
                                ctx, symbol.data(ctx)->resultType, symbol.data(ctx)->owner,
                                ctx.owner.data(ctx)->enclosingClass(ctx),
                                ctx.owner.data(ctx)->enclosingClass(ctx).data(ctx)->selfTypeArgs(ctx));
                        } else {
                            tp.type = data->resultType;
                        }
                        tp.origins.emplace_back(data->loc());
                    } else {
                        tp.origins.emplace_back(core::Loc::none());
                        tp.type = core::Types::untyped(ctx, symbol);
                    }
                } else if (data->isTypeAlias()) {
                    ENFORCE(data->resultType.get() != nullptr);
                    tp.origins.emplace_back(data->loc());
                    tp.type = core::make_type<core::MetaType>(data->resultType);
                } else {
                    Exception::notImplemented();
                }

                pinnedTypes[bind.bind.variable] = tp;
            },
            [&](cfg::SolveConstraint *i) {
                if (i->link->result->main.constr && !i->link->result->main.constr->solve(ctx)) {
                    if (auto e = ctx.beginError(bind.loc, core::errors::Infer::GenericMethodConstaintUnsolved)) {
                        e.setHeader("Could not find valid instantiation of type parameters");
                    }
                }

                core::TypePtr type;
                if (i->link->result->main.constr) {
                    // TODO: this should repeat the same dance with Or and And components that dispatchCall does
                    type = core::Types::instantiate(ctx, i->link->result->main.sendTp, *i->link->result->main.constr);
                } else {
                    type = i->link->result->returnType;
                }
                type = flatmapHack(ctx, i->link->result->main.receiver, type, i->link->fun);
                tp.type = std::move(type);
                tp.origins.emplace_back(core::Loc(ctx.file, bind.loc));
            },
            [&](cfg::LoadArg *i) {
                /* read type from info filled by define_method */
                /*
                 * TODO(nelhage): This should arguably use the klass and targs
                 * off of i->receiver. Right now these should always be
                 * identical, but if we ever typecheck methods on contexts
                 * outside the context in which they are defined, we would want
                 * to respect that.
                 *
                 * For now we can at least enforce a little consistency.
                 */
                // ENFORCE(ctx.owner == i->method); TODO: re-enable when https://github.com/sorbet/sorbet/issues/904 is
                // fixed

                auto argType = i->argument(ctx).argumentTypeAsSeenByImplementation(ctx, constr);
                tp.type = std::move(argType);
                tp.origins.emplace_back(core::Loc(ctx.file, bind.loc));
            },
            [&](cfg::ArgPresent *i) {
                // Return an unanalyzable boolean value that indicates whether or not arg was provided
                // It's unanalyzable because it varies by each individual call site.
                ENFORCE(ctx.owner == i->method);

                tp.type = core::Types::Boolean();
                tp.origins.emplace_back(core::Loc(ctx.file, bind.loc));
            },
            [&](cfg::LoadYieldParams *insn) {
                ENFORCE(insn->link);
                ENFORCE(insn->link->result);
                ENFORCE(insn->link->result->main.blockPreType);
                auto &procType = insn->link->result->main.blockPreType;
                auto params = procType->getCallArguments(ctx, core::Names::call());

                // A multi-arg proc, if provided a single arg which is an array,
                // will implicitly splat it out.
                //
                // TODO(nelhage): If this block is a lambda, not a proc, this
                // rule doesn't apply. We don't model the distinction accurately
                // yet.
                auto &blkArgs = insn->link->argFlags;
                auto *tuple = core::cast_type<core::TupleType>(params.get());
                if (blkArgs.size() > 1 && !blkArgs.front().isRepeated && tuple && tuple->elems.size() == 1 &&
                    tuple->elems.front()->derivesFrom(ctx, core::Symbols::Array())) {
                    tp.type = std::move(tuple->elems.front());
                } else if (params == nullptr) {
                    tp.type = core::Types::untypedUntracked();
                } else {
                    tp.type = params;
                }
                tp.origins.emplace_back(core::Loc(ctx.file, bind.loc));
            },
            [&](cfg::Return *i) {
                tp.type = core::Types::bottom();
                tp.origins.emplace_back(core::Loc(ctx.file, bind.loc));

                const core::TypeAndOrigins &typeAndOrigin = getAndFillTypeAndOrigin(ctx, i->what);
                if (core::Types::isSubType(ctx, core::Types::void_(), methodReturnType)) {
                    methodReturnType = core::Types::untypedUntracked();
                }
                if (!core::Types::isSubTypeUnderConstraint(ctx, constr, typeAndOrigin.type, methodReturnType,
                                                           core::UntypedMode::AlwaysCompatible)) {
                    if (auto e = ctx.beginError(bind.loc, core::errors::Infer::ReturnTypeMismatch)) {
                        auto ownerData = ctx.owner.data(ctx);
                        e.setHeader("Returning value that does not conform to method result type");
                        e.addErrorSection(core::ErrorSection(
                            "Expected " + methodReturnType->show(ctx),
                            {
                                core::ErrorLine::from(ownerData->loc(), "Method `{}` has return type `{}`",
                                                      ownerData->name.show(ctx), methodReturnType->show(ctx)),
                            }));
                        e.addErrorSection(
                            core::ErrorSection("Got " + typeAndOrigin.type->show(ctx) + " originating from:",
                                               typeAndOrigin.origins2Explanations(ctx)));
                    }
                }
            },
            [&](cfg::BlockReturn *i) {
                ENFORCE(i->link);
                ENFORCE(i->link->result->main.blockReturnType != nullptr);

                const core::TypeAndOrigins &typeAndOrigin = getAndFillTypeAndOrigin(ctx, i->what);
                auto expectedType = i->link->result->main.blockReturnType;
                if (core::Types::isSubType(ctx, core::Types::void_(), expectedType)) {
                    expectedType = core::Types::untypedUntracked();
                }
                bool isSubtype;
                if (i->link->result->main.constr) {
                    isSubtype = core::Types::isSubTypeUnderConstraint(ctx, *i->link->result->main.constr,

                                                                      typeAndOrigin.type, expectedType,
                                                                      core::UntypedMode::AlwaysCompatible);
                } else {
                    isSubtype = core::Types::isSubType(ctx, typeAndOrigin.type, expectedType);
                }
                if (!isSubtype) {
                    // TODO(nelhage): We should somehow report location
                    // information about the `send` and/or the
                    // definition of the block type

                    if (auto e = ctx.beginError(bind.loc, core::errors::Infer::ReturnTypeMismatch)) {
                        e.setHeader("Returning value that does not conform to block result type");
                        e.addErrorSection(core::ErrorSection("Expected " + expectedType->show(ctx)));
                        e.addErrorSection(
                            core::ErrorSection("Got " + typeAndOrigin.type->show(ctx) + " originating from:",
                                               typeAndOrigin.origins2Explanations(ctx)));
                    }
                }

                tp.type = core::Types::bottom();
                tp.origins.emplace_back(core::Loc(ctx.file, bind.loc));
            },
            [&](cfg::Literal *i) {
                tp.type = i->value;
                tp.origins.emplace_back(core::Loc(ctx.file, bind.loc));

                if (lspQueryMatch) {
                    core::lsp::QueryResponse::pushQueryResponse(
                        ctx, core::lsp::LiteralResponse(core::Loc(ctx.file, bind.loc), tp));
                }
            },
            [&](cfg::TAbsurd *i) {
                auto ref = lookupLocal(i->what.variable);
                const core::TypeAndOrigins &typeAndOrigin = ref.typeAndOrigin(*this);

                if (auto e = ctx.beginError(bind.loc, core::errors::Infer::NotExhaustive)) {
                    if (typeAndOrigin.type->isUntyped()) {
                        e.setHeader("Control flow could reach `{}` because argument was `{}`", "T.absurd", "T.untyped");
                    } else {
                        e.setHeader("Control flow could reach `{}` because the type `{}` wasn't handled", "T.absurd",
                                    typeAndOrigin.type->show(ctx));
                    }
                    e.addErrorSection(core::ErrorSection("Originating from:", typeAndOrigin.origins2Explanations(ctx)));
                }

                tp.type = core::Types::bottom();
                tp.origins.emplace_back(core::Loc(ctx.file, bind.loc));
            },
            [&](cfg::GetCurrentException *i) {
                tp.type = core::Types::untypedUntracked();
                tp.origins.emplace_back(core::Loc(ctx.file, bind.loc));
            },
            [&](cfg::LoadSelf *l) {
                ENFORCE(l->link);
                if (l->link->result->main.blockSpec.rebind.exists()) {
                    tp.type = l->link->result->main.blockSpec.rebind.data(ctx)->externalType(ctx);
                    tp.origins.emplace_back(core::Loc(ctx.file, bind.loc));

                } else {
                    auto ref = lookupLocal(l->fallback);
                    tp = ref.typeAndOrigin(*this);
                }
            },
            [&](cfg::Cast *c) {
                auto klass = ctx.owner.data(ctx)->enclosingClass(ctx);
                auto castType = core::Types::instantiate(ctx, c->type, klass.data(ctx)->typeMembers(),
                                                         klass.data(ctx)->selfTypeArgs(ctx));

                tp.type = castType;
                tp.origins.emplace_back(core::Loc(ctx.file, bind.loc));

                if (!bindVarRef.hasType(*this)) {
                    noLoopChecking = true;
                }

                const core::TypeAndOrigins &ty = getAndFillTypeAndOrigin(ctx, c->value);
                ENFORCE(c->cast != core::Names::uncheckedLet());
                if (c->cast != core::Names::cast()) {
                    if (c->cast == core::Names::assertType() && ty.type->isUntyped()) {
                        if (auto e = ctx.beginError(bind.loc, core::errors::Infer::CastTypeMismatch)) {
                            e.setHeader("The typechecker was unable to infer the type of the asserted value");
                            e.addErrorSection(core::ErrorSection("Got " + ty.type->show(ctx) + " originating from:",
                                                                 ty.origins2Explanations(ctx)));
                            e.addErrorSection(core::ErrorSection("You may need to add additional `sig` annotations"));
                        }
                    } else if (!core::Types::isSubType(ctx, ty.type, castType)) {
                        if (auto e = ctx.beginError(bind.loc, core::errors::Infer::CastTypeMismatch)) {
                            e.setHeader("Argument does not have asserted type `{}`", castType->show(ctx));
                            e.addErrorSection(core::ErrorSection("Got " + ty.type->show(ctx) + " originating from:",
                                                                 ty.origins2Explanations(ctx)));
                        }
                    }
                } else if (!c->isSynthetic) {
                    if (castType->isUntyped()) {
                        if (auto e = ctx.beginError(bind.loc, core::errors::Infer::InvalidCast)) {
                            e.setHeader("Please use `T.unsafe(...)` to cast to T.untyped");
                        }
                    } else if (!ty.type->isUntyped() && core::Types::isSubType(ctx, ty.type, castType)) {
                        if (auto e = ctx.beginError(bind.loc, core::errors::Infer::InvalidCast)) {
                            e.setHeader("Useless cast: inferred type `{}` is already a subtype of `{}`",
                                        ty.type->show(ctx), castType->show(ctx));
                        }
                    }
                }
                if (c->cast == core::Names::let()) {
                    pinnedTypes[bind.bind.variable] = tp;
                }
            });

        ENFORCE(tp.type.get() != nullptr, "Inferencer did not assign type: {}", bind.value->toString(ctx, inWhat));
        tp.type->sanityCheck(ctx);

        if (checkFullyDefined && !tp.type->isFullyDefined()) {
            if (auto e = ctx.beginError(bind.loc, core::errors::Infer::IncompleteType)) {
                e.setHeader("Expression does not have a fully-defined type (Did you reference another class's type "
                            "members?)");
            }
            tp.type = core::Types::untypedUntracked();
        }
        ENFORCE(ctx.file.data(ctx).hasParseErrors || !tp.origins.empty(), "Inferencer did not assign location");

        if (!noLoopChecking && loopCount != bindMinLoops) {
            auto pin = pinnedTypes.find(bind.bind.variable);
            const core::TypeAndOrigins &cur =
                (pin != pinnedTypes.end()) ? pin->second : bindVarRef.typeAndOrigin(*this);

            bool asGoodAs =
                core::Types::isSubType(ctx, core::Types::dropLiteral(tp.type), core::Types::dropLiteral(cur.type));

            {
                switch (bindMinLoops) {
                    case cfg::CFG::MIN_LOOP_FIELD:
                        if (!asGoodAs) {
                            if (auto e = ctx.beginError(bind.loc, core::errors::Infer::FieldReassignmentTypeMismatch)) {
                                e.setHeader(
                                    "Reassigning field with a value of wrong type: `{}` is not a subtype of `{}`",
                                    tp.type->show(ctx), cur.type->show(ctx));
                            }
                            tp = cur;
                        }
                        break;
                    case cfg::CFG::MIN_LOOP_GLOBAL:
                        if (!asGoodAs) {
                            if (auto e =
                                    ctx.beginError(bind.loc, core::errors::Infer::GlobalReassignmentTypeMismatch)) {
                                e.setHeader(
                                    "Reassigning global with a value of wrong type: `{}` is not a subtype of `{}`",
                                    tp.type->show(ctx), cur.type->show(ctx));
                            }
                            tp = cur;
                        }
                        break;
                    case cfg::CFG::MIN_LOOP_LET:
                        if (!asGoodAs) {
                            if (auto e = ctx.beginError(bind.loc, core::errors::Infer::PinnedVariableMismatch)) {
                                e.setHeader("Incompatible assignment to variable declared via `{}`: `{}` is not a "
                                            "subtype of `{}`",
                                            "let", tp.type->show(ctx), cur.type->show(ctx));
                            }
                            tp = cur;
                        }
                        break;
                    default: {
                        if (!asGoodAs || (tp.type->isUntyped() && !cur.type->isUntyped())) {
                            if (auto ident = cfg::cast_instruction<cfg::Ident>(bind.value.get())) {
                                // See cfg/builder/builder_walk.cc for an explanation of why this is here.
                                if (ident->what.data(inWhat)._name == core::Names::blockBreakAssign()) {
                                    break;
                                }

                                if (ident->what.data(inWhat)._name == core::Names::selfRestore()) {
                                    // this is a restoration of `self` variable.
                                    // our current analysis isn't smart enogh to see that it's safe to do this by
                                    // construction either https://github.com/sorbet/sorbet/issues/222 or
                                    // https://github.com/sorbet/sorbet/issues/224 should allow us to remove this
                                    // case
                                    break;
                                }
                            }
                            if (auto e = ctx.beginError(bind.loc, core::errors::Infer::PinnedVariableMismatch)) {
                                e.setHeader("Changing the type of a variable in a loop is not permitted");
                                e.addErrorSection(core::ErrorSection(core::ErrorColors::format(
                                    "Existing variable has type: `{}`", cur.type->show(ctx))));
                                e.addErrorSection(core::ErrorSection(core::ErrorColors::format(
                                    "Attempting to change type to: `{}`\n", tp.type->show(ctx))));

                                if (cur.origins.size() == 1) {
                                    // NOTE(nelhage): We assume that if there is
                                    // a single definition location, that
                                    // corresponds to an initial
                                    // assignment. This is not necessarily
                                    // correct if the variable came from some
                                    // other source (e.g. a function argument)
                                    auto suggest =
                                        core::Types::any(ctx, dropConstructor(ctx, tp.origins[0], tp.type), cur.type);
                                    e.replaceWith(fmt::format("Initialize as `{}`", suggest->show(ctx)), cur.origins[0],
                                                  "T.let({}, {})", cur.origins[0].source(ctx), suggest->show(ctx));
                                } else {
                                    e.addErrorSection(
                                        core::ErrorSection("Original type from:", cur.origins2Explanations(ctx)));
                                }
                            }

                            tp.type = core::Types::untypedUntracked();
                        }
                        break;
                    }
                }
            }
        }
        setTypeAndOrigin(bindVarRef, tp);

        clearKnowledge(ctx, bindVarRef, knowledgeFilter);
        if (auto *send = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
            updateKnowledge(ctx, bindVarRef, core::Loc(ctx.file, bind.loc), send, knowledgeFilter);
        } else if (auto *i = cfg::cast_instruction<cfg::Ident>(bind.value.get())) {
            propagateKnowledge(ctx, bindVarRef, i->what, knowledgeFilter);
        }

        return move(tp.type);
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = ctx.beginError(bind.loc, core::errors::Internal::InternalError)) {
            e.setHeader("Failed to type (backtrace is above)");
        }
        throw;
    }
}

void Environment::cloneFrom(const Environment &rhs) {
    this->isDead = rhs.isDead;
    this->definedVars = rhs.definedVars;
    this->vars = rhs.vars;
    this->_varState = rhs._varState;
    this->bb = rhs.bb;
    this->pinnedTypes = rhs.pinnedTypes;
    this->typeTestsWithVar.cloneFrom(rhs.typeTestsWithVar);
}

void Environment::initializeBasicBlockArgs(const cfg::BasicBlock &bb) {
    vars.reserve(bb.args.size());
    _varState.reserve(bb.args.size());
    for (const cfg::VariableUseSite &arg : bb.args) {
        auto ref = enterLocal(arg.variable);
        _varState[ref.id()].typeAndOrigins.type = nullptr;
    }
}

void Environment::setUninitializedVarsToNil(const core::Context &ctx, core::Loc origin) {
    for (auto &uninitialized : _varState) {
        if (uninitialized.typeAndOrigins.type.get() == nullptr) {
            uninitialized.typeAndOrigins.type = core::Types::nilClass();
            uninitialized.typeAndOrigins.origins.emplace_back(origin);
        } else {
            uninitialized.typeAndOrigins.type->sanityCheck(ctx);
        }
    }
}

namespace {
core::TypeAndOrigins nilTypesWithOriginWithLoc(core::Loc loc) {
    // I'd love to have this, but keepForIDE intentionally has Loc::none() and
    // sometimes ends up here...
    // ENFORCE(loc.exists());
    core::TypeAndOrigins ret;
    ret.type = core::Types::nilClass();
    ret.origins.emplace_back(loc);
    return ret;
}
} // namespace

Environment::Environment(const cfg::CFG &cfg, core::Loc ownerLoc)
    : uninitialized(nilTypesWithOriginWithLoc(ownerLoc)), definedVars(cfg.numLocalVariables()) {
    // Enter the non-existant 0 ref.
    this->_varState.emplace_back();
    this->_varState[0].knowledge = TestedKnowledge::empty;
    this->_varState[0].typeAndOrigins = uninitialized;
    this->vars.emplace_back(cfg::LocalRef());
}

TestedKnowledge TestedKnowledge::empty;
} // namespace sorbet::infer
