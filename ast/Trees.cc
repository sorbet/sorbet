#include "ast/Trees.h"
#include "absl/synchronization/blocking_counter.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/concurrency/WorkerPool.h"
#include "common/formatting.h"
#include "common/typecase.h"
#include "core/Symbols.h"
#include <sstream>
#include <utility>

using namespace std;

namespace sorbet::ast {

#define CASE_STATEMENT(CASE_BODY, T) \
    case Tag::T: {                   \
        CASE_BODY(T)                 \
        break;                       \
    }

#define GENERATE_TAG_SWITCH(tag, CASE_BODY)              \
    switch (tag) {                                       \
        CASE_STATEMENT(CASE_BODY, EmptyTree)             \
        CASE_STATEMENT(CASE_BODY, Send)                  \
        CASE_STATEMENT(CASE_BODY, ClassDef)              \
        CASE_STATEMENT(CASE_BODY, MethodDef)             \
        CASE_STATEMENT(CASE_BODY, If)                    \
        CASE_STATEMENT(CASE_BODY, While)                 \
        CASE_STATEMENT(CASE_BODY, Break)                 \
        CASE_STATEMENT(CASE_BODY, Retry)                 \
        CASE_STATEMENT(CASE_BODY, Next)                  \
        CASE_STATEMENT(CASE_BODY, Return)                \
        CASE_STATEMENT(CASE_BODY, RescueCase)            \
        CASE_STATEMENT(CASE_BODY, Rescue)                \
        CASE_STATEMENT(CASE_BODY, Local)                 \
        CASE_STATEMENT(CASE_BODY, UnresolvedIdent)       \
        CASE_STATEMENT(CASE_BODY, RestArg)               \
        CASE_STATEMENT(CASE_BODY, KeywordArg)            \
        CASE_STATEMENT(CASE_BODY, OptionalArg)           \
        CASE_STATEMENT(CASE_BODY, BlockArg)              \
        CASE_STATEMENT(CASE_BODY, ShadowArg)             \
        CASE_STATEMENT(CASE_BODY, Assign)                \
        CASE_STATEMENT(CASE_BODY, Cast)                  \
        CASE_STATEMENT(CASE_BODY, Hash)                  \
        CASE_STATEMENT(CASE_BODY, Array)                 \
        CASE_STATEMENT(CASE_BODY, Literal)               \
        CASE_STATEMENT(CASE_BODY, UnresolvedConstantLit) \
        CASE_STATEMENT(CASE_BODY, ConstantLit)           \
        CASE_STATEMENT(CASE_BODY, ZSuperArgs)            \
        CASE_STATEMENT(CASE_BODY, Block)                 \
        CASE_STATEMENT(CASE_BODY, InsSeq)                \
    }

void ExpressionPtr::deleteTagged(Tag tag, void *ptr) noexcept {
    ENFORCE(ptr != nullptr);
#define DELETE_TYPE(T)                                      \
    if (tag != Tag::EmptyTree) {                            \
        /* explicitly not deleting the empty tree pointer*/ \
        delete reinterpret_cast<T *>(ptr);                  \
    }

    GENERATE_TAG_SWITCH(tag, DELETE_TYPE)

#undef DELETE_TYPE
}

string ExpressionPtr::nodeName() const {
    auto *ptr = get();

    ENFORCE(ptr != nullptr);

#define NODE_NAME(name) return reinterpret_cast<name *>(ptr)->nodeName();
    GENERATE_TAG_SWITCH(tag(), NODE_NAME)
#undef NODE_NAME
}

string ExpressionPtr::showRaw(const core::GlobalState &gs, int tabs) {
    auto *ptr = get();

    ENFORCE(ptr != nullptr);

#define SHOW_RAW(name) return reinterpret_cast<name *>(ptr)->showRaw(gs, tabs);
    GENERATE_TAG_SWITCH(tag(), SHOW_RAW)
#undef SHOW_RAW
}

core::LocOffsets ExpressionPtr::loc() const {
    auto *ptr = get();

    ENFORCE(ptr != nullptr);

#define CASE(name) return reinterpret_cast<name *>(ptr)->loc;
    GENERATE_TAG_SWITCH(tag(), CASE)
#undef CASE
}

string ExpressionPtr::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    auto *ptr = get();

    ENFORCE(ptr != nullptr);

#define CASE(name) return reinterpret_cast<name *>(ptr)->toStringWithTabs(gs, tabs);
    GENERATE_TAG_SWITCH(tag(), CASE)
#undef CASE
}

bool ExpressionPtr::isSelfReference() const {
    if (auto *local = cast_tree<Local>(*this)) {
        return local->localVariable == core::LocalVariable::selfVariable();
    }
    return false;
}

bool isa_reference(const ExpressionPtr &what) {
    return isa_tree<Local>(what) || isa_tree<UnresolvedIdent>(what) || isa_tree<RestArg>(what) ||
           isa_tree<KeywordArg>(what) || isa_tree<OptionalArg>(what) || isa_tree<BlockArg>(what) ||
           isa_tree<ShadowArg>(what);
}

bool isa_declaration(const ExpressionPtr &what) {
    return isa_tree<MethodDef>(what) || isa_tree<ClassDef>(what);
}

/** https://git.corp.stripe.com/gist/nelhage/51564501674174da24822e60ad770f64
 *
 *  [] - prototype only
 *
 *                 / Control Flow <- while, if, for, break, next, return, rescue, case
 * Pre-CFG-Node <-
 *                 \ Instruction <- assign, send, [new], ident, named_arg, hash, array, literals(symbols, ints, floats,
 * strings, constants, nil), constants(resolver will desugar it into literals), array_splat(*), hash_splat(**), self,
 * insseq, Block)
 *
 *                  \ Definition  <-  class(name, parent, mixins, body)
 *                                    module
 *                                    def
 *                                    defself
 *                                    const_assign
 *
 *
 *
 * know id for: top, bottom, kernel?, basicobject, class, module [postponed], unit, Hash, Array, String, Symbol, float,
 * int, numeric, double, unknown
 *
 *
 *
 * Desugar string concatenation into series of .to_s calls and string concatenations
 */

ClassDef::ClassDef(core::LocOffsets loc, core::LocOffsets declLoc, core::ClassOrModuleRef symbol, ExpressionPtr name,
                   ANCESTORS_store ancestors, RHS_store rhs, ClassDef::Kind kind)
    : loc(loc), declLoc(declLoc), symbol(symbol), kind(kind), rhs(std::move(rhs)), name(std::move(name)),
      ancestors(std::move(ancestors)) {
    categoryCounterInc("trees", "classdef");
    histogramInc("trees.classdef.kind", (int)kind);
    histogramInc("trees.classdef.ancestors", this->ancestors.size());
    _sanityCheck();
}

MethodDef::MethodDef(core::LocOffsets loc, core::LocOffsets declLoc, core::MethodRef symbol, core::NameRef name,
                     ARGS_store args, ExpressionPtr rhs, Flags flags)
    : loc(loc), declLoc(declLoc), symbol(symbol), rhs(std::move(rhs)), args(std::move(args)), name(name), flags(flags) {
    categoryCounterInc("trees", "methoddef");
    histogramInc("trees.methodDef.args", this->args.size());
    _sanityCheck();
}

If::If(core::LocOffsets loc, ExpressionPtr cond, ExpressionPtr thenp, ExpressionPtr elsep)
    : loc(loc), cond(std::move(cond)), thenp(std::move(thenp)), elsep(std::move(elsep)) {
    categoryCounterInc("trees", "if");
    _sanityCheck();
}

While::While(core::LocOffsets loc, ExpressionPtr cond, ExpressionPtr body)
    : loc(loc), cond(std::move(cond)), body(std::move(body)) {
    categoryCounterInc("trees", "while");
    _sanityCheck();
}

Break::Break(core::LocOffsets loc, ExpressionPtr expr) : loc(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "break");
    _sanityCheck();
}

Retry::Retry(core::LocOffsets loc) : loc(loc) {
    categoryCounterInc("trees", "retry");
    _sanityCheck();
}

Next::Next(core::LocOffsets loc, ExpressionPtr expr) : loc(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "next");
    _sanityCheck();
}

Return::Return(core::LocOffsets loc, ExpressionPtr expr) : loc(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "return");
    _sanityCheck();
}

RescueCase::RescueCase(core::LocOffsets loc, EXCEPTION_store exceptions, ExpressionPtr var, ExpressionPtr body)
    : loc(loc), exceptions(std::move(exceptions)), var(std::move(var)), body(std::move(body)) {
    categoryCounterInc("trees", "rescuecase");
    histogramInc("trees.rescueCase.exceptions", this->exceptions.size());
    _sanityCheck();
}

Rescue::Rescue(core::LocOffsets loc, ExpressionPtr body, RESCUE_CASE_store rescueCases, ExpressionPtr else_,
               ExpressionPtr ensure)
    : loc(loc), body(std::move(body)), rescueCases(std::move(rescueCases)), else_(std::move(else_)),
      ensure(std::move(ensure)) {
    categoryCounterInc("trees", "rescue");
    histogramInc("trees.rescue.rescuecases", this->rescueCases.size());
    _sanityCheck();
}

Local::Local(core::LocOffsets loc, core::LocalVariable localVariable1) : loc(loc), localVariable(localVariable1) {
    categoryCounterInc("trees", "local");
    _sanityCheck();
}

UnresolvedIdent::UnresolvedIdent(core::LocOffsets loc, Kind kind, core::NameRef name)
    : loc(loc), name(name), kind(kind) {
    categoryCounterInc("trees", "unresolvedident");
    _sanityCheck();
    _sanityCheck();
}

Assign::Assign(core::LocOffsets loc, ExpressionPtr lhs, ExpressionPtr rhs)
    : loc(loc), lhs(std::move(lhs)), rhs(std::move(rhs)) {
    categoryCounterInc("trees", "assign");
    _sanityCheck();
}

Send::Send(core::LocOffsets loc, ExpressionPtr recv, core::NameRef fun, core::LocOffsets funLoc, uint16_t numPosArgs,
           Send::ARGS_store args, Flags flags)
    : loc(loc), fun(fun), funLoc(funLoc), flags(flags), numPosArgs_(numPosArgs), recv(std::move(recv)),
      args(std::move(args)) {
    categoryCounterInc("trees", "send");
    if (hasBlock()) {
        counterInc("trees.send.with_block");
    }
    histogramInc("trees.send.args", this->args.size());
    _sanityCheck();
}

Cast::Cast(core::LocOffsets loc, core::TypePtr ty, ExpressionPtr arg, core::NameRef cast)
    : loc(loc), cast(cast), type(std::move(ty)), arg(std::move(arg)) {
    categoryCounterInc("trees", "cast");
    _sanityCheck();
}

ZSuperArgs::ZSuperArgs(core::LocOffsets loc) : loc(loc) {
    categoryCounterInc("trees", "zsuper");
    _sanityCheck();
}

RestArg::RestArg(core::LocOffsets loc, ExpressionPtr arg) : loc(loc), expr(std::move(arg)) {
    categoryCounterInc("trees", "restarg");
    _sanityCheck();
}

KeywordArg::KeywordArg(core::LocOffsets loc, ExpressionPtr expr) : loc(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "keywordarg");
    _sanityCheck();
}

OptionalArg::OptionalArg(core::LocOffsets loc, ExpressionPtr expr, ExpressionPtr default_)
    : loc(loc), expr(std::move(expr)), default_(std::move(default_)) {
    categoryCounterInc("trees", "optionalarg");
    _sanityCheck();
}

ShadowArg::ShadowArg(core::LocOffsets loc, ExpressionPtr expr) : loc(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "shadowarg");
    _sanityCheck();
}

BlockArg::BlockArg(core::LocOffsets loc, ExpressionPtr expr) : loc(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "blockarg");
    _sanityCheck();
}

Literal::Literal(core::LocOffsets loc, const core::TypePtr &value) : loc(loc), value(std::move(value)) {
    categoryCounterInc("trees", "literal");
    _sanityCheck();
}

UnresolvedConstantLit::UnresolvedConstantLit(core::LocOffsets loc, ExpressionPtr scope, core::NameRef cnst)
    : loc(loc), cnst(cnst), scope(std::move(scope)) {
    categoryCounterInc("trees", "constantlit");
    _sanityCheck();
}

ConstantLit::ConstantLit(core::LocOffsets loc, core::SymbolRef symbol, ExpressionPtr original)
    : loc(loc), symbol(symbol), original(std::move(original)) {
    categoryCounterInc("trees", "resolvedconstantlit");
    _sanityCheck();
}

optional<pair<core::SymbolRef, vector<core::NameRef>>>
ConstantLit::fullUnresolvedPath(const core::GlobalState &gs) const {
    if (this->symbol != core::Symbols::StubModule()) {
        return nullopt;
    }
    ENFORCE(this->resolutionScopes != nullptr && !this->resolutionScopes->empty());

    vector<core::NameRef> namesFailedToResolve;
    auto *nested = this;
    {
        while (!nested->resolutionScopes->front().exists()) {
            auto *orig = cast_tree<UnresolvedConstantLit>(nested->original);
            ENFORCE(orig);
            namesFailedToResolve.emplace_back(orig->cnst);
            nested = ast::cast_tree<ast::ConstantLit>(orig->scope);
            ENFORCE(nested);
            ENFORCE(nested->symbol == core::Symbols::StubModule());
            ENFORCE(!nested->resolutionScopes->empty());
        }
        auto *orig = cast_tree<UnresolvedConstantLit>(nested->original);
        ENFORCE(orig);
        namesFailedToResolve.emplace_back(orig->cnst);
        absl::c_reverse(namesFailedToResolve);
    }
    auto prefix = nested->resolutionScopes->front();
    return make_pair(prefix, move(namesFailedToResolve));
}

Block::Block(core::LocOffsets loc, MethodDef::ARGS_store args, ExpressionPtr body)
    : loc(loc), args(std::move(args)), body(std::move(body)) {
    categoryCounterInc("trees", "block");
    _sanityCheck();
};

Hash::Hash(core::LocOffsets loc, ENTRY_store keys, ENTRY_store values)
    : loc(loc), keys(std::move(keys)), values(std::move(values)) {
    categoryCounterInc("trees", "hash");
    histogramInc("trees.hash.entries", this->keys.size());
    _sanityCheck();
}

Array::Array(core::LocOffsets loc, ENTRY_store elems) : loc(loc), elems(std::move(elems)) {
    categoryCounterInc("trees", "array");
    histogramInc("trees.array.elems", this->elems.size());
    _sanityCheck();
}

InsSeq::InsSeq(core::LocOffsets loc, STATS_store stats, ExpressionPtr expr)
    : loc(loc), stats(std::move(stats)), expr(std::move(expr)) {
    categoryCounterInc("trees", "insseq");
    histogramInc("trees.insseq.stats", this->stats.size());
    _sanityCheck();
}

EmptyTree::EmptyTree() : loc(core::LocOffsets::none()) {
    categoryCounterInc("trees", "emptytree");
    _sanityCheck();
}

namespace {

EmptyTree singletonEmptyTree{};

} // namespace

template <> ExpressionPtr make_expression<EmptyTree>() {
    ExpressionPtr result = nullptr;
    result.reset(&singletonEmptyTree);
    return result;
}

namespace {

void printTabs(fmt::memory_buffer &to, int count) {
    int i = 0;
    while (i < count) {
        fmt::format_to(std::back_inserter(to), "  ");
        i++;
    }
}

template <class T> void printElems(const core::GlobalState &gs, fmt::memory_buffer &buf, T &args, int tabs) {
    bool first = true;
    bool didshadow = false;
    for (auto &a : args) {
        if (isa_tree<Block>(a)) {
            continue;
        }
        if (!first) {
            if (isa_tree<ShadowArg>(a) && !didshadow) {
                fmt::format_to(std::back_inserter(buf), "; ");
                didshadow = true;
            } else {
                fmt::format_to(std::back_inserter(buf), ", ");
            }
        }
        first = false;
        fmt::format_to(std::back_inserter(buf), "{}", a.toStringWithTabs(gs, tabs + 1));
    }
};

template <class T> void printArgs(const core::GlobalState &gs, fmt::memory_buffer &buf, T &args, int tabs) {
    fmt::format_to(std::back_inserter(buf), "(");
    printElems(gs, buf, args, tabs);
    fmt::format_to(std::back_inserter(buf), ")");
}

} // namespace

string ClassDef::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    if (kind == ClassDef::Kind::Module) {
        fmt::format_to(std::back_inserter(buf), "module ");
    } else {
        fmt::format_to(std::back_inserter(buf), "class ");
    }
    fmt::format_to(std::back_inserter(buf), "{}<{}> < ", name.toStringWithTabs(gs, tabs),
                   this->symbol.dataAllowingNone(gs)->name.toString(gs));
    printArgs(gs, buf, this->ancestors, tabs);

    if (this->rhs.empty()) {
        fmt::format_to(std::back_inserter(buf), "{}", '\n');
    }

    for (auto &a : this->rhs) {
        fmt::format_to(std::back_inserter(buf), "{}", '\n');
        printTabs(buf, tabs + 1);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.toStringWithTabs(gs, tabs + 1));
    }

    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "end");
    return fmt::to_string(buf);
}

string ClassDef::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "kind = {}\n", kind == ClassDef::Kind::Module ? "module" : "class");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "name = {}\n", name.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "symbol = {}\n", this->symbol.dataAllowingNone(gs)->name.showRaw(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "ancestors = [");
    bool first = true;
    for (auto &a : this->ancestors) {
        if (!first) {
            fmt::format_to(std::back_inserter(buf), ", ");
        }
        first = false;
        fmt::format_to(std::back_inserter(buf), "{}", a.showRaw(gs, tabs + 2));
    }
    fmt::format_to(std::back_inserter(buf), "]\n");

    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "rhs = [\n");

    for (auto &a : this->rhs) {
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.showRaw(gs, tabs + 2));
        if (&a != &this->rhs.back()) {
            fmt::format_to(std::back_inserter(buf), "{}", '\n');
        }
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string InsSeq::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "begin\n");
    for (auto &a : this->stats) {
        printTabs(buf, tabs + 1);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.toStringWithTabs(gs, tabs + 1));
    }

    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "{}\n", expr.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "end");
    return fmt::to_string(buf);
}

string InsSeq::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "stats = [\n");
    for (auto &a : this->stats) {
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.showRaw(gs, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "],\n");

    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "expr = {}\n", expr.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string MethodDef::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;

    if (this->flags.isSelfMethod) {
        fmt::format_to(std::back_inserter(buf), "def self.");
    } else {
        fmt::format_to(std::back_inserter(buf), "def ");
    }
    fmt::format_to(std::back_inserter(buf), "{}", name.toString(gs));
    const auto data = this->symbol.data(gs);
    if (name != data->name) {
        fmt::format_to(std::back_inserter(buf), "<{}>", data->name.toString(gs));
    }
    fmt::format_to(std::back_inserter(buf), "(");
    bool first = true;
    if (this->symbol == core::Symbols::todoMethod()) {
        for (auto &a : this->args) {
            if (!first) {
                fmt::format_to(std::back_inserter(buf), ", ");
            }
            first = false;
            fmt::format_to(std::back_inserter(buf), "{}", a.toStringWithTabs(gs, tabs + 1));
        }
    } else {
        for (auto &a : data->arguments) {
            if (!first) {
                fmt::format_to(std::back_inserter(buf), ", ");
            }
            first = false;
            fmt::format_to(std::back_inserter(buf), "{}", a.argumentName(gs));
        }
    }
    fmt::format_to(std::back_inserter(buf), ")\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "{}\n", this->rhs.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "end");
    return fmt::to_string(buf);
}

string MethodDef::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);

    auto stringifiedFlags = vector<string>{};
    if (this->flags.isSelfMethod) {
        stringifiedFlags.emplace_back("self");
    }
    if (this->flags.isRewriterSynthesized) {
        stringifiedFlags.emplace_back("rewriterSynthesized");
    }
    fmt::format_to(std::back_inserter(buf), "flags = {{{}}}\n", fmt::join(stringifiedFlags, ", "));

    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "name = {}<{}>\n", name.showRaw(gs),
                   this->symbol.data(gs)->name.showRaw(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "args = [");
    bool first = true;
    if (this->symbol == core::Symbols::todoMethod()) {
        for (auto &a : this->args) {
            if (!first) {
                fmt::format_to(std::back_inserter(buf), ", ");
            }
            first = false;
            fmt::format_to(std::back_inserter(buf), "{}", a.showRaw(gs, tabs + 2));
        }
    } else {
        for (auto &a : this->args) {
            if (!first) {
                fmt::format_to(std::back_inserter(buf), ", ");
            }
            first = false;
            fmt::format_to(std::back_inserter(buf), "{}", a.showRaw(gs, tabs + 2));
        }
    }
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "rhs = {}\n", this->rhs.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string If::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "if {}\n", this->cond.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "{}\n", this->thenp.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "else\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "{}\n", this->elsep.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "end");
    return fmt::to_string(buf);
}

string If::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "cond = {}\n", this->cond.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "thenp = {}\n", this->thenp.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "elsep = {}\n", this->elsep.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string Assign::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "lhs = {}\n", this->lhs.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "rhs = {}\n", this->rhs.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string While::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "while {}\n", this->cond.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "{}\n", this->body.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "end");
    return fmt::to_string(buf);
}

string While::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "cond = {}\n", this->cond.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "body = {}\n", this->body.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string EmptyTree::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "<emptyTree>";
}

string UnresolvedConstantLit::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return fmt::format("{}::{}", this->scope.toStringWithTabs(gs, tabs), this->cnst.toString(gs));
}

string UnresolvedConstantLit::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "cnst = {}\n", this->cnst.showRaw(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "scope = {}\n", this->scope.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string ConstantLit::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    if (symbol.exists() && symbol != core::Symbols::StubModule()) {
        return this->symbol.showFullName(gs);
    }
    return "Unresolved: " + this->original.toStringWithTabs(gs, tabs);
}

string ConstantLit::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "symbol = ({} {})\n", this->symbol.showKind(gs),
                   this->symbol.showFullName(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "orig = {}\n",
                   this->original ? this->original.showRaw(gs, tabs + 1) : "nullptr");
    // If resolutionScopes isn't null, it should not be empty.
    ENFORCE(resolutionScopes == nullptr || !resolutionScopes->empty());
    if (resolutionScopes != nullptr && !resolutionScopes->empty()) {
        printTabs(buf, tabs + 1);
        fmt::format_to(std::back_inserter(buf), "resolutionScopes = [{}]\n",
                       fmt::map_join(this->resolutionScopes->begin(), this->resolutionScopes->end(), ", ",
                                     [&](auto sym) { return sym.showFullName(gs); }));
    }
    printTabs(buf, tabs);

    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string Local::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->localVariable.toString(gs);
}

string Local::nodeName() {
    return "Local";
}

string Local::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "localVariable = {}\n", this->localVariable.showRaw(gs));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string UnresolvedIdent::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->name.toString(gs);
}

string UnresolvedIdent::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "kind = ");
    switch (this->kind) {
        case Kind::Local:
            fmt::format_to(std::back_inserter(buf), "Local");
            break;
        case Kind::Instance:
            fmt::format_to(std::back_inserter(buf), "Instance");
            break;
        case Kind::Class:
            fmt::format_to(std::back_inserter(buf), "Class");
            break;
        case Kind::Global:
            fmt::format_to(std::back_inserter(buf), "Global");
            break;
    }
    fmt::format_to(std::back_inserter(buf), "{}", '\n');
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "name = {}\n", this->name.showRaw(gs));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");

    return fmt::to_string(buf);
}

string Return::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + this->expr.showRaw(gs, tabs + 1) + " }";
}

string Next::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + this->expr.showRaw(gs, tabs + 1) + " }";
}

string Break::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + this->expr.showRaw(gs, tabs + 1) + " }";
}

string Retry::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{}";
}

string Return::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "return " + this->expr.toStringWithTabs(gs, tabs + 1);
}

string Next::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "next(" + this->expr.toStringWithTabs(gs, tabs + 1) + ")";
}

string Break::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "break(" + this->expr.toStringWithTabs(gs, tabs + 1) + ")";
}

string Retry::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "retry";
}

string Literal::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ value = " + this->toStringWithTabs(gs, 0) + " }";
}

string Literal::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    string res;
    typecase(
        this->value, [&](const core::LiteralType &l) { res = l.showValue(gs); },
        [&](const core::ClassType &l) {
            if (l.symbol == core::Symbols::NilClass()) {
                res = "nil";
            } else if (l.symbol == core::Symbols::FalseClass()) {
                res = "false";
            } else if (l.symbol == core::Symbols::TrueClass()) {
                res = "true";
            } else {
                res = "literal(" + this->value.toStringWithTabs(gs, tabs) + ")";
            }
        },
        [&](const core::TypePtr &t) { res = "literal(" + this->value.toStringWithTabs(gs, tabs) + ")"; });
    return res;
}

string Assign::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->lhs.toStringWithTabs(gs, tabs) + " = " + this->rhs.toStringWithTabs(gs, tabs);
}

string RescueCase::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "rescue");
    bool first = true;
    for (auto &exception : this->exceptions) {
        if (first) {
            first = false;
            fmt::format_to(std::back_inserter(buf), " ");
        } else {
            fmt::format_to(std::back_inserter(buf), ", ");
        }
        fmt::format_to(std::back_inserter(buf), "{}", exception.toStringWithTabs(gs, tabs));
    }
    fmt::format_to(std::back_inserter(buf), " => {}\n", this->var.toStringWithTabs(gs, tabs));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "{}", this->body.toStringWithTabs(gs, tabs));
    return fmt::to_string(buf);
}

string RescueCase::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "exceptions = [\n");
    for (auto &a : exceptions) {
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.showRaw(gs, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "var = {}\n", this->var.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "body = {}\n", this->body.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string Rescue::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}", this->body.toStringWithTabs(gs, tabs));
    for (auto &rescueCase : this->rescueCases) {
        fmt::format_to(std::back_inserter(buf), "\n");
        printTabs(buf, tabs - 1);
        fmt::format_to(std::back_inserter(buf), "{}", rescueCase.toStringWithTabs(gs, tabs));
    }
    if (!isa_tree<EmptyTree>(this->else_)) {
        fmt::format_to(std::back_inserter(buf), "\n");
        printTabs(buf, tabs - 1);
        fmt::format_to(std::back_inserter(buf), "else\n");
        printTabs(buf, tabs);
        fmt::format_to(std::back_inserter(buf), "{}", this->else_.toStringWithTabs(gs, tabs));
    }
    if (!isa_tree<EmptyTree>(this->ensure)) {
        fmt::format_to(std::back_inserter(buf), "\n");
        printTabs(buf, tabs - 1);
        fmt::format_to(std::back_inserter(buf), "ensure\n");
        printTabs(buf, tabs);
        fmt::format_to(std::back_inserter(buf), "{}", this->ensure.toStringWithTabs(gs, tabs));
    }
    return fmt::to_string(buf);
}

string Rescue::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "body = {}\n", this->body.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "rescueCases = [\n");
    for (auto &a : rescueCases) {
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.showRaw(gs, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "else = {}\n", this->else_.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "ensure = {}\n", this->ensure.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string Send::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}.{}", this->recv.toStringWithTabs(gs, tabs), this->fun.toString(gs));
    printArgs(gs, buf, this->args, tabs);
    if (this->hasBlock()) {
        fmt::format_to(std::back_inserter(buf), "{}", this->block()->toStringWithTabs(gs, tabs));
    }

    return fmt::to_string(buf);
}

string Send::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());

    vector<string> stringifiedFlags;
    if (this->flags.isPrivateOk) {
        stringifiedFlags.emplace_back("privateOk");
    }
    if (this->flags.isRewriterSynthesized) {
        stringifiedFlags.emplace_back("rewriterSynthesized");
    }

    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "flags = {{{}}}\n", fmt::join(stringifiedFlags, ", "));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "recv = {}\n", this->recv.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "fun = {}\n", this->fun.showRaw(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "block = ");
    if (this->hasBlock()) {
        fmt::format_to(std::back_inserter(buf), "{}\n", this->block()->showRaw(gs, tabs + 1));
    } else {
        fmt::format_to(std::back_inserter(buf), "nullptr\n");
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "pos_args = {}\n", this->numPosArgs_);
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "args = [\n");
    for (auto &a : args) {
        if (this->hasBlock() && a == args.back()) {
            continue;
        }
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.showRaw(gs, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");

    return fmt::to_string(buf);
}

const ast::Block *Send::block() const {
    if (hasBlock()) {
        auto block = ast::cast_tree<ast::Block>(this->args.back());
        ENFORCE(block);
        return block;
    } else {
        return nullptr;
    }
}

ast::Block *Send::block() {
    if (hasBlock()) {
        auto block = ast::cast_tree<ast::Block>(this->args.back());
        ENFORCE(block);
        return block;
    } else {
        return nullptr;
    }
}

const ExpressionPtr *Send::rawBlock() const {
    if (hasBlock()) {
        return &this->args.back();
    }
    return nullptr;
}

ExpressionPtr *Send::rawBlock() {
    if (hasBlock()) {
        return &this->args.back();
    }
    return nullptr;
}

const ExpressionPtr *Send::kwSplat() const {
    if (hasKwSplat()) {
        auto index = this->args.size() - 1;
        if (hasBlock()) {
            index = index - 1;
        }
        return &this->args[index];
    }
    return nullptr;
}

core::LocOffsets Send::argsLoc() const {
    if (!this->hasPosArgs() && !this->hasKwArgs()) {
        return core::LocOffsets();
    }
    auto begin = this->args.begin();
    auto end = this->args.end() - 1;
    if (this->hasBlock()) {
        end = end - 1;
    }
    return begin->loc().join(end->loc());
}

ExpressionPtr *Send::kwSplat() {
    if (hasKwSplat()) {
        auto index = this->args.size() - 1;
        if (hasBlock()) {
            index = index - 1;
        }
        return &this->args[index];
    }
    return nullptr;
}

void Send::clearArgs() {
    this->args.clear();
    this->flags.hasBlock = false;
    this->numPosArgs_ = 0;
}

void Send::addPosArg(ExpressionPtr ptr) {
    this->args.emplace(this->args.begin() + numPosArgs_, move(ptr));
    this->numPosArgs_++;
}

void Send::insertPosArg(uint16_t index, ExpressionPtr arg) {
    ENFORCE(index <= numPosArgs_);
    this->args.emplace(this->args.begin() + index, std::move(arg));
    this->numPosArgs_++;
}

void Send::removePosArg(uint16_t index) {
    ENFORCE(index < numPosArgs_);
    this->args.erase(this->args.begin() + index);
    this->numPosArgs_--;
}

void Send::setBlock(ExpressionPtr block) {
    if (hasBlock()) {
        this->args.pop_back();
        flags.hasBlock = false;
    }

    if (block != nullptr) {
        this->args.emplace_back(move(block));
        flags.hasBlock = true;
        ENFORCE(this->block() != nullptr);
    }
}

void Send::setKwSplat(ExpressionPtr splat) {
    this->args.emplace(this->args.begin() + numPosArgs_, move(splat));
}

void Send::addKwArg(ExpressionPtr key, ExpressionPtr value) {
    auto it = this->args.emplace(this->args.end() - (hasBlock() ? 1 : 0) - (hasKwSplat() ? 1 : 0), move(key));
    this->args.emplace(it + 1, move(value));
}

ExpressionPtr Send::withNewBody(core::LocOffsets loc, ExpressionPtr recv, core::NameRef fun) {
    auto rv = make_expression<Send>(loc, move(recv), fun, funLoc, numPosArgs_, std::move(args), flags);

    // Reset important metadata on this function.
    this->numPosArgs_ = 0;
    this->flags.hasBlock = false;

    return rv;
}

string Cast::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "T.{}", this->cast.toString(gs));
    fmt::format_to(std::back_inserter(buf), "({}, {})", this->arg.toStringWithTabs(gs, tabs),
                   this->type.toStringWithTabs(gs, tabs));

    return fmt::to_string(buf);
}

string Cast::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 2);
    fmt::format_to(std::back_inserter(buf), "cast = {},\n", this->cast.showRaw(gs));
    printTabs(buf, tabs + 2);
    fmt::format_to(std::back_inserter(buf), "arg = {}\n", this->arg.showRaw(gs, tabs + 2));
    printTabs(buf, tabs + 2);
    fmt::format_to(std::back_inserter(buf), "type = {},\n", this->type.toString(gs));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}\n");

    return fmt::to_string(buf);
}

string ZSuperArgs::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ }";
}

string Hash::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "pairs = [\n");
    int i = -1;
    for (auto &key : keys) {
        i++;
        auto &value = values[i];

        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "[\n");
        printTabs(buf, tabs + 3);
        fmt::format_to(std::back_inserter(buf), "key = {}\n", key.showRaw(gs, tabs + 3));
        printTabs(buf, tabs + 3);
        fmt::format_to(std::back_inserter(buf), "value = {}\n", value.showRaw(gs, tabs + 3));
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "]\n");
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");

    return fmt::to_string(buf);
}

string Array::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "elems = [\n");
    for (auto &a : elems) {
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.showRaw(gs, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");

    return fmt::to_string(buf);
}

string ZSuperArgs::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "ZSuperArgs";
}

string Hash::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{{");
    bool first = true;
    int i = -1;
    for (auto &key : this->keys) {
        i++;
        auto &value = this->values[i];
        if (!first) {
            fmt::format_to(std::back_inserter(buf), ", ");
        }
        first = false;
        fmt::format_to(std::back_inserter(buf), "{}", key.toStringWithTabs(gs, tabs + 1));
        fmt::format_to(std::back_inserter(buf), " => ");
        fmt::format_to(std::back_inserter(buf), "{}", value.toStringWithTabs(gs, tabs + 1));
    }
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string Array::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "[");
    printElems(gs, buf, this->elems, tabs);
    fmt::format_to(std::back_inserter(buf), "]");
    return fmt::to_string(buf);
}

string Block::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), " do |");
    printElems(gs, buf, this->args, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "|\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "{}\n", this->body.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "end");
    return fmt::to_string(buf);
}

string Block::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{} {{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "args = [\n");
    for (auto &a : this->args) {
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.showRaw(gs, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "body = {}\n", this->body.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string RestArg::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "*" + this->expr.toStringWithTabs(gs, tabs);
}

string KeywordArg::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->expr.toStringWithTabs(gs, tabs) + ":";
}

string OptionalArg::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}", this->expr.toStringWithTabs(gs, tabs));
    if (this->default_) {
        fmt::format_to(std::back_inserter(buf), " = {}", this->default_.toStringWithTabs(gs, tabs));
    }
    return fmt::to_string(buf);
}

string ShadowArg::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->expr.toStringWithTabs(gs, tabs);
}

string BlockArg::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "&" + this->expr.toStringWithTabs(gs, tabs);
}

string RescueCase::nodeName() {
    return "RescueCase";
}
string Rescue::nodeName() {
    return "Rescue";
}
string Next::nodeName() {
    return "Next";
}
string ClassDef::nodeName() {
    return "ClassDef";
}

string MethodDef::nodeName() {
    return "MethodDef";
}
string If::nodeName() {
    return "If";
}
string While::nodeName() {
    return "While";
}
string UnresolvedIdent::nodeName() {
    return "UnresolvedIdent";
}
string Return::nodeName() {
    return "Return";
}
string Break::nodeName() {
    return "Break";
}
string Retry::nodeName() {
    return "Retry";
}

string Assign::nodeName() {
    return "Assign";
}

string Send::nodeName() {
    return "Send";
}

string Cast::nodeName() {
    return "Cast";
}

string ZSuperArgs::nodeName() {
    return "ZSuperArgs";
}

string Hash::nodeName() {
    return "Hash";
}

string Array::nodeName() {
    return "Array";
}

string Literal::nodeName() {
    return "Literal";
}

core::NameRef Literal::asString(const core::GlobalState &gs) const {
    ENFORCE(isString(gs));
    auto t = core::cast_type_nonnull<core::LiteralType>(value);
    core::NameRef res = t.asName(gs);
    return res;
}

core::NameRef Literal::asSymbol(const core::GlobalState &gs) const {
    ENFORCE(isSymbol(gs));
    auto t = core::cast_type_nonnull<core::LiteralType>(value);
    core::NameRef res = t.asName(gs);
    return res;
}

bool Literal::isSymbol(const core::GlobalState &gs) const {
    return core::isa_type<core::LiteralType>(value) &&
           core::cast_type_nonnull<core::LiteralType>(value).derivesFrom(gs, core::Symbols::Symbol());
}

bool Literal::isNil(const core::GlobalState &gs) const {
    return value.derivesFrom(gs, core::Symbols::NilClass());
}

bool Literal::isString(const core::GlobalState &gs) const {
    return core::isa_type<core::LiteralType>(value) &&
           core::cast_type_nonnull<core::LiteralType>(value).derivesFrom(gs, core::Symbols::String());
}

bool Literal::isTrue(const core::GlobalState &gs) const {
    return value.derivesFrom(gs, core::Symbols::TrueClass());
}

bool Literal::isFalse(const core::GlobalState &gs) const {
    return value.derivesFrom(gs, core::Symbols::FalseClass());
}

string UnresolvedConstantLit::nodeName() {
    return "UnresolvedConstantLit";
}

string ConstantLit::nodeName() {
    return "ConstantLit";
}

string Block::nodeName() {
    return "Block";
}

string InsSeq::nodeName() {
    return "InsSeq";
}

string EmptyTree::nodeName() {
    return "EmptyTree";
}

string EmptyTree::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName();
}

string RestArg::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr.showRaw(gs, tabs) + " }";
}

string RestArg::nodeName() {
    return "RestArg";
}

string KeywordArg::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr.showRaw(gs, tabs) + " }";
}

string KeywordArg::nodeName() {
    return "KeywordArg";
}

string OptionalArg::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "expr = {}\n", expr.showRaw(gs, tabs + 1));
    if (default_) {
        printTabs(buf, tabs + 1);
        fmt::format_to(std::back_inserter(buf), "default_ = {}\n", default_.showRaw(gs, tabs + 1));
    }
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");

    return fmt::to_string(buf);
}

string OptionalArg::nodeName() {
    return "OptionalArg";
}

string ShadowArg::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr.showRaw(gs, tabs) + " }";
}

string BlockArg::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr.showRaw(gs, tabs) + " }";
}

string ShadowArg::nodeName() {
    return "ShadowArg";
}

string BlockArg::nodeName() {
    return "BlockArg";
}

ParsedFilesOrCancelled::ParsedFilesOrCancelled() : trees(nullopt){};
ParsedFilesOrCancelled::ParsedFilesOrCancelled(std::vector<ParsedFile> &&trees) : trees(move(trees)) {}

ParsedFilesOrCancelled ParsedFilesOrCancelled::cancel(std::vector<ParsedFile> &&trees, WorkerPool &workers) {
    if (!trees.empty()) {
        // N.B.: `workers.size()` can be `0` when threads are disabled, which would result in undefined behavior for
        // `BlockingCounter`.
        absl::BlockingCounter threadBarrier(std::max(workers.size(), 1));
        auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
        for (auto &tree : trees) {
            fileq->push(move(tree), 1);
        }

        workers.multiplexJob("deleteTrees", [fileq, &threadBarrier]() {
            {
                ast::ParsedFile job;
                for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                    // Do nothing; allow the destructor of `ast::ParsedFile` to run for `job`.
                }
            }
            threadBarrier.DecrementCount();
        });

        // Wait for threads to complete destructing the trees.
        threadBarrier.Wait();
    }

    return ParsedFilesOrCancelled();
}

bool ParsedFilesOrCancelled::hasResult() const {
    return trees.has_value();
}

vector<ParsedFile> &ParsedFilesOrCancelled::result() {
    if (trees.has_value()) {
        return trees.value();
    }
    Exception::raise("Attempted to retrieve result of an AST pass that did not complete.");
}

} // namespace sorbet::ast
