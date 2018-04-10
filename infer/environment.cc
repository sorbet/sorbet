#include "environment.h"
#include <algorithm> // find, remove_if

template struct std::pair<ruby_typer::core::LocalVariable, std::shared_ptr<ruby_typer::core::Type>>;

using namespace std;

namespace ruby_typer {
namespace infer {

shared_ptr<core::Type> dropLiteral(shared_ptr<core::Type> tp) {
    if (auto *a = core::cast_type<core::LiteralType>(tp.get())) {
        return a->underlying;
    }
    return tp;
}
shared_ptr<core::Type> dropConstructor(core::Context ctx, core::Loc loc, shared_ptr<core::Type> tp) {
    if (auto *mt = core::cast_type<core::MetaType>(tp.get())) {
        if (!mt->wrapped->isDynamic()) {
            if (auto e = ctx.state.beginError(loc, core::errors::Infer::BareTypeUsage)) {
                e.setHeader("Unsupported usage of bare type");
            }
        }
        return core::Types::dynamic();
    }
    return tp;
}

KnowledgeFilter::KnowledgeFilter(core::Context ctx, unique_ptr<cfg::CFG> &cfg) {
    for (auto &bb : cfg->basicBlocks) {
        if (bb->bexit.cond != core::LocalVariable::noVariable() && bb->bexit.cond != core::LocalVariable::blockCall()) {
            used_vars.insert(bb->bexit.cond);
        }
        for (auto &bind : bb->exprs) {
            if (cfg::Send *send = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
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
                if (cfg::Ident *id = cfg::cast_instruction<cfg::Ident>(bind.value.get())) {
                    if (isNeeded(bind.bind) && !isNeeded(id->what)) {
                        used_vars.insert(id->what);
                        changed = true;
                    }
                } else if (cfg::Send *send = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
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

void KnowledgeFact::min(core::Context ctx, const KnowledgeFact &other) {
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

void KnowledgeFact::sanityCheck() const {
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

string Environment::toString(core::Context ctx) {
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
        ret.origins.push_back(ctx.owner.data(ctx).definitionLoc);
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
        ret.origins.push_back(ctx.owner.data(ctx).definitionLoc);
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
            if (auto e = ctx.state.beginError(loc, core::errors::Infer::DeadBranchInferencer)) {
                e.setHeader("Hard assert is always false");
            }
        }
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
    return;
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
            thisTO.type = core::Types::lub(ctx, thisTO.type, otherTO.type);
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

    for (auto &blk : other.blockTypes) {
        auto mine = this->blockTypes.find(blk.first);
        if (mine == this->blockTypes.end()) {
            this->blockTypes[blk.first] = blk.second;
        } else {
            ENFORCE(mine->second == blk.second);
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
                    tp.type = core::Types::lub(ctx, tp.type, otherPin->second.type);
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

    this->blockTypes = other.blockTypes;
    this->pinnedTypes = other.pinnedTypes;
}

shared_ptr<core::Type> Environment::getReturnType(core::Context ctx, shared_ptr<core::Type> procType) {
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

shared_ptr<core::Type> Environment::processBinding(core::Context ctx, cfg::Binding &bind, int loopCount,
                                                   int bindMinLoops, KnowledgeFilter &knowledgeFilter) {
    try {
        core::TypeAndOrigins tp;
        bool noLoopChecking =
            cfg::isa_instruction<cfg::Alias>(bind.value.get()) || cfg::isa_instruction<cfg::LoadArg>(bind.value.get());

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
                    tp.type = recvType.type->dispatchCall(ctx, send->fun, bind.loc, args, recvType.type, recvType.type,
                                                          blockTy);
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
                    if (!data.resultType) { // common case
                        tp.type = data.lookupSingletonClass(ctx).data(ctx).externalType(ctx);
                    } else {
                        tp.type = data.resultType;
                    }
                    tp.origins.push_back(symbol.data(ctx).definitionLoc);
                } else if (data.isField() || data.isStaticField() || data.isMethodArgument() || data.isTypeMember()) {
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
                pinnedTypes[bind.bind] = tp;
            },
            [&](cfg::Self *i) {
                tp.type = i->klass.data(ctx).selfType(ctx);
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
                    if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::ReturnTypeMismatch)) {
                        e.setHeader("Returning value that does not conform to method result type");
                        e.addErrorSection(core::ErrorSection(
                            "Expected " + expectedType->show(ctx),
                            {
                                core::ErrorLine::from(ctx.owner.data(ctx).definitionLoc,
                                                      "Method `{}` has return type `{}`",
                                                      ctx.owner.data(ctx).name.toString(ctx), expectedType->show(ctx)),
                            }));
                        e.addErrorSection(
                            core::ErrorSection("Got " + typeAndOrigin.type->show(ctx) + " originating from:",
                                               typeAndOrigin.origins2Explanations(ctx)));
                    }
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
                        if (!core::Types::isSubType(ctx, typeAndOrigin.type, expectedType)) {
                            if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::ReturnTypeMismatch)) {
                                e.setHeader("Returning value that does not conform to block result type");
                                e.addErrorSection(core::ErrorSection("Expected " + expectedType->toString(ctx)));
                                e.addErrorSection(core::ErrorSection("Got " + typeAndOrigin.type->toString(ctx) +
                                                                         " originating from:",
                                                                     typeAndOrigin.origins2Explanations(ctx)));
                            }
                        }
                    }
                }

                tp.type = core::Types::bottom();
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Literal *i) {
                tp.type = i->value;
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Unanalyzable *i) {
                tp.type = core::Types::dynamic();
                tp.origins.push_back(bind.loc);
            },
            [&](cfg::Cast *c) {
                auto klass = ctx.owner.data(ctx).enclosingClass(ctx);
                auto castType =
                    c->type->instantiate(ctx, klass.data(ctx).typeMembers(), klass.data(ctx).selfTypeArgs(ctx));
                if (castType == nullptr) {
                    castType = c->type;
                }

                tp.type = castType;
                tp.origins.push_back(bind.loc);

                if (!hasType(ctx, bind.bind)) {
                    noLoopChecking = true;
                }

                if (c->cast != core::Names::cast()) {
                    auto ty = getTypeAndOrigin(ctx, c->value);
                    if (c->cast == core::Names::assertType() && ty.type->isDynamic()) {
                        if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::CastTypeMismatch)) {
                            e.setHeader("The typechecker was unable to infer the type of the asserted value.");
                            e.addErrorSection(core::ErrorSection("Got " + ty.type->show(ctx) + " originating from:",
                                                                 ty.origins2Explanations(ctx)));
                            e.addErrorSection(core::ErrorSection("You may need to add additional `sig` "
                                                                 "annotations."));
                        }
                    } else if (!core::Types::isSubType(ctx, ty.type, castType)) {
                        if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::CastTypeMismatch)) {
                            e.setHeader("Argument does not have asserted type {}", castType->show(ctx));
                            e.addErrorSection(core::ErrorSection("Got " + ty.type->show(ctx) + " originating from:",
                                                                 ty.origins2Explanations(ctx)));
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

        if (!tp.type->isFullyDefined()) {
            if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::IncompleteType)) {
                e.setHeader("Expression does not have a fully-defined type (Did you reference another class's type "
                            "members?)");
            }
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
            auto pin = pinnedTypes.find(bind.bind);
            if (pin != pinnedTypes.end()) {
                cur = pin->second;
            }
            bool asGoodAs = core::Types::isSubType(ctx, dropLiteral(tp.type), dropLiteral(cur.type));

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
                    default:
                        if (!asGoodAs || (tp.type->isDynamic() && !cur.type->isDynamic())) {
                            if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::PinnedVariableMismatch)) {
                                e.setHeader("Changing type of a variable in a loop, `{}` is not a subtype of `{}`",
                                            tp.type->show(ctx), cur.type->show(ctx));
                            }
                            tp.type = core::Types::dynamic();
                        }
                        break;
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
    } catch (...) {
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
} // namespace ruby_typer
