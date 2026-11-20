#ifndef SORBET_REWRITER_MIXIN_ENCRYPTED_PROP_H
#define SORBET_REWRITER_MIXIN_ENCRYPTED_PROP_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars things of the form
 *
 *   encrypted_prop :foo, EncryptedValue
 *
 * into
 *
 *   sig {returns(T.nilable(String))}
 *   def foo; end
 *   sig {params(arg0: T.nilable(String)).returns(T.nilable(String))}
 *   def foo=(arg0); end
 *   sig {returns(T.nilable(EncryptedValue))}
 *   def encrypted_foo; end
 *   sig {params(arg0: T.nilable(EncryptedValue)).returns(T.nilable(EncryptedValue))}
 *   def encrypted_foo=(arg0); end
 *
 * We try to implement a simple approximation of the functionality that
 * M::Mixins::Encryptable.encrypted_prop has. This isn't full fidelity, but we're trying to
 * straddle the line between complexity and usefulness. Specifically:
 *
 * Any `encrypted_prop` method call in a class body whose shape matches is considered.
 * The `encrypted_foo` accessors use the type passed as the second argument, which must be a
 * constant. With `immutable: true`, the setters are omitted.
 *
 * Any deviation from this expected shape stops the desugaring.
 *
 */
class MixinEncryptedProp final {
public:
    static std::vector<ast::ExpressionPtr> run(core::MutableContext ctx, ast::Send *send);

    MixinEncryptedProp() = delete;
};

} // namespace sorbet::rewriter

#endif
