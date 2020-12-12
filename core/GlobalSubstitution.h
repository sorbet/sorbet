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
        switch (from.kind()) {
            case NameKind::UTF8:
                ENFORCE(from.unsafeTableIndex() < utf8NameSubstitution.size(),
                        "utf8 name substitution index out of bounds, got {} where subsitution size is {}",
                        std::to_string(from.rawId()), std::to_string(utf8NameSubstitution.size()));
                return utf8NameSubstitution[from.unsafeTableIndex()];
            case NameKind::CONSTANT:
                ENFORCE(from.unsafeTableIndex() < constantNameSubstitution.size(),
                        "constant name substitution index out of bounds, got {} where subsitution size is {}",
                        std::to_string(from.rawId()), std::to_string(constantNameSubstitution.size()));
                return constantNameSubstitution[from.unsafeTableIndex()];
            case NameKind::UNIQUE:
                ENFORCE(from.unsafeTableIndex() < uniqueNameSubstitution.size(),
                        "unique name substitution index out of bounds, got {} where subsitution size is {}",
                        std::to_string(from.rawId()), std::to_string(uniqueNameSubstitution.size()));
                return uniqueNameSubstitution[from.unsafeTableIndex()];
        }
    }

    bool useFastPath() const;

private:
    friend NameRefDebugCheck;

    std::vector<NameRef> utf8NameSubstitution;
    std::vector<NameRef> constantNameSubstitution;
    std::vector<NameRef> uniqueNameSubstitution;
    // set if no substitution is actually necessary
    bool fastPath;

    const int toGlobalStateId;
};

} // namespace sorbet::core

#endif
