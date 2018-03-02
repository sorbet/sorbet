#include "dsl/attr_reader.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names/dsl.h"
#include "core/core.h"
#include "core/errors/dsl.h"
#include "dsl/dsl.h"

using namespace std;

namespace ruby_typer {
namespace dsl {

unique_ptr<ast::Expression> mkTUntyped(core::MutableContext ctx, core::Loc loc) {
    return ast::MK::Send0(loc, ast::MK::Ident(loc, core::Symbols::T()), core::Names::untyped());
}

core::NameRef getName(core::MutableContext ctx, ast::Expression *name) {
    if (auto sym = ast::cast_tree<ast::SymbolLit>(name)) {
        return sym->name;
    } else if (auto str = ast::cast_tree<ast::StringLit>(name)) {
        return str->value;
    }
    if (auto e = ctx.state.beginError(name->loc, core::errors::DSL::BadAttrArg)) {
        e.setHeader("arg must be a Symbol or String");
    }
    return core::NameRef::noName();
}

vector<unique_ptr<ast::Expression>> AttrReader::replaceDSL(core::MutableContext ctx, ast::Send *send) {
    vector<unique_ptr<ast::Expression>> empty;

    bool makeReader = false;
    bool makeWriter = false;
    if (send->fun == core::Names::attr() || send->fun == core::Names::attrReader() ||
        send->fun == core::Names::attrAccessor()) {
        makeReader = true;
    }
    if (send->fun == core::Names::attrWriter() || send->fun == core::Names::attrAccessor()) {
        makeWriter = true;
    }
    if (!makeReader && !makeWriter) {
        return empty;
    }

    auto loc = send->loc;
    vector<unique_ptr<ast::Expression>> stats;

    if (makeReader) {
        for (auto &arg : send->args) {
            auto name = getName(ctx, arg.get());
            if (!name.exists()) {
                return empty;
            }

            core::NameRef varName = ctx.state.enterNameUTF8("@" + name.toString(ctx));
            stats.emplace_back(ast::MK::Method0(loc, name, ast::MK::Instance(loc, varName)));
        }
    }

    if (makeWriter) {
        for (auto &arg : send->args) {
            auto name = getName(ctx, arg.get());
            if (!name.exists()) {
                return empty;
            }

            core::NameRef varName = ctx.state.enterNameUTF8("@" + name.toString(ctx));
            core::NameRef setName = ctx.state.enterNameUTF8(name.toString(ctx) + "=");
            auto body = ast::MK::Assign(loc, ast::MK::Instance(loc, varName), ast::MK::Local(loc, core::Names::arg0()));
            stats.emplace_back(ast::MK::Method1(loc, setName, ast::MK::Local(loc, core::Names::arg0()), move(body)));
        }
    }

    return stats;
}

} // namespace dsl
}; // namespace ruby_typer
