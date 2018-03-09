#include "Trees.h"
#include "core/Names/resolver.h"

using namespace std;

namespace ruby_typer {
namespace ast {

void Array::_sanityCheck() {
    for (auto &node : elems) {
        ENFORCE(node);
    }
}

void ArraySplat::_sanityCheck() {
    ENFORCE(arg);
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

void ConstDef::_sanityCheck() {
    ENFORCE(rhs);
}

void ConstantLit::_sanityCheck() {
    ENFORCE(scope);
    ENFORCE(cnst.exists());
}

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

void HashSplat::_sanityCheck() {
    ENFORCE(arg);
}

void Ident::_sanityCheck() {
    ENFORCE(symbol.exists());
}

void If::_sanityCheck() {
    ENFORCE(cond);
    ENFORCE(thenp);
    ENFORCE(elsep);
}

void InsSeq::_sanityCheck() {
    for (auto &node : stats) {
        ENFORCE(node);
    }
    ENFORCE(expr);
}

void KeywordArg::_sanityCheck() {
    ENFORCE(expr);
}

void Local::_sanityCheck() {
    ENFORCE(localVariable.exists());
}

void MethodDef::_sanityCheck() {
    ENFORCE(name.exists());
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
}

void Retry::_sanityCheck() {}

void Self::_sanityCheck() {
    ENFORCE(claz.exists());
}

void Send::_sanityCheck() {
    ENFORCE(recv);
    ENFORCE(fun.exists());
    for (auto &node : args) {
        ENFORCE(node);
    }
}

void ShadowArg::_sanityCheck() {
    ENFORCE(expr);
}

void UnresolvedIdent::_sanityCheck() {
    ENFORCE(name.exists());
}

void While::_sanityCheck() {
    ENFORCE(cond);
    ENFORCE(body);
}

void Yield::_sanityCheck() {
    ENFORCE(expr);
}

void ZSuperArgs::_sanityCheck() {}

} // namespace ast
} // namespace ruby_typer
