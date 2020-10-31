#include "parser/parser.h"
#include <algorithm>
#include <iterator>

using namespace std;

namespace sorbet::parser {

void NodePtr::printTabs(fmt::memory_buffer &to, int count) {
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

#define GENERATE_CASE_CALL(name, type, ...) \
    case NodePtr::Tag::type:                \
        return NodePtr::cast_nonnull<type>(*this).name(__VA_ARGS__);

#define TAG_SWITCH(GENERATOR, tag, name, ...)         \
    switch (tag) {                                    \
        GENERATOR(name, Alias, __VA_ARGS__)           \
        GENERATOR(name, And, __VA_ARGS__)             \
        GENERATOR(name, AndAsgn, __VA_ARGS__)         \
        GENERATOR(name, Arg, __VA_ARGS__)             \
        GENERATOR(name, Args, __VA_ARGS__)            \
        GENERATOR(name, Array, __VA_ARGS__)           \
        GENERATOR(name, Backref, __VA_ARGS__)         \
        GENERATOR(name, Assign, __VA_ARGS__)          \
        GENERATOR(name, Begin, __VA_ARGS__)           \
        GENERATOR(name, Block, __VA_ARGS__)           \
        GENERATOR(name, Blockarg, __VA_ARGS__)        \
        GENERATOR(name, BlockPass, __VA_ARGS__)       \
        GENERATOR(name, Break, __VA_ARGS__)           \
        GENERATOR(name, Case, __VA_ARGS__)            \
        GENERATOR(name, Cbase, __VA_ARGS__)           \
        GENERATOR(name, Class, __VA_ARGS__)           \
        GENERATOR(name, Complex, __VA_ARGS__)         \
        GENERATOR(name, Const, __VA_ARGS__)           \
        GENERATOR(name, ConstLhs, __VA_ARGS__)        \
        GENERATOR(name, CSend, __VA_ARGS__)           \
        GENERATOR(name, CVar, __VA_ARGS__)            \
        GENERATOR(name, CVarLhs, __VA_ARGS__)         \
        GENERATOR(name, DefMethod, __VA_ARGS__)       \
        GENERATOR(name, Defined, __VA_ARGS__)         \
        GENERATOR(name, DefS, __VA_ARGS__)            \
        GENERATOR(name, DString, __VA_ARGS__)         \
        GENERATOR(name, DSymbol, __VA_ARGS__)         \
        GENERATOR(name, EFlipflop, __VA_ARGS__)       \
        GENERATOR(name, EncodingLiteral, __VA_ARGS__) \
        GENERATOR(name, Ensure, __VA_ARGS__)          \
        GENERATOR(name, ERange, __VA_ARGS__)          \
        GENERATOR(name, False, __VA_ARGS__)           \
        GENERATOR(name, FileLiteral, __VA_ARGS__)     \
        GENERATOR(name, For, __VA_ARGS__)             \
        GENERATOR(name, Float, __VA_ARGS__)           \
        GENERATOR(name, GVar, __VA_ARGS__)            \
        GENERATOR(name, GVarLhs, __VA_ARGS__)         \
        GENERATOR(name, Hash, __VA_ARGS__)            \
        GENERATOR(name, Ident, __VA_ARGS__)           \
        GENERATOR(name, If, __VA_ARGS__)              \
        GENERATOR(name, IFlipflop, __VA_ARGS__)       \
        GENERATOR(name, IRange, __VA_ARGS__)          \
        GENERATOR(name, Integer, __VA_ARGS__)         \
        GENERATOR(name, IVar, __VA_ARGS__)            \
        GENERATOR(name, IVarLhs, __VA_ARGS__)         \
        GENERATOR(name, Kwarg, __VA_ARGS__)           \
        GENERATOR(name, Kwnilarg, __VA_ARGS__)        \
        GENERATOR(name, Kwbegin, __VA_ARGS__)         \
        GENERATOR(name, Kwoptarg, __VA_ARGS__)        \
        GENERATOR(name, Kwrestarg, __VA_ARGS__)       \
        GENERATOR(name, Kwsplat, __VA_ARGS__)         \
        GENERATOR(name, LineLiteral, __VA_ARGS__)     \
        GENERATOR(name, LVar, __VA_ARGS__)            \
        GENERATOR(name, LVarLhs, __VA_ARGS__)         \
        GENERATOR(name, MatchAsgn, __VA_ARGS__)       \
        GENERATOR(name, MatchCurLine, __VA_ARGS__)    \
        GENERATOR(name, Masgn, __VA_ARGS__)           \
        GENERATOR(name, Mlhs, __VA_ARGS__)            \
        GENERATOR(name, Module, __VA_ARGS__)          \
        GENERATOR(name, Next, __VA_ARGS__)            \
        GENERATOR(name, Nil, __VA_ARGS__)             \
        GENERATOR(name, NthRef, __VA_ARGS__)          \
        GENERATOR(name, NumParams, __VA_ARGS__)       \
        GENERATOR(name, NumBlock, __VA_ARGS__)        \
        GENERATOR(name, OpAsgn, __VA_ARGS__)          \
        GENERATOR(name, Or, __VA_ARGS__)              \
        GENERATOR(name, OrAsgn, __VA_ARGS__)          \
        GENERATOR(name, Optarg, __VA_ARGS__)          \
        GENERATOR(name, Pair, __VA_ARGS__)            \
        GENERATOR(name, Postexe, __VA_ARGS__)         \
        GENERATOR(name, Preexe, __VA_ARGS__)          \
        GENERATOR(name, Procarg0, __VA_ARGS__)        \
        GENERATOR(name, Rational, __VA_ARGS__)        \
        GENERATOR(name, Redo, __VA_ARGS__)            \
        GENERATOR(name, Regexp, __VA_ARGS__)          \
        GENERATOR(name, Regopt, __VA_ARGS__)          \
        GENERATOR(name, Resbody, __VA_ARGS__)         \
        GENERATOR(name, Rescue, __VA_ARGS__)          \
        GENERATOR(name, Restarg, __VA_ARGS__)         \
        GENERATOR(name, Retry, __VA_ARGS__)           \
        GENERATOR(name, Return, __VA_ARGS__)          \
        GENERATOR(name, SClass, __VA_ARGS__)          \
        GENERATOR(name, Self, __VA_ARGS__)            \
        GENERATOR(name, Send, __VA_ARGS__)            \
        GENERATOR(name, Shadowarg, __VA_ARGS__)       \
        GENERATOR(name, Splat, __VA_ARGS__)           \
        GENERATOR(name, SplatLhs, __VA_ARGS__)        \
        GENERATOR(name, String, __VA_ARGS__)          \
        GENERATOR(name, Super, __VA_ARGS__)           \
        GENERATOR(name, Symbol, __VA_ARGS__)          \
        GENERATOR(name, True, __VA_ARGS__)            \
        GENERATOR(name, Undef, __VA_ARGS__)           \
        GENERATOR(name, Until, __VA_ARGS__)           \
        GENERATOR(name, UntilPost, __VA_ARGS__)       \
        GENERATOR(name, When, __VA_ARGS__)            \
        GENERATOR(name, While, __VA_ARGS__)           \
        GENERATOR(name, WhilePost, __VA_ARGS__)       \
        GENERATOR(name, XString, __VA_ARGS__)         \
        GENERATOR(name, Yield, __VA_ARGS__)           \
        GENERATOR(name, ZSuper, __VA_ARGS__)          \
    }

string NodePtr::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    if (store == 0) {
        return "NULL";
    }
    TAG_SWITCH(GENERATE_CASE_CALL, this->tag(), toStringWithTabs, gs, tabs);
}

string NodePtr::toJSON(const core::GlobalState &gs, int tabs) {
    if (store == 0) {
        return "null";
    }
    TAG_SWITCH(GENERATE_CASE_CALL, this->tag(), toJSON, gs, tabs);
}

string NodePtr::toWhitequark(const core::GlobalState &gs, int tabs) {
    if (store == 0) {
        return "nil";
    }
    TAG_SWITCH(GENERATE_CASE_CALL, this->tag(), toWhitequark, gs, tabs);
}

#define GENERATE_CASE_GET_FIELD(name, type, ...) \
    case NodePtr::Tag::type:                     \
        return NodePtr::cast_nonnull<type>(*this).name;

core::LocOffsets NodePtr::loc() const {
    if (store == 0) {
        return core::LocOffsets::none();
    }
    TAG_SWITCH(GENERATE_CASE_GET_FIELD, this->tag(), loc);
}

string NodePtr::nodeName() {
    ENFORCE_NO_TIMER(store != 0);
    TAG_SWITCH(GENERATE_CASE_CALL, this->tag(), nodeName)
}

#undef GENERATE_CASE_GET_FIELD

#define GENERATE_CASE_SET_FIELD(name, type, value, ...)  \
    case NodePtr::Tag::type:                             \
        NodePtr::cast_nonnull<type>(*this).name = value; \
        break;

void NodePtr::setLoc(core::LocOffsets loc) {
    ENFORCE(store != 0);
    TAG_SWITCH(GENERATE_CASE_SET_FIELD, this->tag(), loc, loc)
}

#undef GENERATE_CASE_SET_FIELD

#define GENERATE_CASE_DELETE(name, type, ...) \
    case NodePtr::Tag::type:                  \
        delete reinterpret_cast<type *>(ptr); \
        break;

void NodePtr::deleteTagged(Tag tag, void *ptr) noexcept {
    ENFORCE_NO_TIMER(ptr != nullptr);
    TAG_SWITCH(GENERATE_CASE_DELETE, tag, unused)
}

#undef GENERATE_CASE_DELETE

} // namespace sorbet::parser
