#include "Trees.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::ast {

void ExpressionPtr::_sanityCheck() const {
    auto *ptr = get();
    ENFORCE(ptr != nullptr);

#define SANITY_CHECK(name)                             \
    case Tag::name:                                    \
        reinterpret_cast<name *>(ptr)->_sanityCheck(); \
        break;
    switch (tag()) {
        SANITY_CHECK(EmptyTree)
        SANITY_CHECK(Send)
        SANITY_CHECK(ClassDef)
        SANITY_CHECK(MethodDef)
        SANITY_CHECK(If)
        SANITY_CHECK(While)
        SANITY_CHECK(Break)
        SANITY_CHECK(Retry)
        SANITY_CHECK(Next)
        SANITY_CHECK(Return)
        SANITY_CHECK(RescueCase)
        SANITY_CHECK(Rescue)
        SANITY_CHECK(Local)
        SANITY_CHECK(UnresolvedIdent)
        SANITY_CHECK(RestArg)
        SANITY_CHECK(KeywordArg)
        SANITY_CHECK(OptionalArg)
        SANITY_CHECK(BlockArg)
        SANITY_CHECK(ShadowArg)
        SANITY_CHECK(Assign)
        SANITY_CHECK(Cast)
        SANITY_CHECK(Hash)
        SANITY_CHECK(Array)
        SANITY_CHECK(Literal)
        SANITY_CHECK(UnresolvedConstantLit)
        SANITY_CHECK(ConstantLit)
        SANITY_CHECK(ZSuperArgs)
        SANITY_CHECK(Block)
        SANITY_CHECK(InsSeq)
        SANITY_CHECK(RuntimeMethodDefinition)
    }
#undef SANITY_CHECK
}

void Array::_sanityCheck() {
    for (auto &node : elems) {
        ENFORCE(node);
    }
}

void Assign::_sanityCheck() {
    ENFORCE(lhs);
    ENFORCE(rhs);
}

void Block::_sanityCheck() {
    for (auto &node : args) {
        ENFORCE(node);
    }
    ENFORCE(body);
}

void BlockArg::_sanityCheck() {
    ENFORCE(expr);
    ENFORCE(!isa_tree<OptionalArg>(expr), "OptionalArgs must be at the top-level of an arg.");
}

void Break::_sanityCheck() {
    ENFORCE(expr);
}

void Cast::_sanityCheck() {
    ENFORCE(arg);
    ENFORCE(type);
    ENFORCE(cast == core::Names::cast() || cast == core::Names::assertType() || cast == core::Names::let() ||
            cast == core::Names::uncheckedLet() || cast == core::Names::bind() ||
            cast == core::Names::syntheticBind() || cast == core::Names::assumeType());
    ENFORCE(typeExpr);
}

void ClassDef::_sanityCheck() {
    ENFORCE(name);
    for (auto &node : ancestors) {
        ENFORCE(node);
    }
    for (auto &node : rhs) {
        ENFORCE(node);
    }
}

void UnresolvedConstantLit::_sanityCheck() {
    ENFORCE(scope);
    ENFORCE(cnst.exists());
}

void ConstantLit::_sanityCheck() {
    ENFORCE(resolutionScopes == nullptr || !resolutionScopes->empty());
}

void EmptyTree::_sanityCheck() {}

void Literal::_sanityCheck() {
    ENFORCE(value != nullptr);

    auto tag = value.tag();
    switch (tag) {
        case core::TypePtr::Tag::IntegerLiteralType:
        case core::TypePtr::Tag::FloatLiteralType:
        case core::TypePtr::Tag::NamedLiteralType:
        case core::TypePtr::Tag::ClassType:
            break;
        default:
            ENFORCE(false, "unexpected TypePtr::Tag: {}", tag);
    }
}

void Hash::_sanityCheck() {
    for (auto &node : keys) {
        ENFORCE(node);
    }
    for (auto &node : values) {
        ENFORCE(node);
    }
    ENFORCE(keys.size() == values.size());
}

void If::_sanityCheck() {
    ENFORCE(cond);
    ENFORCE(thenp);
    ENFORCE(elsep);
}

void InsSeq::_sanityCheck() {
    ENFORCE(!stats.empty());
    for (auto &node : stats) {
        ENFORCE(node);
    }
    ENFORCE(expr);
}

void KeywordArg::_sanityCheck() {
    ENFORCE(expr);
    ENFORCE(!isa_tree<OptionalArg>(expr), "OptionalArgs must be at the top-level of an arg.");
}

void Local::_sanityCheck() {
    ENFORCE(localVariable.exists());
}

void MethodDef::_sanityCheck() {
    ENFORCE(name.exists());
    ENFORCE(!args.empty(), "Every method should have at least one arg (the block arg).\n");
    ENFORCE(isa_tree<BlockArg>(args.back()) || isa_tree<Local>(args.back()),
            "Last arg must be a block arg (or a local, if block args have already been removed).");
    for (auto &node : args) {
        ENFORCE(node);
    }
    ENFORCE(rhs);
}

void Next::_sanityCheck() {
    ENFORCE(expr);
}

void OptionalArg::_sanityCheck() {
    ENFORCE(expr);
    ENFORCE(default_);
    ENFORCE(!isa_tree<OptionalArg>(expr), "OptionalArgs must be at the top-level of an arg.");
}

void Return::_sanityCheck() {
    ENFORCE(expr);
}

void Rescue::_sanityCheck() {
    ENFORCE(body);
    ENFORCE(else_);
    ENFORCE(ensure);
    for (auto &node : rescueCases) {
        ENFORCE(node);
    }
}

void RescueCase::_sanityCheck() {
    ENFORCE(var);
    ENFORCE(body);
    for (auto &node : exceptions) {
        ENFORCE(node);
    }
}

void RestArg::_sanityCheck() {
    ENFORCE(expr);
    ENFORCE(!isa_tree<OptionalArg>(expr), "OptionalArgs must be at the top-level of an arg.");
}

void Retry::_sanityCheck() {}

void Send::_sanityCheck() {
    ENFORCE(recv);
    ENFORCE(fun.exists());
    ENFORCE(numPosArgs_ <= args.size(), "Expected {} positional arguments, but only have {} args", numPosArgs_,
            args.size());

    if (hasBlock() || hasKwArgs()) {
        ENFORCE(args.size() > numPosArgs_);
    }

    if (hasBlock()) {
        ENFORCE(block() != nullptr);
    }

    const int end = args.size() - (hasBlock() ? 1 : 0);
    for (int i = 0; i < end; i++) {
        ENFORCE(args[i].tag() != ast::Tag::Block);
    }

    for (auto &node : args) {
        ENFORCE(node);
    }
}

void ShadowArg::_sanityCheck() {
    ENFORCE(expr);
    ENFORCE(!isa_tree<OptionalArg>(expr), "OptionalArgs must be at the top-level of an arg.");
}

void UnresolvedIdent::_sanityCheck() {
    ENFORCE(name.exists());
}

void While::_sanityCheck() {
    ENFORCE(cond);
    ENFORCE(body);
}

void ZSuperArgs::_sanityCheck() {}

void RuntimeMethodDefinition::_sanityCheck() {
    ENFORCE(name.exists());
}

} // namespace sorbet::ast
