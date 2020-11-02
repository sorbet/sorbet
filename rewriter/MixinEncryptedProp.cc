#include "rewriter/MixinEncryptedProp.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/core.h"
#include "rewriter/Util.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

ast::TreePtr mkNilableEncryptedValue(core::MutableContext ctx, core::LocOffsets loc) {
    auto opus = ast::MK::UnresolvedConstant(loc, ast::MK::EmptyTree(), core::Names::Constants::Opus());
    auto db = ast::MK::UnresolvedConstant(loc, move(opus), core::Names::Constants::DB());
    auto model = ast::MK::UnresolvedConstant(loc, move(db), core::Names::Constants::Model());
    auto mixins = ast::MK::UnresolvedConstant(loc, move(model), core::Names::Constants::Mixins());
    auto enc = ast::MK::UnresolvedConstant(loc, move(mixins), core::Names::Constants::Encryptable());
    auto ev = ast::MK::UnresolvedConstant(loc, move(enc), core::Names::Constants::EncryptedValue());
    return ASTUtil::mkNilable(loc, move(ev));
}

ast::TreePtr mkNilableString(core::LocOffsets loc) {
    return ASTUtil::mkNilable(loc, ast::MK::Constant(loc, core::Symbols::String()));
}

} // namespace

vector<ast::TreePtr> MixinEncryptedProp::run(core::MutableContext ctx, ast::Send *send) {
    vector<ast::TreePtr> empty;

    if (ctx.state.runningUnderAutogen) {
        return empty;
    }

    bool isImmutable = false; // Are there no setters?
    core::NameRef name;
    core::NameRef enc_name;

    if (send->fun._id != core::Names::encryptedProp()._id) {
        return empty;
    }
    if (send->args.empty()) {
        return empty;
    }

    auto loc = send->loc;
    auto *sym = ast::cast_tree<ast::Literal>(send->args[0]);
    if (!sym || !sym->isSymbol(ctx)) {
        return empty;
    }
    name = sym->asSymbol(ctx);
    ENFORCE(core::Loc(ctx.file, sym->loc).source(ctx).size() > 1 &&
            core::Loc(ctx.file, sym->loc).source(ctx)[0] == ':');
    auto nameLoc = core::LocOffsets{sym->loc.beginPos() + 1, sym->loc.endPos()};
    enc_name = name.prepend(ctx, "encrypted_");

    ast::TreePtr rules;
    if (!send->args.empty()) {
        rules = ASTUtil::mkKwArgsHash(send);
    }

    if (auto *hash = ast::cast_tree<ast::Hash>(rules)) {
        if (ASTUtil::hasTruthyHashValue(ctx, *hash, core::Names::immutable())) {
            isImmutable = true;
        }
    }

    vector<ast::TreePtr> stats;

    // Compute the getters

    stats.emplace_back(ast::MK::Sig(loc, {}, mkNilableString(loc)));
    stats.emplace_back(ASTUtil::mkGet(ctx, loc, name, ast::MK::RaiseUnimplemented(loc)));

    stats.emplace_back(ast::MK::Sig(loc, {}, mkNilableEncryptedValue(ctx, loc)));
    stats.emplace_back(ASTUtil::mkGet(ctx, loc, enc_name, ast::MK::RaiseUnimplemented(loc)));
    core::NameRef setName = name.addEq(ctx);
    core::NameRef setEncName = enc_name.addEq(ctx);

    // Compute the setter
    if (!isImmutable) {
        stats.emplace_back(ast::MK::Sig1(loc, ast::MK::Symbol(nameLoc, core::Names::arg0()), mkNilableString(loc),
                                         mkNilableString(loc)));
        stats.emplace_back(ASTUtil::mkSet(ctx, loc, setName, nameLoc, ast::MK::RaiseUnimplemented(loc)));

        stats.emplace_back(ast::MK::Sig1(loc, ast::MK::Symbol(nameLoc, core::Names::arg0()),
                                         mkNilableEncryptedValue(ctx, loc), mkNilableEncryptedValue(ctx, loc)));
        stats.emplace_back(ASTUtil::mkSet(ctx, loc, setEncName, nameLoc, ast::MK::RaiseUnimplemented(loc)));
    }

    return stats;
}

}; // namespace sorbet::rewriter
