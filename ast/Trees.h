#ifndef SORBET_TREES_H
#define SORBET_TREES_H

#include "common/common.h"
#include "core/Context.h"
#include "core/FoundDefinitions.h"
#include "core/KwPair.h"
#include "core/LocalVariable.h"
#include "core/SymbolRef.h"
#include "core/Types.h"
#include "core/UntaggedPtr.h"
#include <memory>
#include <vector>

//
// This file defines the IR that most of the middle phases of Sorbet operate on
// and manipulate. It aims to be a middle ground between the parser output
// (very verbose and fine grained) and the CFG data structure (very easy to
// typecheck but very hard to do ad-hoc transformations on).
//
// This IR is best learned by example. Try using the `--print` option to sorbet
// on a handful of test/testdata files. Since there are multiple phases that
// return this IR, there are multiple valid print options which will show you
// an ast::Expression.
//
// Another good way to discover things is to grep for the class name in the
// various *-raw.exp snapshot tests to fine a test file that uses that node.
// Keep in mind that this IR is meant to be somewhat coarse grained, so one
// node type can likely have been created from multiple Ruby constructs.
//

namespace sorbet {
class WorkerPool;
}

namespace sorbet::ast {

enum class Tag {
    EmptyTree = 1,
    Send,
    ClassDef,
    MethodDef,
    If,
    While,
    Break,
    Retry,
    Next,
    Return,
    RescueCase,
    Rescue,
    Local,
    UnresolvedIdent,
    RestParam,
    KeywordArg,
    OptionalParam,
    BlockParam,
    ShadowArg,
    Assign,
    Cast,
    Hash,
    Array,
    Literal,
    UnresolvedConstantLit,
    ConstantLit,
    ZSuperArgs,
    Block,
    InsSeq,
    RuntimeMethodDefinition,
    Self,
};

// A mapping from tree type to its corresponding tag.
template <typename T> struct ExpressionToTag;

class EmptyTree;

class ExpressionPtr {
public:
    // We store tagged pointers as 64-bit values.
    using tagged_storage = uint64_t;

    // Required for typecase
    template <class To> static bool isa(const ExpressionPtr &tree);
    template <class To> static const To &cast(const ExpressionPtr &tree);
    template <class To> static To &cast(ExpressionPtr &tree) {
        return const_cast<To &>(cast<To>(static_cast<const ExpressionPtr &>(tree)));
    }

private:
    static constexpr tagged_storage TAG_MASK = 0xffff;

    static constexpr tagged_storage PTR_MASK = ~TAG_MASK;

    tagged_storage ptr;

    template <typename E, typename... Args> friend ExpressionPtr make_expression(Args &&...);

    static tagged_storage tagPtr(Tag tag, void *expr) {
        ENFORCE(static_cast<size_t>(tag) != 0);
        ENFORCE(expr != nullptr);

        // Store the tag in the lower 16 bits of the pointer, regardless of size.
        auto val = static_cast<tagged_storage>(tag);
        auto maskedPtr = reinterpret_cast<tagged_storage>(expr) << 16;

        return maskedPtr | val;
    }

    ExpressionPtr(Tag tag, void *expr) : ptr(tagPtr(tag, expr)) {}

    static void deleteTagged(Tag tag, void *ptr) noexcept;

    // A version of release that doesn't mask the tag bits
    tagged_storage releaseTagged() noexcept {
        auto saved = ptr;
        ptr = 0;
        return saved;
    }

    // A version of reset that expects the tagged bits to be set.
    void resetTagged(tagged_storage expr) noexcept {
        Tag tagVal;
        void *saved = nullptr;

        if (ptr != 0) {
            tagVal = tag();
            saved = get();
        }

        ptr = expr;

        if (saved != nullptr) {
            deleteTagged(tagVal, saved);
        }
    }

    uint16_t tagMaybeZero() const noexcept {
        return ptr & TAG_MASK;
    }

public:
    constexpr ExpressionPtr() noexcept : ptr(0) {}

    ExpressionPtr(std::nullptr_t) noexcept : ExpressionPtr() {}

    ~ExpressionPtr() {
        if (ptr != 0) {
            deleteTagged(tag(), get());
        }
    }

    ExpressionPtr(const ExpressionPtr &) = delete;
    ExpressionPtr &operator=(const ExpressionPtr &) = delete;

    ExpressionPtr(ExpressionPtr &&other) noexcept {
        ptr = other.releaseTagged();
    }

    ExpressionPtr &operator=(ExpressionPtr &&other) noexcept {
        if (*this == other) {
            return *this;
        }

        resetTagged(other.releaseTagged());
        return *this;
    }

    void *release() noexcept {
        auto *saved = get();
        ptr = 0;
        return saved;
    }

    void reset() noexcept {
        resetTagged(0);
    }

    void reset(std::nullptr_t) noexcept {
        resetTagged(0);
    }

    void resetToEmpty(EmptyTree *expr) noexcept;

    Tag tag() const noexcept {
        ENFORCE(ptr != 0);

        auto value = reinterpret_cast<tagged_storage>(ptr) & TAG_MASK;
        return static_cast<Tag>(value);
    }

    void *get() const noexcept {
        auto val = ptr & PTR_MASK;
        return reinterpret_cast<void *>(val >> 16);
    }

    explicit operator bool() const noexcept {
        return ptr != 0;
    }

    bool operator==(const ExpressionPtr &other) const noexcept {
        return ptr == other.ptr;
    }

    bool operator==(std::nullptr_t) const noexcept {
        return ptr == 0;
    }

    bool operator!=(const ExpressionPtr &other) const noexcept {
        return ptr != other.ptr;
    }

    bool operator!=(std::nullptr_t) const noexcept {
        return ptr != 0;
    }

    template <class To> core::UntaggedPtr<To> as_tree() {
        bool isValid = tagMaybeZero() == uint16_t(ExpressionToTag<To>::value);
        auto *ptr = isValid ? reinterpret_cast<To *>(get()) : nullptr;
        return core::UntaggedPtr<To>(ptr, isValid);
    }

    template <class To> core::UntaggedPtr<const To> as_tree() const {
        bool isValid = tagMaybeZero() == uint16_t(ExpressionToTag<To>::value);
        auto *ptr = isValid ? reinterpret_cast<const To *>(get()) : nullptr;
        return core::UntaggedPtr<const To>(ptr, isValid);
    }

    ExpressionPtr deepCopy() const;
    // Compare the entire tree for equality, ignoring locations.
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    // Compare the entire tree for equality, including locations.
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string_view nodeName() const;

    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;

    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;

    bool isSelfReference() const;

    void _sanityCheck() const;

    core::LocOffsets loc() const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;

    std::string toString(const core::GlobalState &gs) const {
        return toStringWithTabs(gs);
    }

    void swap(ExpressionPtr &other) noexcept {
        using std::swap;
        swap(this->ptr, other.ptr);
    }

    template <typename E> static ExpressionPtr fromUnique(std::unique_ptr<E> ptr) noexcept {
        if (ptr == nullptr) {
            return nullptr;
        }
        return ExpressionPtr(ExpressionToTag<E>::value, ptr.release());
    }

    template <typename E> std::unique_ptr<E> toUnique() noexcept {
        if (this->ptr == 0) {
            return nullptr;
        }
        auto p = as_tree<E>();
        ENFORCE(p);
        release();
        return std::unique_ptr<E>(p.get());
    }
};

template <class E, typename... Args> ExpressionPtr make_expression(Args &&...args) {
    return ExpressionPtr(ExpressionToTag<E>::value, new E(std::forward<Args>(args)...));
}

struct ParsedFile {
    ExpressionPtr tree;
    core::FileRef file;

    struct Flags {
        // if 'true' file is completely cached in kvstore
        bool cached : 1;

        Flags() : cached{false} {}
    };

    Flags flags;

    ParsedFile() = default;
    ParsedFile(ast::ExpressionPtr tree, core::FileRef file) : tree{std::move(tree)}, file{file}, flags{} {}

    void swap(ParsedFile &other) noexcept {
        using std::swap;
        this->tree.swap(other.tree);
        swap(this->file, other.file);
    }

    bool cached() const {
        return this->flags.cached;
    }

    void setCached(bool cached) {
        this->flags.cached = cached;
    }
};

inline void swap(ParsedFile &a, ParsedFile &b) {
    a.swap(b);
}

/**
 * Stores a vector of `ParsedFile`s. May be empty if pass was canceled or encountered an error.
 * TODO: Modify to store reason if we ever have multiple reasons for a pass to stop. Currently, it's only empty if the
 * pass is canceled in LSP mode.
 */
class ParsedFilesOrCancelled final {
private:
    std::optional<std::vector<ParsedFile>> trees;
    ParsedFilesOrCancelled();

public:
    ParsedFilesOrCancelled(std::vector<ParsedFile> &&trees);

    // Produces a `ParsedFilesOrCancelled` in the cancelled state.
    // Trees passed to this function will be destructed in parallel with `workers`, which improves LSP responsiveness on
    // large projects.
    static ParsedFilesOrCancelled cancel(std::vector<ParsedFile> &&trees, WorkerPool &workers);

    bool hasResult() const;
    std::vector<ParsedFile> &result();
};

template <class To> bool isa_tree(const ExpressionPtr &what) {
    return bool(what.as_tree<To>());
}

bool isa_reference(const ExpressionPtr &what);

template <class To> core::UntaggedPtr<To> cast_tree(ExpressionPtr &what) {
    return what.as_tree<To>();
}

template <class To> core::UntaggedPtr<const To> cast_tree(const ExpressionPtr &what) {
    return what.as_tree<To>();
}

// We disallow casting on temporary values because the lifetime of the returned value is
// tied to the temporary, but it is possible for the temporary to be destroyed at the end
// of the current statement, leading to use-after-free bugs.
template <class To> To *cast_tree(ExpressionPtr &&what) = delete;

template <class To> To &cast_tree_nonnull(ExpressionPtr &what) {
    ENFORCE(isa_tree<To>(what), "cast_tree_nonnull failed!");
    return *reinterpret_cast<To *>(what.get());
}

template <class To> const To &cast_tree_nonnull(const ExpressionPtr &what) {
    ENFORCE(isa_tree<To>(what), "cast_tree_nonnull failed!");
    return *reinterpret_cast<To *>(what.get());
}

// We disallow casting on temporary values because the lifetime of the returned value is
// tied to the temporary, but it is possible for the temporary to be destroyed at the end
// of the current statement, leading to use-after-free bugs.
template <class To> To *cast_tree_nonnull(ExpressionPtr &&what) = delete;

template <class To> inline bool ExpressionPtr::isa(const ExpressionPtr &what) {
    return isa_tree<To>(what);
}

template <class To> inline To const &ExpressionPtr::cast(const ExpressionPtr &what) {
    return cast_tree_nonnull<To>(what);
}

template <> inline bool ExpressionPtr::isa<ExpressionPtr>(const ExpressionPtr &tree) {
    return true;
}

template <> inline const ExpressionPtr &ExpressionPtr::cast<ExpressionPtr>(const ExpressionPtr &tree) {
    return tree;
}

#define EXPRESSION(name)                        \
    class name;                                 \
    template <> struct ExpressionToTag<name> {  \
        static constexpr Tag value = Tag::name; \
    };                                          \
    class __attribute__((aligned(8))) name final

EXPRESSION(ClassDef) {
public:
    const core::LocOffsets loc;
    core::LocOffsets declLoc;
    core::ClassOrModuleRef symbol;

    enum class Kind : uint8_t {
        Module,
        Class,
    };
    Kind kind;

    static core::FoundClass::Kind kindToFoundClassKind(Kind kind);
    static constexpr int EXPECTED_RHS_COUNT = 4;
    using RHS_store = InlinedVector<ExpressionPtr, EXPECTED_RHS_COUNT>;

    RHS_store rhs;
    ExpressionPtr name;
    // For unresolved names. Once they are typeAlias to Symbols they go into the Symbol

    static constexpr int EXPECTED_ANCESTORS_COUNT = 2;
    using ANCESTORS_store = InlinedVector<ExpressionPtr, EXPECTED_ANCESTORS_COUNT>;

    ANCESTORS_store ancestors;
    ANCESTORS_store singletonAncestors;

    ClassDef(core::LocOffsets loc, core::LocOffsets declLoc, core::ClassOrModuleRef symbol, ExpressionPtr name,
             ANCESTORS_store ancestors, RHS_store rhs, ClassDef::Kind kind);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(ClassDef, 120, 8);

EXPRESSION(MethodDef) {
public:
    const core::LocOffsets loc;
    core::LocOffsets declLoc;
    core::MethodRef symbol;

    ExpressionPtr rhs;

    using PARAMS_store = InlinedVector<ExpressionPtr, core::SymbolRef::EXPECTED_METHOD_PARAMS_COUNT>;
    PARAMS_store params;

    core::NameRef name;

    using Flags = core::FoundMethod::Flags;
    Flags flags;

    MethodDef(core::LocOffsets loc, core::LocOffsets declLoc, core::MethodRef symbol, core::NameRef name,
              PARAMS_store params, ExpressionPtr rhs, Flags flags);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(MethodDef, 64, 8);

EXPRESSION(If) {
public:
    const core::LocOffsets loc;

    ExpressionPtr cond;
    ExpressionPtr thenp;
    ExpressionPtr elsep;

    If(core::LocOffsets loc, ExpressionPtr cond, ExpressionPtr thenp, ExpressionPtr elsep);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(If, 32, 8);

EXPRESSION(While) {
public:
    const core::LocOffsets loc;

    ExpressionPtr cond;
    ExpressionPtr body;

    While(core::LocOffsets loc, ExpressionPtr cond, ExpressionPtr body);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(While, 24, 8);

EXPRESSION(Break) {
public:
    const core::LocOffsets loc;

    ExpressionPtr expr;

    Break(core::LocOffsets loc, ExpressionPtr expr);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(Break, 16, 8);

EXPRESSION(Retry) {
public:
    const core::LocOffsets loc;

    Retry(core::LocOffsets loc);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(Retry, 8, 8);

EXPRESSION(Next) {
public:
    const core::LocOffsets loc;

    ExpressionPtr expr;

    Next(core::LocOffsets loc, ExpressionPtr expr);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(Next, 16, 8);

EXPRESSION(Return) {
public:
    const core::LocOffsets loc;

    ExpressionPtr expr;

    Return(core::LocOffsets loc, ExpressionPtr expr);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(Return, 16, 8);

EXPRESSION(RescueCase) {
public:
    const core::LocOffsets loc;

    static constexpr int EXPECTED_EXCEPTION_COUNT = 2;
    using EXCEPTION_store = InlinedVector<ExpressionPtr, EXPECTED_EXCEPTION_COUNT>;

    EXCEPTION_store exceptions;

    // If present, var is always an UnresolvedIdent[kind=Local] up until the
    // namer, at which point it is a Local.
    ExpressionPtr var;
    ExpressionPtr body;

    RescueCase(core::LocOffsets loc, EXCEPTION_store exceptions, ExpressionPtr var, ExpressionPtr body);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(RescueCase, 48, 8);

EXPRESSION(Rescue) {
public:
    const core::LocOffsets loc;

    static constexpr int EXPECTED_RESCUE_CASE_COUNT = 2;
    using RESCUE_CASE_store = InlinedVector<ExpressionPtr, EXPECTED_RESCUE_CASE_COUNT>;

    ExpressionPtr body;
    RESCUE_CASE_store rescueCases;
    ExpressionPtr else_;
    ExpressionPtr ensure;

    Rescue(core::LocOffsets loc, ExpressionPtr body, RESCUE_CASE_store rescueCases, ExpressionPtr else_,
           ExpressionPtr ensure);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(Rescue, 56, 8);

EXPRESSION(Local) {
public:
    const core::LocOffsets loc;

    core::LocalVariable localVariable;

    Local(core::LocOffsets loc, core::LocalVariable localVariable1);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(Local, 16, 8);

EXPRESSION(UnresolvedIdent) {
public:
    const core::LocOffsets loc;

    enum class Kind : uint8_t {
        Local,
        Instance,
        Class,
        Global,
    };
    core::NameRef name;
    Kind kind;

    UnresolvedIdent(core::LocOffsets loc, Kind kind, core::NameRef name);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(UnresolvedIdent, 16, 8);

EXPRESSION(RestParam) {
public:
    const core::LocOffsets loc;

    ExpressionPtr expr;

    RestParam(core::LocOffsets loc, ExpressionPtr arg);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(RestParam, 16, 8);

EXPRESSION(KeywordArg) {
public:
    const core::LocOffsets loc;

    ExpressionPtr expr;

    KeywordArg(core::LocOffsets loc, ExpressionPtr expr);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(KeywordArg, 16, 8);

EXPRESSION(OptionalParam) {
public:
    const core::LocOffsets loc;

    ExpressionPtr expr;
    ExpressionPtr default_;

    OptionalParam(core::LocOffsets loc, ExpressionPtr expr, ExpressionPtr default_);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(OptionalParam, 24, 8);

EXPRESSION(BlockParam) {
public:
    const core::LocOffsets loc;

    ExpressionPtr expr;

    BlockParam(core::LocOffsets loc, ExpressionPtr expr);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(BlockParam, 16, 8);

EXPRESSION(ShadowArg) {
public:
    const core::LocOffsets loc;

    ExpressionPtr expr;

    ShadowArg(core::LocOffsets loc, ExpressionPtr expr);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(ShadowArg, 16, 8);

EXPRESSION(Assign) {
public:
    const core::LocOffsets loc;

    ExpressionPtr lhs;
    ExpressionPtr rhs;

    Assign(core::LocOffsets loc, ExpressionPtr lhs, ExpressionPtr rhs);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(Assign, 24, 8);

class Block;
EXPRESSION(Send) {
public:
    const core::LocOffsets loc;

    core::NameRef fun;
    const core::LocOffsets funLoc;

    struct Flags {
        // True if the receiver was self (either implicit like `foo()` or explicit like `self.foo()`)
        //   - Prior to Ruby 2.7, it was illegal to call a private method with an explicit receiver.
        //   - As of Ruby 2.7, it became legal to call private methods on self, e.g. `self.foo()`.
        bool isPrivateOk : 1;
        bool isRewriterSynthesized : 1;
        bool hasBlock : 1;

        // In C++20 we can replace this with bit field initialzers
        Flags() : isPrivateOk(false), isRewriterSynthesized(false), hasBlock(false) {}
        bool operator==(const Flags &other) const noexcept {
            return isPrivateOk == other.isPrivateOk && isRewriterSynthesized == other.isRewriterSynthesized &&
                   hasBlock == other.hasBlock;
        }

        bool operator!=(const Flags &other) const noexcept {
            return !(*this == other);
        }
    };
    CheckSize(Flags, 1, 1);

    Flags flags;

private:
    uint16_t numPosArgs_;

public:
    ExpressionPtr recv;

    static constexpr int EXPECTED_ARGS_COUNT = 2;
    using ARGS_store = InlinedVector<ExpressionPtr, EXPECTED_ARGS_COUNT>;

private:
    // The arguments vector has the following layout:
    //
    // for n = numPosArgs, m = number of keyword arg pairs
    //
    // +--------------------------+-------------------------------+------------------+----------------+
    // | positional arguments     | interleaved keyword arg pairs | optional kwsplat | optional block |
    // +--------------------------+-------------------------------+------------------+----------------+
    // | pos_0, ... , pos_(n - 1) | sym_0, val_0, .. sym_m, val_m | value            | value          |
    // +--------------------------+-------------------------------+------------------+----------------+
    //
    // for the following send:
    //
    // > foo(a, b, c: 10, d: nil, &blk)
    //
    // the arguments vector would look like the following, with numPosArgs = 2:
    //
    // > <a, b, c, 10, d, nil, &blk>
    //
    // We make the args store private to avoid code that is tightly bound to this layout.
    ARGS_store args;

public:
    Send(core::LocOffsets loc, ExpressionPtr recv, core::NameRef fun, core::LocOffsets funLoc, uint16_t numPosArgs,
         ARGS_store args, Flags flags = {});

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    // Returns nullptr if no block present.
    const ast::Block *block() const;
    ast::Block *block();

    // Returns nullptr if no block present.
    const ExpressionPtr *rawBlock() const;
    ExpressionPtr *rawBlock();

    // Returns null if no kwsplat present.
    const ExpressionPtr *kwSplat() const;
    ExpressionPtr *kwSplat();

    void setBlock(ExpressionPtr block);

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    // Add the given positional argument as the last positional argument.
    void addPosArg(ExpressionPtr ptr);

    // Insert the given positional argument at the given position, shifting existing arguments over.
    void insertPosArg(uint16_t index, ExpressionPtr arg);

    // Returns the raw arguments vector. Please avoid using unless absolutely necessary; it is easier to query
    // arguments via methods on Send. It is a footgun.
    const ARGS_store &rawArgsDoNotUse() const {
        return args;
    }

    // Returns a loc covering the arguments in the send. Does not cover the block argument.
    core::LocOffsets argsLoc() const;

    uint16_t numPosArgs() const {
        return numPosArgs_;
    }

    bool onlyPosArgs() const {
        return numPosArgs_ == args.size();
    }

    // The number of keyword arguments in the Send.
    uint16_t numKwArgs() const {
        uint16_t range = args.size() - numPosArgs_ - (hasKwSplat() ? 1 : 0) - (hasBlock() ? 1 : 0);
        ENFORCE(range % 2 == 0);
        return range / 2;
    }

    // Get the ith positional argument.
    ExpressionPtr &getPosArg(uint16_t idx) {
        ENFORCE(idx < numPosArgs_);
        return args[idx];
    }

    const ExpressionPtr &getPosArg(uint16_t idx) const {
        ENFORCE(idx < numPosArgs_);
        return args[idx];
    }

    absl::Span<const ExpressionPtr> posArgs() const {
        return absl::MakeSpan(args.begin(), numPosArgs_);
    }

    absl::Span<ExpressionPtr> posArgs() {
        return absl::MakeSpan(args.begin(), numPosArgs_);
    }

    absl::Span<const ExpressionPtr> nonBlockArgs() const {
        return absl::MakeSpan(args.begin(), numNonBlockArgs());
    }

    absl::Span<ExpressionPtr> nonBlockArgs() {
        return absl::MakeSpan(args.begin(), numNonBlockArgs());
    }

    // Remove all arguments to the function, including the block argument.
    void clearArgs();

    core::KwPairSpan<ExpressionPtr> kwArgPairs() {
        const auto pairs = numKwArgs();
        auto i = args.begin() + numPosArgs_;
        return core::KwPairSpan<ExpressionPtr>{absl::MakeSpan(i, i + pairs * 2)};
    }

    core::KwPairSpan<const ExpressionPtr> kwArgPairs() const {
        const auto pairs = numKwArgs();
        auto i = args.begin() + numPosArgs_;
        return core::KwPairSpan<const ExpressionPtr>{absl::MakeSpan(i, i + pairs * 2)};
    }

    // Get the ith keyword argument key. Indices begin at 0, regardless of the number of positional arguments.
    ExpressionPtr &getKwKey(uint16_t argnum) {
        ENFORCE(argnum < numKwArgs());
        auto rawIdx = numPosArgs_ + (argnum * 2);
        return args[rawIdx];
    }

    const ExpressionPtr &getKwKey(uint16_t argnum) const {
        ENFORCE(argnum < numKwArgs());
        auto rawIdx = numPosArgs_ + (argnum * 2);
        return args[rawIdx];
    }

    // Get the ith keyword argument value. Indices begin at 0, regardless of the number of positional arguments.
    ExpressionPtr &getKwValue(uint16_t argnum) {
        ENFORCE(argnum < numKwArgs());
        auto rawIdx = numPosArgs_ + (argnum * 2) + 1;
        return args[rawIdx];
    }

    const ExpressionPtr &getKwValue(uint16_t argnum) const {
        ENFORCE(argnum < numKwArgs());
        auto rawIdx = numPosArgs_ + (argnum * 2) + 1;
        return args[rawIdx];
    }

    // True when this send contains a block argument.
    bool hasBlock() const {
        return flags.hasBlock;
    }

    // True when this send contains at least 1 position argument.
    bool hasPosArgs() const {
        return numPosArgs_ > 0;
    }

    // True when there are either keyword args, or a keyword splat.
    bool hasKwArgs() const {
        return args.size() > (numPosArgs_ + (hasBlock() ? 1 : 0));
    }

    // True when there is a keyword args splat present. hasKwSplat -> hasKwArgs, but not the other way around.
    bool hasKwSplat() const {
        return (args.size() - numPosArgs_ - (hasBlock() ? 1 : 0)) & 0x1;
    }

    // True when this send contains at least one of the following:
    // - a positional argument
    // - a keyword argument
    // - a keyword splat
    bool hasNonBlockArgs() const {
        return hasPosArgs() || hasKwArgs();
    }

    // Returns the number of non-block arguments. Keyword arguments are represented as two separate arguments (key and
    // value).
    uint16_t numNonBlockArgs() const {
        return numPosArgs_ + (numKwArgs() * 2) + (hasKwSplat() ? 1 : 0);
    }

    const ExpressionPtr &getNonBlockArg(uint16_t idx) const {
        ENFORCE(idx < args.size() - (hasBlock() ? 1 : 0));
        return args[idx];
    }

    ExpressionPtr &getNonBlockArg(uint16_t idx) {
        ENFORCE(idx < args.size() - (hasBlock() ? 1 : 0));
        return args[idx];
    }

    // Returns a new ast::Send with a different loc, receiver, and function.
    // _Moves_ the arguments from this Send into the new Send.
    // The original send turns into a Send with 0 arguments.
    ExpressionPtr withNewBody(core::LocOffsets loc, ExpressionPtr recv, core::NameRef fun);

    void _sanityCheck();
};
CheckSize(Send, 56, 8);

EXPRESSION(Cast) {
public:
    const core::LocOffsets loc;

    // The name of the cast operator.
    core::NameRef cast;

    core::TypePtr type;
    ExpressionPtr arg;

    ExpressionPtr typeExpr;

    Cast(core::LocOffsets loc, core::TypePtr ty, ExpressionPtr arg, core::NameRef cast, ExpressionPtr typeExpr);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(Cast, 40, 8);

EXPRESSION(Hash) {
public:
    const core::LocOffsets loc;

    static constexpr int EXPECTED_ENTRY_COUNT = 2;
    using ENTRY_store = InlinedVector<ExpressionPtr, EXPECTED_ENTRY_COUNT>;

    ENTRY_store keys;
    ENTRY_store values;

    Hash(core::LocOffsets loc, ENTRY_store keys, ENTRY_store values);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(Hash, 56, 8);

EXPRESSION(Array) {
public:
    const core::LocOffsets loc;

    static constexpr int EXPECTED_ENTRY_COUNT = 4;
    using ENTRY_store = InlinedVector<ExpressionPtr, EXPECTED_ENTRY_COUNT>;

    ENTRY_store elems;

    Array(core::LocOffsets loc, ENTRY_store elems);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(Array, 48, 8);

EXPRESSION(Literal) {
public:
    const core::LocOffsets loc;

    core::TypePtr value;

    Literal(core::LocOffsets loc, const core::TypePtr &value);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;
    bool isString() const;
    bool isSymbol() const;
    bool isName() const;
    bool isNil(const core::GlobalState &gs) const;
    core::NameRef asString() const;
    core::NameRef asSymbol() const;
    core::NameRef asName() const;
    bool isTrue(const core::GlobalState &gs) const;
    bool isFalse(const core::GlobalState &gs) const;

    void _sanityCheck();
};
CheckSize(Literal, 16, 8);

EXPRESSION(UnresolvedConstantLit) {
public:
    const core::LocOffsets loc;

    core::NameRef cnst;
    ExpressionPtr scope;

    UnresolvedConstantLit(core::LocOffsets loc, ExpressionPtr scope, core::NameRef cnst);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(UnresolvedConstantLit, 24, 8);

EXPRESSION(ConstantLit) {
private:
    // This is a lot of complexity, but codebases can have a large number of resolved constants,
    // so we put in some work to ensure that these take up the minimal amount of space possible.
    //
    // A ConstantLit can be in one of three states:
    //
    // 1. A known constant that we create ourselves (e.g. core::Symbols::root()).  This requires
    //    12 bytes of storage:
    //
    //    struct KnownSymbol {
    //        core::SymbolRef sym;
    //        core::LocOffsets loc;
    //    };
    //
    //    The loc is actually important to propagate information into later passes and to ensure
    //    that various ENFORCEs around loc consistency hold.
    //
    // 2. A resolved constant.  We cheat a little bit here because this is actually encompassing
    //    both a constant that we are attempting to resolve and a constant that we have finished
    //    resolving.  This requires 16 bytes of storage due to alignment requirements:
    //
    //    struct ResolvedSymbol {
    //        core::SymbolRef sym;
    //        std::unique_ptr<UnresolvedConstantLit> original;
    //    };
    //
    //    The loc for this constant is derivable from `original`.
    //
    // 3. A constant that we attempted to resolve, but resolution failed.  This requires 16
    //    bytes of storage.
    //
    //    struct FailedResolutionSymbol {
    //        std::unique_ptr<std::vector<core::SymbolRef>> resolutionScopes;
    //        std::unique_ptr<UnresolvedConstantLit> original;
    //    };
    //
    //    We transition the second case -- where the second case is representing a symbol that
    //    is in the process of resolving -- to this case, and then we never transition out.
    //
    // We can represent all three of these in 16 bytes of space by doing a bunch of manual
    // pointer tagging.
    enum class Tag : uint8_t {
        KnownSymbol = 1,
        ResolvedSymbol,
        FailedResolutionSymbol,
    };

    struct Storage {
        using tagged_storage = uint64_t;

        static constexpr tagged_storage TAG_MASK = 0xffff;
        static constexpr tagged_storage PTR_MASK = ~TAG_MASK;

        // This member is a tagged quantity; its upper 48 bits will be either a raw symbol
        // ID or a pointer to a vector of SymbolRefs representing the resolution scopes
        // for a constant that failed to resolve.
        //
        // The tag determines the value for both this member and ptr2.
        tagged_storage ptr1;
        // This member is either a LocOffsets (in the case of a known symbol) or a pointer
        // to the associated UnresolvedConstantLit.
        tagged_storage ptr2;

        tagged_storage untaggedPtr1() const noexcept {
            auto val = ptr1 & PTR_MASK;
            return val >> 16;
        }

        Tag tag() const {
            auto value = ptr1 & TAG_MASK;
            return static_cast<Tag>(value);
        }

        static tagged_storage tagValue(Tag tag, tagged_storage value) {
            auto tagval = static_cast<tagged_storage>(tag);
            auto shifted = value << 16;

            return shifted | tagval;
        }

        static tagged_storage tagSymbol(Tag tag, core::SymbolRef symbol) {
            return tagValue(tag, static_cast<tagged_storage>(symbol.rawId()));
        }

        static tagged_storage tagPtr(Tag tag, void *ptr) {
            return tagValue(tag, reinterpret_cast<tagged_storage>(ptr));
        }

        static tagged_storage allocateResolutionScopes() {
            return tagPtr(Tag::FailedResolutionSymbol, new std::vector<core::SymbolRef>());
        }

        Storage(core::LocOffsets loc, core::SymbolRef symbol) : ptr1(tagSymbol(Tag::KnownSymbol, symbol)) {
            static_assert(sizeof(loc) == sizeof(ptr2), "LocOffsets is too big to fit!");
            new (&ptr2) core::LocOffsets(loc);
        }

        Storage(core::SymbolRef symbol, std::unique_ptr<UnresolvedConstantLit> original)
            : ptr1(tagSymbol(Tag::ResolvedSymbol, symbol)), ptr2(reinterpret_cast<tagged_storage>(original.release())) {
        }

        ~Storage() {
            switch (tag()) {
                case Tag::KnownSymbol: {
                    // ptr1 just holds the raw ID for the symbol, so only delete LocOffsets.
                    auto *p = reinterpret_cast<core::LocOffsets *>(&ptr2);
                    p->~LocOffsets();
                    break;
                }
                case Tag::ResolvedSymbol: {
                    // ptr1 just holds the raw ID for the symbol.
                    auto *ucl = original();
                    delete ucl;
                    break;
                }
                case Tag::FailedResolutionSymbol: {
                    auto *scopes = resolutionScopes();
                    delete scopes;
                    auto *ucl = original();
                    delete ucl;
                    break;
                }
            }
        }

        UnresolvedConstantLit *original() {
            switch (tag()) {
                case Tag::KnownSymbol:
                    return nullptr;
                case Tag::ResolvedSymbol:
                case Tag::FailedResolutionSymbol:
                    return reinterpret_cast<UnresolvedConstantLit *>(ptr2);
            }
        }

        const UnresolvedConstantLit *original() const {
            switch (tag()) {
                case Tag::KnownSymbol:
                    return nullptr;
                case Tag::ResolvedSymbol:
                case Tag::FailedResolutionSymbol:
                    return reinterpret_cast<const UnresolvedConstantLit *>(ptr2);
            }
        }

        core::LocOffsets loc() const {
            switch (tag()) {
                case Tag::KnownSymbol:
                    return *reinterpret_cast<const core::LocOffsets *>(&ptr2);
                case Tag::ResolvedSymbol:
                case Tag::FailedResolutionSymbol:
                    return original()->loc;
            }
        }

        core::SymbolRef symbol() const {
            switch (tag()) {
                case Tag::KnownSymbol:
                case Tag::ResolvedSymbol:
                    return core::SymbolRef::fromRaw(static_cast<uint32_t>(untaggedPtr1()));
                case Tag::FailedResolutionSymbol:
                    return core::Symbols::StubModule();
            }
        }

        std::vector<core::SymbolRef> *resolutionScopes() {
            switch (tag()) {
                case Tag::KnownSymbol:
                case Tag::ResolvedSymbol:
                    return nullptr;
                case Tag::FailedResolutionSymbol:
                    return reinterpret_cast<std::vector<core::SymbolRef> *>(untaggedPtr1());
            }
        }

        const std::vector<core::SymbolRef> *resolutionScopes() const {
            switch (tag()) {
                case Tag::KnownSymbol:
                case Tag::ResolvedSymbol:
                    return nullptr;
                case Tag::FailedResolutionSymbol:
                    return reinterpret_cast<const std::vector<core::SymbolRef> *>(untaggedPtr1());
            }
        }

        void setSymbol(core::SymbolRef sym) {
            ENFORCE(tag() == Tag::ResolvedSymbol);
            ENFORCE(sym != core::Symbols::StubModule());
            ptr1 = tagSymbol(Tag::ResolvedSymbol, sym);
        }

        void markUnresolved() {
            ENFORCE(tag() == Tag::ResolvedSymbol);
            if (tag() == Tag::ResolvedSymbol) {
                ptr1 = allocateResolutionScopes();
                ENFORCE(tag() == Tag::FailedResolutionSymbol);
            }
        }
    };

    Storage storage;

public:
    ConstantLit(core::LocOffsets loc, core::SymbolRef symbol);
    ConstantLit(core::SymbolRef symbol, std::unique_ptr<UnresolvedConstantLit> original);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;
    std::optional<std::pair<core::SymbolRef, std::vector<core::NameRef>>> fullUnresolvedPath(core::Context ctx) const;

    core::LocOffsets loc() const {
        return storage.loc();
    }

    core::SymbolRef symbol() const {
        return storage.symbol();
    }

    void setSymbol(core::SymbolRef symbol) {
        storage.setSymbol(symbol);
    }

    std::vector<core::SymbolRef> *resolutionScopes() {
        return storage.resolutionScopes();
    }

    const std::vector<core::SymbolRef> *resolutionScopes() const {
        return storage.resolutionScopes();
    }

    // Marks this constant as unresolved and allocates a vector for `resolutionScopes`.
    void markUnresolved() {
        storage.markUnresolved();
    }

    UnresolvedConstantLit *original() {
        return storage.original();
    }

    const UnresolvedConstantLit *original() const {
        return storage.original();
    }

    void _sanityCheck();
};
CheckSize(ConstantLit, 16, 8);

EXPRESSION(ZSuperArgs) {
public:
    const core::LocOffsets loc;

    // null if no block passed
    ZSuperArgs(core::LocOffsets loc);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(ZSuperArgs, 8, 8);

EXPRESSION(Block) {
public:
    const core::LocOffsets loc;

    MethodDef::PARAMS_store params;
    ExpressionPtr body;

    Block(core::LocOffsets loc, MethodDef::PARAMS_store params, ExpressionPtr body);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;
    void _sanityCheck();
};
CheckSize(Block, 40, 8);

EXPRESSION(InsSeq) {
public:
    const core::LocOffsets loc;

    static constexpr int EXPECTED_STATS_COUNT = 4;
    using STATS_store = InlinedVector<ExpressionPtr, EXPECTED_STATS_COUNT>;
    // Statements
    STATS_store stats;

    // The distinguished final expression (determines return value)
    ExpressionPtr expr;

    InsSeq(core::LocOffsets locOffsets, STATS_store stats, ExpressionPtr expr);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(InsSeq, 56, 8);

EXPRESSION(RuntimeMethodDefinition) {
public:
    // This should be equivalent to MethodDef.declLoc.
    // It's called `loc` because every node in the AST has to have a `loc` field.
    const core::LocOffsets loc;
    core::NameRef name;
    const bool isSelfMethod;

    RuntimeMethodDefinition(core::LocOffsets loc, core::NameRef name, bool isSelfMethod);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(RuntimeMethodDefinition, 16, 8);

EXPRESSION(Self) {
public:
    const core::LocOffsets loc;

    Self(core::LocOffsets loc);

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(Self, 8, 8);

EXPRESSION(EmptyTree) {
public:
    const core::LocOffsets loc;

    EmptyTree();

    ExpressionPtr deepCopy() const;
    bool structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;
    bool prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) const;

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRaw(const core::GlobalState &gs, int tabs = 0) const;
    std::string showRawWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) const;
    std::string_view nodeName() const;

    void _sanityCheck();
};
CheckSize(EmptyTree, 8, 8);

// This specialization of make_expression exists to ensure that we only ever create one empty tree.
template <> ExpressionPtr make_expression<EmptyTree>();

/** https://git.corp.stripe.com/gist/nelhage/51564501674174da24822e60ad770f64
 *
 *  [] - prototype only
 *
 *                 / Control Flow <- while, if, for, break, next, retry, return, rescue, case
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

} // namespace sorbet::ast

#endif // SORBET_TREES_H
