#include "dsl/ChalkODMProp.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names/dsl.h"
#include "core/core.h"
#include "dsl/dsl.h"

using namespace std;

namespace ruby_typer {
namespace dsl {

unique_ptr<ast::Expression> mkGet(core::Loc loc, core::NameRef name, unique_ptr<ast::Expression> rhs) {
    return ast::MK::Method0(loc, name, move(rhs));
}

unique_ptr<ast::Expression> mkSet(core::Loc loc, core::NameRef name, unique_ptr<ast::Expression> rhs) {
    return ast::MK::Method1(loc, name, ast::MK::Local(loc, core::Names::arg0()), move(rhs));
}

unique_ptr<ast::Expression> mkNilable(core::Loc loc, unique_ptr<ast::Expression> type) {
    return ast::MK::Send1(loc, ast::MK::Ident(loc, core::Symbols::T()), core::Names::nilable(), move(type));
}

unique_ptr<ast::Expression> dupType(ast::Expression *orig) {
    auto send = ast::cast_tree<ast::Send>(orig);
    if (send) {
        ast::Send::ARGS_store args;
        auto dupRecv = dupType(send->recv.get());
        if (!dupRecv) {
            return nullptr;
        }
        for (auto &arg : send->args) {
            auto dupArg = dupType(arg.get());
            if (!dupArg) {
                // This isn't a Type signature, bail out
                return nullptr;
            }
            args.emplace_back(move(dupArg));
        }
        return ast::MK::Send(send->loc, move(dupRecv), send->fun, move(args));
    }

    auto ident = ast::cast_tree<ast::Ident>(orig);
    if (ident) {
        return ast::MK::Ident(ident->loc, ident->symbol);
    }

    auto cons = ast::cast_tree<ast::ConstantLit>(orig);
    if (!cons) {
        return nullptr;
    }

    auto scopeCnst = ast::cast_tree<ast::ConstantLit>(cons->scope.get());
    if (!scopeCnst) {
        ENFORCE(ast::isa_tree<ast::EmptyTree>(cons->scope.get()));
        return ast::MK::Constant(cons->loc, ast::MK::EmptyTree(cons->loc), cons->cnst);
    }
    return ast::MK::Constant(cons->loc, dupType(scopeCnst), cons->cnst);
}

ast::Expression *getHashValue(ast::Hash *hash, core::NameRef name) {
    int i = -1;
    for (auto &keyExpr : hash->keys) {
        i++;
        auto *key = ast::cast_tree<ast::SymbolLit>(keyExpr.get());
        if (key && key->name == name) {
            return hash->values[i].get();
        }
    }
    return nullptr;
}

vector<unique_ptr<ast::Expression>> ChalkODMProp::replaceDSL(core::Context ctx, ast::Send *send) {
    bool isOptional = false;  // Can the setter be passed nil?
    bool isImmutable = false; // Are there no setters?
    bool isNilable = true;    // Can the getter return nil?
    vector<unique_ptr<ast::Expression>> empty;

    if (send->fun == core::Names::optional()) {
        isOptional = true;
    } else if (send->fun == core::Names::const_()) {
        isImmutable = true;
    } else if (send->fun == core::Names::prop()) {
        // Nothing special
    } else {
        return empty;
    }
    if (send->args.size() < 1 || send->args.size() > 3) {
        return empty;
    }
    auto loc = send->loc;

    ast::SymbolLit *name = ast::cast_tree<ast::SymbolLit>(send->args[0].get());
    if (name == nullptr) {
        return empty;
    }

    // This section's goal is to populate this type Expression
    unique_ptr<ast::Expression> type;
    if (send->args.size() == 1) {
        type = ast::MK::Ident(loc, core::Symbols::Object());
    } else {
        type = dupType(send->args[1].get());
    }

    ast::Hash *rules = ast::cast_tree<ast::Hash>(send->args.back().get());
    if (send->args.size() == 3 && !rules) {
        return empty;
    }
    if (type == nullptr) {
        if (rules == nullptr) {
            return empty;
        }
        type = dupType(getHashValue(rules, core::Names::type()));
    }

    if (type == nullptr) {
        if (rules == nullptr) {
            return empty;
        }
        auto arrayType = dupType(getHashValue(rules, core::Names::array()));
        if (arrayType) {
            type = ast::MK::Send1(loc, ast::MK::Ident(loc, core::Symbols::T_Array()), core::Names::squareBrackets(),
                                  move(arrayType));
        }
    }

    if (type == nullptr) {
        return empty;
    }
    // Yay, we have a type
    ENFORCE(type != nullptr, "No obvious type AST for this prop");

    if (auto send = ast::cast_tree<ast::Send>(type.get())) {
        if (isOptional && send->fun == core::Names::new_()) {
            // A huristic for detecting the API Param Spec
            return empty;
        }
    }

    vector<unique_ptr<ast::Expression>> stats;

    // Compute the getters

    if (rules) {
        auto optional = getHashValue(rules, core::Names::optional());
        auto boolOptional = ast::cast_tree<ast::BoolLit>(optional);
        if (boolOptional && boolOptional->value) {
            isOptional = true;
        }

        auto immutable = ast::cast_tree<ast::BoolLit>(getHashValue(rules, core::Names::immutable()));
        if (immutable && immutable->value) {
            isImmutable = true;
        }

        if (getHashValue(rules, core::Names::default_())) {
            isNilable = false;
        }
        if (getHashValue(rules, core::Names::factory())) {
            isNilable = false;
        }

        // In Chalk::ODM `optional String, optional: false` IS optional so
        // we don't falsify any booleans here
    }

    auto getType = dupType(type.get());
    if (isNilable) {
        getType = mkNilable(loc, move(getType));
    }
    stats.emplace_back(ast::MK::Sig(loc, ast::MK::Hash0(loc), dupType(getType.get())));
    stats.emplace_back(mkGet(loc, name->name, ast::MK::Cast(loc, move(getType))));

    // Compute the setters

    if (!isImmutable) {
        auto setType = move(type); // This is the last use so we can move() not dupType()
        if (isOptional) {
            setType = mkNilable(loc, move(setType));
        }

        stats.emplace_back(
            ast::MK::Sig(loc, ast::MK::Hash1(loc, ast::MK::Symbol(loc, core::Names::arg0()), dupType(setType.get())),
                         dupType(setType.get())));
        core::NameRef setName = ctx.state.enterNameUTF8(name->name.toString(ctx) + "=");
        stats.emplace_back(mkSet(loc, setName, ast::MK::Cast(loc, move(setType))));
    }

    return stats;
}

} // namespace dsl
}; // namespace ruby_typer
