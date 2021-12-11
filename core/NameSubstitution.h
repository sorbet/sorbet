#ifndef SORBET_CORE_NAME_SUBSTITUTION_H
#define SORBET_CORE_NAME_SUBSTITUTION_H

#include "common/common.h"
#include "core/NameHash.h"
#include "core/NameRef.h"
#include "core/SymbolRef.h"
#include <vector>

namespace sorbet::core {
class GlobalState;

/**
 * With the ast::substitute pass, GlobalSubstitution makes it possible to rewrite ASTs from one GlobalState into ASTs
 * from a second GlobalState.
 *
 * The constructor builds up a lookup table from every NameRef in `from` to an equivalent NameRef in `to`, inserting new
 * names into `to` where needed. Then, that table can be used to rewrite multiple ASTs from `from`.
 */
class GlobalSubstitution final {
public:
    GlobalSubstitution(const GlobalState &from, GlobalState &to);

    static void mergeFileTables(const GlobalState &from, GlobalState &to);

    NameRef substitute(NameRef from, bool allowSameFromTo = false) const {
        if (!allowSameFromTo) {
            from.sanityCheckSubstitution(*this);
        }
        switch (from.kind()) {
            case NameKind::UTF8:
                ENFORCE(from.utf8Index() < utf8NameSubstitution.size(),
                        "utf8 name substitution index out of bounds, got {} where subsitution size is {}",
                        std::to_string(from.rawId()), std::to_string(utf8NameSubstitution.size()));
                return utf8NameSubstitution[from.utf8Index()];
            case NameKind::CONSTANT:
                ENFORCE(from.constantIndex() < constantNameSubstitution.size(),
                        "constant name substitution index out of bounds, got {} where subsitution size is {}",
                        std::to_string(from.rawId()), std::to_string(constantNameSubstitution.size()));
                return constantNameSubstitution[from.constantIndex()];
            case NameKind::UNIQUE:
                ENFORCE(from.uniqueIndex() < uniqueNameSubstitution.size(),
                        "unique name substitution index out of bounds, got {} where subsitution size is {}",
                        std::to_string(from.rawId()), std::to_string(uniqueNameSubstitution.size()));
                return uniqueNameSubstitution[from.uniqueIndex()];
        }
    }

    NameRef substituteSymbolName(NameRef from, bool allowSameFromTo = false) const {
        return substitute(from, allowSameFromTo);
    }

    NameRef substituteSend(NameRef from, bool allowSameFromTo = false) const {
        return substitute(from, allowSameFromTo);
    }

private:
    friend NameRefDebugCheck;

    std::vector<NameRef> utf8NameSubstitution;
    std::vector<NameRef> constantNameSubstitution;
    std::vector<NameRef> uniqueNameSubstitution;

    int toGlobalStateId;
};

/**
 * GlobalSubstitution, but lazily populates `nameSubstitution` _and_ builds up a UsageHash for the file.
 * Used in the hashing package as a part of the AST hashing process, which rewrites ASTs from the main GlobalState into
 * ASTs for new and empty GlobalStates.
 *
 * Unlike the GlobalSubstitution case, LazyGlobalSubstitution is intended to be used for rewriting a single AST. Hence,
 * the `nameSubstitution` map is sparse and built up lazily, since a single AST will only reference a small subset of
 * names in GlobalState.
 */
class LazyGlobalSubstitution final {
    const core::GlobalState &fromGS;
    core::GlobalState &toGS;

    UnorderedMap<core::NameRef, core::NameRef> nameSubstitution;
    core::UsageHash acc;

    NameRef defineName(NameRef from, bool allowSameFromTo);

public:
    LazyGlobalSubstitution(const GlobalState &fromGS, GlobalState &toGS);
    ~LazyGlobalSubstitution() = default;

    NameRef substitute(NameRef from, bool allowSameFromTo = false) {
        if (&fromGS == &toGS) {
            return from;
        }

        auto it = nameSubstitution.find(from);
        if (it == nameSubstitution.end()) {
            return defineName(from, allowSameFromTo);
        }
        return it->second;
    }

    NameRef substituteSymbolName(NameRef from, bool allowSameFromTo = false) {
        acc.symbols.emplace_back(fromGS, from);
        return substitute(from, allowSameFromTo);
    }

    NameRef substituteSend(NameRef from, bool allowSameFromTo = false) {
        acc.sends.emplace_back(fromGS, from);
        return substitute(from, allowSameFromTo);
    }

    core::UsageHash getAllNames();
};

} // namespace sorbet::core

#endif
