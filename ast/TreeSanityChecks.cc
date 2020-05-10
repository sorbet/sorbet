#include "Trees.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::ast {

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
    ENFORCE(cast == core::Names::cast() || cast == core::Names::assertType() || cast == core::Names::let());
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

void ConstantLit::_sanityCheck() {}

void EmptyTree::_sanityCheck() {}

void Literal::_sanityCheck() {
    ENFORCE(value != nullptr);
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

} // namespace sorbet::ast
