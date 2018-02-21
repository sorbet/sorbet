#include <memory>
#include <vector>

#include "ast/ast.h"

namespace ruby_typer {
namespace resolver {

struct ParsedSig {
    struct ArgSpec {
        core::Loc loc;
        core::NameRef name;
        std::shared_ptr<core::Type> type;
    };
    std::vector<ArgSpec> argTypes;
    std::shared_ptr<core::Type> returns;

    struct {
        bool sig = false;
        bool proc = false;
        bool args = false;
        bool abstract = false;
        bool override_ = false;
        bool overridable = false;
        bool implementation = false;
        bool returns = false;
        bool checked = false;
    } seen;
};

class TypeSyntax {
public:
    static bool isSig(core::MutableContext ctx, ast::Send *send);
    static ParsedSig parseSig(core::MutableContext ctx, ast::Send *send);
    static std::shared_ptr<core::Type> getResultType(core::MutableContext ctx, std::unique_ptr<ast::Expression> &expr);

    TypeSyntax() = delete;
};

} // namespace resolver
} // namespace ruby_typer
