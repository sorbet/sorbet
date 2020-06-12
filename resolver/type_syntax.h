#ifndef SORBET_RESOLVER_TYPE_SYNTAX_H
#define SORBET_RESOLVER_TYPE_SYNTAX_H
#include <memory>
#include <vector>

#include "ast/ast.h"
#include "core/Symbols.h"

namespace sorbet::resolver {

struct ParsedSig {
    struct ArgSpec {
        core::Loc loc;
        core::NameRef name;
        core::TypePtr type;
        core::SymbolRef rebind;
    };
    core::SymbolRef bind;
    std::vector<ArgSpec> argTypes;
    core::TypePtr returns;

    struct TypeArgSpec {
        core::Loc loc;
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
    const bool allowSelfType = false;
    const bool allowRebind = false;
    const bool allowTypeMember = false;
    const core::SymbolRef untypedBlame;

    TypeSyntaxArgs withoutRebind() const {
        return TypeSyntaxArgs{allowSelfType, false, allowTypeMember, untypedBlame};
    }

    TypeSyntaxArgs withRebind() const {
        return TypeSyntaxArgs{allowSelfType, true, allowTypeMember, untypedBlame};
    }

    TypeSyntaxArgs withoutSelfType() const {
        return TypeSyntaxArgs{false, allowRebind, allowTypeMember, untypedBlame};
    }

    TypeSyntaxArgs withoutTypeMember() const {
        return TypeSyntaxArgs{allowSelfType, allowRebind, false, untypedBlame};
    }
};

class TypeSyntax {
public:
    static bool isSig(core::Context ctx, ast::Send *send);
    static ParsedSig parseSig(core::MutableContext ctx, ast::Send *send, const ParsedSig *parent, TypeSyntaxArgs args);

    struct ResultType {
        core::TypePtr type;
        core::SymbolRef rebind;
    };
    static ResultType getResultTypeAndBind(core::MutableContext ctx, ast::TreePtr &expr, const ParsedSig &,
                                           TypeSyntaxArgs args);
    static core::TypePtr getResultType(core::MutableContext ctx, ast::TreePtr &expr, const ParsedSig &,
                                       TypeSyntaxArgs args);

    TypeSyntax() = delete;
};

} // namespace sorbet::resolver

#endif // SORBET_RESOLVER_TYPE_SYNTAX_H
