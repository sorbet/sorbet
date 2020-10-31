#include "common/common.h"
#include "core/core.h"
#include <memory>
#include <string>

namespace sorbet::parser {

class NodePtr {
public:
    // We store tagged pointers as 64-bit values.
    using tagged_storage = u8;
    // Is generated.
    enum class Tag;

    // A mapping from type to its corresponding tag.
    template <typename T> struct TypeToTag;

    // Required for typecase.
    template <class To> static bool isa(const NodePtr &what) {
        return what != nullptr && TypeToTag<To>::value == what.tag();
    }

    template <class To> static To const &cast_nonnull(const NodePtr &what) {
        ENFORCE_NO_TIMER(isa<To>(what));
        return *reinterpret_cast<To *>(what.get());
    }

    template <class To> static To &cast_nonnull(NodePtr &what) {
        return const_cast<To &>(cast_nonnull<To>(static_cast<const NodePtr &>(what)));
    }

private:
    tagged_storage store;

    static constexpr tagged_storage TAG_MASK = 0xffff000000000007;

    static constexpr tagged_storage PTR_MASK = ~TAG_MASK;

    static tagged_storage tagPtr(Tag tag, void *expr) {
        auto val = static_cast<tagged_storage>(tag);
        if (val >= 8) {
            // Store the tag in the upper 16 bits of the pointer, as it won't fit in the lower three bits.
            val <<= 48;
        }

        auto maskedPtr = reinterpret_cast<tagged_storage>(expr) & PTR_MASK;

        return maskedPtr | val;
    }

    NodePtr(Tag tag, void *expr) : store(tagPtr(tag, expr)) {}

    static void deleteTagged(Tag tag, void *ptr) noexcept;

    // Note: Doesn't mask the tag bits
    tagged_storage releaseTagged() noexcept {
        auto saved = store;
        store = 0;
        return saved;
    }

public:
    constexpr NodePtr() noexcept : store(0) {}

    NodePtr(std::nullptr_t) noexcept : NodePtr() {}

    NodePtr(NodePtr &&other) noexcept : store(other.releaseTagged()){};

    NodePtr(const NodePtr &other) = delete;

    ~NodePtr() {
        if (store != 0) {
            deleteTagged(this->tag(), this->get());
        }
    }

    NodePtr &operator=(NodePtr &&other) noexcept {
        if (this->store != 0) {
            deleteTagged(this->tag(), this->get());
        }
        store = other.releaseTagged();
        return *this;
    };

    NodePtr &operator=(const NodePtr &other) = delete;

    operator bool() const {
        return (bool)store;
    }

    Tag tag() const noexcept {
        ENFORCE_NO_TIMER(store != 0);

        auto value = reinterpret_cast<tagged_storage>(store) & TAG_MASK;
        if (value <= 7) {
            return static_cast<Tag>(value);
        } else {
            return static_cast<Tag>(value >> 48);
        }
    }

    void *get() const {
        auto val = store & PTR_MASK;
        if constexpr (sizeof(void *) == 4) {
            return reinterpret_cast<void *>(val);
        } else {
            // sign extension for the upper 16 bits
            return reinterpret_cast<void *>((val << 16) >> 16);
        }
    }
    bool operator!=(const NodePtr &other) const {
        return store != other.store;
    }
    bool operator==(const NodePtr &other) const {
        return store == other.store;
    }
    bool operator!=(std::nullptr_t n) const {
        return store != 0;
    }
    bool operator==(std::nullptr_t n) const {
        return store == 0;
    }

    void swap(NodePtr &n) {
        auto temp = this->store;
        this->store = n.store;
        n.store = temp;
    }

    static void printTabs(fmt::memory_buffer &to, int count);
    void printNode(fmt::memory_buffer &to, const core::GlobalState &gs, int tabs) const;
    void printNodeJSON(fmt::memory_buffer &to, const core::GlobalState &gs, int tabs);
    void printNodeWhitequark(fmt::memory_buffer &to, const core::GlobalState &gs, int tabs);
    core::LocOffsets loc() const;
    void setLoc(core::LocOffsets loc);
    std::string nodeName();

    std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    std::string toString(const core::GlobalState &gs) const {
        return toStringWithTabs(gs);
    }
    std::string toJSON(const core::GlobalState &gs, int tabs = 0);
    std::string toWhitequark(const core::GlobalState &gs, int tabs = 0);

    template <class T, class... Args> friend NodePtr make_node(Args &&... args);
};
CheckSize(NodePtr, 8, 8);

template <> inline bool NodePtr::isa<NodePtr>(const NodePtr &what) {
    return true;
}

template <> inline NodePtr const &NodePtr::cast_nonnull<NodePtr>(const NodePtr &what) {
    return what;
}

template <class To> To *cast_node(NodePtr &what) {
    if (NodePtr::isa<To>(what)) {
        return &NodePtr::cast_nonnull<To>(what);
    }
    return nullptr;
}

template <class To> bool isa_node(const NodePtr &what) {
    return NodePtr::isa<To>(what);
}

template <class T, class... Args> NodePtr make_node(Args &&... args) {
    return NodePtr(NodePtr::TypeToTag<T>::value, new T(std::forward<Args>(args)...));
}

using NodeVec = InlinedVector<NodePtr, 4>;

#include "parser/Node_gen.h"
}; // namespace sorbet::parser
