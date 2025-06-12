#ifndef SORBET_RESOLVER_TYPE_SYNTAX_H
#define SORBET_RESOLVER_TYPE_SYNTAX_H
#include <memory>
#include <vector>

#include "ast/ast.h"
#include "core/Symbols.h"

namespace sorbet::resolver {

struct ParsedSig {
    struct ArgSpec {
        core::LocOffsets nameLoc;
        core::NameRef name;
        core::LocOffsets typeLoc;
        core::ClassOrModuleRef rebind;
        core::TypePtr type;
    };
    core::ClassOrModuleRef bind;
    std::vector<ArgSpec> argTypes;
    core::TypePtr returns;
    core::LocOffsets returnsLoc;

    struct TypeArgSpec {
        core::LocOffsets loc;
        core::NameRef name;
        core::TypePtr type;
    };
    std::vector<TypeArgSpec> typeArgs;

    struct {
        bool sig = false;
        bool proc = false;
        bool bind = false;
        bool params = false;
        bool abstract = false;
        bool override_ = false;
        bool overridable = false;
        bool returns = false;
        bool void_ = false;
        bool checked = false;
        bool final = false;
        bool incompatibleOverride = false;
    } seen;

    TypeArgSpec &enterTypeArgByName(core::NameRef name);
    const TypeArgSpec &findTypeArgByName(core::NameRef name) const;
};

struct TypeSyntaxArgs {
    const bool allowSelfType;
    const bool allowRebind;
    enum class TypeMember : uint8_t {
        Allowed,
        BannedInTypeMember,
        BannedInTypeAlias,
    };
    const TypeMember typeMember;
    const bool allowUnspecifiedTypeParameter;
    const core::SymbolRef untypedBlame;

    TypeSyntaxArgs(bool allowSelfType, bool allowRebind, TypeMember typeMember, bool allowUnspecifiedTypeParameter,
                   core::SymbolRef untypedBlame)
        : allowSelfType(allowSelfType), allowRebind(allowRebind), typeMember(typeMember),
          allowUnspecifiedTypeParameter(allowUnspecifiedTypeParameter), untypedBlame(untypedBlame) {}

    TypeSyntaxArgs withoutRebind() const {
        return TypeSyntaxArgs{allowSelfType, false, typeMember, allowUnspecifiedTypeParameter, untypedBlame};
    }

    TypeSyntaxArgs withRebind() const {
        return TypeSyntaxArgs{allowSelfType, true, typeMember, allowUnspecifiedTypeParameter, untypedBlame};
    }

    TypeSyntaxArgs withoutSelfType() const {
        return TypeSyntaxArgs{false, allowRebind, typeMember, allowUnspecifiedTypeParameter, untypedBlame};
    }
};

class TypeSyntax {
    static std::optional<ParsedSig> parseSig(core::Context ctx, const ast::Send &send, const ParsedSig *parent,
                                             TypeSyntaxArgs args);

public:
    static bool isSig(core::Context ctx, const ast::Send &send);
    static ParsedSig parseSigTop(core::Context ctx, const ast::Send &send, core::SymbolRef blameSymbol);

    struct ResultType {
        core::TypePtr type;
        core::ClassOrModuleRef rebind;
    };
    static core::TypePtr getResultType(core::Context ctx, const ast::ExpressionPtr &expr, const ParsedSig &,
                                       TypeSyntaxArgs args);

    TypeSyntax() = delete;
};

} // namespace sorbet::resolver

#endif // SORBET_RESOLVER_TYPE_SYNTAX_H
