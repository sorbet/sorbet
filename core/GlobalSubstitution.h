#ifndef SORBET_CORE_GLOBAL_SUBSTITUTION_H
#define SORBET_CORE_GLOBAL_SUBSTITUTION_H

#include "common/common.h"
#include "core/NameHash.h"
#include "core/NameRef.h"
#include "core/SymbolRef.h"
#include <vector>

namespace sorbet::core {
class GlobalState;

class GlobalSubstitution {
public:
    GlobalSubstitution(const GlobalState &from, GlobalState &to, const GlobalState *optionalCommonParent = nullptr);

    NameRef substitute(NameRef from, bool allowSameFromTo = false) const {
        if (!allowSameFromTo) {
            from.sanityCheckSubstitution(*this);
        }
        ENFORCE(from._id < nameSubstitution.size(),
                "name substitution index out of bounds, got {} where subsitution size is {}", std::to_string(from._id),
                std::to_string(nameSubstitution.size()));
        return nameSubstitution[from._id];
    }

    NameRef substituteConstant(NameRef from, bool allowSameFromTo = false) const {
        return substitute(from, allowSameFromTo);
    }

    NameRef substituteSend(NameRef from, bool allowSameFromTo = false) const {
        return substitute(from, allowSameFromTo);
    }

    bool useFastPath() const;

private:
    friend NameRefDebugCheck;

    std::vector<NameRef> nameSubstitution;
    // set if no substitution is actually necessary
    bool fastPath;

    const int toGlobalStateId;
};

/**
 * GlobalSubstitution, but lazily populates `nameSubstitution`.
 */
class LazyGlobalSubstitution final {
    const core::GlobalState &fromGS;
    core::GlobalState &toGS;

    UnorderedMap<core::NameRef, core::NameRef> nameSubstitution;
    core::UsageHash acc;

    void defineName(NameRef from, NameRef &to, bool allowSameFromTo);

public:
    LazyGlobalSubstitution(const GlobalState &fromGS, GlobalState &toGS);
    ~LazyGlobalSubstitution() = default;

    NameRef substitute(NameRef from, bool allowSameFromTo = false) {
        auto &ref = nameSubstitution[from];
        if (!ref.exists() && from.exists()) {
            defineName(from, ref, allowSameFromTo);
        }
        return ref;
    }

    NameRef substituteConstant(NameRef from, bool allowSameFromTo = false) {
        acc.constants.emplace_back(fromGS, from.data(fromGS));
        return substitute(from, allowSameFromTo);
    }

    NameRef substituteSend(NameRef from, bool allowSameFromTo = false) {
        acc.sends.emplace_back(fromGS, from.data(fromGS));
        return substitute(from, allowSameFromTo);
    }

    core::UsageHash getAllNames();
};

} // namespace sorbet::core

#endif
