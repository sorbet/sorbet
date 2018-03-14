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
    core::NameRef res = core::NameRef::noName();
    if (auto lit = ast::cast_tree<ast::Literal>(name)) {
        if (lit->isSymbol(ctx)) {
            res = lit->asSymbol(ctx);
        } else if (lit->isString(ctx)) {
            res = lit->asString(ctx);
        }
    }
    if (!res.exists()) {
        if (auto e = ctx.state.beginError(name->loc, core::errors::DSL::BadAttrArg)) {
            e.setHeader("arg must be a Symbol or String");
        }
    }
    return res;
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

            core::NameRef varName = name.addAt(ctx);
            stats.emplace_back(ast::MK::Method0(loc, name, ast::MK::Instance(loc, varName)));
        }
    }

    if (makeWriter) {
        for (auto &arg : send->args) {
            auto name = getName(ctx, arg.get());
            if (!name.exists()) {
                return empty;
            }

            core::NameRef varName = name.addAt(ctx);
            core::NameRef setName = name.addEq(ctx);
            auto body = ast::MK::Assign(loc, ast::MK::Instance(loc, varName), ast::MK::Local(loc, name));
            stats.emplace_back(ast::MK::Method1(loc, setName, ast::MK::Local(loc, name), move(body)));
        }
    }

    return stats;
}

} // namespace dsl
}; // namespace ruby_typer
