#include "parser/parser.h"
#include <algorithm>
#include <iterator>

using namespace std;

namespace sorbet::parser {

void NodePtr::printTabs(fmt::memory_buffer &to, int count) const {
    int i = 0;
    while (i < count) {
        fmt::format_to(to, "{}", "  ");
        i++;
    }
}

void NodePtr::printNode(fmt::memory_buffer &to, const core::GlobalState &gs, int tabs) const {
    fmt::format_to(to, "{}\n", toStringWithTabs(gs, tabs));
}

void NodePtr::printNodeJSON(fmt::memory_buffer &to, const core::GlobalState &gs, int tabs) {
    fmt::format_to(to, "{}", toJSON(gs, tabs));
}

void NodePtr::printNodeWhitequark(fmt::memory_buffer &to, const core::GlobalState &gs, int tabs) {
    fmt::format_to(to, "{}", toWhitequark(gs, tabs));
}

#define GENERATE_CASE(name, type, ...) \
    case NodePtr::Tag::type:           \
        return NodePtr::cast_nonnull<type>(*this).name(__VA_ARGS__);

#define TAG_SWITCH(name, ...)                             \
    switch (this->tag()) {                                \
        GENERATE_CASE(name, Alias, __VA_ARGS__)           \
        GENERATE_CASE(name, And, __VA_ARGS__)             \
        GENERATE_CASE(name, AndAsgn, __VA_ARGS__)         \
        GENERATE_CASE(name, Arg, __VA_ARGS__)             \
        GENERATE_CASE(name, Args, __VA_ARGS__)            \
        GENERATE_CASE(name, Array, __VA_ARGS__)           \
        GENERATE_CASE(name, Backref, __VA_ARGS__)         \
        GENERATE_CASE(name, Assign, __VA_ARGS__)          \
        GENERATE_CASE(name, Begin, __VA_ARGS__)           \
        GENERATE_CASE(name, Block, __VA_ARGS__)           \
        GENERATE_CASE(name, Blockarg, __VA_ARGS__)        \
        GENERATE_CASE(name, BlockPass, __VA_ARGS__)       \
        GENERATE_CASE(name, Break, __VA_ARGS__)           \
        GENERATE_CASE(name, Case, __VA_ARGS__)            \
        GENERATE_CASE(name, Cbase, __VA_ARGS__)           \
        GENERATE_CASE(name, Class, __VA_ARGS__)           \
        GENERATE_CASE(name, Complex, __VA_ARGS__)         \
        GENERATE_CASE(name, Const, __VA_ARGS__)           \
        GENERATE_CASE(name, ConstLhs, __VA_ARGS__)        \
        GENERATE_CASE(name, CSend, __VA_ARGS__)           \
        GENERATE_CASE(name, CVar, __VA_ARGS__)            \
        GENERATE_CASE(name, CVarLhs, __VA_ARGS__)         \
        GENERATE_CASE(name, DefMethod, __VA_ARGS__)       \
        GENERATE_CASE(name, Defined, __VA_ARGS__)         \
        GENERATE_CASE(name, DefS, __VA_ARGS__)            \
        GENERATE_CASE(name, DString, __VA_ARGS__)         \
        GENERATE_CASE(name, DSymbol, __VA_ARGS__)         \
        GENERATE_CASE(name, EFlipflop, __VA_ARGS__)       \
        GENERATE_CASE(name, EncodingLiteral, __VA_ARGS__) \
        GENERATE_CASE(name, Ensure, __VA_ARGS__)          \
        GENERATE_CASE(name, ERange, __VA_ARGS__)          \
        GENERATE_CASE(name, False, __VA_ARGS__)           \
        GENERATE_CASE(name, FileLiteral, __VA_ARGS__)     \
        GENERATE_CASE(name, For, __VA_ARGS__)             \
        GENERATE_CASE(name, Float, __VA_ARGS__)           \
        GENERATE_CASE(name, GVar, __VA_ARGS__)            \
        GENERATE_CASE(name, GVarLhs, __VA_ARGS__)         \
        GENERATE_CASE(name, Hash, __VA_ARGS__)            \
        GENERATE_CASE(name, Ident, __VA_ARGS__)           \
        GENERATE_CASE(name, If, __VA_ARGS__)              \
        GENERATE_CASE(name, IFlipflop, __VA_ARGS__)       \
        GENERATE_CASE(name, IRange, __VA_ARGS__)          \
        GENERATE_CASE(name, Integer, __VA_ARGS__)         \
        GENERATE_CASE(name, IVar, __VA_ARGS__)            \
        GENERATE_CASE(name, IVarLhs, __VA_ARGS__)         \
        GENERATE_CASE(name, Kwarg, __VA_ARGS__)           \
        GENERATE_CASE(name, Kwnilarg, __VA_ARGS__)        \
        GENERATE_CASE(name, Kwbegin, __VA_ARGS__)         \
        GENERATE_CASE(name, Kwoptarg, __VA_ARGS__)        \
        GENERATE_CASE(name, Kwrestarg, __VA_ARGS__)       \
        GENERATE_CASE(name, Kwsplat, __VA_ARGS__)         \
        GENERATE_CASE(name, LineLiteral, __VA_ARGS__)     \
        GENERATE_CASE(name, LVar, __VA_ARGS__)            \
        GENERATE_CASE(name, LVarLhs, __VA_ARGS__)         \
        GENERATE_CASE(name, MatchAsgn, __VA_ARGS__)       \
        GENERATE_CASE(name, MatchCurLine, __VA_ARGS__)    \
        GENERATE_CASE(name, Masgn, __VA_ARGS__)           \
        GENERATE_CASE(name, Mlhs, __VA_ARGS__)            \
        GENERATE_CASE(name, Module, __VA_ARGS__)          \
        GENERATE_CASE(name, Next, __VA_ARGS__)            \
        GENERATE_CASE(name, Nil, __VA_ARGS__)             \
        GENERATE_CASE(name, NthRef, __VA_ARGS__)          \
        GENERATE_CASE(name, NumParams, __VA_ARGS__)       \
        GENERATE_CASE(name, NumBlock, __VA_ARGS__)        \
        GENERATE_CASE(name, OpAsgn, __VA_ARGS__)          \
        GENERATE_CASE(name, Or, __VA_ARGS__)              \
        GENERATE_CASE(name, OrAsgn, __VA_ARGS__)          \
        GENERATE_CASE(name, Optarg, __VA_ARGS__)          \
        GENERATE_CASE(name, Pair, __VA_ARGS__)            \
        GENERATE_CASE(name, Postexe, __VA_ARGS__)         \
        GENERATE_CASE(name, Preexe, __VA_ARGS__)          \
        GENERATE_CASE(name, Procarg0, __VA_ARGS__)        \
        GENERATE_CASE(name, Rational, __VA_ARGS__)        \
        GENERATE_CASE(name, Redo, __VA_ARGS__)            \
        GENERATE_CASE(name, Regexp, __VA_ARGS__)          \
        GENERATE_CASE(name, Regopt, __VA_ARGS__)          \
        GENERATE_CASE(name, Resbody, __VA_ARGS__)         \
        GENERATE_CASE(name, Rescue, __VA_ARGS__)          \
        GENERATE_CASE(name, Restarg, __VA_ARGS__)         \
        GENERATE_CASE(name, Retry, __VA_ARGS__)           \
        GENERATE_CASE(name, Return, __VA_ARGS__)          \
        GENERATE_CASE(name, SClass, __VA_ARGS__)          \
        GENERATE_CASE(name, Self, __VA_ARGS__)            \
        GENERATE_CASE(name, Send, __VA_ARGS__)            \
        GENERATE_CASE(name, Shadowarg, __VA_ARGS__)       \
        GENERATE_CASE(name, Splat, __VA_ARGS__)           \
        GENERATE_CASE(name, SplatLhs, __VA_ARGS__)        \
        GENERATE_CASE(name, String, __VA_ARGS__)          \
        GENERATE_CASE(name, Super, __VA_ARGS__)           \
        GENERATE_CASE(name, Symbol, __VA_ARGS__)          \
        GENERATE_CASE(name, True, __VA_ARGS__)            \
        GENERATE_CASE(name, Undef, __VA_ARGS__)           \
        GENERATE_CASE(name, Until, __VA_ARGS__)           \
        GENERATE_CASE(name, UntilPost, __VA_ARGS__)       \
        GENERATE_CASE(name, When, __VA_ARGS__)            \
        GENERATE_CASE(name, While, __VA_ARGS__)           \
        GENERATE_CASE(name, WhilePost, __VA_ARGS__)       \
        GENERATE_CASE(name, XString, __VA_ARGS__)         \
        GENERATE_CASE(name, Yield, __VA_ARGS__)           \
        GENERATE_CASE(name, ZSuper, __VA_ARGS__)          \
    }

string NodePtr::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    if (store == 0) {
        return "NULL";
    }
    TAG_SWITCH(toStringWithTabs, gs, tabs);
}

string NodePtr::toJSON(const core::GlobalState &gs, int tabs) {
    if (store == 0) {
        return "null";
    }
    TAG_SWITCH(toJSON, gs, tabs);
}

string NodePtr::toWhitequark(const core::GlobalState &gs, int tabs) {
    if (store == 0) {
        return "nil";
    }
    TAG_SWITCH(toWhitequark, gs, tabs);
}

} // namespace sorbet::parser
