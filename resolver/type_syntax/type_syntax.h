#ifndef SORBET_RESOLVER_TYPE_SYNTAX_H
#define SORBET_RESOLVER_TYPE_SYNTAX_H
#include <memory>
#include <vector>

#include "ast/ast.h"
#include "core/Symbols.h"

namespace sorbet::resolver {

struct ParsedSig {
    struct ArgSpec {
        core::Loc nameLoc;
        core::NameRef name;
        core::Loc typeLoc;
        core::ClassOrModuleRef rebind;
        core::TypePtr type;
    };
    core::ClassOrModuleRef bind;
    std::vector<ArgSpec> argTypes;
    core::TypePtr returns;

    struct TypeArgSpec {
        core::Loc loc;
        core::NameRef name;
        core::TypePtr type;
    };
    std::vector<TypeArgSpec> typeArgs;

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
    ast::Send *origSend;

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
    const bool allowSelfType = false;
    const bool allowRebind = false;
    const bool allowTypeMember = false;
    const bool allowUnspecifiedTypeParameter = false;
    const core::SymbolRef untypedBlame;

    TypeSyntaxArgs withoutRebind() const {
        return TypeSyntaxArgs{allowSelfType, false, allowTypeMember, allowUnspecifiedTypeParameter, untypedBlame};
    }

    TypeSyntaxArgs withRebind() const {
        return TypeSyntaxArgs{allowSelfType, true, allowTypeMember, allowUnspecifiedTypeParameter, untypedBlame};
    }

    TypeSyntaxArgs withoutSelfType() const {
        return TypeSyntaxArgs{false, allowRebind, allowTypeMember, allowUnspecifiedTypeParameter, untypedBlame};
    }

    TypeSyntaxArgs withoutTypeMember() const {
        return TypeSyntaxArgs{allowSelfType, allowRebind, false, allowUnspecifiedTypeParameter, untypedBlame};
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
    static ResultType getResultTypeAndBind(core::Context ctx, const ast::ExpressionPtr &expr, const ParsedSig &,
                                           TypeSyntaxArgs args);
    static core::TypePtr getResultType(core::Context ctx, const ast::ExpressionPtr &expr, const ParsedSig &,
                                       TypeSyntaxArgs args);

    TypeSyntax() = delete;
};

} // namespace sorbet::resolver

#endif // SORBET_RESOLVER_TYPE_SYNTAX_H
