#ifndef SORBET_REWRITER_INTERFACE_WRAPPER_H
#define SORBET_REWRITER_INTERFACE_WRAPPER_H
#include "ast/ast.h"

namespace sorbet::rewriter {

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
    static ast::TreePtr run(core::MutableContext ctx, ast::Send *send);

    InterfaceWrapper() = delete;
};

} // namespace sorbet::rewriter

#endif
