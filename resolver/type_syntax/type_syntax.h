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

    // Store the original send that parsed into this structure, so we can modify
    // it after the sig has been associated with a method.
    //
    // The argument for why it is safe to store this runs as follows:
    //
    // 1. The phase that parses sigs and associates them with methods doesn't modify
    //    the AST.
    // 2. The phase that applies the knowledge from parsing the sigs to the methods
    //    in the symbol table runs entirely serially and does not modify the AST
    //    (except for modifying the Send pointed to by this pointer).
    //
    // So we don't have to worry about this pointer being dropped from underneath us.
    ast::Send *origSend = nullptr;

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
