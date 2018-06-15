#include <memory>
#include <vector>

#include "ast/ast.h"

namespace sorbet {
namespace resolver {

struct ParsedSig {
    struct ArgSpec {
        core::Loc loc;
        core::NameRef name;
        std::shared_ptr<core::Type> type;
    };
    std::vector<ArgSpec> argTypes;
    std::shared_ptr<core::Type> returns;

    struct TypeArgSpec {
        core::Loc loc;
        core::NameRef name;
        std::shared_ptr<core::TypeVar> type;
    };
    std::vector<TypeArgSpec> typeArgs;

    struct {
        bool sig = false;
        bool proc = false;
        bool args = false;
        bool abstract = false;
        bool override_ = false;
        bool overridable = false;
        bool implementation = false;
        bool returns = false;
        bool void_ = false;
        bool checked = false;
    } seen;

    TypeArgSpec &enterTypeArgByName(core::NameRef name);
    const TypeArgSpec &findTypeArgByName(core::NameRef name) const;
};

class TypeSyntax {
public:
    static bool isSig(core::MutableContext ctx, ast::Send *send);
    static ParsedSig parseSig(core::MutableContext ctx, ast::Send *send, const ParsedSig *parent, bool allowSelfType);
    static std::shared_ptr<core::Type> getResultType(core::MutableContext ctx, std::unique_ptr<ast::Expression> &expr,
                                                     const ParsedSig &, bool allowSelfType);

    TypeSyntax() = delete;
};

} // namespace resolver
} // namespace sorbet
