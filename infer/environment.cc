#include "environment.h"
#include "common/typecase.h"
#include "core/GlobalState.h"
#include "core/TypeConstraint.h"
#include <algorithm> // find, remove_if

template struct std::pair<sorbet::core::LocalVariable, std::shared_ptr<sorbet::core::Type>>;

using namespace std;

namespace sorbet::infer {

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

KnowledgeFilter::KnowledgeFilter(core::Context ctx, unique_ptr<cfg::CFG> &cfg) {
    for (auto &bb : cfg->basicBlocks) {
        if (bb->bexit.cond.variable != core::LocalVariable::noVariable() &&
            bb->bexit.cond.variable != core::LocalVariable::blockCall()) {
            used_vars.insert(bb->bexit.cond.variable);
        }
    }
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto &bb : cfg->forwardsTopoSort) {
            for (auto &bind : bb->exprs) {
                if (auto *id = cfg::cast_instruction<cfg::Ident>(bind.value.get())) {
                    if (isNeeded(bind.bind.variable) && !isNeeded(id->what)) {
                        used_vars.insert(id->what);
                        changed = true;
                    }
                } else if (auto *send = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
                    if (send->fun == core::Names::bang()) {
                        if (send->args.empty()) {
                            if (isNeeded(bind.bind.variable) && !isNeeded(send->recv.variable)) {
                                used_vars.insert(send->recv.variable);
                                changed = true;
                            }
                        }
                    } else if (send->fun == core::Names::eqeq()) {
                        if (send->args.size() == 1) {
                            if (isNeeded(send->args[0].variable) && !isNeeded(send->recv.variable)) {
                                used_vars.insert(send->recv.variable);
                                changed = true;
                            } else if (isNeeded(send->recv.variable) && !isNeeded(send->args[0].variable)) {
                                used_vars.insert(send->args[0].variable);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

bool KnowledgeFilter::isNeeded(core::LocalVariable var) {
    return used_vars.find(var) != used_vars.end();
}

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
    bool enteringLoop = (bb->flags & cfg::CFG::LOOP_HEADER) != 0;
    for (auto &pair : env.vars) {
        auto local = pair.first;
        auto &state = pair.second;
        if (enteringLoop && bb->outerLoops <= inWhat.maxLoopWrite[local]) {
            continue;
        }
        auto fnd = absl::c_find_if(copy->yesTypeTests, [&](auto const &e) -> bool { return e.first == local; });
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
                copy.mutate().yesTypeTests.emplace_back(local, type);
            }
        } else {
            auto &second = fnd->second;
            auto &typeAndOrigin = state.typeAndOrigins;
            auto combinedType = core::Types::all(ctx, typeAndOrigin.type, second);
            if (combinedType->isBottom()) {
                copy.mutate().isDead = true;
                break;
            }
        }
    }
    return copy;
}

void KnowledgeFact::min(core::Context ctx, const KnowledgeFact &other) {
    for (auto it = yesTypeTests.begin(); it != yesTypeTests.end(); /* nothing */) {
        auto &entry = *it;
        core::LocalVariable local = entry.first;
        auto fnd = absl::c_find_if(other.yesTypeTests, [&](auto const &elem) -> bool { return elem.first == local; });
        if (fnd == other.yesTypeTests.end()) {
            it = yesTypeTests.erase(it);
        } else {
            entry.second = core::Types::any(ctx, fnd->second, entry.second);
            it++;
        }
    }
    for (auto it = noTypeTests.begin(); it != noTypeTests.end(); /* nothing */) {
        auto &entry = *it;
        core::LocalVariable local = entry.first;
        auto fnd = absl::c_find_if(other.noTypeTests, [&](auto const &elem) -> bool { return elem.first == local; });
        if (fnd == other.noTypeTests.end()) {
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

string KnowledgeFact::toString(core::Context ctx) const {
    vector<string> buf1, buf2;

    for (auto &el : yesTypeTests) {
        buf1.emplace_back(fmt::format("    {} to be {}\n", el.first.showRaw(ctx), el.second->toStringWithTabs(ctx, 0)));
    }
    for (auto &el : noTypeTests) {
        buf2.emplace_back(
            fmt::format("    {} NOT to be {}\n", el.first.showRaw(ctx), el.second->toStringWithTabs(ctx, 0)));
    }
    fast_sort(buf1);
    fast_sort(buf2);

    return fmt::format("{}{}", fmt::join(buf1, ""), fmt::join(buf2, ""));
}

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

string TestedKnowledge::toString(core::Context ctx) const {
    fmt::memory_buffer buf;
    if (!truthy->noTypeTests.empty() || !truthy->yesTypeTests.empty()) {
        fmt::format_to(buf, "  Being truthy entails:\n{}", truthy->toString(ctx));
    }
    if (!falsy->noTypeTests.empty() || !falsy->yesTypeTests.empty()) {
        fmt::format_to(buf, "  Being falsy entails:\n{}", falsy->toString(ctx));
    }
    return to_string(buf);
}

void TestedKnowledge::sanityCheck() const {
    if (!debug_mode) {
        return;
    }
    truthy->sanityCheck();
    falsy->sanityCheck();
    ENFORCE(TestedKnowledge::empty.falsy->yesTypeTests.empty());
    ENFORCE(TestedKnowledge::empty.falsy->noTypeTests.empty());
    ENFORCE(TestedKnowledge::empty.truthy->noTypeTests.empty());
    ENFORCE(TestedKnowledge::empty.truthy->yesTypeTests.empty());
}

string Environment::toString(core::Context ctx) const {
    fmt::memory_buffer buf;
    if (isDead) {
        fmt::format_to(buf, "dead={:d}\n", isDead);
    }
    vector<pair<core::LocalVariable, VariableState>> sorted;
    for (const auto &pair : vars) {
        sorted.emplace_back(pair);
    }
    fast_sort(sorted, [](const auto &lhs, const auto &rhs) -> bool {
        return lhs.first._name.id() < rhs.first._name.id() ||
               (lhs.first._name.id() == rhs.first._name.id() && lhs.first.unique < rhs.first.unique);
    });
    for (const auto &pair : sorted) {
        auto var = pair.first;
        auto &state = pair.second;
        if (var._name == core::Names::debugEnvironmentTemp()) {
            continue;
        }
        fmt::format_to(buf, "{}: {}{}\n{}\n", var.showRaw(ctx), state.typeAndOrigins.type->toStringWithTabs(ctx, 0),
                       state.knownTruthy ? " (and truthy)\n" : "", state.knowledge.toString(ctx));
    }
    return to_string(buf);
}

bool Environment::hasType(core::Context ctx, core::LocalVariable symbol) const {
    auto fnd = vars.find(symbol);
    if (fnd == vars.end()) {
        return false;
    }
    // We don't distinguish between nullptr and "not set"
    return fnd->second.typeAndOrigins.type.get() != nullptr;
}

const core::TypeAndOrigins &Environment::getTypeAndOrigin(core::Context ctx, core::LocalVariable symbol) const {
    auto fnd = vars.find(symbol);
    if (fnd == vars.end()) {
        return uninitialized;
    }
    ENFORCE(fnd->second.typeAndOrigins.type.get() != nullptr);
    return fnd->second.typeAndOrigins;
}

const core::TypeAndOrigins &Environment::getAndFillTypeAndOrigin(core::Context ctx,
                                                                 cfg::VariableUseSite &symbol) const {
    const auto &ret = getTypeAndOrigin(ctx, symbol.variable);
    symbol.type = ret.type;
    return ret;
}

bool Environment::getKnownTruthy(core::LocalVariable var) const {
    auto fnd = vars.find(var);
    if (fnd == vars.end()) {
        return false;
    }
    return fnd->second.knownTruthy;
}

void Environment::propagateKnowledge(core::Context ctx, core::LocalVariable to, core::LocalVariable from,
                                     KnowledgeFilter &knowledgeFilter) {
    if (knowledgeFilter.isNeeded(to) && knowledgeFilter.isNeeded(from)) {
        auto &fromState = vars[from];
        auto &toState = vars[to];
        toState.knownTruthy = fromState.knownTruthy;
        auto &toKnowledge = toState.knowledge;
        auto &fromKnowledge = fromState.knowledge;
        if (fromState.typeAndOrigins.type.get() == nullptr) {
            fromState.typeAndOrigins.type = core::Types::nilClass();
        }
        if (toState.typeAndOrigins.type.get() == nullptr) {
            toState.typeAndOrigins.type = core::Types::nilClass();
        }

        toKnowledge = fromKnowledge;
        toKnowledge.truthy.mutate().noTypeTests.emplace_back(from, core::Types::falsyTypes());
        toKnowledge.falsy.mutate().yesTypeTests.emplace_back(from, core::Types::falsyTypes());
        fromKnowledge.truthy.mutate().noTypeTests.emplace_back(to, core::Types::falsyTypes());
        fromKnowledge.falsy.mutate().yesTypeTests.emplace_back(to, core::Types::falsyTypes());
        fromKnowledge.sanityCheck();
        toKnowledge.sanityCheck();
    }
}

void Environment::clearKnowledge(core::Context ctx, core::LocalVariable reassigned, KnowledgeFilter &knowledgeFilter) {
    for (auto &el : vars) {
        auto &k = el.second.knowledge;
        if (knowledgeFilter.isNeeded(el.first)) {
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
    auto fnd = vars.find(reassigned);
    ENFORCE(fnd != vars.end());
    fnd->second.knownTruthy = false;
}

bool isSingleton(core::Context ctx, core::SymbolRef sym) {
    return sym == core::Symbols::NilClass() || sym == core::Symbols::FalseClass() ||
           sym == core::Symbols::TrueClass() ||
           (sym.data(ctx)->derivesFrom(ctx, core::Symbols::Singleton()) && sym.data(ctx)->isClassFinal());
}

void Environment::updateKnowledge(core::Context ctx, core::LocalVariable local, core::Loc loc, const cfg::Send *send,
                                  KnowledgeFilter &knowledgeFilter) {
    if (!knowledgeFilter.isNeeded(local)) {
        return;
    }

    if (send->fun == core::Names::bang()) {
        auto &whoKnows = getKnowledge(local);
        auto fnd = vars.find(send->recv.variable);
        if (fnd != vars.end()) {
            whoKnows.truthy = fnd->second.knowledge.falsy;
            whoKnows.falsy = fnd->second.knowledge.truthy;
            fnd->second.knowledge.truthy.mutate().yesTypeTests.emplace_back(local, core::Types::falsyTypes());
            fnd->second.knowledge.falsy.mutate().noTypeTests.emplace_back(local, core::Types::falsyTypes());
        }
        whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv.variable, core::Types::falsyTypes());
        whoKnows.falsy.mutate().noTypeTests.emplace_back(send->recv.variable, core::Types::falsyTypes());

        whoKnows.sanityCheck();
    } else if (send->fun == core::Names::nil_p()) {
        auto &whoKnows = getKnowledge(local);
        whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv.variable, core::Types::nilClass());
        whoKnows.falsy.mutate().noTypeTests.emplace_back(send->recv.variable, core::Types::nilClass());
        whoKnows.sanityCheck();
    } else if (send->fun == core::Names::blank_p()) {
        // Note that this assumes that .blank? is a rails-compatible monkey patch.
        // In other cases this flow analysis might make incorrect assumptions.
        auto &originalType = send->recv.type;
        auto knowledgeTypeWithoutFalsy = core::Types::approximateSubtract(ctx, originalType, core::Types::falsyTypes());

        if (!core::Types::equiv(ctx, knowledgeTypeWithoutFalsy, originalType)) {
            auto &whoKnows = getKnowledge(local);
            whoKnows.falsy.mutate().yesTypeTests.emplace_back(send->recv.variable, knowledgeTypeWithoutFalsy);
            whoKnows.sanityCheck();
        }
    } else if (send->fun == core::Names::present_p()) {
        // Note that this assumes that .present? is a rails-compatible monkey patch.
        // In other cases this flow analysis might make incorrect assumptions.
        auto &originalType = send->recv.type;
        auto knowledgeTypeWithoutFalsy = core::Types::approximateSubtract(ctx, originalType, core::Types::falsyTypes());

        if (!core::Types::equiv(ctx, knowledgeTypeWithoutFalsy, originalType)) {
            auto &whoKnows = getKnowledge(local);
            whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv.variable, knowledgeTypeWithoutFalsy);
            whoKnows.sanityCheck();
        }
    }

    if (send->args.empty()) {
        return;
    }
    if (send->fun == core::Names::kind_of() || send->fun == core::Names::is_a_p()) {
        auto &whoKnows = getKnowledge(local);
        auto &klassType = send->args[0].type;
        core::SymbolRef klass = core::Types::getRepresentedClass(ctx, klassType.get());
        if (klass.exists()) {
            auto ty = klass.data(ctx)->externalType(ctx);
            if (!ty->isUntyped()) {
                whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv.variable, ty);
                whoKnows.falsy.mutate().noTypeTests.emplace_back(send->recv.variable, ty);
            }
            whoKnows.sanityCheck();
        }
    } else if (send->fun == core::Names::eqeq() || send->fun == core::Names::neq()) {
        auto &whoKnows = getKnowledge(local);
        const auto &argType = send->args[0].type;
        const auto &recvType = send->recv.type;

        auto &truthy = send->fun == core::Names::eqeq() ? whoKnows.truthy : whoKnows.falsy;
        auto &falsy = send->fun == core::Names::eqeq() ? whoKnows.falsy : whoKnows.truthy;

        ENFORCE(argType.get() != nullptr);
        ENFORCE(recvType.get() != nullptr);
        if (!argType->isUntyped()) {
            truthy.mutate().yesTypeTests.emplace_back(send->recv.variable, argType);
        }
        if (!recvType->isUntyped()) {
            truthy.mutate().yesTypeTests.emplace_back(send->args[0].variable, recvType);
        }
        if (auto s = core::cast_type<core::ClassType>(argType.get())) {
            // check if s is a singleton. in this case we can learn that
            // a failed comparison means that type test would also fail
            if (isSingleton(ctx, s->symbol)) {
                falsy.mutate().noTypeTests.emplace_back(send->recv.variable, argType);
            }
        }
        if (auto s = core::cast_type<core::ClassType>(recvType.get())) {
            // check if s is a singleton. in this case we can learn that
            // a failed comparison means that type test would also fail
            if (isSingleton(ctx, s->symbol)) {
                falsy.mutate().noTypeTests.emplace_back(send->args[0].variable, recvType);
            }
        }
        whoKnows.sanityCheck();
    } else if (send->fun == core::Names::tripleEq()) {
        auto &whoKnows = getKnowledge(local);
        const auto &recvType = send->recv.type;

        // `when` against class literal
        core::SymbolRef representedClass = core::Types::getRepresentedClass(ctx, recvType.get());
        if (representedClass.exists()) {
            auto representedType = representedClass.data(ctx)->externalType(ctx);
            if (!representedType->isUntyped()) {
                whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->args[0].variable, representedType);
                whoKnows.falsy.mutate().noTypeTests.emplace_back(send->args[0].variable, representedType);
            }
        }

        // `when` against singleton
        if (auto s = core::cast_type<core::ClassType>(recvType.get())) {
            // check if s is a singleton. in this case we can learn that
            // a failed comparison means that type test would also fail
            if (isSingleton(ctx, s->symbol)) {
                whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->args[0].variable, recvType);
                whoKnows.falsy.mutate().noTypeTests.emplace_back(send->args[0].variable, recvType);
            }
        }
        whoKnows.sanityCheck();

    } else if (send->fun == core::Names::lessThan()) {
        const auto &recvKlass = send->recv.type;
        const auto &argType = send->args[0].type;
        auto *argClass = core::cast_type<core::ClassType>(argType.get());
        if (!argClass || !recvKlass->derivesFrom(ctx, core::Symbols::Class()) ||
            !argClass->symbol.data(ctx)->derivesFrom(ctx, core::Symbols::Class())) {
            return;
        }
        auto &whoKnows = getKnowledge(local);
        whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv.variable, argType);
        whoKnows.falsy.mutate().noTypeTests.emplace_back(send->recv.variable, argType);
        whoKnows.sanityCheck();
    }
}

void Environment::setTypeAndOrigin(core::LocalVariable symbol, const core::TypeAndOrigins &typeAndOrigins) {
    ENFORCE(typeAndOrigins.type.get() != nullptr);
    vars[symbol].typeAndOrigins = typeAndOrigins;
}

const Environment &Environment::withCond(core::Context ctx, const Environment &env, Environment &copy, bool isTrue,
                                         const UnorderedMap<core::LocalVariable, VariableState> &filter) {
    if (!env.bb->bexit.cond.variable.exists() || env.bb->bexit.cond.variable == core::LocalVariable::blockCall()) {
        return env;
    }
    copy.cloneFrom(env);
    copy.assumeKnowledge(ctx, isTrue, env.bb->bexit.cond.variable, env.bb->bexit.loc, filter);
    return copy;
}

void Environment::assumeKnowledge(core::Context ctx, bool isTrue, core::LocalVariable cond, core::Loc loc,
                                  const UnorderedMap<core::LocalVariable, VariableState> &filter) {
    auto &thisKnowledge = getKnowledge(cond, false);
    thisKnowledge.sanityCheck();
    if (!isTrue) {
        if (getKnownTruthy(cond)) {
            isDead = true;
            return;
        }

        core::TypeAndOrigins tp = getTypeAndOrigin(ctx, cond);
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
        setTypeAndOrigin(cond, tp);
    } else {
        core::TypeAndOrigins tp = getTypeAndOrigin(ctx, cond);
        tp.origins.emplace_back(loc);
        tp.type = core::Types::dropSubtypesOf(ctx, core::Types::dropSubtypesOf(ctx, tp.type, core::Symbols::NilClass()),
                                              core::Symbols::FalseClass());
        if (tp.type->isBottom()) {
            isDead = true;
            return;
        }
        setTypeAndOrigin(cond, tp);
        vars[cond].knownTruthy = true;
    }

    if (isDead) {
        return;
    }

    auto &knowledgeToChoose = isTrue ? thisKnowledge.truthy : thisKnowledge.falsy;
    auto &yesTests = knowledgeToChoose->yesTypeTests;
    auto &noTests = knowledgeToChoose->noTypeTests;

    for (auto &typeTested : yesTests) {
        if (filter.find(typeTested.first) == filter.end()) {
            continue;
        }
        core::TypeAndOrigins tp = getTypeAndOrigin(ctx, typeTested.first);
        auto glbbed = core::Types::all(ctx, tp.type, typeTested.second);
        if (tp.type != glbbed) {
            tp.origins.emplace_back(loc);
            tp.type = glbbed;
        }
        setTypeAndOrigin(typeTested.first, tp);
        if (tp.type->isBottom()) {
            isDead = true;
            return;
        }
    }

    for (auto &typeTested : noTests) {
        if (filter.find(typeTested.first) == filter.end()) {
            continue;
        }
        core::TypeAndOrigins tp = getTypeAndOrigin(ctx, typeTested.first);
        tp.origins.emplace_back(loc);

        if (!tp.type->isUntyped()) {
            tp.type = core::Types::approximateSubtract(ctx, tp.type, typeTested.second);
            setTypeAndOrigin(typeTested.first, tp);
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
    for (auto &pair : vars) {
        auto var = pair.first;
        const auto &otherTO = other.getTypeAndOrigin(ctx, var);
        auto &thisTO = pair.second.typeAndOrigins;
        if (thisTO.type.get() != nullptr) {
            thisTO.type = core::Types::any(ctx, thisTO.type, otherTO.type);
            thisTO.type->sanityCheck(ctx);
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

        if (((bb->flags & cfg::CFG::LOOP_HEADER) != 0) && bb->outerLoops <= inWhat.maxLoopWrite[var]) {
            continue;
        }
        bool canBeFalsy = core::Types::canBeFalsy(ctx, otherTO.type) && !other.getKnownTruthy(var);
        bool canBeTruthy = core::Types::canBeTruthy(ctx, otherTO.type);

        if (canBeTruthy) {
            auto &thisKnowledge = getKnowledge(var);
            auto otherTruthy = KnowledgeFact::under(ctx, other.getKnowledge(var, false).truthy, other, loc, inWhat, bb,
                                                    knowledgeFilter.isNeeded(var));
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
            auto otherFalsy = KnowledgeFact::under(ctx, other.getKnowledge(var, false).falsy, other, loc, inWhat, bb,
                                                   knowledgeFilter.isNeeded(var));
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
}

void Environment::computePins(core::Context ctx, const vector<Environment> &envs, const cfg::CFG &inWhat,
                              const cfg::BasicBlock *bb) {
    if (bb->backEdges.empty()) {
        return;
    }

    for (auto &pair : vars) {
        auto var = pair.first;
        core::TypeAndOrigins tp;

        auto bindMinLoops = inWhat.minLoops.at(var);
        if (bb->outerLoops == bindMinLoops || bindMinLoops == inWhat.maxLoopWrite.at(var)) {
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
                    tp.type->sanityCheck(ctx);
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
    for (auto &pair : vars) {
        auto var = pair.first;
        pair.second.typeAndOrigins = other.getTypeAndOrigin(ctx, var);
        pair.second.knowledge = other.getKnowledge(var, false);
        pair.second.knownTruthy = other.getKnownTruthy(var);
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

    return core::Types::arrayOf(ctx, flattenArrays(ctx, returnType));
}

core::TypePtr Environment::processBinding(core::Context ctx, cfg::Binding &bind, int loopCount, int bindMinLoops,
                                          KnowledgeFilter &knowledgeFilter, core::TypeConstraint &constr,
                                          core::TypePtr &methodReturnType) {
    try {
        core::TypeAndOrigins tp;
        bool noLoopChecking = cfg::isa_instruction<cfg::Alias>(bind.value.get()) ||
                              cfg::isa_instruction<cfg::LoadArg>(bind.value.get()) ||
                              cfg::isa_instruction<cfg::LoadSelf>(bind.value.get());

        bool checkFullyDefined = true;
        const core::lsp::Query &lspQuery = ctx.state.lspQuery;
        bool lspQueryMatch = lspQuery.matchesLoc(bind.loc);

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
                        ctx, core::lsp::SendResponse(bind.loc, retainedResult, send->fun));
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
                tp.origins.emplace_back(bind.loc);
            },
            [&](cfg::Ident *i) {
                const core::TypeAndOrigins &typeAndOrigin = getTypeAndOrigin(ctx, i->what);
                tp.type = typeAndOrigin.type;
                tp.origins = typeAndOrigin.origins;

                if (lspQueryMatch) {
                    core::lsp::QueryResponse::pushQueryResponse(
                        ctx, core::lsp::IdentResponse(ctx.owner, bind.loc, i->what, tp));
                }

                ENFORCE((bind.loc.exists() && bind.loc.file().data(ctx).hasParseErrors) || !tp.origins.empty(),
                        "Inferencer did not assign location");
            },
            [&](cfg::Alias *a) {
                core::SymbolRef symbol = a->what.data(ctx)->dealias(ctx);
                const auto &data = symbol.data(ctx);
                if (data->isClass()) {
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
                                tp.type = lambdaParam->upperBound;
                            } else {
                                tp.type = core::make_type<core::SelfTypeParam>(symbol);
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
                    if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::GenericMethodConstaintUnsolved)) {
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
                tp.origins.emplace_back(bind.loc);
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
                tp.origins.emplace_back(bind.loc);
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
                tp.origins.emplace_back(bind.loc);
            },
            [&](cfg::Return *i) {
                tp.type = core::Types::bottom();
                tp.origins.emplace_back(bind.loc);

                const core::TypeAndOrigins &typeAndOrigin = getAndFillTypeAndOrigin(ctx, i->what);
                if (core::Types::isSubType(ctx, core::Types::void_(), methodReturnType)) {
                    methodReturnType = core::Types::untypedUntracked();
                }
                if (!core::Types::isSubTypeUnderConstraint(ctx, constr, true, typeAndOrigin.type, methodReturnType)) {
                    if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::ReturnTypeMismatch)) {
                        e.setHeader("Returning value that does not conform to method result type");
                        e.addErrorSection(core::ErrorSection(
                            "Expected " + methodReturnType->show(ctx),
                            {
                                core::ErrorLine::from(ctx.owner.data(ctx)->loc(), "Method `{}` has return type `{}`",
                                                      ctx.owner.data(ctx)->name.show(ctx), methodReturnType->show(ctx)),
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
                    isSubtype = core::Types::isSubTypeUnderConstraint(ctx, *i->link->result->main.constr, true,
                                                                      typeAndOrigin.type, expectedType);
                } else {
                    isSubtype = core::Types::isSubType(ctx, typeAndOrigin.type, expectedType);
                }
                if (!isSubtype) {
                    // TODO(nelhage): We should somehow report location
                    // information about the `send` and/or the
                    // definition of the block type

                    if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::ReturnTypeMismatch)) {
                        e.setHeader("Returning value that does not conform to block result type");
                        e.addErrorSection(core::ErrorSection("Expected " + expectedType->show(ctx)));
                        e.addErrorSection(
                            core::ErrorSection("Got " + typeAndOrigin.type->show(ctx) + " originating from:",
                                               typeAndOrigin.origins2Explanations(ctx)));
                    }
                }

                tp.type = core::Types::bottom();
                tp.origins.emplace_back(bind.loc);
            },
            [&](cfg::Literal *i) {
                tp.type = i->value;
                tp.origins.emplace_back(bind.loc);

                if (lspQueryMatch) {
                    core::lsp::QueryResponse::pushQueryResponse(ctx,
                                                                core::lsp::LiteralResponse(ctx.owner, bind.loc, tp));
                }
            },
            [&](cfg::TAbsurd *i) {
                const core::TypeAndOrigins &typeAndOrigin = getTypeAndOrigin(ctx, i->what.variable);

                if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::NotExhaustive)) {
                    if (typeAndOrigin.type->isUntyped()) {
                        e.setHeader("Control flow could reach `{}` because argument was `{}`", "T.absurd", "T.untyped");
                    } else {
                        e.setHeader("Control flow could reach `{}` because the type `{}` wasn't handled", "T.absurd",
                                    typeAndOrigin.type->show(ctx));
                    }
                    e.addErrorSection(core::ErrorSection("Originating from:", typeAndOrigin.origins2Explanations(ctx)));
                }

                tp.type = core::Types::bottom();
                tp.origins.emplace_back(bind.loc);
            },
            [&](cfg::Unanalyzable *i) {
                tp.type = core::Types::untypedUntracked();
                tp.origins.emplace_back(bind.loc);
            },
            [&](cfg::LoadSelf *l) {
                ENFORCE(l->link);
                if (l->link->result->main.blockSpec.rebind.exists()) {
                    tp.type = l->link->result->main.blockSpec.rebind.data(ctx)->externalType(ctx);
                    tp.origins.emplace_back(bind.loc);

                } else {
                    tp = getTypeAndOrigin(ctx, l->fallback);
                }
            },
            [&](cfg::Cast *c) {
                auto klass = ctx.owner.data(ctx)->enclosingClass(ctx);
                auto castType = core::Types::instantiate(ctx, c->type, klass.data(ctx)->typeMembers(),
                                                         klass.data(ctx)->selfTypeArgs(ctx));

                tp.type = castType;
                tp.origins.emplace_back(bind.loc);

                if (!hasType(ctx, bind.bind.variable)) {
                    noLoopChecking = true;
                }

                const core::TypeAndOrigins &ty = getAndFillTypeAndOrigin(ctx, c->value);
                if (c->cast != core::Names::cast()) {
                    if (c->cast == core::Names::assertType() && ty.type->isUntyped()) {
                        if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::CastTypeMismatch)) {
                            e.setHeader("The typechecker was unable to infer the type of the asserted value");
                            e.addErrorSection(core::ErrorSection("Got " + ty.type->show(ctx) + " originating from:",
                                                                 ty.origins2Explanations(ctx)));
                            e.addErrorSection(core::ErrorSection("You may need to add additional `sig` annotations"));
                        }
                    } else if (!core::Types::isSubType(ctx, ty.type, castType)) {
                        if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::CastTypeMismatch)) {
                            e.setHeader("Argument does not have asserted type `{}`", castType->show(ctx));
                            e.addErrorSection(core::ErrorSection("Got " + ty.type->show(ctx) + " originating from:",
                                                                 ty.origins2Explanations(ctx)));
                        }
                    }
                } else {
                    if (castType->isUntyped()) {
                        if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::InvalidCast)) {
                            e.setHeader("Please use `T.unsafe(...)` to cast to T.untyped");
                        }
                    } else if (!c->isSynthetic && !ty.type->isUntyped() &&
                               core::Types::isSubType(ctx, ty.type, castType)) {
                        if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::InvalidCast)) {
                            e.setHeader("Useless cast: inferred type `{}` is already a subtype of `{}`",
                                        ty.type->show(ctx), castType->show(ctx));
                        }
                    }
                }
                if (c->cast == core::Names::let()) {
                    pinnedTypes[bind.bind.variable] = tp;
                }
            });

        ENFORCE(tp.type.get() != nullptr, "Inferencer did not assign type: {}", bind.value->toString(ctx));
        tp.type->sanityCheck(ctx);

        if (checkFullyDefined && !tp.type->isFullyDefined()) {
            if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::IncompleteType)) {
                e.setHeader("Expression does not have a fully-defined type (Did you reference another class's type "
                            "members?)");
            }
            tp.type = core::Types::untypedUntracked();
        }
        ENFORCE((bind.loc.exists() && bind.loc.file().data(ctx).hasParseErrors) || !tp.origins.empty(),
                "Inferencer did not assign location");

        if (!noLoopChecking && loopCount != bindMinLoops) {
            auto pin = pinnedTypes.find(bind.bind.variable);
            const core::TypeAndOrigins &cur =
                (pin != pinnedTypes.end()) ? pin->second : getTypeAndOrigin(ctx, bind.bind.variable);

            bool asGoodAs =
                core::Types::isSubType(ctx, core::Types::dropLiteral(tp.type), core::Types::dropLiteral(cur.type));

            {
                switch (bindMinLoops) {
                    case cfg::CFG::MIN_LOOP_FIELD:
                        if (!asGoodAs) {
                            if (auto e = ctx.state.beginError(bind.loc,
                                                              core::errors::Infer::FieldReassignmentTypeMismatch)) {
                                e.setHeader(
                                    "Reassigning field with a value of wrong type: `{}` is not a subtype of `{}`",
                                    tp.type->show(ctx), cur.type->show(ctx));
                            }
                            tp = cur;
                        }
                        break;
                    case cfg::CFG::MIN_LOOP_GLOBAL:
                        if (!asGoodAs) {
                            if (auto e = ctx.state.beginError(bind.loc,
                                                              core::errors::Infer::GlobalReassignmentTypeMismatch)) {
                                e.setHeader(
                                    "Reassigning global with a value of wrong type: `{}` is not a subtype of `{}`",
                                    tp.type->show(ctx), cur.type->show(ctx));
                            }
                            tp = cur;
                        }
                        break;
                    case cfg::CFG::MIN_LOOP_LET:
                        if (!asGoodAs) {
                            if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::PinnedVariableMismatch)) {
                                e.setHeader("Incompatible assignment to variable declared via `let`: `{}` is not a "
                                            "subtype of `{}`",
                                            tp.type->show(ctx), cur.type->show(ctx));
                            }
                            tp = cur;
                        }
                        break;
                    default: {
                        if (!asGoodAs || (tp.type->isUntyped() && !cur.type->isUntyped())) {
                            if (auto ident = cfg::cast_instruction<cfg::Ident>(bind.value.get())) {
                                // See cfg/builder/builder_walk.cc for an explanation of why this is here.
                                if (ident->what._name == core::Names::blockBreakAssign()) {
                                    break;
                                }

                                if (ident->what._name == core::Names::selfRestore()) {
                                    // this is a restoration of `self` variable.
                                    // our current analysis isn't smart enogh to see that it's safe to do this by
                                    // construction either https://github.com/sorbet/sorbet/issues/222 or
                                    // https://github.com/sorbet/sorbet/issues/224 should allow us to remove this
                                    // case
                                    break;
                                }
                            }
                            if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::PinnedVariableMismatch)) {
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
        setTypeAndOrigin(bind.bind.variable, tp);

        clearKnowledge(ctx, bind.bind.variable, knowledgeFilter);
        if (auto *send = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
            updateKnowledge(ctx, bind.bind.variable, bind.loc, send, knowledgeFilter);
        } else if (auto *i = cfg::cast_instruction<cfg::Ident>(bind.value.get())) {
            propagateKnowledge(ctx, bind.bind.variable, i->what, knowledgeFilter);
        }

        return move(tp.type);
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = ctx.state.beginError(bind.loc, core::errors::Internal::InternalError)) {
            e.setHeader("Failed to type (backtrace is above)");
        }
        throw;
    }
}

void Environment::cloneFrom(const Environment &rhs) {
    this->isDead = rhs.isDead;
    this->vars = rhs.vars;
    this->bb = rhs.bb;
    this->pinnedTypes = rhs.pinnedTypes;
}

const TestedKnowledge &Environment::getKnowledge(core::LocalVariable symbol, bool shouldFail) const {
    auto fnd = vars.find(symbol);
    if (fnd == vars.end()) {
        ENFORCE(!shouldFail, "Missing knowledge?");
        return TestedKnowledge::empty;
    }
    fnd->second.knowledge.sanityCheck();
    return fnd->second.knowledge;
}

core::TypeAndOrigins nilTypesWithOriginWithLoc(core::Loc loc) {
    // I'd love to have this, but keepForIDE intentionally has Loc::none() and
    // sometimes ends up here...
    // ENFORCE(loc.exists());
    core::TypeAndOrigins ret;
    ret.type = core::Types::nilClass();
    ret.origins.emplace_back(loc);
    return ret;
}

Environment::Environment(core::Loc ownerLoc) : uninitialized(nilTypesWithOriginWithLoc(ownerLoc)) {}

TestedKnowledge TestedKnowledge::empty;
} // namespace sorbet::infer
