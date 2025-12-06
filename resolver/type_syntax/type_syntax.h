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

    struct TypeParamSpec {
        core::LocOffsets loc;
        core::NameRef name;
        core::TypePtr type;
    };
    std::vector<TypeParamSpec> typeParams;

    struct {
        core::LocOffsets sig = core::LocOffsets::none();
        core::LocOffsets proc = core::LocOffsets::none();
        core::LocOffsets bind = core::LocOffsets::none();
        core::LocOffsets params = core::LocOffsets::none();
        core::LocOffsets abstract = core::LocOffsets::none();
        core::LocOffsets override_ = core::LocOffsets::none();
        core::LocOffsets overridable = core::LocOffsets::none();
        core::LocOffsets returns = core::LocOffsets::none();
        core::LocOffsets void_ = core::LocOffsets::none();
        core::LocOffsets checked = core::LocOffsets::none();
        core::LocOffsets final = core::LocOffsets::none();
        core::LocOffsets incompatibleOverride = core::LocOffsets::none();
        core::LocOffsets incompatibleOverrideVisibility = core::LocOffsets::none();
    } seen;

    TypeParamSpec &enterTypeParamByName(core::NameRef name);
    const TypeParamSpec &findTypeParamByName(core::NameRef name) const;
};

struct TypeSyntaxArgs {
    const bool allowRebind;
    enum class TypeMember : uint8_t {
        Allowed,
        BannedInTypeMember,
        BannedInTypeAlias,
    };
    const TypeMember typeMember;
    const bool allowUnspecifiedTypeParameter;
    const core::SymbolRef untypedBlame;

    TypeSyntaxArgs(bool allowRebind, TypeMember typeMember, bool allowUnspecifiedTypeParameter,
                   core::SymbolRef untypedBlame)
        : allowRebind(allowRebind), typeMember(typeMember),
          allowUnspecifiedTypeParameter(allowUnspecifiedTypeParameter), untypedBlame(untypedBlame) {}

    TypeSyntaxArgs withoutRebind() const {
        return TypeSyntaxArgs{false, typeMember, allowUnspecifiedTypeParameter, untypedBlame};
    }

    TypeSyntaxArgs withRebind() const {
        return TypeSyntaxArgs{true, typeMember, allowUnspecifiedTypeParameter, untypedBlame};
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
