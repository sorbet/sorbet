#include "environment.h"
#include "core/TypeConstraint.h"
#include <algorithm> // find, remove_if

template struct std::pair<sorbet::core::LocalVariable, std::shared_ptr<sorbet::core::Type>>;

using namespace std;

namespace sorbet {
namespace infer {

shared_ptr<core::Type> dropConstructor(core::Context ctx, core::Loc loc, shared_ptr<core::Type> tp) {
    if (auto *mt = core::cast_type<core::MetaType>(tp.get())) {
        if (!mt->wrapped->isUntyped()) {
            if (auto e = ctx.state.beginError(loc, core::errors::Infer::BareTypeUsage)) {
                e.setHeader("Unsupported usage of bare type");
            }
        }
        return core::Types::untyped();
    }
    return tp;
}

KnowledgeFilter::KnowledgeFilter(core::Context ctx, unique_ptr<cfg::CFG> &cfg) {
    for (auto &bb : cfg->basicBlocks) {
        if (bb->bexit.cond != core::LocalVariable::noVariable() && bb->bexit.cond != core::LocalVariable::blockCall()) {
            used_vars.insert(bb->bexit.cond);
        }
        for (auto &bind : bb->exprs) {
            if (auto *send = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
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
        for (auto &bb : cfg->forwardsTopoSort) {
            for (auto &bind : bb->exprs) {
                if (auto *id = cfg::cast_instruction<cfg::Ident>(bind.value.get())) {
                    if (isNeeded(bind.bind) && !isNeeded(id->what)) {
                        used_vars.insert(id->what);
                        changed = true;
                    }
                } else if (auto *send = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
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
            //
            if (isNeeded && !env.types[i].type->isUntyped()) {
                copy.mutate().yesTypeTests.emplace_back(local, env.types[i].type);
            }
        } else {
            auto &second = fnd->second;
            auto &typeAndOrigin = env.types[i];
            auto combinedType =
                core::Types::all(ctx, dropConstructor(ctx, typeAndOrigin.origins[0], typeAndOrigin.type), second);
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
        auto fnd = find_if(other.yesTypeTests.begin(), other.yesTypeTests.end(),
                           [&](auto const &elem) -> bool { return elem.first == local; });
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
        auto fnd = find_if(other.noTypeTests.begin(), other.noTypeTests.end(),
                           [&](auto const &elem) -> bool { return elem.first == local; });
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
    stringstream buf;

    for (auto &el : yesTypeTests) {
        buf << "    " << el.first.toString(ctx) << " to be " << el.second->toString(ctx, 0) << '\n';
    }
    for (auto &el : noTypeTests) {
        buf << "    " << el.first.toString(ctx) << " NOT to be " << el.second->toString(ctx, 0) << '\n';
    }
    return buf.str();
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
    stringstream buf;
    if (!truthy->noTypeTests.empty() || !truthy->yesTypeTests.empty()) {
        buf << "  Being truthy entails:" << '\n';
    }
    buf << truthy->toString(ctx);
    if (!falsy->noTypeTests.empty() || !falsy->yesTypeTests.empty()) {
        buf << "  Being falsy entails:" << '\n';
    }
    buf << falsy->toString(ctx);
    return buf.str();
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
    stringstream buf;
    if (isDead) {
        buf << "dead=" << isDead << '\n';
    }
    int i = -1;
    for (auto var : vars) {
        i++;
        if (var._name == core::Names::debugEnvironmentTemp()) {
            continue;
        }
        buf << var.toString(ctx) << ": " << types[i].type->toString(ctx, 0);
        if (knownTruthy[i]) {
            buf << " (and truthy)" << '\n';
        }
        buf << '\n';
        buf << knowledge[i].toString(ctx) << '\n';
    }
    return buf.str();
}

bool Environment::hasType(core::Context ctx, core::LocalVariable symbol) const {
    auto fnd = find(vars.begin(), vars.end(), symbol);
    if (fnd == vars.end()) {
        return false;
    }
    // We don't distinguish between nullptr and "not set"
    return types[fnd - vars.begin()].type.get() != nullptr;
}

core::TypeAndOrigins Environment::getTypeAndOrigin(core::Context ctx, core::LocalVariable symbol) const {
    auto fnd = find(vars.begin(), vars.end(), symbol);
    if (fnd == vars.end()) {
        core::TypeAndOrigins ret;
        ret.type = core::Types::nilClass();
        ret.origins.push_back(ctx.owner.data(ctx).loc);
        return ret;
    }
    ENFORCE(types[fnd - vars.begin()].type.get() != nullptr);
    return types[fnd - vars.begin()];
}

bool Environment::getKnownTruthy(core::LocalVariable var) const {
    auto fnd = find(vars.begin(), vars.end(), var);
    if (fnd == vars.end()) {
        return false;
    }
    return knownTruthy[fnd - vars.begin()];
}

core::TypeAndOrigins Environment::getOrCreateTypeAndOrigin(core::Context ctx, core::LocalVariable symbol) {
    auto fnd = find(vars.begin(), vars.end(), symbol);
    if (fnd == vars.end()) {
        core::TypeAndOrigins ret;
        ret.type = core::Types::nilClass();
        ret.origins.push_back(ctx.owner.data(ctx).loc);
        vars.emplace_back(symbol);
        types.push_back(ret);
        knowledge.emplace_back();
        knownTruthy.emplace_back(false);
        return ret;
    }
    ENFORCE(types[fnd - vars.begin()].type.get() != nullptr);
    return types[fnd - vars.begin()];
}

void Environment::propagateKnowledge(core::Context ctx, core::LocalVariable to, core::LocalVariable from,
                                     KnowledgeFilter &knowledgeFilter) {
    if (knowledgeFilter.isNeeded(to) && knowledgeFilter.isNeeded(from)) {
        auto &toKnowledge = getKnowledge(to);
        auto &fromKnowledge = getKnowledge(from);

        toKnowledge = fromKnowledge;
        toKnowledge.truthy.mutate().noTypeTests.emplace_back(from, core::Types::falsyTypes());
        toKnowledge.falsy.mutate().yesTypeTests.emplace_back(from, core::Types::falsyTypes());
        fromKnowledge.truthy.mutate().noTypeTests.emplace_back(to, core::Types::falsyTypes());
        fromKnowledge.falsy.mutate().yesTypeTests.emplace_back(to, core::Types::falsyTypes());
        fromKnowledge.sanityCheck();
        toKnowledge.sanityCheck();

        auto fromidx = &fromKnowledge - &knowledge.front();
        auto toidx = &toKnowledge - &knowledge.front();
        knownTruthy[toidx] = knownTruthy[fromidx];
    }
}

void Environment::clearKnowledge(core::Context ctx, core::LocalVariable reassigned, KnowledgeFilter &knowledgeFilter) {
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
    auto fnd = find(vars.begin(), vars.end(), reassigned);
    ENFORCE(fnd != vars.end());
    knownTruthy[fnd - vars.begin()] = false;
}

void Environment::updateKnowledge(core::Context ctx, core::LocalVariable local, core::Loc loc, cfg::Send *send,
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
        whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv, core::Types::nilClass());
        whoKnows.falsy.mutate().noTypeTests.emplace_back(send->recv, core::Types::nilClass());
        whoKnows.sanityCheck();
    } else if (send->fun == core::Names::present_p()) {
        if (!knowledgeFilter.isNeeded(local)) {
            return;
        }
        // Note that this assumes that .present? is a rails compatible monkey patch on both NilClass
        // and Object. In all other cases this flow analysis might produce incorrect assumptions.
        core::TypeAndOrigins receiverType = getTypeAndOrigin(ctx, send->recv);
        auto originalType = receiverType.type;
        auto knowledgeTypeWithoutNil =
            core::Types::approximateSubtract(ctx, receiverType.type, core::Types::nilClass());
        auto knowledgeTypeWithoutFalse =
            core::Types::approximateSubtract(ctx, knowledgeTypeWithoutNil, core::Types::falseClass());

        if (!core::Types::equiv(ctx, knowledgeTypeWithoutFalse, originalType)) {
            auto &whoKnows = getKnowledge(local);
            whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv, knowledgeTypeWithoutFalse);
            whoKnows.sanityCheck();
        }
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
                if (!ty->isUntyped()) {
                    whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv, ty);
                    whoKnows.falsy.mutate().noTypeTests.emplace_back(send->recv, ty);
                }
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

        ENFORCE(tp1.type.get() != nullptr);
        ENFORCE(tp2.type.get() != nullptr);
        if (!tp1.type->isUntyped()) {
            whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv, tp1.type);
        }
        if (!tp2.type->isUntyped()) {
            whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->args[0], tp2.type);
        }
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
            if (!ty->isUntyped()) {
                whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->args[0], ty);
                whoKnows.falsy.mutate().noTypeTests.emplace_back(send->args[0], ty);
            }
        }
        whoKnows.sanityCheck();

    } else if (send->fun == core::Names::hardAssert()) {
        core::TypeAndOrigins recvKlass = getTypeAndOrigin(ctx, send->recv);
        if (!recvKlass.type->derivesFrom(ctx, core::Symbols::Kernel())) {
            return;
        }

        assumeKnowledge(ctx, true, send->args[0], loc, vars);
        if (this->isDead) {
            if (auto e = ctx.state.beginError(loc, core::errors::Infer::DeadBranchInferencer)) {
                e.setHeader("Hard assert is always false");
            }
        }
    } else if (send->fun == core::Names::lessThan()) {
        core::TypeAndOrigins recvKlass = getTypeAndOrigin(ctx, send->recv);
        core::TypeAndOrigins argType = getTypeAndOrigin(ctx, send->args[0]);
        auto *argClass = core::cast_type<core::ClassType>(argType.type.get());
        if (!argClass || !recvKlass.type->derivesFrom(ctx, core::Symbols::Class()) ||
            !argClass->symbol.data(ctx).derivesFrom(ctx, core::Symbols::Class())) {
            return;
        }
        auto &whoKnows = getKnowledge(local);
        whoKnows.truthy.mutate().yesTypeTests.emplace_back(send->recv, argType.type);
        whoKnows.falsy.mutate().noTypeTests.emplace_back(send->recv, argType.type);
        whoKnows.sanityCheck();
    }
}

void Environment::setTypeAndOrigin(core::LocalVariable symbol, core::TypeAndOrigins typeAndOrigins) {
    ENFORCE(typeAndOrigins.type.get() != nullptr);

    auto fnd = find(vars.begin(), vars.end(), symbol);
    if (fnd == vars.end()) {
        vars.emplace_back(symbol);
        types.push_back(typeAndOrigins);
        knowledge.emplace_back();
        knownTruthy.emplace_back();
        return;
    }
    types[fnd - vars.begin()] = typeAndOrigins;
}

const Environment &Environment::withCond(core::Context ctx, const Environment &env, Environment &copy, bool isTrue,
                                         const vector<core::LocalVariable> &filter) {
    if (!env.bb->bexit.cond.exists() || env.bb->bexit.cond == core::LocalVariable::blockCall()) {
        return env;
    }
    copy.cloneFrom(env);
    copy.assumeKnowledge(ctx, isTrue, env.bb->bexit.cond, env.bb->bexit.loc, filter);
    return copy;
}

void Environment::assumeKnowledge(core::Context ctx, bool isTrue, core::LocalVariable cond, core::Loc loc,
                                  const vector<core::LocalVariable> &filter) {
    auto fnd = find(vars.begin(), vars.end(), cond);
    if (fnd == vars.end()) {
        // Can't use getKnowledge because of loops
        return;
    }
    auto &thisKnowledge = this->knowledge[fnd - vars.begin()];
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
        auto fnd = find(vars.begin(), vars.end(), cond);
        knownTruthy[fnd - vars.begin()] = true;
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
        if (tp.type->isUntyped()) { // this is actually incorrect, as it may be some more exact type, but this rule
            // makes it easier to migrate code
            tp.type = typeTested.second;
            tp.origins.emplace_back(loc);
        } else {
            auto lubbed = core::Types::all(ctx, tp.type, typeTested.second);
            if (tp.type != lubbed) {
                tp.origins.emplace_back(loc);
                tp.type = lubbed;
            }
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

core::TypeAndOrigins Environment::getTypeAndOriginFromOtherEnv(core::Context ctx, core::LocalVariable var,
                                                               const Environment &other) {
    auto otherTO = other.getTypeAndOrigin(ctx, var);
    otherTO.type = dropConstructor(ctx, otherTO.origins.front(), otherTO.type);
    return otherTO;
}

void Environment::mergeWith(core::Context ctx, const Environment &other, core::Loc loc, cfg::CFG &inWhat,
                            cfg::BasicBlock *bb, KnowledgeFilter &knowledgeFilter) {
    int i = -1;
    this->isDead |= other.isDead;
    for (core::LocalVariable var : vars) {
        i++;
        auto otherTO = getTypeAndOriginFromOtherEnv(ctx, var, other);
        auto &thisTO = types[i];
        if (thisTO.type.get() != nullptr) {
            thisTO.type = core::Types::any(ctx, thisTO.type, otherTO.type);
            thisTO.type->sanityCheck(ctx);
            for (auto origin : otherTO.origins) {
                if (find(thisTO.origins.begin(), thisTO.origins.end(), origin) == thisTO.origins.end()) {
                    thisTO.origins.push_back(origin);
                }
            }
            knownTruthy[i] = knownTruthy[i] && other.getKnownTruthy(var);
        } else {
            types[i] = otherTO;
            knownTruthy[i] = other.getKnownTruthy(var);
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

    for (core::LocalVariable var : vars) {
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
                        if (find(tp.origins.begin(), tp.origins.end(), origin) == tp.origins.end()) {
                            tp.origins.push_back(origin);
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
        knownTruthy[i] = other.getKnownTruthy(var);
    }

    this->pinnedTypes = other.pinnedTypes;
}

shared_ptr<core::Type> Environment::getReturnType(core::Context ctx, shared_ptr<core::Type> procType) {
    if (!procType->derivesFrom(ctx, core::Symbols::Proc())) {
        return core::Types::untyped();
    }
    auto *applied = core::cast_type<core::AppliedType>(procType.get());
    if (applied == nullptr || applied->targs.empty()) {
        return core::Types::untyped();
    }
    // Proc types have their return type as the first targ
    return applied->targs.front();
}

void Environment::setQueryResponse(core::Context ctx, core::QueryResponse::Kind kind,
                                   core::DispatchResult::ComponentVec dispatchComponents,
                                   std::shared_ptr<core::TypeConstraint> constraint, core::Loc termLoc,
                                   core::TypeAndOrigins retType) {
    auto queryResponse = make_unique<core::QueryResponse>();
    queryResponse->kind = kind;
    queryResponse->dispatchComponents = std::move(dispatchComponents);
    queryResponse->constraint = constraint;
    queryResponse->termLoc = termLoc;
    queryResponse->retType = retType;

    ctx.state.errorQueue->pushQueryResponse(std::move(queryResponse));
}

shared_ptr<core::Type> flattenArrays(core::Context ctx, shared_ptr<core::Type> type) {
    shared_ptr<core::Type> result;

    typecase(type.get(),

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

             [&](core::Type *t) { result = move(type); });
    return result;
}

shared_ptr<core::Type> flatmapHack(core::Context ctx, const std::shared_ptr<core::SendAndBlockLink> &link,
                                   shared_ptr<core::Type> returnType) {
    if (link->fun != core::Names::flatMap()) {
        return returnType;
    }
    if (!link->receiver->derivesFrom(ctx, core::Symbols::Enumerable())) {
        return returnType;
    }

    return core::Types::arrayOf(ctx, flattenArrays(ctx, returnType));
}

shared_ptr<core::Type> Environment::processBinding(core::Context ctx, cfg::Binding &bind, int loopCount,
                                                   int bindMinLoops, KnowledgeFilter &knowledgeFilter,
                                                   core::TypeConstraint &constr) {
    try {
        core::TypeAndOrigins tp;
        bool noLoopChecking =
            cfg::isa_instruction<cfg::Alias>(bind.value.get()) || cfg::isa_instruction<cfg::LoadArg>(bind.value.get());

        bool checkFullyDefined = true;
        bool lspQueryMatch = !ctx.state.lspInfoQueryLoc.is_none() && bind.loc.contains(ctx.state.lspInfoQueryLoc);

        typecase(
            bind.value.get(),
            [&](cfg::Send *send) {
                vector<core::TypeAndOrigins> args;

                args.reserve(send->args.size());
                for (core::LocalVariable arg : send->args) {
                    args.emplace_back(getTypeAndOrigin(ctx, arg));
                }

                auto recvType = getTypeAndOrigin(ctx, send->recv);
                if (send->link) {
                    send->link->receiver = recvType.type;
                    send->link->returnTp = core::Types::untyped();
                    send->link->blockPreType = core::Types::untyped();
                    send->link->sendTp = core::Types::untyped();
                    checkFullyDefined = false;
                }
                if (send->fun == core::Names::super()) {
                    // TODO
                    tp.type = core::Types::untyped();
                } else {
                    auto dispatched = recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type,
                                                                  recvType.type, send->link);

                    core::histogramInc("dispatchCall.components", dispatched.components.size());
                    tp.type = dispatched.returnType;
                    for (auto &comp : dispatched.components) {
                        for (auto &err : comp.errors) {
                            ctx.state._error(move(err));
                        }
                    }

                    if (lspQueryMatch) {
                        setQueryResponse(ctx, core::QueryResponse::Kind::SEND, std::move(dispatched.components),
                                         send->link ? send->link->constr : nullptr, bind.loc, tp);
                    }
                }
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Ident *i) {
                auto typeAndOrigin = getOrCreateTypeAndOrigin(ctx, i->what);
                tp.type = typeAndOrigin.type;
                tp.origins = typeAndOrigin.origins;

                if (lspQueryMatch) {
                    setQueryResponse(ctx, core::QueryResponse::Kind::IDENT, {}, nullptr, bind.loc, tp);
                }

                ENFORCE(!tp.origins.empty(), "Inferencer did not assign location");
            },
            [&](cfg::Alias *a) {
                core::SymbolRef symbol = a->what.data(ctx).dealias(ctx);
                const core::Symbol &data = symbol.data(ctx);
                if (data.isClass()) {
                    if (!data.resultType) { // common case
                        tp.type = data.lookupSingletonClass(ctx).data(ctx).externalType(ctx);
                    } else {
                        tp.type = data.resultType;
                    }
                    tp.origins.push_back(symbol.data(ctx).loc);
                } else if (data.isField() || data.isStaticField() || data.isMethodArgument() || data.isTypeMember()) {
                    if (data.resultType.get() != nullptr) {
                        if (data.isField()) {
                            tp.type = core::Types::resultTypeAsSeenFrom(
                                ctx, symbol, ctx.owner.data(ctx).enclosingClass(ctx),
                                ctx.owner.data(ctx).enclosingClass(ctx).data(ctx).selfTypeArgs(ctx));
                        } else {
                            tp.type = data.resultType;
                        }
                        tp.origins.push_back(data.loc);
                    } else {
                        tp.origins.push_back(core::Loc::none());
                        tp.type = core::Types::untyped();
                    }
                } else {
                    Error::notImplemented();
                }

                if (lspQueryMatch) {
                    core::DispatchResult::ComponentVec components;
                    components.push_back(core::DispatchComponent{tp.type, symbol, {}});
                    setQueryResponse(ctx, core::QueryResponse::Kind::CONSTANT, std::move(components), nullptr, bind.loc,
                                     tp);
                }
                pinnedTypes[bind.bind] = tp;
            },
            [&](cfg::SolveConstraint *i) {
                if (!i->link->constr->solve(ctx)) {
                    if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::GenericMethodConstaintUnsolved)) {
                        e.setHeader("Could not find valid instantiation of type parameters");
                    }
                }

                auto type = core::Types::instantiate(ctx, i->link->sendTp, *i->link->constr);
                type = flatmapHack(ctx, i->link, type);
                tp.type = move(type);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Self *i) {
                tp.type = i->klass.data(ctx).selfType(ctx);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::LoadArg *i) {
                /* read type from info filled by define_method */
                auto argType = getTypeAndOrigin(ctx, i->receiver).type->getCallArgumentType(ctx, i->method, i->arg);
                ENFORCE(argType != nullptr);
                tp.type = core::Types::instantiate(ctx, argType, constr);
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::LoadYieldParams *insn) {
                auto &procType = insn->link->blockPreType;
                vector<shared_ptr<core::Type>> types;
                auto narg = insn->block.data(ctx).arguments().size();

                int lastArg = 0;
                for (int i = 0; i < narg; ++i) {
                    shared_ptr<core::Type> arg;
                    if (procType == nullptr) {
                        arg = core::Types::untyped();
                    } else {
                        arg = procType->getCallArgumentType(ctx, core::Names::call(), i);
                    }
                    if (arg == nullptr) {
                        arg = core::Types::untyped();
                    } else {
                        lastArg = i;
                    }
                    types.emplace_back(move(arg));
                }

                // A multi-arg proc, if provided a single arg which is an array,
                // will implicitly splat it out.
                //
                // TODO(nelhage): If this block is a lambda, not a proc, this
                // rule doesn't apply. We don't model the distinction accurately
                // yet.
                if (lastArg == 0 && narg > 1 && types.front()->derivesFrom(ctx, core::Symbols::Array()) &&
                    !types.front()->isUntyped()) {
                    tp.type = move(types.front());
                } else {
                    tp.type = make_shared<core::TupleType>(core::Types::arrayOfUntyped(), move(types));
                }

                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Return *i) {
                auto expectedType = ctx.owner.data(ctx).resultType;
                if (!expectedType) {
                    expectedType = core::Types::untyped();
                } else {
                    expectedType = core::Types::instantiate(
                        ctx,
                        core::Types::resultTypeAsSeenFrom(
                            ctx, ctx.owner, ctx.owner.data(ctx).enclosingClass(ctx),
                            ctx.owner.data(ctx).enclosingClass(ctx).data(ctx).selfTypeArgs(ctx)),
                        constr);
                    expectedType = core::Types::replaceSelfType(
                        ctx, expectedType, ctx.owner.data(ctx).enclosingClass(ctx).data(ctx).selfType(ctx));
                }

                if (core::Types::isSubType(ctx, core::Types::void_(), expectedType)) {
                    expectedType = core::Types::untyped();
                }

                tp.type = core::Types::bottom();
                tp.origins.push_back(bind.loc);

                auto typeAndOrigin = getTypeAndOrigin(ctx, i->what);
                if (!core::Types::isSubType(ctx, typeAndOrigin.type, expectedType)) {
                    if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::ReturnTypeMismatch)) {
                        e.setHeader("Returning value that does not conform to method result type");
                        e.addErrorSection(core::ErrorSection(
                            "Expected " + expectedType->show(ctx),
                            {
                                core::ErrorLine::from(ctx.owner.data(ctx).loc, "Method `{}` has return type `{}`",
                                                      ctx.owner.data(ctx).name.toString(ctx),
                                                      expectedType->toString(ctx)),
                            }));
                        e.addErrorSection(
                            core::ErrorSection("Got " + typeAndOrigin.type->toString(ctx) + " originating from:",
                                               typeAndOrigin.origins2Explanations(ctx)));
                    }
                }
            },
            [&](cfg::BlockReturn *i) {
                ENFORCE(i->link);
                ENFORCE(i->link->returnTp != nullptr);

                auto typeAndOrigin = getTypeAndOrigin(ctx, i->what);
                auto expectedType = i->link->returnTp;
                if (!core::Types::isSubTypeUnderConstraint(ctx, *i->link->constr, typeAndOrigin.type, expectedType)) {
                    // TODO(nelhage): We should somehow report location
                    // information about the `send` and/or the
                    // definition of the block type

                    if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::ReturnTypeMismatch)) {
                        e.setHeader("Returning value that does not conform to block result type");
                        e.addErrorSection(core::ErrorSection("Expected " + expectedType->toString(ctx)));
                        e.addErrorSection(
                            core::ErrorSection("Got " + typeAndOrigin.type->toString(ctx) + " originating from:",
                                               typeAndOrigin.origins2Explanations(ctx)));
                    }
                }

                tp.type = core::Types::bottom();
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Literal *i) {
                tp.type = i->value;
                tp.origins.push_back(bind.loc);

                if (lspQueryMatch) {
                    setQueryResponse(ctx, core::QueryResponse::Kind::LITERAL, {}, nullptr, bind.loc, tp);
                }
            },
            [&](cfg::Unanalyzable *i) {
                tp.type = core::Types::untyped();
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Cast *c) {
                auto klass = ctx.owner.data(ctx).enclosingClass(ctx);
                auto castType = core::Types::instantiate(ctx, c->type, klass.data(ctx).typeMembers(),
                                                         klass.data(ctx).selfTypeArgs(ctx));

                tp.type = castType;
                tp.origins.push_back(bind.loc);

                if (!hasType(ctx, bind.bind)) {
                    noLoopChecking = true;
                }

                auto ty = getTypeAndOrigin(ctx, c->value);
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
                    } else if (!ty.type->isUntyped() && core::Types::isSubType(ctx, ty.type, castType)) {
                        if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::InvalidCast)) {
                            e.setHeader("Useless cast: inferred type `{}` is already a subtype of `{}`",
                                        ty.type->show(ctx), castType->show(ctx));
                        }
                    }
                }
                if (c->cast == core::Names::let()) {
                    pinnedTypes[bind.bind] = tp;
                }
            },
            [&](cfg::DebugEnvironment *d) {
                d->str = toString(ctx);
                tp.type = core::Types::nilClass();
                tp.origins.push_back(bind.loc);
            });

        ENFORCE(tp.type.get() != nullptr, "Inferencer did not assign type: ", bind.value->toString(ctx));
        tp.type->sanityCheck(ctx);

        if (checkFullyDefined && !tp.type->isFullyDefined()) {
            if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::IncompleteType)) {
                e.setHeader("Expression does not have a fully-defined type (Did you reference another class's type "
                            "members?)");
            }
            tp.type = core::Types::untyped();
        }
        ENFORCE(!tp.origins.empty(), "Inferencer did not assign location");

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
            auto pin = pinnedTypes.find(bind.bind);
            if (pin != pinnedTypes.end()) {
                cur = pin->second;
            }
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
                                    e.addErrorSection(core::ErrorSection(
                                        "Consider declaring the variable with T.let():",
                                        {core::ErrorLine::from(cur.origins[0], "T.let(..., {})", suggest->show(ctx))}));
                                } else {
                                    e.addErrorSection(
                                        core::ErrorSection("Original type from:", cur.origins2Explanations(ctx)));
                                }
                            }

                            tp.type = core::Types::untyped();
                        }
                        break;
                    }
                }
            }
        }

        clearKnowledge(ctx, bind.bind, knowledgeFilter);
        if (auto *send = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
            updateKnowledge(ctx, bind.bind, bind.loc, send, knowledgeFilter);
        } else if (auto *i = cfg::cast_instruction<cfg::Ident>(bind.value.get())) {
            propagateKnowledge(ctx, bind.bind, i->what, knowledgeFilter);
        }
        setTypeAndOrigin(bind.bind, tp);

        return tp.type;
    } catch (SRubyException &) {
        if (auto e = ctx.state.beginError(bind.loc, core::errors::Internal::InternalError)) {
            e.setHeader("Failed to type (backtrace is above)");
        }
        throw;
    }
}

void Environment::cloneFrom(const Environment &rhs) {
    *this = rhs;
}

const TestedKnowledge &Environment::getKnowledge(core::LocalVariable symbol, bool shouldFail) const {
    auto fnd = find(vars.begin(), vars.end(), symbol);
    if (fnd == vars.end()) {
        ENFORCE(!shouldFail, "Missing knowledge?");
        return TestedKnowledge::empty;
    }
    auto &r = knowledge[fnd - vars.begin()];
    r.sanityCheck();
    return r;
}

TestedKnowledge TestedKnowledge::empty;

} // namespace infer
} // namespace sorbet
