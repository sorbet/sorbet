#ifndef SORBET_CORE_GLOBAL_SUBSTITUTION_H
#define SORBET_CORE_GLOBAL_SUBSTITUTION_H

#include "common/common.h"
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
        if (!from.exists()) {
            return from;
        }
        switch (from.kind()) {
            case NameRef::Kind::CONSTANT:
                ENFORCE(from.constantIndex() < constantNameSubstitution.size(),
                        "name substitution index out of bounds, got {} where subsitution size is {}",
                        std::to_string(from.constantIndex()), std::to_string(constantNameSubstitution.size()));
                return constantNameSubstitution[from.constantIndex()];
            case NameRef::Kind::UNIQUE:
                ENFORCE(from.uniqueIndex() < uniqueNameSubstitution.size(),
                        "name substitution index out of bounds, got {} where subsitution size is {}",
                        std::to_string(from.uniqueIndex()), std::to_string(uniqueNameSubstitution.size()));
                return uniqueNameSubstitution[from.uniqueIndex()];
            case NameRef::Kind::UTF8:
                ENFORCE(from.utf8Index() < utf8NameSubstitution.size(),
                        "name substitution index out of bounds, got {} where subsitution size is {}",
                        std::to_string(from.utf8Index()), std::to_string(utf8NameSubstitution.size()));
                return utf8NameSubstitution[from.utf8Index()];
        }
    }

    bool useFastPath() const;

private:
    friend NameRefDebugCheck;

    std::vector<NameRef> uniqueNameSubstitution;
    std::vector<NameRef> utf8NameSubstitution;
    std::vector<NameRef> constantNameSubstitution;
    // set if no substitution is actually necessary
    bool fastPath;

    const int toGlobalStateId;
};

} // namespace sorbet::core

#endif
