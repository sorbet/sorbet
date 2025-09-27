#include "ast/Trees.h"
#include "absl/strings/escaping.h"
#include "common/concurrency/Parallel.h"
#include "common/strings/formatting.h"
#include "common/typecase.h"
#include "core/Symbols.h"
#include <utility>

using namespace std;

namespace sorbet::ast {

#define CASE_STATEMENT(CASE_BODY, T) \
    case Tag::T: {                   \
        CASE_BODY(T)                 \
        break;                       \
    }

#define GENERATE_TAG_SWITCH(tag, CASE_BODY)                \
    switch (tag) {                                         \
        CASE_STATEMENT(CASE_BODY, EmptyTree)               \
        CASE_STATEMENT(CASE_BODY, Send)                    \
        CASE_STATEMENT(CASE_BODY, ClassDef)                \
        CASE_STATEMENT(CASE_BODY, MethodDef)               \
        CASE_STATEMENT(CASE_BODY, If)                      \
        CASE_STATEMENT(CASE_BODY, While)                   \
        CASE_STATEMENT(CASE_BODY, Break)                   \
        CASE_STATEMENT(CASE_BODY, Retry)                   \
        CASE_STATEMENT(CASE_BODY, Next)                    \
        CASE_STATEMENT(CASE_BODY, Return)                  \
        CASE_STATEMENT(CASE_BODY, RescueCase)              \
        CASE_STATEMENT(CASE_BODY, Rescue)                  \
        CASE_STATEMENT(CASE_BODY, Local)                   \
        CASE_STATEMENT(CASE_BODY, UnresolvedIdent)         \
        CASE_STATEMENT(CASE_BODY, RestParam)               \
        CASE_STATEMENT(CASE_BODY, KeywordArg)              \
        CASE_STATEMENT(CASE_BODY, OptionalParam)           \
        CASE_STATEMENT(CASE_BODY, BlockArg)                \
        CASE_STATEMENT(CASE_BODY, ShadowArg)               \
        CASE_STATEMENT(CASE_BODY, Assign)                  \
        CASE_STATEMENT(CASE_BODY, Cast)                    \
        CASE_STATEMENT(CASE_BODY, Hash)                    \
        CASE_STATEMENT(CASE_BODY, Array)                   \
        CASE_STATEMENT(CASE_BODY, Literal)                 \
        CASE_STATEMENT(CASE_BODY, UnresolvedConstantLit)   \
        CASE_STATEMENT(CASE_BODY, ConstantLit)             \
        CASE_STATEMENT(CASE_BODY, ZSuperArgs)              \
        CASE_STATEMENT(CASE_BODY, Block)                   \
        CASE_STATEMENT(CASE_BODY, InsSeq)                  \
        CASE_STATEMENT(CASE_BODY, RuntimeMethodDefinition) \
        CASE_STATEMENT(CASE_BODY, Self)                    \
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

string_view ExpressionPtr::nodeName() const {
    auto *ptr = get();

    ENFORCE(ptr != nullptr);

#define NODE_NAME(name) return reinterpret_cast<name *>(ptr)->nodeName();
    GENERATE_TAG_SWITCH(tag(), NODE_NAME)
#undef NODE_NAME
}

string ExpressionPtr::showRaw(const core::GlobalState &gs, int tabs) const {
    auto *ptr = get();

    ENFORCE(ptr != nullptr);

#define SHOW_RAW(name) return reinterpret_cast<name *>(ptr)->showRaw(gs, tabs);
    GENERATE_TAG_SWITCH(tag(), SHOW_RAW)
#undef SHOW_RAW
}

string ExpressionPtr::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    auto *ptr = get();

    ENFORCE(ptr != nullptr);

#define SHOW_RAW_WITH_LOCS(name) return reinterpret_cast<name *>(ptr)->showRawWithLocs(gs, file, tabs);
    GENERATE_TAG_SWITCH(tag(), SHOW_RAW_WITH_LOCS)
#undef SHOW_RAW_WITH_LOCS
}

namespace {
template <typename T> struct LocGetter {
    static core::LocOffsets loc(void *ptr) {
        return reinterpret_cast<T *>(ptr)->loc;
    }
};

template <> struct LocGetter<ConstantLit> {
    static core::LocOffsets loc(void *ptr) {
        return reinterpret_cast<ConstantLit *>(ptr)->loc();
    }
};
} // namespace

core::LocOffsets ExpressionPtr::loc() const {
    auto *ptr = get();

    ENFORCE(ptr != nullptr);

#define CASE(name) return LocGetter<name>::loc(ptr);
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
    return isa_tree<Self>(*this);
}

void ExpressionPtr::resetToEmpty(EmptyTree *expr) noexcept {
    ENFORCE(expr != nullptr);
    resetTagged(tagPtr(ExpressionToTag<EmptyTree>::value, expr));
}

bool isa_reference(const ExpressionPtr &what) {
    return isa_tree<Local>(what) || isa_tree<UnresolvedIdent>(what) || isa_tree<RestParam>(what) ||
           isa_tree<KeywordArg>(what) || isa_tree<OptionalParam>(what) || isa_tree<BlockArg>(what) ||
           isa_tree<ShadowArg>(what) || isa_tree<Self>(what);
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
    histogramInc("trees.classdef.rhs", this->rhs.size());
    _sanityCheck();
}

MethodDef::MethodDef(core::LocOffsets loc, core::LocOffsets declLoc, core::MethodRef symbol, core::NameRef name,
                     PARAMS_store params, ExpressionPtr rhs, Flags flags)
    : loc(loc), declLoc(declLoc), symbol(symbol), rhs(std::move(rhs)), params(std::move(params)), name(name),
      flags(flags) {
    categoryCounterInc("trees", "methoddef");
    histogramInc("trees.methodDef.params", this->params.size());
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
    ENFORCE(localVariable != core::LocalVariable::selfVariable(), "use a Self node");
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

Cast::Cast(core::LocOffsets loc, core::TypePtr ty, ExpressionPtr arg, core::NameRef cast, ExpressionPtr typeExpr)
    : loc(loc), cast(cast), type(std::move(ty)), arg(std::move(arg)), typeExpr(std::move(typeExpr)) {
    categoryCounterInc("trees", "cast");
    _sanityCheck();
}

ZSuperArgs::ZSuperArgs(core::LocOffsets loc) : loc(loc) {
    categoryCounterInc("trees", "zsuper");
    _sanityCheck();
}

RestParam::RestParam(core::LocOffsets loc, ExpressionPtr arg) : loc(loc), expr(std::move(arg)) {
    categoryCounterInc("trees", "restparam");
    _sanityCheck();
}

KeywordArg::KeywordArg(core::LocOffsets loc, ExpressionPtr expr) : loc(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "keywordarg");
    _sanityCheck();
}

OptionalParam::OptionalParam(core::LocOffsets loc, ExpressionPtr expr, ExpressionPtr default_)
    : loc(loc), expr(std::move(expr)), default_(std::move(default_)) {
    categoryCounterInc("trees", "optionalparam");
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

ConstantLit::ConstantLit(core::LocOffsets loc, core::SymbolRef symbol) : storage(loc, symbol) {
    categoryCounterInc("trees", "resolvedconstantlit");
    _sanityCheck();
}

ConstantLit::ConstantLit(core::SymbolRef symbol, unique_ptr<UnresolvedConstantLit> original)
    : storage(symbol, std::move(original)) {
    categoryCounterInc("trees", "resolvedconstantlit");
    _sanityCheck();
}

optional<pair<core::SymbolRef, vector<core::NameRef>>> ConstantLit::fullUnresolvedPath(const core::Context ctx) const {
    if (this->symbol() != core::Symbols::StubModule()) {
        return nullopt;
    }
    ENFORCE(this->resolutionScopes() != nullptr && !this->resolutionScopes()->empty());

    vector<core::NameRef> namesFailedToResolve;
    auto *nested = this;
    {
        while (true) {
            if (nested->resolutionScopes() == nullptr || nested->resolutionScopes()->empty()) [[unlikely]] {
                ENFORCE(false);
                bool hasScopes = nested->resolutionScopes() != nullptr;
                fatalLogger->error(R"(msg="Bad fullUnresolvedPath" loc="{}" hasScopes={})",
                                   ctx.locAt(this->loc()).showRaw(ctx), hasScopes);
                fatalLogger->error("source=\"{}\"", absl::CEscape(ctx.file.data(ctx).source()));
            }

            if (nested->resolutionScopes()->front().exists()) {
                break;
            }

            auto &orig = *nested->original();
            namesFailedToResolve.emplace_back(orig.cnst);
            nested = ast::cast_tree<ast::ConstantLit>(orig.scope);
            ENFORCE(nested);
            ENFORCE(nested->symbol() == core::Symbols::StubModule());
            ENFORCE(!nested->resolutionScopes()->empty());
        }
        auto &orig = *nested->original();
        namesFailedToResolve.emplace_back(orig.cnst);
        absl::c_reverse(namesFailedToResolve);
    }
    auto prefix = nested->resolutionScopes()->front();
    return make_pair(prefix, move(namesFailedToResolve));
}

Block::Block(core::LocOffsets loc, MethodDef::PARAMS_store params, ExpressionPtr body)
    : loc(loc), params(std::move(params)), body(std::move(body)) {
    categoryCounterInc("trees", "block");
    histogramInc("trees.block.params", this->params.size());
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

RuntimeMethodDefinition::RuntimeMethodDefinition(core::LocOffsets loc, core::NameRef name, bool isSelfMethod)
    : loc(loc), name(name), isSelfMethod(isSelfMethod) {
    categoryCounterInc("trees", "runtimemethoddefinition");
    _sanityCheck();
}

Self::Self(core::LocOffsets loc) : loc(loc) {
    categoryCounterInc("trees", "self");
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
    result.resetToEmpty(&singletonEmptyTree);
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

void printElems(const core::GlobalState &gs, fmt::memory_buffer &buf, absl::Span<const ExpressionPtr> args, int tabs) {
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

void printArgs(const core::GlobalState &gs, fmt::memory_buffer &buf, absl::Span<const ExpressionPtr> args, int tabs) {
    fmt::format_to(std::back_inserter(buf), "(");
    printElems(gs, buf, args, tabs);
    fmt::format_to(std::back_inserter(buf), ")");
}

} // namespace

core::FoundClass::Kind ClassDef::kindToFoundClassKind(Kind kind) {
    switch (kind) {
        case Kind::Module:
            return core::FoundClass::Kind::Module;
        case Kind::Class:
            return core::FoundClass::Kind::Class;
    }
}

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

string ClassDef::showRaw(const core::GlobalState &gs, int tabs) const {
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

string InsSeq::showRaw(const core::GlobalState &gs, int tabs) const {
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
        for (auto &p : this->params) {
            if (!first) {
                fmt::format_to(std::back_inserter(buf), ", ");
            }
            first = false;
            fmt::format_to(std::back_inserter(buf), "{}", p.toStringWithTabs(gs, tabs + 1));
        }
    } else {
        for (auto &p : data->parameters) {
            if (!first) {
                fmt::format_to(std::back_inserter(buf), ", ");
            }
            first = false;
            fmt::format_to(std::back_inserter(buf), "{}", p.parameterName(gs));
        }
    }
    fmt::format_to(std::back_inserter(buf), ")\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "{}\n", this->rhs.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "end");
    return fmt::to_string(buf);
}

string MethodDef::showRaw(const core::GlobalState &gs, int tabs) const {
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
    fmt::format_to(std::back_inserter(buf), "params = [");
    bool first = true;
    if (this->symbol == core::Symbols::todoMethod()) {
        for (auto &p : this->params) {
            if (!first) {
                fmt::format_to(std::back_inserter(buf), ", ");
            }
            first = false;
            fmt::format_to(std::back_inserter(buf), "{}", p.showRaw(gs, tabs + 2));
        }
    } else {
        for (auto &p : this->params) {
            if (!first) {
                fmt::format_to(std::back_inserter(buf), ", ");
            }
            first = false;
            fmt::format_to(std::back_inserter(buf), "{}", p.showRaw(gs, tabs + 2));
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

string If::showRaw(const core::GlobalState &gs, int tabs) const {
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

string Assign::showRaw(const core::GlobalState &gs, int tabs) const {
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

string While::showRaw(const core::GlobalState &gs, int tabs) const {
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

string UnresolvedConstantLit::showRaw(const core::GlobalState &gs, int tabs) const {
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
    if (symbol().exists() && symbol() != core::Symbols::StubModule()) {
        return this->symbol().showFullName(gs);
    }
    return "Unresolved: " + this->original()->toStringWithTabs(gs, tabs);
}

string ConstantLit::showRaw(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "symbol = ({} {})\n", this->symbol().showKind(gs),
                   this->symbol().showFullName(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "orig = {}\n",
                   this->original() ? this->original()->showRaw(gs, tabs + 1) : "nullptr");
    // If resolutionScopes isn't null, it should not be empty.
    ENFORCE(resolutionScopes() == nullptr || !resolutionScopes()->empty());
    if (resolutionScopes() != nullptr && !resolutionScopes()->empty()) {
        printTabs(buf, tabs + 1);
        fmt::format_to(std::back_inserter(buf), "resolutionScopes = [{}]\n",
                       fmt::map_join(*this->resolutionScopes(), ", ", [&](auto sym) { return sym.showFullName(gs); }));
    }
    printTabs(buf, tabs);

    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string Local::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->localVariable.toString(gs);
}

string_view Local::nodeName() const {
    return "Local";
}

string Local::showRaw(const core::GlobalState &gs, int tabs) const {
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

string UnresolvedIdent::showRaw(const core::GlobalState &gs, int tabs) const {
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

string Return::showRaw(const core::GlobalState &gs, int tabs) const {
    return fmt::format("{}{{ expr = {} }}", nodeName(), this->expr.showRaw(gs, tabs + 1));
}

string Next::showRaw(const core::GlobalState &gs, int tabs) const {
    return fmt::format("{}{{ expr = {} }}", nodeName(), this->expr.showRaw(gs, tabs + 1));
}

string Break::showRaw(const core::GlobalState &gs, int tabs) const {
    return fmt::format("{}{{ expr = {} }}", nodeName(), this->expr.showRaw(gs, tabs + 1));
}

string Retry::showRaw(const core::GlobalState &gs, int tabs) const {
    return fmt::format("{}{{}}", nodeName());
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

string Literal::showRaw(const core::GlobalState &gs, int tabs) const {
    return fmt::format("{}{{ value = {} }}", nodeName(), this->toStringWithTabs(gs, 0));
}

string Literal::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    string res;
    typecase(
        this->value, [&](const core::NamedLiteralType &l) { res = l.showValue(gs); },
        [&](const core::IntegerLiteralType &i) { res = i.showValue(gs); },
        [&](const core::FloatLiteralType &i) { res = i.showValue(gs); },
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

string RescueCase::showRaw(const core::GlobalState &gs, int tabs) const {
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

string Rescue::showRaw(const core::GlobalState &gs, int tabs) const {
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

string Send::showRaw(const core::GlobalState &gs, int tabs) const {
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

string RuntimeMethodDefinition::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    string prefix = this->isSelfMethod ? "self." : "";
    return fmt::format("<runtime method definition of {}{}>", prefix, this->name.toString(gs));
}

string RuntimeMethodDefinition::showRaw(const core::GlobalState &gs, int tabs) const {
    return this->toStringWithTabs(gs, tabs);
}

string Self::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "<self>";
}

string Self::showRaw(const core::GlobalState &gs, int tabs) const {
    return "Self";
}

string Self::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    return fmt::format("{}{{ loc = {} }}", nodeName(), core::Loc(file, this->loc).fileShortPosToString(gs));
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

ExpressionPtr Send::withNewBody(core::LocOffsets loc, ExpressionPtr recv, core::NameRef fun) {
    auto rv = make_expression<Send>(loc, move(recv), fun, funLoc, numPosArgs_, std::move(args), flags);

    // Reset important metadata on this function.
    this->numPosArgs_ = 0;
    this->flags.hasBlock = false;

    return rv;
}

string Cast::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "<cast:{}>", this->cast.toString(gs));
    fmt::format_to(std::back_inserter(buf), "({}, {}, {})", this->arg.toStringWithTabs(gs, tabs),
                   this->type.toStringWithTabs(gs, tabs), this->typeExpr.toStringWithTabs(gs, tabs));

    return fmt::to_string(buf);
}

string Cast::showRaw(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 2);
    fmt::format_to(std::back_inserter(buf), "cast = {},\n", this->cast.showRaw(gs));
    printTabs(buf, tabs + 2);
    fmt::format_to(std::back_inserter(buf), "arg = {}\n", this->arg.showRaw(gs, tabs + 2));
    printTabs(buf, tabs + 2);
    fmt::format_to(std::back_inserter(buf), "type = {},\n", this->type.showWithMoreInfo(gs));
    printTabs(buf, tabs + 2);
    fmt::format_to(std::back_inserter(buf), "typeExpr = {},\n", this->typeExpr.showRaw(gs, tabs + 2));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}\n");

    return fmt::to_string(buf);
}

string ZSuperArgs::showRaw(const core::GlobalState &gs, int tabs) const {
    return fmt::format("{}{{ }}", nodeName());
}

string Hash::showRaw(const core::GlobalState &gs, int tabs) const {
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

string Array::showRaw(const core::GlobalState &gs, int tabs) const {
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
    printElems(gs, buf, this->params, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "|\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "{}\n", this->body.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "end");
    return fmt::to_string(buf);
}

string Block::showRaw(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{} {{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "params = [\n");
    for (auto &p : this->params) {
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", p.showRaw(gs, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "body = {}\n", this->body.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string RestParam::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "*" + this->expr.toStringWithTabs(gs, tabs);
}

string KeywordArg::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->expr.toStringWithTabs(gs, tabs) + ":";
}

string OptionalParam::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
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

string_view RescueCase::nodeName() const {
    return "RescueCase";
}
string_view Rescue::nodeName() const {
    return "Rescue";
}
string_view Next::nodeName() const {
    return "Next";
}
string_view ClassDef::nodeName() const {
    return "ClassDef";
}

string_view MethodDef::nodeName() const {
    return "MethodDef";
}
string_view If::nodeName() const {
    return "If";
}
string_view While::nodeName() const {
    return "While";
}
string_view UnresolvedIdent::nodeName() const {
    return "UnresolvedIdent";
}
string_view Return::nodeName() const {
    return "Return";
}
string_view Break::nodeName() const {
    return "Break";
}
string_view Retry::nodeName() const {
    return "Retry";
}

string_view Assign::nodeName() const {
    return "Assign";
}

string_view Send::nodeName() const {
    return "Send";
}

string_view Cast::nodeName() const {
    return "Cast";
}

string_view ZSuperArgs::nodeName() const {
    return "ZSuperArgs";
}

string_view Hash::nodeName() const {
    return "Hash";
}

string_view Array::nodeName() const {
    return "Array";
}

string_view Literal::nodeName() const {
    return "Literal";
}

core::NameRef Literal::asString() const {
    ENFORCE(isString());
    return asName();
}

core::NameRef Literal::asSymbol() const {
    ENFORCE(isSymbol());
    return asName();
}

core::NameRef Literal::asName() const {
    ENFORCE(isName());
    auto t = core::cast_type_nonnull<core::NamedLiteralType>(value);
    return t.name;
}

bool Literal::isSymbol() const {
    return core::isa_type<core::NamedLiteralType>(value) &&
           core::cast_type_nonnull<core::NamedLiteralType>(value).kind == core::NamedLiteralType::Kind::Symbol;
}

bool Literal::isNil(const core::GlobalState &gs) const {
    return value.derivesFrom(gs, core::Symbols::NilClass());
}

bool Literal::isString() const {
    return core::isa_type<core::NamedLiteralType>(value) &&
           core::cast_type_nonnull<core::NamedLiteralType>(value).kind == core::NamedLiteralType::Kind::String;
}

bool Literal::isName() const {
    return isString() || isSymbol();
}

bool Literal::isTrue(const core::GlobalState &gs) const {
    return value.derivesFrom(gs, core::Symbols::TrueClass());
}

bool Literal::isFalse(const core::GlobalState &gs) const {
    return value.derivesFrom(gs, core::Symbols::FalseClass());
}

string_view UnresolvedConstantLit::nodeName() const {
    return "UnresolvedConstantLit";
}

string_view ConstantLit::nodeName() const {
    return "ConstantLit";
}

string_view Block::nodeName() const {
    return "Block";
}

string_view InsSeq::nodeName() const {
    return "InsSeq";
}

string_view EmptyTree::nodeName() const {
    return "EmptyTree";
}

string EmptyTree::showRaw(const core::GlobalState &gs, int tabs) const {
    return string(nodeName());
}

string RestParam::showRaw(const core::GlobalState &gs, int tabs) const {
    return fmt::format("{}{{ expr = {} }}", nodeName(), expr.showRaw(gs, tabs));
}

string_view RestParam::nodeName() const {
    return "RestParam";
}

string KeywordArg::showRaw(const core::GlobalState &gs, int tabs) const {
    return fmt::format("{}{{ expr = {} }}", nodeName(), expr.showRaw(gs, tabs));
}

string_view KeywordArg::nodeName() const {
    return "KeywordArg";
}

string OptionalParam::showRaw(const core::GlobalState &gs, int tabs) const {
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

string_view OptionalParam::nodeName() const {
    return "OptionalParam";
}

string ShadowArg::showRaw(const core::GlobalState &gs, int tabs) const {
    return fmt::format("{}{{ expr = {} }}", nodeName(), expr.showRaw(gs, tabs));
}

string BlockArg::showRaw(const core::GlobalState &gs, int tabs) const {
    return fmt::format("{}{{ expr = {} }}", nodeName(), expr.showRaw(gs, tabs));
}

string_view ShadowArg::nodeName() const {
    return "ShadowArg";
}

string_view BlockArg::nodeName() const {
    return "BlockArg";
}

string_view RuntimeMethodDefinition::nodeName() const {
    return "RuntimeMethodDefinition";
}

string_view Self::nodeName() const {
    return "Self";
}

ParsedFilesOrCancelled::ParsedFilesOrCancelled() : trees(nullopt){};
ParsedFilesOrCancelled::ParsedFilesOrCancelled(vector<ParsedFile> &&trees) : trees(move(trees)) {}

ParsedFilesOrCancelled ParsedFilesOrCancelled::cancel(std::vector<ParsedFile> &&trees, WorkerPool &workers) {
    Parallel::iterate(workers, "deleteTrees", absl::MakeSpan(trees), [](auto &job) {
        // Force the destructor of `ast::ExpressionPtr` to run for `job.tree`.
        job.tree.reset();
    });

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

string ClassDef::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "kind = {}\n", kind == ClassDef::Kind::Module ? "module" : "class");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "name = {}\n", name.showRawWithLocs(gs, file, tabs + 1));
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
        fmt::format_to(std::back_inserter(buf), "{}", a.showRawWithLocs(gs, file, tabs + 2));
    }
    fmt::format_to(std::back_inserter(buf), "]\n");

    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "rhs = [\n");

    for (auto &a : this->rhs) {
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.showRawWithLocs(gs, file, tabs + 2));
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

string MethodDef::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
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
        for (auto &p : this->params) {
            if (!first) {
                fmt::format_to(std::back_inserter(buf), ", ");
            }
            first = false;
            fmt::format_to(std::back_inserter(buf), "{}", p.showRawWithLocs(gs, file, tabs + 2));
        }
    } else {
        for (auto &p : this->params) {
            if (!first) {
                fmt::format_to(std::back_inserter(buf), ", ");
            }
            first = false;
            fmt::format_to(std::back_inserter(buf), "{}", p.showRawWithLocs(gs, file, tabs + 2));
        }
    }
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "rhs = {}\n", this->rhs.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string If::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "cond = {}\n", this->cond.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "thenp = {}\n", this->thenp.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "elsep = {}\n", this->elsep.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string While::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "cond = {}\n", this->cond.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "body = {}\n", this->body.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string Assign::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "lhs = {}\n", this->lhs.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "rhs = {}\n", this->rhs.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string EmptyTree::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    return string(nodeName()); // Empty trees never have a valid location, so don't bother printing it.
}

string ConstantLit::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}-{}\n", this->loc().beginLoc, this->loc().endLoc);
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "symbol = ({} {})\n", this->symbol().showKind(gs),
                   this->symbol().showFullName(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "orig = {}\n",
                   this->original() ? this->original()->showRawWithLocs(gs, file, tabs + 1) : "nullptr");
    // If resolutionScopes isn't null, it should not be empty.
    ENFORCE(resolutionScopes() == nullptr || !resolutionScopes()->empty());
    if (resolutionScopes() != nullptr && !resolutionScopes()->empty()) {
        printTabs(buf, tabs + 1);
        fmt::format_to(std::back_inserter(buf), "resolutionScopes = [{}]\n",
                       fmt::map_join(*this->resolutionScopes(), ", ", [&](auto sym) { return sym.showFullName(gs); }));
    }
    printTabs(buf, tabs);

    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string UnresolvedIdent::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
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

string Send::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));

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
    fmt::format_to(std::back_inserter(buf), "recv = {}\n", this->recv.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "fun = {}\n", this->fun.showRaw(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "block = ");
    if (this->hasBlock()) {
        fmt::format_to(std::back_inserter(buf), "{}\n", this->block()->showRawWithLocs(gs, file, tabs + 1));
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
        fmt::format_to(std::back_inserter(buf), "{}\n", a.showRawWithLocs(gs, file, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");

    return fmt::to_string(buf);
}

string Literal::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    return fmt::format("{}{{ loc = {}, value = {} }}", nodeName(), core::Loc(file, this->loc).fileShortPosToString(gs),
                       this->toStringWithTabs(gs, 0));
}

string UnresolvedConstantLit::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "cnst = {}\n", this->cnst.showRaw(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "scope = {}\n", this->scope.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string Block::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{} {{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "params = [\n");
    for (auto &p : this->params) {
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", p.showRawWithLocs(gs, file, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "body = {}\n", this->body.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string ZSuperArgs::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    return fmt::format("{}{{ loc = {} }}", nodeName(), core::Loc(file, this->loc).fileShortPosToString(gs));
}

string RuntimeMethodDefinition::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    return fmt::format("{}{{ loc = {}, name = {}, isSelfMethod = {} }}", nodeName(),
                       core::Loc(file, this->loc).fileShortPosToString(gs), this->name.toString(gs),
                       this->isSelfMethod);
}

string InsSeq::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "stats = [\n");
    for (auto &a : this->stats) {
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.showRawWithLocs(gs, file, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "],\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "expr = {}\n", expr.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string Array::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "elems = [\n");
    for (auto &a : elems) {
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.showRawWithLocs(gs, file, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string Next::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    return fmt::format("{}{{ loc = {}, expr = {} }}", nodeName(), core::Loc(file, this->loc).fileShortPosToString(gs),
                       this->expr.showRawWithLocs(gs, file, tabs + 1));
}

string RescueCase::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "exceptions = [\n");
    for (auto &a : exceptions) {
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.showRawWithLocs(gs, file, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "var = {}\n", this->var.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "body = {}\n", this->body.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string ShadowArg::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    return fmt::format("{}{{ loc = {}, expr = {} }}", nodeName(), core::Loc(file, this->loc).fileShortPosToString(gs),
                       expr.showRawWithLocs(gs, file, tabs));
}

string Break::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    return fmt::format("{}{{ loc = {}, expr = {} }}", nodeName(), core::Loc(file, this->loc).fileShortPosToString(gs),
                       this->expr.showRawWithLocs(gs, file, tabs + 1));
}

string Hash::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "pairs = [\n");
    int i = -1;
    for (auto &key : keys) {
        i++;
        auto &value = values[i];
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "[\n");
        printTabs(buf, tabs + 3);
        fmt::format_to(std::back_inserter(buf), "key = {}\n", key.showRawWithLocs(gs, file, tabs + 3));
        printTabs(buf, tabs + 3);
        fmt::format_to(std::back_inserter(buf), "value = {}\n", value.showRawWithLocs(gs, file, tabs + 3));
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "]\n");
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string Cast::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 2);
    fmt::format_to(std::back_inserter(buf), "cast = {},\n", this->cast.showRaw(gs));
    printTabs(buf, tabs + 2);
    fmt::format_to(std::back_inserter(buf), "arg = {}\n", this->arg.showRawWithLocs(gs, file, tabs + 2));
    printTabs(buf, tabs + 2);
    fmt::format_to(std::back_inserter(buf), "type = {},\n", this->type.showWithMoreInfo(gs));
    printTabs(buf, tabs + 2);
    fmt::format_to(std::back_inserter(buf), "typeExpr = {},\n", this->typeExpr.showRawWithLocs(gs, file, tabs + 2));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}\n");
    return fmt::to_string(buf);
}

string BlockArg::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    return fmt::format("{}{{ loc = {}, expr = {} }}", nodeName(), core::Loc(file, this->loc).fileShortPosToString(gs),
                       expr.showRawWithLocs(gs, file, tabs));
}

string Retry::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    return fmt::format("{}{{ loc = {} }}", nodeName(), core::Loc(file, this->loc).fileShortPosToString(gs));
}

string KeywordArg::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    return fmt::format("{}{{ loc = {}, expr = {} }}", nodeName(), core::Loc(file, this->loc).fileShortPosToString(gs),
                       expr.showRawWithLocs(gs, file, tabs));
}

string Return::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    return fmt::format("{}{{ loc = {}, expr = {} }}", nodeName(), core::Loc(file, this->loc).fileShortPosToString(gs),
                       this->expr.showRawWithLocs(gs, file, tabs + 1));
}

string Local::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "localVariable = {}\n", this->localVariable.showRaw(gs));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string Rescue::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "body = {}\n", this->body.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "rescueCases = [\n");
    for (auto &a : rescueCases) {
        printTabs(buf, tabs + 2);
        fmt::format_to(std::back_inserter(buf), "{}\n", a.showRawWithLocs(gs, file, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "]\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "else = {}\n", this->else_.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "ensure = {}\n", this->ensure.showRawWithLocs(gs, file, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

string RestParam::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    return fmt::format("{}{{ loc = {}, expr = {} }}", nodeName(), core::Loc(file, this->loc).fileShortPosToString(gs),
                       expr.showRawWithLocs(gs, file, tabs));
}

string OptionalParam::showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "loc = {}\n", core::Loc(file, this->loc).fileShortPosToString(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(std::back_inserter(buf), "expr = {}\n", expr.showRawWithLocs(gs, file, tabs + 1));
    if (default_) {
        printTabs(buf, tabs + 1);
        fmt::format_to(std::back_inserter(buf), "default_ = {}\n", default_.showRawWithLocs(gs, file, tabs + 1));
    }
    printTabs(buf, tabs);
    fmt::format_to(std::back_inserter(buf), "}}");
    return fmt::to_string(buf);
}

} // namespace sorbet::ast
