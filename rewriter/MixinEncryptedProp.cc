#include "rewriter/MixinEncryptedProp.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/core.h"
#include "rewriter/Util.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

ast::ExpressionPtr mkNilableEncryptedValue(core::MutableContext ctx, core::LocOffsets loc) {
    auto parts = vector<core::NameRef>{
        core::Names::Constants::Opus(),        core::Names::Constants::DB(),
        core::Names::Constants::Model(),       core::Names::Constants::Mixins(),
        core::Names::Constants::Encryptable(), core::Names::Constants::EncryptedValue(),
    };

    return ASTUtil::mkNilable(loc, ast::MK::UnresolvedConstantParts(loc, ast::MK::EmptyTree(), parts));
}

ast::ExpressionPtr mkNilableString(core::LocOffsets loc) {
    return ASTUtil::mkNilable(loc, ast::MK::Constant(loc, core::Symbols::String()));
}

} // namespace

vector<ast::ExpressionPtr> MixinEncryptedProp::run(core::MutableContext ctx, ast::Send *send) {
    vector<ast::ExpressionPtr> empty;

    if (ctx.state.runningUnderAutogen) {
        return empty;
    }

    bool isImmutable = false; // Are there no setters?
    core::NameRef name;
    core::NameRef enc_name;

    if (send->fun != core::Names::encryptedProp()) {
        return empty;
    }
    if (send->numPosArgs() == 0) {
        return empty;
    }

    auto loc = send->loc;
    auto *sym = ast::cast_tree<ast::Literal>(send->getPosArg(0));
    if (!sym || !sym->isSymbol()) {
        return empty;
    }
    name = sym->asSymbol();
    ENFORCE(ctx.locAt(sym->loc).exists());
    ENFORCE(ctx.locAt(sym->loc).source(ctx).value().size() > 1 && ctx.locAt(sym->loc).source(ctx).value()[0] == ':');
    auto nameLoc = core::LocOffsets{sym->loc.beginPos() + 1, sym->loc.endPos()};
    enc_name = name.prepend(ctx, "encrypted_");

    ast::ExpressionPtr rules;
    rules = ASTUtil::mkKwArgsHash(send);

    if (auto *hash = ast::cast_tree<ast::Hash>(rules)) {
        if (ASTUtil::hasTruthyHashValue(ctx, *hash, core::Names::immutable())) {
            isImmutable = true;
        }
    }

    vector<ast::ExpressionPtr> stats;

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
