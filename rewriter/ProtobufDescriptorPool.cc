#include "rewriter/ProtobufDescriptorPool.h"
#include "ast/Helpers.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::rewriter {

ast::ExpressionPtr ProtobufDescriptorPool::run(core::MutableContext ctx, ast::Assign *asgn) {
    if (!ast::isa_tree<ast::UnresolvedConstantLit>(asgn->lhs)) {
        return nullptr;
    }

    auto sendMsgclass = ast::cast_tree<ast::Send>(asgn->rhs);
    if (sendMsgclass == nullptr) {
        return nullptr;
    }
    if (sendMsgclass->fun != core::Names::msgclass() && sendMsgclass->fun != core::Names::enummodule()) {
        return nullptr;
    }
    auto kind = sendMsgclass->fun == core::Names::msgclass() ? ast::ClassDef::Kind::Class : ast::ClassDef::Kind::Module;

    auto sendLookup = ast::cast_tree<ast::Send>(sendMsgclass->recv);
    if (sendLookup == nullptr || sendLookup->fun != core::Names::lookup()) {
        return nullptr;
    }

    auto sendGeneratedPool = ast::cast_tree<ast::Send>(sendLookup->recv);
    if (sendGeneratedPool == nullptr || sendGeneratedPool->fun != core::Names::generatedPool()) {
        return nullptr;
    }

    auto cnstDescriptorPool = ast::cast_tree<ast::UnresolvedConstantLit>(sendGeneratedPool->recv);
    if (cnstDescriptorPool == nullptr || cnstDescriptorPool->cnst != core::Names::Constants::DescriptorPool()) {
        return nullptr;
    }

    auto cnstProtobuf = ast::cast_tree<ast::UnresolvedConstantLit>(cnstDescriptorPool->scope);
    if (cnstProtobuf == nullptr || cnstProtobuf->cnst != core::Names::Constants::Protobuf()) {
        return nullptr;
    }

    auto cnstGoogle = ast::cast_tree<ast::UnresolvedConstantLit>(cnstProtobuf->scope);
    if (cnstGoogle == nullptr || cnstGoogle->cnst != core::Names::Constants::Google()) {
        return nullptr;
    }

    // Put the actual send in the tree too to require that the Protobuf constant / methods are actually there.
    // (also needed for autogen to be able to see the behavior)
    ast::ClassDef::RHS_store rhs;
    rhs.emplace_back(std::move(asgn->rhs));

    if (sendMsgclass->fun == core::Names::msgclass()) {
        auto arg0 = ast::MK::Local(asgn->loc, core::Names::arg0());
        auto arg = ast::MK::OptionalArg(asgn->loc, std::move(arg0), ast::MK::Hash0(asgn->loc));
        rhs.emplace_back(ast::MK::SyntheticMethod1(asgn->loc, asgn->loc, core::Names::initialize(), std::move(arg),
                                                   ast::MK::EmptyTree()));
    }

    return ast::MK::ClassOrModule(asgn->loc, asgn->loc, asgn->lhs.deepCopy(), {}, std::move(rhs), kind);
}

}; // namespace sorbet::rewriter
