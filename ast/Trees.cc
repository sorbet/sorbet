#include "ast/Trees.h"
#include "common/formatting.h"
#include "common/typecase.h"
#include "core/Symbols.h"
#include <sstream>
#include <utility>

// makes lldb work. Don't remove please
template class std::unique_ptr<sorbet::ast::ClassDef>;
template class std::unique_ptr<sorbet::ast::MethodDef>;
template class std::unique_ptr<sorbet::ast::If>;
template class std::unique_ptr<sorbet::ast::While>;
template class std::unique_ptr<sorbet::ast::Break>;
template class std::unique_ptr<sorbet::ast::Retry>;
template class std::unique_ptr<sorbet::ast::Next>;
template class std::unique_ptr<sorbet::ast::Return>;
template class std::unique_ptr<sorbet::ast::RescueCase>;
template class std::unique_ptr<sorbet::ast::Rescue>;
template class std::unique_ptr<sorbet::ast::Local>;
template class std::unique_ptr<sorbet::ast::UnresolvedIdent>;
template class std::unique_ptr<sorbet::ast::RestArg>;
template class std::unique_ptr<sorbet::ast::KeywordArg>;
template class std::unique_ptr<sorbet::ast::OptionalArg>;
template class std::unique_ptr<sorbet::ast::BlockArg>;
template class std::unique_ptr<sorbet::ast::ShadowArg>;
template class std::unique_ptr<sorbet::ast::Assign>;
template class std::unique_ptr<sorbet::ast::Send>;
template class std::unique_ptr<sorbet::ast::Cast>;
template class std::unique_ptr<sorbet::ast::Hash>;
template class std::unique_ptr<sorbet::ast::Array>;
template class std::unique_ptr<sorbet::ast::Literal>;
template class std::unique_ptr<sorbet::ast::UnresolvedConstantLit>;
template class std::unique_ptr<sorbet::ast::ZSuperArgs>;
template class std::unique_ptr<sorbet::ast::Block>;
template class std::unique_ptr<sorbet::ast::InsSeq>;
template class std::unique_ptr<sorbet::ast::EmptyTree>;
template class std::unique_ptr<sorbet::ast::ConstantLit>;

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

void TreePtr::deleteTagged(Tag tag, void *ptr) noexcept {
    ENFORCE(ptr != nullptr);
#define DELETE_TYPE(T)                                      \
    if (tag != Tag::EmptyTree) {                            \
        /* explicitly not deleting the empty tree pointer*/ \
        delete reinterpret_cast<T *>(ptr);                  \
    }

    GENERATE_TAG_SWITCH(tag, DELETE_TYPE)

#undef DELETE_TYPE
}

string TreePtr::nodeName() const {
    auto *ptr = get();

    ENFORCE(ptr != nullptr);

#define NODE_NAME(name) return reinterpret_cast<name *>(ptr)->nodeName();
    GENERATE_TAG_SWITCH(tag(), NODE_NAME)
#undef NODE_NAME
}

string TreePtr::showRaw(const core::GlobalState &gs, int tabs) {
    auto *ptr = get();

    ENFORCE(ptr != nullptr);

#define SHOW_RAW(name) return reinterpret_cast<name *>(ptr)->showRaw(gs, tabs);
    GENERATE_TAG_SWITCH(tag(), SHOW_RAW)
#undef SHOW_RAW
}

core::LocOffsets TreePtr::loc() const {
    auto *ptr = get();

    ENFORCE(ptr != nullptr);

#define CASE(name) return reinterpret_cast<name *>(ptr)->loc;
    GENERATE_TAG_SWITCH(tag(), CASE)
#undef CASE
}

string TreePtr::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    auto *ptr = get();

    ENFORCE(ptr != nullptr);

#define CASE(name) return reinterpret_cast<name *>(ptr)->toStringWithTabs(gs, tabs);
    GENERATE_TAG_SWITCH(tag(), CASE)
#undef CASE
}

bool TreePtr::isSelfReference() const {
    if (auto *local = cast_tree<Local>(*this)) {
        return local->localVariable == core::LocalVariable::selfVariable();
    }
    return false;
}

bool isa_reference(const TreePtr &what) {
    return isa_tree<Local>(what) || isa_tree<UnresolvedIdent>(what) || isa_tree<RestArg>(what) ||
           isa_tree<KeywordArg>(what) || isa_tree<OptionalArg>(what) || isa_tree<BlockArg>(what) ||
           isa_tree<ShadowArg>(what);
}

bool isa_declaration(const TreePtr &what) {
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

ClassDef::ClassDef(core::LocOffsets loc, core::LocOffsets declLoc, core::SymbolRef symbol, TreePtr name,
                   ANCESTORS_store ancestors, RHS_store rhs, ClassDef::Kind kind)
    : loc(loc), declLoc(declLoc), symbol(symbol), kind(kind), rhs(std::move(rhs)), name(std::move(name)),
      ancestors(std::move(ancestors)) {
    categoryCounterInc("trees", "classdef");
    histogramInc("trees.classdef.kind", (int)kind);
    histogramInc("trees.classdef.ancestors", this->ancestors.size());
    _sanityCheck();
}

MethodDef::MethodDef(core::LocOffsets loc, core::LocOffsets declLoc, core::SymbolRef symbol, core::NameRef name,
                     ARGS_store args, TreePtr rhs, Flags flags)
    : loc(loc), declLoc(declLoc), symbol(symbol), rhs(std::move(rhs)), args(std::move(args)), name(name), flags(flags) {
    categoryCounterInc("trees", "methoddef");
    histogramInc("trees.methodDef.args", this->args.size());
    _sanityCheck();
}

If::If(core::LocOffsets loc, TreePtr cond, TreePtr thenp, TreePtr elsep)
    : loc(loc), cond(std::move(cond)), thenp(std::move(thenp)), elsep(std::move(elsep)) {
    categoryCounterInc("trees", "if");
    _sanityCheck();
}

While::While(core::LocOffsets loc, TreePtr cond, TreePtr body)
    : loc(loc), cond(std::move(cond)), body(std::move(body)) {
    categoryCounterInc("trees", "while");
    _sanityCheck();
}

Break::Break(core::LocOffsets loc, TreePtr expr) : loc(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "break");
    _sanityCheck();
}

Retry::Retry(core::LocOffsets loc) : loc(loc) {
    categoryCounterInc("trees", "retry");
    _sanityCheck();
}

Next::Next(core::LocOffsets loc, TreePtr expr) : loc(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "next");
    _sanityCheck();
}

Return::Return(core::LocOffsets loc, TreePtr expr) : loc(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "return");
    _sanityCheck();
}

RescueCase::RescueCase(core::LocOffsets loc, EXCEPTION_store exceptions, TreePtr var, TreePtr body)
    : loc(loc), exceptions(std::move(exceptions)), var(std::move(var)), body(std::move(body)) {
    categoryCounterInc("trees", "rescuecase");
    histogramInc("trees.rescueCase.exceptions", this->exceptions.size());
    _sanityCheck();
}

Rescue::Rescue(core::LocOffsets loc, TreePtr body, RESCUE_CASE_store rescueCases, TreePtr else_, TreePtr ensure)
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

Assign::Assign(core::LocOffsets loc, TreePtr lhs, TreePtr rhs) : loc(loc), lhs(std::move(lhs)), rhs(std::move(rhs)) {
    categoryCounterInc("trees", "assign");
    _sanityCheck();
}

Send::Send(core::LocOffsets loc, TreePtr recv, core::NameRef fun, u2 numPosArgs, Send::ARGS_store args, TreePtr block,
           Flags flags)
    : loc(loc), fun(fun), flags(flags), numPosArgs(numPosArgs), recv(std::move(recv)), args(std::move(args)),
      block(std::move(block)) {
    categoryCounterInc("trees", "send");
    if (block) {
        counterInc("trees.send.with_block");
    }
    histogramInc("trees.send.args", this->args.size());
    _sanityCheck();
}

Cast::Cast(core::LocOffsets loc, core::TypePtr ty, TreePtr arg, core::NameRef cast)
    : loc(loc), cast(cast), type(std::move(ty)), arg(std::move(arg)) {
    categoryCounterInc("trees", "cast");
    _sanityCheck();
}

ZSuperArgs::ZSuperArgs(core::LocOffsets loc) : loc(loc) {
    categoryCounterInc("trees", "zsuper");
    _sanityCheck();
}

RestArg::RestArg(core::LocOffsets loc, TreePtr arg) : loc(loc), expr(std::move(arg)) {
    categoryCounterInc("trees", "restarg");
    _sanityCheck();
}

KeywordArg::KeywordArg(core::LocOffsets loc, TreePtr expr) : loc(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "keywordarg");
    _sanityCheck();
}

OptionalArg::OptionalArg(core::LocOffsets loc, TreePtr expr, TreePtr default_)
    : loc(loc), expr(std::move(expr)), default_(std::move(default_)) {
    categoryCounterInc("trees", "optionalarg");
    _sanityCheck();
}

ShadowArg::ShadowArg(core::LocOffsets loc, TreePtr expr) : loc(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "shadowarg");
    _sanityCheck();
}

BlockArg::BlockArg(core::LocOffsets loc, TreePtr expr) : loc(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "blockarg");
    _sanityCheck();
}

Literal::Literal(core::LocOffsets loc, const core::TypePtr &value) : loc(loc), value(std::move(value)) {
    categoryCounterInc("trees", "literal");
    _sanityCheck();
}

UnresolvedConstantLit::UnresolvedConstantLit(core::LocOffsets loc, TreePtr scope, core::NameRef cnst)
    : loc(loc), cnst(cnst), scope(std::move(scope)) {
    categoryCounterInc("trees", "constantlit");
    _sanityCheck();
}

ConstantLit::ConstantLit(core::LocOffsets loc, core::SymbolRef symbol, TreePtr original)
    : loc(loc), symbol(symbol), original(std::move(original)) {
    categoryCounterInc("trees", "resolvedconstantlit");
    _sanityCheck();
}

optional<pair<core::SymbolRef, vector<core::NameRef>>>
ConstantLit::fullUnresolvedPath(const core::GlobalState &gs) const {
    if (this->symbol != core::Symbols::StubModule()) {
        return nullopt;
    }
    ENFORCE(!this->resolutionScopes.empty());
    vector<core::NameRef> namesFailedToResolve;
    auto *nested = this;
    {
        while (!nested->resolutionScopes.front().exists()) {
            auto *orig = cast_tree<UnresolvedConstantLit>(nested->original);
            ENFORCE(orig);
            namesFailedToResolve.emplace_back(orig->cnst);
            nested = ast::cast_tree<ast::ConstantLit>(orig->scope);
            ENFORCE(nested);
            ENFORCE(nested->symbol == core::Symbols::StubModule());
            ENFORCE(!nested->resolutionScopes.empty());
        }
        auto *orig = cast_tree<UnresolvedConstantLit>(nested->original);
        ENFORCE(orig);
        namesFailedToResolve.emplace_back(orig->cnst);
        absl::c_reverse(namesFailedToResolve);
    }
    auto prefix = nested->resolutionScopes.front();
    return make_pair(prefix, move(namesFailedToResolve));
}

Block::Block(core::LocOffsets loc, MethodDef::ARGS_store args, TreePtr body)
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

InsSeq::InsSeq(core::LocOffsets loc, STATS_store stats, TreePtr expr)
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

template <> TreePtr make_tree<EmptyTree>() {
    TreePtr result = nullptr;
    result.reset(&singletonEmptyTree);
    return result;
}

namespace {

void printTabs(fmt::memory_buffer &to, int count) {
    int i = 0;
    while (i < count) {
        fmt::format_to(to, "  ");
        i++;
    }
}

template <class T> void printElems(const core::GlobalState &gs, fmt::memory_buffer &buf, T &args, int tabs) {
    bool first = true;
    bool didshadow = false;
    for (auto &a : args) {
        if (!first) {
            if (isa_tree<ShadowArg>(a) && !didshadow) {
                fmt::format_to(buf, "; ");
                didshadow = true;
            } else {
                fmt::format_to(buf, ", ");
            }
        }
        first = false;
        fmt::format_to(buf, "{}", a.toStringWithTabs(gs, tabs + 1));
    }
};

template <class T> void printArgs(const core::GlobalState &gs, fmt::memory_buffer &buf, T &args, int tabs) {
    fmt::format_to(buf, "(");
    printElems(gs, buf, args, tabs);
    fmt::format_to(buf, ")");
}

} // namespace

string ClassDef::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    if (kind == ClassDef::Kind::Module) {
        fmt::format_to(buf, "module ");
    } else {
        fmt::format_to(buf, "class ");
    }
    fmt::format_to(buf, "{}<{}> < ", name.toStringWithTabs(gs, tabs),
                   this->symbol.dataAllowingNone(gs)->name.data(gs)->toString(gs));
    printArgs(gs, buf, this->ancestors, tabs);

    if (this->rhs.empty()) {
        fmt::format_to(buf, "{}", '\n');
    }

    for (auto &a : this->rhs) {
        fmt::format_to(buf, "{}", '\n');
        printTabs(buf, tabs + 1);
        fmt::format_to(buf, "{}\n", a.toStringWithTabs(gs, tabs + 1));
    }

    printTabs(buf, tabs);
    fmt::format_to(buf, "end");
    return fmt::to_string(buf);
}

string ClassDef::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "kind = {}\n", kind == ClassDef::Kind::Module ? "module" : "class");
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "name = {}<{}>\n", name.showRaw(gs, tabs + 1),
                   this->symbol.dataAllowingNone(gs)->name.data(gs)->showRaw(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "ancestors = [");
    bool first = true;
    for (auto &a : this->ancestors) {
        if (!first) {
            fmt::format_to(buf, ", ");
        }
        first = false;
        fmt::format_to(buf, "{}", a.showRaw(gs, tabs + 2));
    }
    fmt::format_to(buf, "]\n");

    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "rhs = [\n");

    for (auto &a : this->rhs) {
        printTabs(buf, tabs + 2);
        fmt::format_to(buf, "{}\n", a.showRaw(gs, tabs + 2));
        if (&a != &this->rhs.back()) {
            fmt::format_to(buf, "{}", '\n');
        }
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "]\n");
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");
    return fmt::to_string(buf);
}

string InsSeq::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "begin\n");
    for (auto &a : this->stats) {
        printTabs(buf, tabs + 1);
        fmt::format_to(buf, "{}\n", a.toStringWithTabs(gs, tabs + 1));
    }

    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "{}\n", expr.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "end");
    return fmt::to_string(buf);
}

string InsSeq::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "stats = [\n");
    for (auto &a : this->stats) {
        printTabs(buf, tabs + 2);
        fmt::format_to(buf, "{}\n", a.showRaw(gs, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "],\n");

    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "expr = {}\n", expr.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");
    return fmt::to_string(buf);
}

string MethodDef::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;

    if (this->flags.isSelfMethod) {
        fmt::format_to(buf, "def self.");
    } else {
        fmt::format_to(buf, "def ");
    }
    fmt::format_to(buf, "{}", name.data(gs)->toString(gs));
    auto &data = this->symbol.dataAllowingNone(gs);
    if (name != data->name) {
        fmt::format_to(buf, "<{}>", data->name.data(gs)->toString(gs));
    }
    fmt::format_to(buf, "(");
    bool first = true;
    if (this->symbol == core::Symbols::todo()) {
        for (auto &a : this->args) {
            if (!first) {
                fmt::format_to(buf, ", ");
            }
            first = false;
            fmt::format_to(buf, "{}", a.toStringWithTabs(gs, tabs + 1));
        }
    } else {
        for (auto &a : data->arguments()) {
            if (!first) {
                fmt::format_to(buf, ", ");
            }
            first = false;
            fmt::format_to(buf, "{}", a.argumentName(gs));
        }
    }
    fmt::format_to(buf, ")\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "{}\n", this->rhs.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "end");
    return fmt::to_string(buf);
}

string MethodDef::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);

    auto stringifiedFlags = vector<string>{};
    if (this->flags.isSelfMethod) {
        stringifiedFlags.emplace_back("self");
    }
    if (this->flags.isRewriterSynthesized) {
        stringifiedFlags.emplace_back("rewriter");
    }
    fmt::format_to(buf, "flags = {{{}}}\n", fmt::join(stringifiedFlags, ", "));

    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "name = {}<{}>\n", name.data(gs)->showRaw(gs),
                   this->symbol.dataAllowingNone(gs)->name.data(gs)->showRaw(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "args = [");
    bool first = true;
    if (this->symbol == core::Symbols::todo()) {
        for (auto &a : this->args) {
            if (!first) {
                fmt::format_to(buf, ", ");
            }
            first = false;
            fmt::format_to(buf, "{}", a.showRaw(gs, tabs + 2));
        }
    } else {
        for (auto &a : this->args) {
            if (!first) {
                fmt::format_to(buf, ", ");
            }
            first = false;
            fmt::format_to(buf, "{}", a.showRaw(gs, tabs + 2));
        }
    }
    fmt::format_to(buf, "]\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "rhs = {}\n", this->rhs.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");
    return fmt::to_string(buf);
}

string If::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;

    fmt::format_to(buf, "if {}\n", this->cond.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "{}\n", this->thenp.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "else\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "{}\n", this->elsep.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "end");
    return fmt::to_string(buf);
}

string If::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;

    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "cond = {}\n", this->cond.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "thenp = {}\n", this->thenp.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "elsep = {}\n", this->elsep.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");
    return fmt::to_string(buf);
}

string Assign::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;

    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "lhs = {}\n", this->lhs.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "rhs = {}\n", this->rhs.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");
    return fmt::to_string(buf);
}

string While::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;

    fmt::format_to(buf, "while {}\n", this->cond.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "{}\n", this->body.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "end");
    return fmt::to_string(buf);
}

string While::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;

    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "cond = {}\n", this->cond.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "body = {}\n", this->body.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");
    return fmt::to_string(buf);
}

string EmptyTree::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "<emptyTree>";
}

string UnresolvedConstantLit::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return fmt::format("{}::{}", this->scope.toStringWithTabs(gs, tabs), this->cnst.data(gs)->toString(gs));
}

string UnresolvedConstantLit::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;

    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "scope = {}\n", this->scope.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "cnst = {}\n", this->cnst.data(gs)->showRaw(gs));
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");
    return fmt::to_string(buf);
}

string ConstantLit::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    if (symbol.exists() && symbol != core::Symbols::StubModule()) {
        return this->symbol.dataAllowingNone(gs)->showFullName(gs);
    }
    return "Unresolved: " + this->original.toStringWithTabs(gs, tabs);
}

string ConstantLit::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;

    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "orig = {}\n", this->original ? this->original.showRaw(gs, tabs + 1) : "nullptr");
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "symbol = ({} {})\n", this->symbol.dataAllowingNone(gs)->showKind(gs),
                   this->symbol.dataAllowingNone(gs)->showFullName(gs));
    if (!resolutionScopes.empty()) {
        printTabs(buf, tabs + 1);
        fmt::format_to(buf, "resolutionScopes = [{}]\n",
                       fmt::map_join(this->resolutionScopes.begin(), this->resolutionScopes.end(), ", ",
                                     [&](auto sym) { return sym.data(gs)->showFullName(gs); }));
    }
    printTabs(buf, tabs);

    fmt::format_to(buf, "}}");
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
    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "localVariable = {}\n", this->localVariable.showRaw(gs));
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");
    return fmt::to_string(buf);
}

string UnresolvedIdent::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->name.toString(gs);
}

string UnresolvedIdent::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "kind = ");
    switch (this->kind) {
        case Kind::Local:
            fmt::format_to(buf, "Local");
            break;
        case Kind::Instance:
            fmt::format_to(buf, "Instance");
            break;
        case Kind::Class:
            fmt::format_to(buf, "Class");
            break;
        case Kind::Global:
            fmt::format_to(buf, "Global");
            break;
    }
    fmt::format_to(buf, "{}", '\n');
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "name = {}\n", this->name.showRaw(gs));
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");

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
    fmt::format_to(buf, "rescue");
    bool first = true;
    for (auto &exception : this->exceptions) {
        if (first) {
            first = false;
            fmt::format_to(buf, " ");
        } else {
            fmt::format_to(buf, ", ");
        }
        fmt::format_to(buf, "{}", exception.toStringWithTabs(gs, tabs));
    }
    fmt::format_to(buf, " => {}\n", this->var.toStringWithTabs(gs, tabs));
    printTabs(buf, tabs);
    fmt::format_to(buf, "{}", this->body.toStringWithTabs(gs, tabs));
    return fmt::to_string(buf);
}

string RescueCase::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "exceptions = [\n");
    for (auto &a : exceptions) {
        printTabs(buf, tabs + 2);
        fmt::format_to(buf, "{}\n", a.showRaw(gs, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "]\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "var = {}\n", this->var.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "body = {}\n", this->body.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");
    return fmt::to_string(buf);
}

string Rescue::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}", this->body.toStringWithTabs(gs, tabs));
    for (auto &rescueCase : this->rescueCases) {
        fmt::format_to(buf, "\n");
        printTabs(buf, tabs - 1);
        fmt::format_to(buf, "{}", rescueCase.toStringWithTabs(gs, tabs));
    }
    if (!isa_tree<EmptyTree>(this->else_)) {
        fmt::format_to(buf, "\n");
        printTabs(buf, tabs - 1);
        fmt::format_to(buf, "else\n");
        printTabs(buf, tabs);
        fmt::format_to(buf, "{}", this->else_.toStringWithTabs(gs, tabs));
    }
    if (!isa_tree<EmptyTree>(this->ensure)) {
        fmt::format_to(buf, "\n");
        printTabs(buf, tabs - 1);
        fmt::format_to(buf, "ensure\n");
        printTabs(buf, tabs);
        fmt::format_to(buf, "{}", this->ensure.toStringWithTabs(gs, tabs));
    }
    return fmt::to_string(buf);
}

string Rescue::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "body = {}\n", this->body.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "rescueCases = [\n");
    for (auto &a : rescueCases) {
        printTabs(buf, tabs + 2);
        fmt::format_to(buf, "{}\n", a.showRaw(gs, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "]\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "else = {}\n", this->else_.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "ensure = {}\n", this->ensure.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");
    return fmt::to_string(buf);
}

string Send::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}.{}", this->recv.toStringWithTabs(gs, tabs), this->fun.data(gs)->toString(gs));
    printArgs(gs, buf, this->args, tabs);
    if (this->block != nullptr) {
        fmt::format_to(buf, "{}", this->block.toStringWithTabs(gs, tabs));
    }

    return fmt::to_string(buf);
}

string Send::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "recv = {}\n", this->recv.showRaw(gs, tabs + 1));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "fun = {}\n", this->fun.data(gs)->showRaw(gs));
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "block = ");
    if (this->block) {
        fmt::format_to(buf, "{}\n", this->block.showRaw(gs, tabs + 1));
    } else {
        fmt::format_to(buf, "nullptr\n");
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "pos_args = {}\n", this->numPosArgs);
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "args = [\n");
    for (auto &a : args) {
        printTabs(buf, tabs + 2);
        fmt::format_to(buf, "{}\n", a.showRaw(gs, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "]\n");
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");

    return fmt::to_string(buf);
}

string Cast::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "T.{}", this->cast.toString(gs));
    fmt::format_to(buf, "({}, {})", this->arg.toStringWithTabs(gs, tabs), this->type.toStringWithTabs(gs, tabs));

    return fmt::to_string(buf);
}

string Cast::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 2);
    fmt::format_to(buf, "cast = {},\n", this->cast.showRaw(gs));
    printTabs(buf, tabs + 2);
    fmt::format_to(buf, "arg = {}\n", this->arg.showRaw(gs, tabs + 2));
    printTabs(buf, tabs + 2);
    fmt::format_to(buf, "type = {},\n", this->type.toString(gs));
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}\n");

    return fmt::to_string(buf);
}

string ZSuperArgs::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ }";
}

string Hash::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "pairs = [\n");
    int i = -1;
    for (auto &key : keys) {
        i++;
        auto &value = values[i];

        printTabs(buf, tabs + 2);
        fmt::format_to(buf, "[\n");
        printTabs(buf, tabs + 3);
        fmt::format_to(buf, "key = {}\n", key.showRaw(gs, tabs + 3));
        printTabs(buf, tabs + 3);
        fmt::format_to(buf, "value = {}\n", value.showRaw(gs, tabs + 3));
        printTabs(buf, tabs + 2);
        fmt::format_to(buf, "]\n");
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "]\n");
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");

    return fmt::to_string(buf);
}

string Array::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "elems = [\n");
    for (auto &a : elems) {
        printTabs(buf, tabs + 2);
        fmt::format_to(buf, "{}\n", a.showRaw(gs, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "]\n");
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");

    return fmt::to_string(buf);
}

string ZSuperArgs::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "ZSuperArgs";
}

string Hash::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{{");
    bool first = true;
    int i = -1;
    for (auto &key : this->keys) {
        i++;
        auto &value = this->values[i];
        if (!first) {
            fmt::format_to(buf, ", ");
        }
        first = false;
        fmt::format_to(buf, "{}", key.toStringWithTabs(gs, tabs + 1));
        fmt::format_to(buf, " => ");
        fmt::format_to(buf, "{}", value.toStringWithTabs(gs, tabs + 1));
    }
    fmt::format_to(buf, "}}");
    return fmt::to_string(buf);
}

string Array::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "[");
    printElems(gs, buf, this->elems, tabs);
    fmt::format_to(buf, "]");
    return fmt::to_string(buf);
}

string Block::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    fmt::format_to(buf, " do |");
    printElems(gs, buf, this->args, tabs + 1);
    fmt::format_to(buf, "|\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "{}\n", this->body.toStringWithTabs(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "end");
    return fmt::to_string(buf);
}

string Block::showRaw(const core::GlobalState &gs, int tabs) {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{} {{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "args = [\n");
    for (auto &a : this->args) {
        printTabs(buf, tabs + 2);
        fmt::format_to(buf, "{}\n", a.showRaw(gs, tabs + 2));
    }
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "]\n");
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "body = {}\n", this->body.showRaw(gs, tabs + 1));
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");
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
    fmt::format_to(buf, "{}", this->expr.toStringWithTabs(gs, tabs));
    if (this->default_) {
        fmt::format_to(buf, " = {}", this->default_.toStringWithTabs(gs, tabs));
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
    core::NameRef res(gs, t.value);
    return res;
}

core::NameRef Literal::asSymbol(const core::GlobalState &gs) const {
    ENFORCE(isSymbol(gs));
    auto t = core::cast_type_nonnull<core::LiteralType>(value);
    core::NameRef res(gs, t.value);
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
    fmt::format_to(buf, "{}{{\n", nodeName());
    printTabs(buf, tabs + 1);
    fmt::format_to(buf, "expr = {}\n", expr.showRaw(gs, tabs + 1));
    if (default_) {
        printTabs(buf, tabs + 1);
        fmt::format_to(buf, "default_ = {}\n", default_.showRaw(gs, tabs + 1));
    }
    printTabs(buf, tabs);
    fmt::format_to(buf, "}}");

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
