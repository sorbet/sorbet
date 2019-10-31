#ifndef SORBET_DSL_INTERFACE_WRAPPER_H
#define SORBET_DSL_INTERFACE_WRAPPER_H
#include "ast/ast.h"

namespace sorbet::dsl {

/**
 * This class desugars `wrap_interface` into a simple cast:
 *
 *    SomeClass.wrap_interface(obj)
 *
 * =>
 *
 *    T.let(obj, SomeClass)
 *
 * This suffices to:
 *
 *  (a) check that `obj : SomeClass`
 *  (b) statically prevent calling any methods on `obj` not defined on `SomeClass`
 *
 * which mirrors the runtime behavior.
 */
class InterfaceWrapper final {
public:
    static std::unique_ptr<ast::Expression> replaceDSL(core::MutableContext ctx, std::unique_ptr<ast::Send> send);

    InterfaceWrapper() = delete;
};

} // namespace sorbet::dsl

#endif
