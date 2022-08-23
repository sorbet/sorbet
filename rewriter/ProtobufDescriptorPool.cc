#include "rewriter/ProtobufDescriptorPool.h"
#include "ast/Helpers.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::rewriter {

vector<ast::ExpressionPtr> ProtobufDescriptorPool::run(core::MutableContext ctx, ast::Assign *asgn) {
    vector<ast::ExpressionPtr> empty;

    auto sendMsgclass = ast::cast_tree<ast::Send>(asgn->rhs);
    if (sendMsgclass == nullptr) {
        return empty;
    }
    if (sendMsgclass->fun != core::Names::msgclass() && sendMsgclass->fun != core::Names::enummodule()) {
        return empty;
    }
    auto kind = sendMsgclass->fun == core::Names::msgclass() ? ast::ClassDef::Kind::Class : ast::ClassDef::Kind::Module;

    auto sendLookup = ast::cast_tree<ast::Send>(sendMsgclass->recv);
    if (sendLookup == nullptr || sendLookup->fun != core::Names::lookup()) {
        return empty;
    }

    auto sendGeneratedPool = ast::cast_tree<ast::Send>(sendLookup->recv);
    if (sendGeneratedPool == nullptr || sendGeneratedPool->fun != core::Names::generatedPool()) {
        return empty;
    }

    auto cnstDescriptorPool = ast::cast_tree<ast::UnresolvedConstantLit>(sendGeneratedPool->recv);
    if (cnstDescriptorPool == nullptr || cnstDescriptorPool->cnst != core::Names::Constants::DescriptorPool()) {
        return empty;
    }

    auto cnstProtobuf = ast::cast_tree<ast::UnresolvedConstantLit>(cnstDescriptorPool->scope);
    if (cnstProtobuf == nullptr || cnstProtobuf->cnst != core::Names::Constants::Protobuf()) {
        return empty;
    }

    auto cnstGoogle = ast::cast_tree<ast::UnresolvedConstantLit>(cnstProtobuf->scope);
    if (cnstGoogle == nullptr || cnstGoogle->cnst != core::Names::Constants::Google()) {
        return empty;
    }

    if (!ast::isa_tree<ast::UnresolvedConstantLit>(asgn->lhs)) {
        return empty;
    }

    // Put the actual send in the tree too to require that the Protobuf constant / methods are actually there.
    // (also needed for autogen to be able to see the )
    ast::ClassDef::RHS_store rhs;
    rhs.emplace_back(asgn->rhs.deepCopy());

    if (sendMsgclass->fun == core::Names::msgclass()) {
        auto arg0 = ast::MK::Local(asgn->loc, core::Names::arg0());
        auto arg = ast::MK::OptionalArg(asgn->loc, std::move(arg0), ast::MK::Hash0(asgn->loc));
        rhs.emplace_back(ast::MK::SyntheticMethod1(asgn->loc, asgn->loc, core::Names::initialize(), std::move(arg),
                                                   ast::MK::EmptyTree()));
    }

    vector<ast::ExpressionPtr> res;
    res.emplace_back(ast::MK::ClassOrModule(asgn->loc, asgn->loc, asgn->lhs.deepCopy(), {}, std::move(rhs), kind));
    return res;
}

}; // namespace sorbet::rewriter
