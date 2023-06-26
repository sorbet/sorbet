#include "environment.h"
#include "absl/strings/match.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "common/typecase.h"
#include "core/GlobalState.h"
#include "core/TypeConstraint.h"
#include "core/TypeErrorDiagnostics.h"
#include <algorithm> // find, remove_if

using namespace std;

namespace sorbet::infer {

namespace {
core::TypePtr dropConstructor(core::Context ctx, core::Loc loc, core::TypePtr tp) {
    if (auto *mt = core::cast_type<core::MetaType>(tp)) {
        if (!mt->wrapped.isUntyped()) {
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

void TypeTestReverseIndex::addToIndex(cfg::LocalRef from, cfg::LocalRef to) {
    auto &list = index[from];
    // Maintain invariant: lists are sorted sets (no duplicate entries)
    // lower_bound returns the first location that is >= to's ID, which is where `to` should be inserted if *it != to
    // (uses binary search, O(log n))
    auto it = absl::c_lower_bound(list, to, [](const auto &a, const auto &b) -> bool { return a.id() < b.id(); });
    if (it == list.end() || *it != to) {
        list.insert(it, to);
    }
}

const InlinedVector<cfg::LocalRef, 1> &TypeTestReverseIndex::get(cfg::LocalRef from) const {
    auto it = index.find(from);
    if (it == index.end()) {
        return empty;
    }
    return it->second;
}

void TypeTestReverseIndex::replace(cfg::LocalRef from, InlinedVector<cfg::LocalRef, 1> &&list) {
    index[from] = std::move(list);
}

void TypeTestReverseIndex::cloneFrom(const TypeTestReverseIndex &index) {
    this->index = index.index;
}

const InlinedVector<cfg::LocalRef, 1> TypeTestReverseIndex::empty;

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
                if (auto *id = cfg::cast_instruction<cfg::Ident>(bind.value)) {
                    if (isNeeded(bind.bind.variable) && !isNeeded(id->what)) {
                        used_vars[id->what.id()] = true;
                        changed = true;
                    }
                } else if (auto *send = cfg::cast_instruction<cfg::Send>(bind.value)) {
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
}

bool KnowledgeFilter::isNeeded(cfg::LocalRef var) {
    return used_vars[var.id()];
}

KnowledgeRef KnowledgeRef::under(core::Context ctx, const Environment &env, cfg::CFG &inWhat, cfg::BasicBlock *bb,
                                 bool isNeeded) const {
    if (knowledge->yesTypeTests.empty() && !isNeeded) {
        return *this;
    }
    KnowledgeRef copy = *this;
    if (env.isDead) {
        copy.markDead();
        return copy;
    }
    bool enteringLoop = (bb->flags & cfg::CFG::LOOP_HEADER) != 0;
    for (auto &pair : env.vars()) {
        auto local = pair.first;
        auto &state = pair.second;
        if (enteringLoop && bb->outerLoops <= local.maxLoopWrite(inWhat)) {
            continue;
        }
        auto fnd = absl::c_find_if(copy->yesTypeTests, [&](auto const &e) -> bool { return e.first == local; });
        if (fnd == copy->yesTypeTests.end()) {
            // add info from env to knowledge
            ENFORCE(state.typeAndOrigins.type != nullptr);
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

            auto &type = state.typeAndOrigins.type;
            if (isNeeded && !type.isUntyped() && !core::isa_type<core::MetaType>(type)) {
                // Direct mutation of `yesTypeTests` rather than going through `addYesTypeTest`.
                // This is fine since `copy` is unmoored from a particular environment.
                copy.mutate().yesTypeTests.emplace_back(local, type);
            }
        } else {
            auto &second = fnd->second;
            auto &typeAndOrigin = state.typeAndOrigins;
            auto combinedType = core::Types::all(ctx, typeAndOrigin.type, second);
            if (combinedType.isBottom()) {
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
        ENFORCE(a.second != nullptr);
        ENFORCE(!a.second.isUntyped());
    }
    for (auto &a : noTypeTests) {
        ENFORCE(a.second != nullptr);
        ENFORCE(!a.second.isUntyped());
    }
}

string KnowledgeFact::toString(const core::GlobalState &gs, const cfg::CFG &cfg) const {
    vector<string> buf1, buf2;

    for (auto &el : yesTypeTests) {
        buf1.emplace_back(
            fmt::format("    {} to be {}\n", el.first.showRaw(gs, cfg), el.second.toStringWithTabs(gs, 0)));
    }
    for (auto &el : noTypeTests) {
        buf2.emplace_back(
            fmt::format("    {} NOT to be {}\n", el.first.showRaw(gs, cfg), el.second.toStringWithTabs(gs, 0)));
    }
    fast_sort(buf1);
    fast_sort(buf2);

    return fmt::format("{}{}", fmt::join(buf1, ""), fmt::join(buf2, ""));
}

KnowledgeRef::KnowledgeRef() : knowledge(std::make_shared<KnowledgeFact>()) {}

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

void KnowledgeRef::addYesTypeTest(cfg::LocalRef of, TypeTestReverseIndex &index, cfg::LocalRef ref,
                                  core::TypePtr type) {
    index.addToIndex(ref, of);
    this->mutate().yesTypeTests.emplace_back(ref, move(type));
}

void KnowledgeRef::addNoTypeTest(cfg::LocalRef of, TypeTestReverseIndex &index, cfg::LocalRef ref, core::TypePtr type) {
    index.addToIndex(ref, of);
    this->mutate().noTypeTests.emplace_back(ref, move(type));
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
        fmt::format_to(std::back_inserter(buf), "  Being truthy entails:\n{}", _truthy->toString(gs, cfg));
    }
    if (!_falsy->noTypeTests.empty() || !_falsy->yesTypeTests.empty()) {
        fmt::format_to(std::back_inserter(buf), "  Being falsy entails:\n{}", _falsy->toString(gs, cfg));
    }
    return to_string(buf);
}

void TestedKnowledge::replaceTruthy(cfg::LocalRef of, TypeTestReverseIndex &index, const KnowledgeRef &newTruthy) {
    // Note: No need to remove old items from Environment::typeTestsWithVar since it is an overapproximation
    for (auto &entry : newTruthy->yesTypeTests) {
        index.addToIndex(entry.first, of);
    }

    for (auto &entry : newTruthy->noTypeTests) {
        index.addToIndex(entry.first, of);
    }

    _truthy = newTruthy;
}

void TestedKnowledge::replaceFalsy(cfg::LocalRef of, TypeTestReverseIndex &index, const KnowledgeRef &newFalsy) {
    // Note: No need to remove old items from Environment::typeTestsWithVar since it is an overapproximation
    for (auto &entry : newFalsy->yesTypeTests) {
        index.addToIndex(entry.first, of);
    }

    for (auto &entry : newFalsy->noTypeTests) {
        index.addToIndex(entry.first, of);
    }

    _falsy = newFalsy;
}

void TestedKnowledge::replace(cfg::LocalRef of, TypeTestReverseIndex &index, const TestedKnowledge &knowledge) {
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

string Environment::toString(const core::GlobalState &gs, const cfg::CFG &cfg) const {
    fmt::memory_buffer buf;
    if (isDead) {
        fmt::format_to(std::back_inserter(buf), "dead={:d}\n", isDead);
    }
    vector<pair<cfg::LocalRef, VariableState>> sorted;
    for (const auto &pair : _vars) {
        sorted.emplace_back(pair);
    }
    fast_sort(sorted, [&cfg](const auto &lhs, const auto &rhs) -> bool {
        return lhs.first.data(cfg)._name.rawId() < rhs.first.data(cfg)._name.rawId() ||
               (lhs.first.data(cfg)._name.rawId() == rhs.first.data(cfg)._name.rawId() &&
                lhs.first.data(cfg).unique < rhs.first.data(cfg).unique);
    });
    for (const auto &pair : sorted) {
        auto var = pair.first;
        auto &state = pair.second;
        if (var.data(cfg)._name == core::Names::debugEnvironmentTemp()) {
            continue;
        }
        fmt::format_to(std::back_inserter(buf), "{}: {}{}\n{}\n", var.showRaw(gs, cfg),
                       state.typeAndOrigins.type.toStringWithTabs(gs, 0), state.knownTruthy ? " (and truthy)\n" : "",
                       state.knowledge.toString(gs, cfg));
    }
    return to_string(buf);
}

bool Environment::hasType(core::Context ctx, cfg::LocalRef symbol) const {
    auto fnd = _vars.find(symbol);
    if (fnd == _vars.end()) {
        return false;
    }
    // We don't distinguish between nullptr and "not set"
    return fnd->second.typeAndOrigins.type != nullptr;
}

const core::TypeAndOrigins &Environment::getTypeAndOrigin(core::Context ctx, cfg::LocalRef symbol) const {
    auto fnd = _vars.find(symbol);
    if (fnd == _vars.end()) {
        return uninitialized;
    }
    ENFORCE(fnd->second.typeAndOrigins.type != nullptr);
    return fnd->second.typeAndOrigins;
}

const core::TypeAndOrigins &Environment::getAndFillTypeAndOrigin(core::Context ctx,
                                                                 cfg::VariableUseSite &symbol) const {
    const auto &ret = getTypeAndOrigin(ctx, symbol.variable);
    symbol.type = ret.type;
    return ret;
}

bool Environment::getKnownTruthy(cfg::LocalRef var) const {
    auto fnd = _vars.find(var);
    if (fnd == _vars.end()) {
        return false;
    }
    return fnd->second.knownTruthy;
}

void Environment::propagateKnowledge(core::Context ctx, cfg::LocalRef to, cfg::LocalRef from,
                                     KnowledgeFilter &knowledgeFilter) {
    if (knowledgeFilter.isNeeded(to) && knowledgeFilter.isNeeded(from)) {
        auto &fromState = _vars[from];
        auto &toState = _vars[to];
        toState.knownTruthy = fromState.knownTruthy;
        auto &toKnowledge = toState.knowledge;
        auto &fromKnowledge = fromState.knowledge;
        if (fromState.typeAndOrigins.type == nullptr) {
            fromState.typeAndOrigins.type = core::Types::nilClass();
        }
        if (toState.typeAndOrigins.type == nullptr) {
            toState.typeAndOrigins.type = core::Types::nilClass();
        }

        // Copy properties from fromKnowledge to toKnowledge
        toKnowledge.replace(to, typeTestsWithVar, fromKnowledge);

        toKnowledge.truthy().addNoTypeTest(to, typeTestsWithVar, from, core::Types::falsyTypes());
        toKnowledge.falsy().addYesTypeTest(to, typeTestsWithVar, from, core::Types::falsyTypes());
        fromKnowledge.truthy().addNoTypeTest(from, typeTestsWithVar, to, core::Types::falsyTypes());
        fromKnowledge.falsy().addYesTypeTest(from, typeTestsWithVar, to, core::Types::falsyTypes());
        fromKnowledge.sanityCheck();
        toKnowledge.sanityCheck();
    }
}

void Environment::clearKnowledge(core::Context ctx, cfg::LocalRef reassigned, KnowledgeFilter &knowledgeFilter) {
    auto &typesWithReassigned = typeTestsWithVar.get(reassigned);
    if (!typesWithReassigned.empty()) {
        InlinedVector<cfg::LocalRef, 1> replacement;
        for (auto &var : typesWithReassigned) {
            if (knowledgeFilter.isNeeded(var)) {
                auto &k = getKnowledge(var);
                k.removeReferencesToVar(reassigned);
                k.sanityCheck();
            } else {
                replacement.emplace_back(var);
            }
        }
        typeTestsWithVar.replace(reassigned, std::move(replacement));
    }

    auto fnd = _vars.find(reassigned);
    ENFORCE(fnd != _vars.end());
    fnd->second.knownTruthy = false;
}

namespace {

// A singleton is a class which is inhabited by a single value.
//
// If a variable != a value whose type is a singleton, then that variable's type must not be that value's type.
//
// This is not the case for most other equality tests. e.g., x != 0 does not imply ¬ x : Integer.
//
// This powers (among other things) exhaustiveness checking for T::Enum.
bool isSingleton(core::Context ctx, core::ClassOrModuleRef sym) {
    // Singletons that are built into the Ruby VM
    if (sym == core::Symbols::NilClass() || sym == core::Symbols::FalseClass() || sym == core::Symbols::TrueClass()) {
        return true;
    }

    // T::Enum values are modeled as singletons of their own fake class.
    if (sym.data(ctx)->name.isTEnumName(ctx)) {
        return true;
    }

    // attachedClass on untyped symbol is defined to return itself
    if (sym != core::Symbols::untyped() && sym.data(ctx)->attachedClass(ctx).exists() && sym.data(ctx)->flags.isFinal) {
        // This is a Ruby singleton class object
        return true;
    }

    // The Ruby stdlib has a Singleton module which lets people invent their own singletons.
    return (sym.data(ctx)->derivesFrom(ctx, core::Symbols::Singleton()) && sym.data(ctx)->flags.isFinal);
}

} // namespace

void Environment::updateKnowledge(core::Context ctx, cfg::LocalRef local, core::Loc loc, const cfg::Send *send,
                                  KnowledgeFilter &knowledgeFilter) {
    if (!send->fun.isUpdateKnowledgeName()) {
        // We short circuit here (1) for an honestly negligible performance improvement, but
        // importantly (2) so that if a new method is added to this method, you'll be forced to add it
        // to the above list, as that list of names is special more than just inside this method.
        return;
    }

    if (send->fun == core::Names::bang()) {
        if (!knowledgeFilter.isNeeded(local)) {
            return;
        }
        auto &whoKnows = getKnowledge(local);
        auto fnd = _vars.find(send->recv.variable);
        if (fnd != _vars.end()) {
            whoKnows.replaceTruthy(local, typeTestsWithVar, fnd->second.knowledge.falsy());
            whoKnows.replaceFalsy(local, typeTestsWithVar, fnd->second.knowledge.truthy());
            fnd->second.knowledge.truthy().addYesTypeTest(fnd->first, typeTestsWithVar, local,
                                                          core::Types::falsyTypes());
            fnd->second.knowledge.falsy().addNoTypeTest(fnd->first, typeTestsWithVar, local, core::Types::falsyTypes());
        }
        whoKnows.truthy().addYesTypeTest(local, typeTestsWithVar, send->recv.variable, core::Types::falsyTypes());
        whoKnows.falsy().addNoTypeTest(local, typeTestsWithVar, send->recv.variable, core::Types::falsyTypes());

        whoKnows.sanityCheck();
        return;
    }

    if (send->fun == core::Names::nil_p()) {
        if (!knowledgeFilter.isNeeded(local)) {
            return;
        }
        auto &whoKnows = getKnowledge(local);
        whoKnows.truthy().addYesTypeTest(local, typeTestsWithVar, send->recv.variable, core::Types::nilClass());
        whoKnows.falsy().addNoTypeTest(local, typeTestsWithVar, send->recv.variable, core::Types::nilClass());
        whoKnows.sanityCheck();
        return;
    }

    if (send->fun == core::Names::blank_p()) {
        if (!knowledgeFilter.isNeeded(local)) {
            return;
        }
        // Note that this assumes that .blank? is a rails-compatible monkey patch.
        // In other cases this flow analysis might make incorrect assumptions.
        auto &originalType = send->recv.type;
        auto knowledgeTypeWithoutFalsy = core::Types::approximateSubtract(ctx, originalType, core::Types::falsyTypes());

        if (!core::Types::equiv(ctx, knowledgeTypeWithoutFalsy, originalType)) {
            auto &whoKnows = getKnowledge(local);
            whoKnows.falsy().addYesTypeTest(local, typeTestsWithVar, send->recv.variable, knowledgeTypeWithoutFalsy);
            whoKnows.sanityCheck();
        }
        return;
    }

    if (send->fun == core::Names::present_p()) {
        if (!knowledgeFilter.isNeeded(local)) {
            return;
        }
        // Note that this assumes that .present? is a rails-compatible monkey patch.
        // In other cases this flow analysis might make incorrect assumptions.
        auto &originalType = send->recv.type;
        auto knowledgeTypeWithoutFalsy = core::Types::approximateSubtract(ctx, originalType, core::Types::falsyTypes());

        if (!core::Types::equiv(ctx, knowledgeTypeWithoutFalsy, originalType)) {
            auto &whoKnows = getKnowledge(local);
            whoKnows.truthy().addYesTypeTest(local, typeTestsWithVar, send->recv.variable, knowledgeTypeWithoutFalsy);
            whoKnows.sanityCheck();
        }
        return;
    }

    if (send->args.empty()) {
        return;
    }

    // TODO(jez) We should probably update this to be aware of T::NonForcingConstants.non_forcing_is_a?
    if (send->fun == core::Names::kindOf_p() || send->fun == core::Names::isA_p()) {
        if (!knowledgeFilter.isNeeded(local)) {
            return;
        }
        auto &whoKnows = getKnowledge(local);
        auto &klassType = send->args[0].type;
        core::ClassOrModuleRef klass = core::Types::getRepresentedClass(ctx, klassType);
        if (klass.exists()) {
            auto ty = klass.data(ctx)->externalType();
            if (!ty.isUntyped()) {
                whoKnows.truthy().addYesTypeTest(local, typeTestsWithVar, send->recv.variable, ty);
                whoKnows.falsy().addNoTypeTest(local, typeTestsWithVar, send->recv.variable, ty);
            }
            whoKnows.sanityCheck();
        }
        return;
    }

    if (send->fun == core::Names::eqeq() || send->fun == core::Names::equal_p() || send->fun == core::Names::neq()) {
        if (!knowledgeFilter.isNeeded(local)) {
            return;
        }
        auto &whoKnows = getKnowledge(local);
        const auto &argType = send->args[0].type;
        const auto &recvType = send->recv.type;

        auto funIsEq = send->fun == core::Names::eqeq() || send->fun == core::Names::equal_p();
        auto &truthy = funIsEq ? whoKnows.truthy() : whoKnows.falsy();
        auto &falsy = funIsEq ? whoKnows.falsy() : whoKnows.truthy();

        ENFORCE(argType != nullptr);
        ENFORCE(recvType != nullptr);
        if (!argType.isUntyped()) {
            truthy.addYesTypeTest(local, typeTestsWithVar, send->recv.variable, argType);

            auto argSymbol = core::Symbols::noClassOrModule();
            if (core::isa_type<core::ClassType>(argType)) {
                auto c = core::cast_type_nonnull<core::ClassType>(argType);
                argSymbol = c.symbol;
            } else if (core::isa_type<core::AppliedType>(argType)) {
                auto &a = core::cast_type_nonnull<core::AppliedType>(argType);
                argSymbol = a.klass;
            }
            if (argSymbol.exists() && isSingleton(ctx, argSymbol)) {
                falsy.addNoTypeTest(local, typeTestsWithVar, send->recv.variable, argType);
            }
        }

        if (!recvType.isUntyped()) {
            truthy.addYesTypeTest(local, typeTestsWithVar, send->args[0].variable, recvType);

            core::ClassOrModuleRef recvSymbol = core::Symbols::noClassOrModule();
            if (core::isa_type<core::ClassType>(recvType)) {
                auto c = core::cast_type_nonnull<core::ClassType>(recvType);
                recvSymbol = c.symbol;
            } else if (core::isa_type<core::AppliedType>(recvType)) {
                auto &a = core::cast_type_nonnull<core::AppliedType>(recvType);
                recvSymbol = a.klass;
            }
            if (recvSymbol.exists() && isSingleton(ctx, recvSymbol)) {
                falsy.addNoTypeTest(local, typeTestsWithVar, send->args[0].variable, recvType);
            }
        }

        whoKnows.sanityCheck();
        return;
    }

    if (send->fun == core::Names::tripleEq()) {
        if (!knowledgeFilter.isNeeded(local)) {
            return;
        }
        auto &whoKnows = getKnowledge(local);
        const auto &recvType = send->recv.type;

        // `when` against class literal
        core::ClassOrModuleRef representedClass = core::Types::getRepresentedClass(ctx, recvType);
        if (representedClass.exists()) {
            auto representedType = representedClass.data(ctx)->externalType();
            if (!representedType.isUntyped()) {
                whoKnows.truthy().addYesTypeTest(local, typeTestsWithVar, send->args[0].variable, representedType);
                whoKnows.falsy().addNoTypeTest(local, typeTestsWithVar, send->args[0].variable, representedType);
            }
        }

        // `when` against singleton
        if (core::isa_type<core::ClassType>(recvType)) {
            auto s = core::cast_type_nonnull<core::ClassType>(recvType);
            // check if s is a singleton. in this case we can learn that
            // a failed comparison means that type test would also fail
            if (isSingleton(ctx, s.symbol)) {
                whoKnows.truthy().addYesTypeTest(local, typeTestsWithVar, send->args[0].variable, recvType);
                whoKnows.falsy().addNoTypeTest(local, typeTestsWithVar, send->args[0].variable, recvType);
            }
        }
        whoKnows.sanityCheck();
        return;
    }

    if (send->fun == core::Names::lessThan() || send->fun == core::Names::leq()) {
        const auto &recvKlass = send->recv.type;
        const auto &argType = send->args[0].type;

        if (core::isa_type<core::ClassType>(argType)) {
            auto argClass = core::cast_type_nonnull<core::ClassType>(argType);
            if (!recvKlass.derivesFrom(ctx, core::Symbols::Class()) ||
                !argClass.symbol.data(ctx)->derivesFrom(ctx, core::Symbols::Class())) {
                return;
            }
        } else if (auto *argClass = core::cast_type<core::AppliedType>(argType)) {
            if (!recvKlass.derivesFrom(ctx, core::Symbols::Class()) ||
                !argClass->klass.data(ctx)->derivesFrom(ctx, core::Symbols::Class())) {
                return;
            }
        } else {
            return;
        }

        auto &whoKnows = getKnowledge(local);
        whoKnows.truthy().addYesTypeTest(local, typeTestsWithVar, send->recv.variable, argType);
        if (send->fun == core::Names::leq()) {
            // We only know the NoTypeTest for `<=`, not `<`, because `x < A` being false could mean
            // that `x == A`, which would mean `x` still has type `T.class_of(A)`.
            whoKnows.falsy().addNoTypeTest(local, typeTestsWithVar, send->recv.variable, argType);
        }
        whoKnows.sanityCheck();
        return;
    }
}

void Environment::setTypeAndOrigin(cfg::LocalRef symbol, const core::TypeAndOrigins &typeAndOrigins) {
    ENFORCE(typeAndOrigins.type != nullptr);
    _vars[symbol].typeAndOrigins = typeAndOrigins;
}

const Environment &Environment::withCond(core::Context ctx, const Environment &env, Environment &copy, bool isTrue,
                                         const UnorderedMap<cfg::LocalRef, VariableState> &filter) {
    ENFORCE(env.bb->bexit.cond.variable.exists());
    if (env.bb->bexit.cond.variable == cfg::LocalRef::unconditional() ||
        env.bb->bexit.cond.variable == cfg::LocalRef::blockCall()) {
        return env;
    }
    copy.cloneFrom(env);
    copy.assumeKnowledge(ctx, isTrue, env.bb->bexit.cond.variable, ctx.locAt(env.bb->bexit.loc), filter);
    return copy;
}

void Environment::assumeKnowledge(core::Context ctx, bool isTrue, cfg::LocalRef cond, core::Loc loc,
                                  const UnorderedMap<cfg::LocalRef, VariableState> &filter) {
    const auto &thisKnowledge = getKnowledge(cond, false);
    thisKnowledge.sanityCheck();
    if (!isTrue) {
        if (getKnownTruthy(cond)) {
            isDead = true;
            return;
        }

        core::TypeAndOrigins tp = getTypeAndOrigin(ctx, cond);
        tp.origins.emplace_back(loc);
        if (tp.type.isUntyped()) {
            tp.type = core::Types::falsyTypes();
        } else {
            tp.type = core::Types::all(ctx, tp.type, core::Types::falsyTypes());
            if (tp.type.isBottom()) {
                isDead = true;
                return;
            }
        }
        setTypeAndOrigin(cond, tp);
    } else {
        core::TypeAndOrigins tp = getTypeAndOrigin(ctx, cond);
        tp.origins.emplace_back(loc);
        tp.type = core::Types::dropSubtypesOf(ctx, core::Types::dropSubtypesOf(ctx, tp.type, core::Symbols::NilClass()),
                                              core::Symbols::FalseClass());
        if (tp.type.isBottom()) {
            isDead = true;
            return;
        }
        setTypeAndOrigin(cond, tp);
        _vars[cond].knownTruthy = true;
    }

    if (isDead) {
        return;
    }

    auto &knowledgeToChoose = isTrue ? thisKnowledge.truthy() : thisKnowledge.falsy();
    auto &yesTests = knowledgeToChoose->yesTypeTests;
    auto &noTests = knowledgeToChoose->noTypeTests;

    for (auto &typeTested : yesTests) {
        if (!filter.contains(typeTested.first)) {
            continue;
        }
        core::TypeAndOrigins tp = getTypeAndOrigin(ctx, typeTested.first);
        auto glbbed = core::Types::all(ctx, tp.type, typeTested.second);
        if (tp.type != glbbed) {
            tp.origins.emplace_back(loc);
            tp.type = std::move(glbbed);
        }
        setTypeAndOrigin(typeTested.first, tp);
        if (tp.type.isBottom()) {
            isDead = true;
            return;
        }
    }

    for (auto &typeTested : noTests) {
        if (!filter.contains(typeTested.first)) {
            continue;
        }
        core::TypeAndOrigins tp = getTypeAndOrigin(ctx, typeTested.first);
        tp.origins.emplace_back(loc);

        if (!tp.type.isUntyped()) {
            tp.type = core::Types::approximateSubtract(ctx, tp.type, typeTested.second);
            setTypeAndOrigin(typeTested.first, tp);
            if (tp.type.isBottom()) {
                isDead = true;
                return;
            }
        }
    }
}

void Environment::mergeWith(core::Context ctx, const Environment &other, cfg::CFG &inWhat, cfg::BasicBlock *bb,
                            KnowledgeFilter &knowledgeFilter) {
    this->isDead |= other.isDead;
    for (auto &pair : _vars) {
        auto var = pair.first;
        const auto &otherTO = other.getTypeAndOrigin(ctx, var);
        auto &thisTO = pair.second.typeAndOrigins;
        if (thisTO.type != nullptr) {
            thisTO.type = core::Types::any(ctx, thisTO.type, otherTO.type);
            thisTO.type.sanityCheck(ctx);
            for (auto origin : otherTO.origins) {
                if (!absl::c_linear_search(thisTO.origins, origin)) {
                    thisTO.origins.emplace_back(origin);
                }
            }
            pair.second.knownTruthy = pair.second.knownTruthy && other.getKnownTruthy(var);
        } else {
            thisTO = otherTO;
            pair.second.knownTruthy = other.getKnownTruthy(var);
        }

        if (((bb->flags & cfg::CFG::LOOP_HEADER) != 0) && bb->outerLoops <= var.maxLoopWrite(inWhat)) {
            continue;
        }
        bool canBeFalsy = core::Types::canBeFalsy(ctx, otherTO.type) && !other.getKnownTruthy(var);
        bool canBeTruthy = core::Types::canBeTruthy(ctx, otherTO.type);

        if (canBeTruthy) {
            auto &thisKnowledge = getKnowledge(var);
            auto otherTruthy =
                other.getKnowledge(var, false).truthy().under(ctx, other, inWhat, bb, knowledgeFilter.isNeeded(var));
            if (!otherTruthy->isDead) {
                if (!thisKnowledge.seenTruthyOption) {
                    thisKnowledge.seenTruthyOption = true;
                    thisKnowledge.replaceTruthy(var, typeTestsWithVar, otherTruthy);
                } else {
                    thisKnowledge.truthy().min(ctx, *otherTruthy);
                }
            }
        }

        if (canBeFalsy) {
            auto &thisKnowledge = getKnowledge(var);
            auto otherFalsy =
                other.getKnowledge(var, false).falsy().under(ctx, other, inWhat, bb, knowledgeFilter.isNeeded(var));
            if (!otherFalsy->isDead) {
                if (!thisKnowledge.seenFalsyOption) {
                    thisKnowledge.seenFalsyOption = true;
                    thisKnowledge.replaceFalsy(var, typeTestsWithVar, otherFalsy);
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

    for (auto &pair : _vars) {
        auto var = pair.first;
        core::TypeAndOrigins tp;

        auto bindMinLoops = var.minLoops(inWhat);
        if (bb->outerLoops == bindMinLoops || bindMinLoops == var.maxLoopWrite(inWhat)) {
            continue;
        }

        for (cfg::BasicBlock *parent : bb->backEdges) {
            auto &other = envs[parent->id];
            auto otherPin = other.pinnedTypes.find(var);
            if (otherPin != other.pinnedTypes.end()) {
                if (tp.type != nullptr) {
                    tp.type = core::Types::any(ctx, tp.type, otherPin->second.type);
                    for (auto origin : otherPin->second.origins) {
                        if (!absl::c_linear_search(tp.origins, origin)) {
                            tp.origins.emplace_back(origin);
                        }
                    }
                    tp.type.sanityCheck(ctx);
                } else {
                    tp = otherPin->second;
                }
            }
        }

        if (tp.type != nullptr) {
            pinnedTypes[var] = tp;
        }
    }
}

void Environment::populateFrom(core::Context ctx, const Environment &other) {
    this->isDead = other.isDead;
    for (auto &pair : _vars) {
        auto var = pair.first;
        pair.second.typeAndOrigins = other.getTypeAndOrigin(ctx, var);
        pair.second.knowledge.replace(var, typeTestsWithVar, other.getKnowledge(var, false));
        pair.second.knownTruthy = other.getKnownTruthy(var);
    }

    this->pinnedTypes = other.pinnedTypes;
}

core::TypePtr flatmapHack(core::Context ctx, const core::TypePtr &receiver, const core::TypePtr &returnType,
                          core::NameRef fun, const core::Loc &loc) {
    if (fun != core::Names::flatMap()) {
        return returnType;
    }
    if (!receiver.derivesFrom(ctx, core::Symbols::Enumerable())) {
        return returnType;
    }

    if (!receiver.isUntyped() && (receiver.derivesFrom(ctx, core::Symbols::Enumerator_Lazy()) ||
                                  receiver.derivesFrom(ctx, core::Symbols::Enumerator_Chain()))

    ) {
        return returnType;
    }

    int64_t flattenDepth = 1;
    auto mapType = core::Types::arrayOf(ctx, returnType);

    const core::TypeAndOrigins recvType{mapType, loc};
    core::TypeAndOrigins arg{core::make_type<core::IntegerLiteralType>((int64_t)flattenDepth), recvType.origins};
    InlinedVector<const core::TypeAndOrigins *, 2> args{&arg};

    InlinedVector<core::LocOffsets, 2> argLocs{loc.offsets()};
    core::CallLocs locs{
        ctx.file, loc.offsets(), loc.offsets(), loc.offsets(), argLocs,
    };

    core::DispatchArgs dispatchArgs{core::Names::flatten(), locs,    1,   args, recvType.type, recvType,
                                    recvType.type,          nullptr, loc, true, false};

    auto dispatched = recvType.type.dispatchCall(ctx, dispatchArgs);
    if (dispatched.main.errors.empty()) {
        return std::move(dispatched.returnType);
    }

    return mapType;
}

core::TypePtr
Environment::processBinding(core::Context ctx, const cfg::CFG &inWhat, cfg::Binding &bind, int loopCount,
                            int bindMinLoops, KnowledgeFilter &knowledgeFilter, core::TypeConstraint &constr,
                            core::TypePtr &methodReturnType,
                            const optional<cfg::BasicBlock::BlockExitCondInfo> &parentUpdateKnowledgeReceiver) {
    try {
        core::TypeAndOrigins tp;
        bool noLoopChecking = cfg::isa_instruction<cfg::Alias>(bind.value) ||
                              cfg::isa_instruction<cfg::LoadArg>(bind.value) ||
                              bind.bind.variable == cfg::LocalRef::selfVariable();

        bool checkFullyDefined = true;
        const core::lsp::Query &lspQuery = ctx.state.lspQuery;
        bool lspQueryMatch = lspQuery.matchesLoc(ctx.locAt(bind.loc));

        typecase(
            bind.value,
            [&](cfg::Send &send) {
                // Overwrite lspQueryMatch for send nodes, so that we only produce a result when
                // hovering over the method or arguments (not the block)
                auto locWithoutBlock = send.locWithoutBlock(bind.loc);
                if (lspQueryMatch && locWithoutBlock.exists() && !locWithoutBlock.empty()) {
                    lspQueryMatch = lspQuery.matchesLoc(ctx.locAt(locWithoutBlock));
                }

                InlinedVector<const core::TypeAndOrigins *, 2> args;

                args.reserve(send.args.size());
                for (cfg::VariableUseSite &arg : send.args) {
                    args.emplace_back(&getAndFillTypeAndOrigin(ctx, arg));
                }

                const core::TypeAndOrigins &recvType = getAndFillTypeAndOrigin(ctx, send.recv);
                if (send.link) {
                    checkFullyDefined = false;
                }
                core::CallLocs locs{
                    ctx.file, bind.loc, send.receiverLoc, send.funLoc, send.argLocs,
                };

                // This is the main place where we type check a method, so we default by assuming
                // that we want to report all errors (supressing nothing).
                auto suppressErrors = false;
                core::DispatchArgs dispatchArgs{
                    send.fun,  locs,     send.numPosArgs,  args,          recvType.type, recvType, recvType.type,
                    send.link, ownerLoc, send.isPrivateOk, suppressErrors};
                auto dispatched = recvType.type.dispatchCall(ctx, dispatchArgs);

                auto it = &dispatched;
                while (it != nullptr) {
                    for (auto &err : it->main.errors) {
                        if (err->what != core::errors::Infer::UnknownMethod ||
                            !parentUpdateKnowledgeReceiver.has_value()) {
                            ctx.state._error(std::move(err));
                            continue;
                        }

                        auto &parentRecv = parentUpdateKnowledgeReceiver.value();
                        auto recvLoc = ctx.locAt(send.receiverLoc);
                        auto parentRecvLoc = ctx.locAt(parentRecv.loc);

                        if (recvLoc.source(ctx) != parentRecvLoc.source(ctx) ||
                            !core::Types::equivNoUntyped(ctx, recvType.type, parentRecv.recv.type)) {
                            // Safeguard against most unrelated missing method errors
                            // Out of ~~laziness~~ simplicitity, this does not account for things
                            // like `==` or `===` where the relevant type to check is arg0, not recv
                            ctx.state._error(std::move(err));
                            continue;
                        }

                        if (auto e = ctx.state.beginError(err->loc, core::errors::Infer::UnknownMethod)) {
                            e.setHeader("{}", err->header);

                            // This copies the section intentionally (no auto&) because all err fields are const
                            for (auto section : err->sections) {
                                if (!absl::StartsWith(section.header, "Autocorrect") &&
                                    !absl::StartsWith(section.header, "Did you mean")) {
                                    // Calling addAutocorrect below recreates the sections that go with autocorrects
                                    e.addErrorSection(std::move(section));
                                }
                            }

                            e.addErrorSection(core::ErrorSection(
                                "Possible misuse of flow-sensitive typing:",
                                {
                                    core::ErrorLine::from(
                                        parentRecvLoc,
                                        "The preceding condition calls `{}` on an expression, not a variable",
                                        parentRecv.fun.show(ctx)),
                                    core::ErrorLine::fromWithoutLoc(
                                        "Sorbet only tracks flow-sensitive knowledge on variables, not methods."),
                                    core::ErrorLine::fromWithoutLoc(
                                        "See https://sorbet.org/docs/flow-sensitive#limitations-of-flow-sensitivity"),
                                    core::ErrorLine::fromWithoutLoc(
                                        "To fix, refactor so the `{}` call is on a variable and use that variable "
                                        "everywhere{}",
                                        parentRecv.fun.show(ctx),
                                        err->autocorrects.empty()
                                            ? "."
                                            : ",\n    or accept the autocorrect to silence this error."),
                                }));

                            // It would be nice to report an autocorrect suggestion to factor out a
                            // variable, but we don't have an easy way to find all expressions in
                            // the body that would need to be factored out. So we pass through any
                            // autocorrects untouched.

                            // This copies the section intentionally (no auto&) because all err fields are const
                            for (auto autocorrect : err->autocorrects) {
                                e.addAutocorrect(std::move(autocorrect));
                            }
                        }
                    }

                    if (it->main.method.exists() && it->main.method.data(ctx)->flags.isPackagePrivate) {
                        core::ClassOrModuleRef klass = it->main.method.data(ctx)->owner;
                        if (klass.exists()) {
                            const auto &curPkg = ctx.state.packageDB().getPackageForFile(ctx, ctx.file);
                            if (curPkg.exists() && !curPkg.ownsSymbol(ctx, klass)) {
                                if (auto e = ctx.beginError(bind.loc, core::errors::Infer::PackagePrivateMethod)) {
                                    auto &curPkgName = curPkg.fullName();
                                    // TODO (aadi-stripe, add name of owning package to message).
                                    e.setHeader(
                                        "Method `{}` on `{}` is package-private and cannot be called from package `{}`",
                                        it->main.method.data(ctx)->name.show(ctx), klass.show(ctx),
                                        fmt::map_join(curPkgName, "::", [&](const auto &nr) { return nr.show(ctx); }));
                                    e.addErrorLine(it->main.method.data(ctx)->loc(), "Defined in `{}` here",
                                                   it->main.method.data(ctx)->owner.show(ctx));
                                }
                            }
                        }
                    }

                    lspQueryMatch = lspQueryMatch || lspQuery.matchesSymbol(it->main.method);
                    it = it->secondary.get();
                }
                shared_ptr<core::DispatchResult> retainedResult;
                if (send.link) {
                    // this type should never be used, thus we put a useless type
                    tp.type = core::Types::void_();
                } else {
                    tp.type = dispatched.returnType;
                }
                if (send.link || lspQueryMatch) {
                    retainedResult = make_shared<core::DispatchResult>(std::move(dispatched));
                }
                if (send.link) {
                    // This should eventually become ENFORCEs but currently they are wrong
                    if (!retainedResult->main.blockReturnType) {
                        retainedResult->main.blockReturnType = core::Types::untyped(retainedResult->main.method);
                    }
                    if (!retainedResult->main.blockPreType) {
                        retainedResult->main.blockPreType = core::Types::untyped(retainedResult->main.method);
                    }
                    ENFORCE(retainedResult->main.sendTp);
                }

                // For `case x; when X ...`, desugar produces `X.===(x)`, but with
                // a zero-length funLoc.  We tried producing a zero-length loc for
                // the entire send so there would never be a match here, but that
                // interacted poorly with a number of testcases (see discussion in
                // https://github.com/sorbet/sorbet/pull/5015).  The next-best thing
                // seemed to be detecting `===` calls that appear to have been produced
                // by desugar and redacting the `SendResponse` so LSP features work
                // more like developers expect.
                const bool isDesugarTripleEqSend = send.fun == core::Names::tripleEq() && send.funLoc.empty();
                // Something like `X = Foo` will be rewritten to `X = Magic.<suggest-constant-type>(Foo)` if `Foo` fails
                // to resolve. We don't want to report a send response for the <suggest-constant-type> call here.
                const bool isSuggestConstantType = send.fun == core::Names::suggestConstantType();

                const bool ignoreSendForLSPQuery = isDesugarTripleEqSend || isSuggestConstantType;
                if (lspQueryMatch && !ignoreSendForLSPQuery) {
                    auto fun = send.fun;
                    if (fun == core::Names::checkAndAnd() && core::isa_type<core::NamedLiteralType>(args[1]->type)) {
                        auto lit = core::cast_type_nonnull<core::NamedLiteralType>(args[2]->type);
                        if (lit.derivesFrom(ctx, core::Symbols::Symbol())) {
                            fun = lit.asName();
                        }
                    }
                    core::lsp::QueryResponse::pushQueryResponse(
                        ctx,
                        core::lsp::SendResponse(retainedResult, send.argLocs, fun, ctx.owner.asMethodRef(),
                                                send.isPrivateOk, ctx.file, bind.loc, send.receiverLoc, send.funLoc));
                }
                if (send.link) {
                    send.link->result = move(retainedResult);
                }
                if (send.fun == core::Names::toHashDup()) {
                    ENFORCE(send.args.size() == 1, "Desugar invariant");
                    tp.origins = args[0]->origins;
                }
                tp.origins.emplace_back(ctx.locAt(bind.loc));
            },
            [&](cfg::Ident &i) {
                const core::TypeAndOrigins &typeAndOrigin = getTypeAndOrigin(ctx, i.what);
                tp.type = typeAndOrigin.type;
                tp.origins = typeAndOrigin.origins;

                if (lspQueryMatch && !bind.value.isSynthetic()) {
                    core::lsp::QueryResponse::pushQueryResponse(
                        ctx, core::lsp::IdentResponse(ctx.locAt(bind.loc), i.what.data(inWhat), tp,
                                                      ctx.owner.asMethodRef(), ctx.locAt(inWhat.loc)));
                }

                ENFORCE(ctx.file.data(ctx).hasParseErrors() || !tp.origins.empty(),
                        "Inferencer did not assign location");
            },
            [&](cfg::Alias &a) {
                core::SymbolRef symbol = a.what.dealias(ctx);
                if (symbol.isClassOrModule()) {
                    auto singletonClass = symbol.asClassOrModuleRef().data(ctx)->lookupSingletonClass(ctx);
                    ENFORCE(singletonClass.exists(), "Every class should have a singleton class by now.");
                    tp.type = singletonClass.data(ctx)->externalType();
                    tp.origins.emplace_back(ctx.locAt(bind.loc));
                } else if (symbol.isField(ctx) ||
                           (symbol.isStaticField(ctx) &&
                            !symbol.asFieldRef().data(ctx)->flags.isStaticFieldTypeAlias) ||
                           symbol.isTypeMember()) {
                    auto resultType = symbol.resultType(ctx);
                    if (resultType != nullptr) {
                        if (symbol.isTypeMember()) {
                            auto tm = symbol.asTypeMemberRef();
                            if (tm.data(ctx)->flags.isFixed) {
                                // pick the upper bound here, as
                                // isFixed() => lowerBound == upperBound.
                                auto lambdaParam = core::cast_type<core::LambdaParam>(resultType);
                                ENFORCE(lambdaParam != nullptr);
                                tp.type = core::make_type<core::MetaType>(lambdaParam->upperBound);
                            } else {
                                tp.type = core::make_type<core::MetaType>(core::make_type<core::SelfTypeParam>(symbol));
                            }
                        } else if (symbol.isField(ctx)) {
                            auto field = symbol.asFieldRef();
                            tp.type = core::Types::resultTypeAsSeenFrom(
                                ctx, field.data(ctx)->resultType, symbol.owner(ctx).asClassOrModuleRef(),
                                ctx.owner.enclosingClass(ctx),
                                ctx.owner.enclosingClass(ctx).data(ctx)->selfTypeArgs(ctx));
                        } else {
                            tp.type = resultType;
                        }
                        tp.origins.emplace_back(symbol.loc(ctx));
                    } else {
                        tp.origins.emplace_back(core::Loc::none());
                        tp.type = core::Types::untyped(symbol);
                    }
                } else if (symbol.isTypeAlias(ctx)) {
                    ENFORCE(symbol.resultType(ctx) != nullptr);
                    tp.origins.emplace_back(symbol.loc(ctx));
                    tp.type = core::make_type<core::MetaType>(symbol.resultType(ctx));
                } else if (symbol.isTypeArgument()) {
                    ENFORCE(symbol.resultType(ctx) != nullptr);
                    tp.origins.emplace_back(ctx.locAt(bind.loc));

                    auto owner = ctx.owner.asMethodRef();
                    auto klass = owner.enclosingClass(ctx);
                    ENFORCE(symbol.resultType(ctx) != nullptr);
                    auto instantiated = core::Types::resultTypeAsSeenFrom(ctx, symbol.resultType(ctx), klass, klass,
                                                                          klass.data(ctx)->selfTypeArgs(ctx));
                    if (owner.data(ctx)->flags.isGenericMethod) {
                        // instantiate requires a frozen constraint, but the constraint might not be
                        // frozen when we're running in guessTypes mode (and we never guess types if
                        // the method is already generic).
                        instantiated = core::Types::instantiate(ctx, instantiated, constr);
                    }
                    tp.type = core::make_type<core::MetaType>(instantiated);
                } else {
                    Exception::notImplemented();
                }

                pinnedTypes[bind.bind.variable] = tp;
            },
            [&](cfg::SolveConstraint &i) {
                core::TypePtr type;
                // TODO: this should repeat the same dance with Or and And components that dispatchCall does
                const auto &main = i.link->result->main;
                if (main.constr) {
                    if (!main.constr->solve(ctx)) {
                        if (auto e = ctx.beginError(bind.loc, core::errors::Infer::GenericMethodConstraintUnsolved)) {
                            e.setHeader("Could not find valid instantiation of type parameters for `{}`",
                                        main.method.show(ctx));
                            e.addErrorLine(main.method.data(ctx)->loc(), "`{}` defined here", main.method.show(ctx));
                            e.addErrorSection(main.constr->explain(ctx));
                        }
                        type = core::Types::untypedUntracked();
                    } else {
                        type = core::Types::instantiate(ctx, main.sendTp, *main.constr);
                    }
                } else {
                    type = i.link->result->returnType;
                }
                auto loc = ctx.locAt(bind.loc);
                type = flatmapHack(ctx, main.receiver, type, i.link->fun, loc);
                tp.type = std::move(type);
                tp.origins.emplace_back(loc);
            },
            [&](cfg::LoadArg &i) {
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
                ENFORCE(ctx.owner == i.method);

                const auto &argInfo = i.argument(ctx);
                auto argType = argInfo.argumentTypeAsSeenByImplementation(ctx, constr);
                tp.type = std::move(argType);
                tp.origins.emplace_back(ctx.locAt(bind.loc));

                if (lspQuery.matchesLoc(argInfo.loc)) {
                    core::lsp::QueryResponse::pushQueryResponse(
                        ctx, core::lsp::IdentResponse(argInfo.loc, bind.bind.variable.data(inWhat), tp,
                                                      ctx.owner.asMethodRef(), ctx.locAt(inWhat.loc)));
                }
            },
            [&](cfg::ArgPresent &i) {
                // Return an unanalyzable boolean value that indicates whether or not arg was provided
                // It's unanalyzable because it varies by each individual call site.
                ENFORCE(ctx.owner == i.method);

                tp.type = core::Types::Boolean();
                tp.origins.emplace_back(ctx.locAt(bind.loc));
            },
            [&](cfg::LoadYieldParams &insn) {
                ENFORCE(insn.link);
                ENFORCE(insn.link->result);
                ENFORCE(insn.link->result->main.blockPreType);

                auto &procType = insn.link->result->main.blockPreType;
                auto params = procType.getCallArguments(ctx, core::Names::call());
                auto it = insn.link->result->secondary.get();
                while (it != nullptr) {
                    auto &secondaryProcType = it->main.blockPreType;
                    if (secondaryProcType != nullptr) {
                        auto secondaryParams = secondaryProcType.getCallArguments(ctx, core::Names::call());
                        switch (insn.link->result->secondaryKind) {
                            case core::DispatchResult::Combinator::OR:
                                params = core::Types::any(ctx, params, secondaryParams);
                                break;
                            case core::DispatchResult::Combinator::AND:
                                params = core::Types::all(ctx, params, secondaryParams);
                                break;
                        }
                    } else {
                        // One of the components doesn't have a blockPreType. An error was already
                        // reported by calls.cc (like "Method `.map` doesn't exist on `NilClass`")
                        // Here, we just want to not crash.
                        //
                        // We could do something like set `params` to `T.untyped`, but it's probably
                        // nicer to let it be whatever it would have been. This way they'll get
                        // completion results as if the previous type error was fixed. So do nothing.
                    }

                    it = it->secondary.get();
                }

                // A multi-arg proc, if provided a single arg which is an array,
                // will implicitly splat it out.
                //
                // TODO(nelhage): If this block is a lambda, not a proc, this
                // rule doesn't apply. We don't model the distinction accurately
                // yet.
                auto &blkArgs = insn.link->argFlags;
                auto *tuple = core::cast_type<core::TupleType>(params);
                if (blkArgs.size() > 1 && !blkArgs.front().isRepeated && tuple && tuple->elems.size() == 1 &&
                    tuple->elems.front().derivesFrom(ctx, core::Symbols::Array())) {
                    tp.type = std::move(tuple->elems.front());
                } else if (params == nullptr) {
                    tp.type = core::Types::untypedUntracked();
                } else {
                    tp.type = params;
                }
                tp.origins.emplace_back(ctx.locAt(bind.loc));
            },
            [&](cfg::YieldParamPresent &i) {
                // Return an unanalyzable boolean value that indicates whether or not arg was provided
                // It's unanalyzable because it varies by each individual call site.
                tp.type = core::Types::Boolean();
                tp.origins.emplace_back(ctx.locAt(bind.loc));
            },
            [&](cfg::YieldLoadArg &i) {
                // Mixing positional and rest args in blocks is not well supported.
                // Use these special cases for now.
                // TODO: handle kwsplats here as well.
                if (i.flags.isRepeated) {
                    if (i.argId == 0) {
                        const core::TypeAndOrigins &argsType = getAndFillTypeAndOrigin(ctx, i.yieldParam);
                        tp.type = argsType.type;
                    } else {
                        tp.type = core::Types::untypedUntracked();
                    }
                    tp.origins.emplace_back(ctx.locAt(bind.loc));
                    return;
                }

                // Fetch the type for the argument out of the parameters for the block
                // by simulating a blockParam[i] call.
                const core::TypeAndOrigins &recvType = getAndFillTypeAndOrigin(ctx, i.yieldParam);

                if (recvType.type.isUntyped()) {
                    // This avoids reporting an untyped usage for ->(x) { 0 }. Sorbet would
                    // initialize the type of the local `x` by calling <blk>.[](0), which makes it
                    // look like we're "using" an untyped value, but that's purely internal to
                    // Sorbet. By early returning here, we'll only report an untyped usage if that
                    // real argument ends up then getting used.
                    tp.type = recvType.type;
                } else {
                    core::TypePtr argType = core::make_type<core::IntegerLiteralType>((int64_t)i.argId);

                    core::TypeAndOrigins arg{argType, recvType.origins};
                    InlinedVector<const core::TypeAndOrigins *, 2> args;
                    args.emplace_back(&arg);

                    InlinedVector<core::LocOffsets, 2> argLocs;
                    argLocs.emplace_back(bind.loc);
                    core::CallLocs locs{
                        ctx.file, bind.loc, bind.loc, bind.loc, argLocs,
                    };

                    const auto numPosArgs = 1;
                    const auto suppressErrors = true;
                    const auto isPrivateOk = true;
                    const std::shared_ptr<const core::SendAndBlockLink> block = nullptr;
                    core::DispatchArgs dispatchArgs{core::Names::squareBrackets(),
                                                    locs,
                                                    numPosArgs,
                                                    args,
                                                    recvType.type,
                                                    recvType,
                                                    recvType.type,
                                                    block,
                                                    ctx.locAt(bind.loc),
                                                    isPrivateOk,
                                                    suppressErrors};
                    auto dispatched = recvType.type.dispatchCall(ctx, dispatchArgs);
                    tp.type = dispatched.returnType;
                }
                tp.origins.emplace_back(ctx.locAt(bind.loc));

                if (lspQueryMatch) {
                    core::lsp::QueryResponse::pushQueryResponse(
                        ctx, core::lsp::IdentResponse(ctx.locAt(bind.loc), bind.bind.variable.data(inWhat), tp,
                                                      ctx.owner.asMethodRef(), ctx.locAt(inWhat.loc)));
                }
            },
            [&](cfg::Return &i) {
                tp.type = core::Types::bottom();
                tp.origins.emplace_back(ctx.locAt(bind.loc));

                const core::TypeAndOrigins &typeAndOrigin = getAndFillTypeAndOrigin(ctx, i.what);
                if (core::Types::isSubType(ctx, core::Types::void_(), methodReturnType)) {
                    methodReturnType = core::Types::top();
                }
                if (!core::Types::isSubTypeUnderConstraint(ctx, constr, typeAndOrigin.type, methodReturnType,
                                                           core::UntypedMode::AlwaysCompatible)) {
                    if (auto e = ctx.beginError(bind.loc, core::errors::Infer::ReturnTypeMismatch)) {
                        auto owner = ctx.owner;
                        e.setHeader("Expected `{}` but found `{}` for method result type", methodReturnType.show(ctx),
                                    typeAndOrigin.type.show(ctx));
                        auto for_ = core::ErrorColors::format("result type of method `{}`", owner.name(ctx).show(ctx));
                        e.addErrorSection(
                            core::TypeAndOrigins::explainExpected(ctx, methodReturnType, owner.loc(ctx), for_));
                        e.addErrorSection(typeAndOrigin.explainGot(ctx, ownerLoc));
                        core::TypeErrorDiagnostics::explainTypeMismatch(ctx, e, methodReturnType, typeAndOrigin.type);
                        if (i.whatLoc != inWhat.implicitReturnLoc) {
                            auto replaceLoc = ctx.locAt(i.whatLoc);
                            core::TypeErrorDiagnostics::maybeAutocorrect(ctx, e, replaceLoc, constr, methodReturnType,
                                                                         typeAndOrigin.type);
                        }
                    }
                } else if (!methodReturnType.isUntyped() && !methodReturnType.isTop() &&
                           typeAndOrigin.type.isUntyped()) {
                    auto what = core::errors::Infer::errorClassForUntyped(ctx, ctx.file, typeAndOrigin.type);
                    if (auto e = ctx.beginError(bind.loc, what)) {
                        e.setHeader("Value returned from method is `{}`", "T.untyped");
                        core::TypeErrorDiagnostics::explainUntyped(ctx, e, what, typeAndOrigin, ownerLoc);
                    }
                }
            },
            [&](cfg::BlockReturn &i) {
                ENFORCE(i.link);
                ENFORCE(i.link->result->main.blockReturnType != nullptr);

                const core::TypeAndOrigins &typeAndOrigin = getAndFillTypeAndOrigin(ctx, i.what);
                auto expectedType = i.link->result->main.blockReturnType;
                if (core::Types::isSubType(ctx, core::Types::void_(), expectedType)) {
                    expectedType = core::Types::top();
                }
                bool isSubtype;
                if (i.link->result->main.constr) {
                    isSubtype =
                        core::Types::isSubTypeUnderConstraint(ctx, *i.link->result->main.constr, typeAndOrigin.type,
                                                              expectedType, core::UntypedMode::AlwaysCompatible);
                } else {
                    isSubtype = core::Types::isSubType(ctx, typeAndOrigin.type, expectedType);
                }
                if (!isSubtype) {
                    if (auto e = ctx.beginError(bind.loc, core::errors::Infer::ReturnTypeMismatch)) {
                        e.setHeader("Expected `{}` but found `{}` for block result type", expectedType.show(ctx),
                                    typeAndOrigin.type.show(ctx));

                        const auto &bspec = i.link->result->main.method.data(ctx)->arguments.back();
                        ENFORCE(bspec.flags.isBlock, "The last symbol must be the block arg");
                        e.addErrorSection(
                            core::TypeAndOrigins::explainExpected(ctx, expectedType, bspec.loc, "block result type"));

                        e.addErrorSection(typeAndOrigin.explainGot(ctx, ownerLoc));
                        core::TypeErrorDiagnostics::explainTypeMismatch(ctx, e, expectedType, typeAndOrigin.type);
                    }
                } else if (!expectedType.isUntyped() && !expectedType.isTop() && typeAndOrigin.type.isUntyped()) {
                    auto what = core::errors::Infer::errorClassForUntyped(ctx, ctx.file, typeAndOrigin.type);
                    if (auto e = ctx.beginError(bind.loc, what)) {
                        e.setHeader("Value returned from block is `{}`", "T.untyped");
                        core::TypeErrorDiagnostics::explainUntyped(ctx, e, what, typeAndOrigin, ownerLoc);
                    }
                }

                tp.type = core::Types::bottom();
                tp.origins.emplace_back(ctx.locAt(bind.loc));
            },
            [&](cfg::Literal &i) {
                tp.type = i.value;
                tp.origins.emplace_back(ctx.locAt(bind.loc));

                if (lspQueryMatch) {
                    core::lsp::QueryResponse::pushQueryResponse(ctx,
                                                                core::lsp::LiteralResponse(ctx.locAt(bind.loc), tp));
                }
            },
            [&](cfg::TAbsurd &i) {
                const core::TypeAndOrigins &typeAndOrigin = getTypeAndOrigin(ctx, i.what.variable);

                if (auto e = ctx.beginError(bind.loc, core::errors::Infer::NotExhaustive)) {
                    if (typeAndOrigin.type.isUntyped()) {
                        e.setHeader("Control flow could reach `{}` because argument was `{}`", "T.absurd", "T.untyped");
                    } else {
                        e.setHeader("Control flow could reach `{}` because the type `{}` wasn't handled", "T.absurd",
                                    typeAndOrigin.type.show(ctx));
                    }
                    e.addErrorSection(typeAndOrigin.explainGot(ctx, ownerLoc));
                }

                tp.type = core::Types::bottom();
                tp.origins.emplace_back(ctx.locAt(bind.loc));
            },
            [&](cfg::KeepAlive &i) {
                tp.type = core::Types::untypedUntracked();
                tp.origins.emplace_back(ctx.locAt(bind.loc));
            },
            [&](cfg::GetCurrentException &i) {
                tp.type = core::Types::untypedUntracked();
                tp.origins.emplace_back(ctx.locAt(bind.loc));
            },
            [&](cfg::LoadSelf &l) {
                ENFORCE(l.link);
                auto tpo = getTypeFromRebind(ctx, l.link->result->main, l.fallback);
                auto it = l.link->result->secondary.get();
                while (it != nullptr) {
                    auto secondaryTpo = getTypeFromRebind(ctx, it->main, l.fallback);
                    switch (l.link->result->secondaryKind) {
                        case core::DispatchResult::Combinator::OR:
                            tpo.type = core::Types::any(ctx, tpo.type, secondaryTpo.type);
                            break;
                        case core::DispatchResult::Combinator::AND:
                            tpo.type = core::Types::all(ctx, tpo.type, secondaryTpo.type);
                            break;
                    }
                    tpo.origins.insert(tpo.origins.begin(), make_move_iterator(secondaryTpo.origins.begin()),
                                       make_move_iterator(secondaryTpo.origins.end()));

                    it = it->secondary.get();
                }

                tp = tpo;
            },
            [&](cfg::Cast &c) {
                auto klass = ctx.owner.enclosingClass(ctx);

                auto castType = core::Types::instantiate(ctx, c.type, klass.data(ctx)->typeMembers(),
                                                         klass.data(ctx)->selfTypeArgs(ctx));
                if (inWhat.symbol.data(ctx)->flags.isGenericMethod) {
                    // ^ This mimics the check in LoadArg's call to argumentTypeAsSeenByImplementation
                    // It instantiates any `T.type_parameter(:U)`'s in the type (which are only
                    // valid in a method body if the method's signature is generic).
                    castType = core::Types::instantiate(ctx, castType, constr);
                }

                tp.type = castType;
                tp.origins.emplace_back(ctx.locAt(bind.loc));

                if (!hasType(ctx, bind.bind.variable)) {
                    noLoopChecking = true;
                }

                const core::TypeAndOrigins &ty = getAndFillTypeAndOrigin(ctx, c.value);
                ENFORCE(c.cast != core::Names::uncheckedLet() && c.cast != core::Names::bind() &&
                        c.cast != core::Names::syntheticBind());

                // TODO(jez) Should we allow `T.let` / `T.cast` opt out of the untyped code error?
                if (c.cast != core::Names::cast()) {
                    if (c.cast == core::Names::assertType() && ty.type.isUntyped()) {
                        if (auto e = ctx.beginError(bind.loc, core::errors::Infer::CastTypeMismatch)) {
                            e.setHeader("Expected a type but found `{}` for `{}`", "T.untyped", "T.assert_type!");
                            e.addErrorSection(ty.explainGot(ctx, ownerLoc));
                            e.addErrorNote("You may need to add additional `{}` annotations", "sig");
                        }
                    } else if (!core::Types::isSubType(ctx, ty.type, castType)) {
                        if (c.cast == core::Names::assumeType()) {
                            if (auto e = ctx.beginError(bind.loc, core::errors::Infer::IncorrectlyAssumedType)) {
                                e.setHeader("Assumed expression had type `{}` but found `{}`", castType.show(ctx),
                                            ty.type.show(ctx));
                                e.addErrorSection(ty.explainGot(ctx, ownerLoc));
                                e.addErrorNote("Please add an explicit type annotation to correct this assumption");
                                if (bind.loc.exists() && c.valueLoc.exists()) {
                                    e.replaceWith("Add explicit annotation", ctx.locAt(bind.loc), "T.let({}, {})",
                                                  ctx.locAt(c.valueLoc).source(ctx).value(), ty.type.show(ctx));
                                }
                            }
                        } else {
                            if (auto e = ctx.beginError(bind.loc, core::errors::Infer::CastTypeMismatch)) {
                                e.setHeader("Argument does not have asserted type `{}`", castType.show(ctx));
                                e.addErrorSection(ty.explainGot(ctx, ownerLoc));
                                core::TypeErrorDiagnostics::maybeAutocorrect(ctx, e, ctx.locAt(c.valueLoc), constr,
                                                                             castType, ty.type);
                            }
                        }
                    }
                } else if (!bind.value.isSynthetic()) {
                    if (castType.isUntyped()) {
                        if (auto e = ctx.beginError(bind.loc, core::errors::Infer::InvalidCast)) {
                            e.setHeader("Please use `{}` to cast to `{}`", "T.unsafe", "T.untyped");
                            auto argLoc = core::Loc{ctx.file, c.valueLoc};
                            if (argLoc.exists()) {
                                e.replaceWith("Replace with `T.unsafe`", ctx.locAt(bind.loc), "T.unsafe({})",
                                              argLoc.source(ctx).value());
                            }
                        }
                    } else if (!ty.type.isUntyped() && core::Types::isSubType(ctx, ty.type, castType)) {
                        if (auto e = ctx.beginError(bind.loc, core::errors::Infer::InvalidCast)) {
                            e.setHeader("`{}` is useless because `{}` is already a subtype of `{}`", "T.cast",
                                        ty.type.show(ctx), castType.show(ctx));
                            e.addErrorSection(ty.explainGot(ctx, ownerLoc));
                            auto argLoc = ctx.locAt(c.valueLoc);
                            if (argLoc.exists()) {
                                if (ctx.state.suggestUnsafe.has_value()) {
                                    e.replaceWith("Convert to `T.unsafe`", ctx.locAt(bind.loc), "{}({})",
                                                  ctx.state.suggestUnsafe.value(), argLoc.source(ctx).value());
                                } else {
                                    e.replaceWith("Delete `T.cast`", ctx.locAt(bind.loc), "{}",
                                                  argLoc.source(ctx).value());
                                }
                            }
                        }
                    }
                }
                if (c.cast == core::Names::let()) {
                    pinnedTypes[bind.bind.variable] = tp;
                }
            });

        ENFORCE(tp.type != nullptr, "Inferencer did not assign type: {}", bind.value.toString(ctx, inWhat));
        tp.type.sanityCheck(ctx);

        if (checkFullyDefined && !tp.type.isFullyDefined()) {
            if (auto e = ctx.beginError(bind.loc, core::errors::Infer::IncompleteType)) {
                e.setHeader("Expression does not have a fully-defined type (Did you reference another class's type "
                            "members?)");
                e.addErrorNote(
                    "Historically, users seeing this error has represented finding a bug in Sorbet\n"
                    "    (or at least finding a test case for which there could be a better error message).\n"
                    "    Consider searching for similar bugs at https://github.com/sorbet/sorbet/issues\n"
                    "    or reporting a new one.");
            }
            tp.type = core::Types::untypedUntracked();
        }
        ENFORCE(ctx.file.data(ctx).hasParseErrors() || !tp.origins.empty(), "Inferencer did not assign location");

        if (!noLoopChecking && loopCount != bindMinLoops) {
            auto pin = pinnedTypes.find(bind.bind.variable);
            const core::TypeAndOrigins &cur =
                (pin != pinnedTypes.end()) ? pin->second : getTypeAndOrigin(ctx, bind.bind.variable);

            // TODO(jez) What should we do about untyped code and pinning?
            bool asGoodAs = core::Types::isSubType(ctx, core::Types::dropLiteral(ctx, tp.type),
                                                   core::Types::dropLiteral(ctx, cur.type));

            {
                switch (bindMinLoops) {
                    case cfg::CFG::MIN_LOOP_FIELD:
                        if (!asGoodAs) {
                            if (auto e = ctx.beginError(bind.loc, core::errors::Infer::FieldReassignmentTypeMismatch)) {
                                e.setHeader("Expected `{}` but found `{}` for field", cur.type.show(ctx),
                                            tp.type.show(ctx));

                                // It would be nice to be able to actually show the name of the field in the error
                                // message, but we don't have a convenient way to compute this at the moment.
                                e.addErrorSection(cur.explainExpected(ctx, "field defined here", ownerLoc));
                                e.addErrorSection(tp.explainGot(ctx, ownerLoc));
                                core::TypeErrorDiagnostics::explainTypeMismatch(ctx, e, cur.type, tp.type);
                                auto replaceLoc = ctx.locAt(bind.loc);
                                // We are not processing a method call, so there is no constraint.
                                auto &constr = core::TypeConstraint::EmptyFrozenConstraint;
                                core::TypeErrorDiagnostics::maybeAutocorrect(ctx, e, replaceLoc, constr, cur.type,
                                                                             tp.type);
                            }
                            tp = cur;
                        }
                        break;
                    case cfg::CFG::MIN_LOOP_LET:
                        if (!asGoodAs) {
                            if (auto e = ctx.beginError(bind.loc, core::errors::Infer::PinnedVariableMismatch)) {
                                e.setHeader("Incompatible assignment to variable declared via `{}`: `{}` is not a "
                                            "subtype of `{}`",
                                            "let", tp.type.show(ctx), cur.type.show(ctx));
                            }
                            tp = cur;
                        }
                        break;
                    default: {
                        if (!asGoodAs || (tp.type.isUntyped() && !cur.type.isUntyped())) {
                            if (auto ident = cfg::cast_instruction<cfg::Ident>(bind.value)) {
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
                                e.addErrorSection(core::ErrorSection(
                                    core::ErrorColors::format("Existing variable has type: `{}`", cur.type.show(ctx))));
                                e.addErrorSection(core::ErrorSection(core::ErrorColors::format(
                                    "Attempting to change type to: `{}`\n", tp.type.show(ctx))));

                                if (cur.origins.size() == 1 && cur.origins[0].exists() &&
                                    // sometimes a variable has a given type because of a constant outside this method
                                    ctx.locAt(inWhat.loc).contains(cur.origins[0]) &&
                                    // don't attempt to insert a `T.let` into the method params list
                                    (inWhat.symbol.data(ctx)->name.isAnyStaticInitName(ctx) ||
                                     !inWhat.symbol.data(ctx)->loc().contains(cur.origins[0]))) {
                                    auto suggest =
                                        core::Types::any(ctx, dropConstructor(ctx, tp.origins[0], tp.type), cur.type);
                                    auto replacement = suggest.show(ctx, core::ShowOptions().withShowForRBI());
                                    e.replaceWith(fmt::format("Initialize as `{}`", replacement), cur.origins[0],
                                                  "T.let({}, {})", cur.origins[0].source(ctx).value(), replacement);
                                } else {
                                    e.addErrorSection(core::ErrorSection("Original type from:",
                                                                         cur.origins2Explanations(ctx, ownerLoc)));
                                }
                            }

                            tp.type = core::Types::untypedUntracked();
                        }
                        break;
                    }
                }
            }
        }
        setTypeAndOrigin(bind.bind.variable, tp);

        clearKnowledge(ctx, bind.bind.variable, knowledgeFilter);
        if (auto *send = cfg::cast_instruction<cfg::Send>(bind.value)) {
            updateKnowledge(ctx, bind.bind.variable, ctx.locAt(bind.loc), send, knowledgeFilter);
        } else if (auto *i = cfg::cast_instruction<cfg::Ident>(bind.value)) {
            propagateKnowledge(ctx, bind.bind.variable, i->what, knowledgeFilter);
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
    this->_vars = rhs._vars;
    this->bb = rhs.bb;
    this->pinnedTypes = rhs.pinnedTypes;
    this->typeTestsWithVar.cloneFrom(rhs.typeTestsWithVar);
}

core::TypeAndOrigins Environment::getTypeFromRebind(core::Context ctx, const core::DispatchComponent &main,
                                                    cfg::LocalRef fallback) {
    auto rebind = main.blockSpec.rebind;

    if (rebind.exists()) {
        core::TypeAndOrigins result;
        if (rebind == core::Symbols::MagicBindToSelfType()) {
            result.type = main.receiver;
        } else if (rebind == core::Symbols::MagicBindToAttachedClass()) {
            auto appliedType = core::cast_type<core::AppliedType>(main.receiver);
            auto attachedClass = appliedType->klass.data(ctx)->findMember(ctx, core::Names::Constants::AttachedClass());

            auto lambdaParam =
                core::cast_type<core::LambdaParam>(attachedClass.asTypeMemberRef().data(ctx)->resultType);

            result.type = lambdaParam->upperBound;
        } else {
            result.type = rebind.data(ctx)->selfType(ctx);
        }

        result.origins.emplace_back(main.blockSpec.loc);
        return result;
    } else {
        return getTypeAndOrigin(ctx, fallback);
    }
}

const TestedKnowledge &Environment::getKnowledge(cfg::LocalRef symbol, bool shouldFail) const {
    auto fnd = _vars.find(symbol);
    if (fnd == _vars.end()) {
        ENFORCE(!shouldFail, "Missing knowledge?");
        return TestedKnowledge::empty;
    }
    fnd->second.knowledge.sanityCheck();
    return fnd->second.knowledge;
}

void Environment::initializeBasicBlockArgs(const cfg::BasicBlock &bb) {
    _vars.reserve(bb.args.size());
    for (const cfg::VariableUseSite &arg : bb.args) {
        _vars[arg.variable].typeAndOrigins.type = nullptr;
    }
}

void Environment::setUninitializedVarsToNil(const core::Context &ctx, core::Loc origin) {
    for (auto &uninitialized : _vars) {
        if (uninitialized.second.typeAndOrigins.type == nullptr) {
            uninitialized.second.typeAndOrigins.type = core::Types::nilClass();
            uninitialized.second.typeAndOrigins.origins.emplace_back(origin);
        } else {
            uninitialized.second.typeAndOrigins.type.sanityCheck(ctx);
        }
    }
}

namespace {
core::TypeAndOrigins nilTypesWithOriginWithLoc(core::Loc loc) {
    // I'd love to have this, but keepForIDE intentionally has Loc::none() and
    // sometimes ends up here...
    // ENFORCE(loc.exists());
    core::TypeAndOrigins ret{core::Types::nilClass(), loc};
    return ret;
}
} // namespace

Environment::Environment(core::Loc ownerLoc) : uninitialized(nilTypesWithOriginWithLoc(ownerLoc)), ownerLoc(ownerLoc) {}

TestedKnowledge TestedKnowledge::empty;
} // namespace sorbet::infer
