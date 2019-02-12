#include "dsl/MixinEncryptedProp.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/core.h"
#include "dsl/helpers.h"
#include "dsl/util.h"

using namespace std;

namespace sorbet::dsl {

unique_ptr<ast::Expression> mkNilableEncryptedValue(core::MutableContext ctx, core::Loc loc) {
    auto opus =
        ast::MK::UnresolvedConstant(loc, ast::MK::EmptyTree(), ctx.state.enterNameConstant(core::Names::Opus()));
    auto db = ast::MK::UnresolvedConstant(loc, move(opus), ctx.state.enterNameConstant(core::Names::DB()));
    auto model = ast::MK::UnresolvedConstant(loc, move(db), ctx.state.enterNameConstant(core::Names::Model()));
    auto mixins = ast::MK::UnresolvedConstant(loc, move(model), ctx.state.enterNameConstant(core::Names::Mixins()));
    auto enc = ast::MK::UnresolvedConstant(loc, move(mixins), ctx.state.enterNameConstant(core::Names::Encryptable()));
    auto ev = ast::MK::UnresolvedConstant(loc, move(enc), ctx.state.enterNameConstant(core::Names::EncryptedValue()));
    return mkNilable(loc, move(ev));
}

unique_ptr<ast::Expression> mkNilableString(core::Loc loc) {
    return mkNilable(loc, ast::MK::Constant(loc, core::Symbols::String()));
}

vector<unique_ptr<ast::Expression>> MixinEncryptedProp::replaceDSL(core::MutableContext ctx, ast::Send *send) {
    bool isImmutable = false; // Are there no setters?

    vector<unique_ptr<ast::Expression>> empty;
    core::NameRef name = core::NameRef::noName();
    core::NameRef enc_name = core::NameRef::noName();

    if (send->fun._id != core::Names::encrypted_prop()._id) {
        return empty;
    }

    auto loc = send->loc;
    auto *sym = ast::cast_tree<ast::Literal>(send->args[0].get());
    if (!sym || !sym->isSymbol(ctx)) {
        return empty;
    }
    name = sym->asSymbol(ctx);
    ENFORCE(sym->loc.source(ctx).size() > 1 && sym->loc.source(ctx)[0] == ':');
    auto nameLoc = core::Loc(sym->loc.file(), sym->loc.beginPos() + 1, sym->loc.endPos());
    enc_name = name.prepend(ctx, "encrypted_");

    ast::Hash *rules = nullptr;
    if (!send->args.empty()) {
        rules = ast::cast_tree<ast::Hash>(send->args.back().get());
    }

    if (rules) {
        if (ASTUtil::hasHashValue(ctx, rules, core::Names::immutable())) {
            isImmutable = true;
        }
    }

    vector<unique_ptr<ast::Expression>> stats;

    // Compute the getters

    stats.emplace_back(ast::MK::Sig(loc, ast::MK::Hash0(loc), mkNilableString(loc)));
    stats.emplace_back(mkGet(loc, name, ast::MK::Cast(loc, mkNilableString(loc))));

    stats.emplace_back(ast::MK::Sig(loc, ast::MK::Hash0(loc), mkNilableEncryptedValue(ctx, loc)));
    stats.emplace_back(mkGet(loc, enc_name, ast::MK::Cast(loc, mkNilableEncryptedValue(ctx, loc))));
    core::NameRef setName = name.addEq(ctx);
    core::NameRef setEncName = enc_name.addEq(ctx);

    // Compute the setter
    if (!isImmutable) {
        stats.emplace_back(
            ast::MK::Sig(loc, ast::MK::Hash1(loc, ast::MK::Symbol(nameLoc, core::Names::arg0()), mkNilableString(loc)),
                         mkNilableString(loc)));
        stats.emplace_back(mkSet(loc, setName, nameLoc, ast::MK::Cast(loc, mkNilableString(loc))));

        stats.emplace_back(ast::MK::Sig(
            loc, ast::MK::Hash1(loc, ast::MK::Symbol(nameLoc, core::Names::arg0()), mkNilableEncryptedValue(ctx, loc)),
            mkNilableEncryptedValue(ctx, loc)));
        stats.emplace_back(mkSet(loc, setEncName, nameLoc, ast::MK::Cast(loc, mkNilableEncryptedValue(ctx, loc))));
    }

    // Compute the Mutator
    {
        // Compute a setter
        ast::ClassDef::RHS_store rhs;
        rhs.emplace_back(
            ast::MK::Sig(loc, ast::MK::Hash1(nameLoc, ast::MK::Symbol(loc, core::Names::arg0()), mkNilableString(loc)),
                         mkNilableString(loc)));
        rhs.emplace_back(mkSet(loc, setName, nameLoc, ast::MK::Cast(loc, mkNilableString(loc))));

        rhs.emplace_back(ast::MK::Sig(
            loc, ast::MK::Hash1(loc, ast::MK::Symbol(nameLoc, core::Names::arg0()), mkNilableEncryptedValue(ctx, loc)),
            mkNilableEncryptedValue(ctx, loc)));
        rhs.emplace_back(mkSet(loc, setEncName, nameLoc, ast::MK::Cast(loc, mkNilableEncryptedValue(ctx, loc))));
    }

    return stats;
}

}; // namespace sorbet::dsl
