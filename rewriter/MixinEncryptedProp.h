#ifndef SORBET_REWRITER_MIXIN_ENCRYPTED_PROP_H
#define SORBET_REWRITER_MIXIN_ENCRYPTED_PROP_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars things of the form
 *
 *   encrypted_prop :foo
 *
 * into
 *
 *   sig {returns(T.nilable(String))}
 *   def foo; T.cast(nil, T.nilable(String)); end
 *   sig {params(arg0: String).returns(NilClass)}
 *   def foo=(arg0); end
 *   sig {returns(T.nilable(String))}
 *   def encrypted_foo; T.cast(nil, T.nilable(String)); end
 *   sig {params(arg0: String).returns(NilClass)}
 *   def encrypted_foo=(arg0); end
 *   class Mutator < Chalk::ODM::Mutator
 *     sig {params(arg0: String).returns(NilClass)}
 *     def foo=(arg0); end
 *     sig {params(arg0: String).returns(NilClass)}
 *     def encrypted_foo=(arg0); end
 *   end
 *
 * We try to implement a simple approximation of the functionality that
 * M::Mixins::Encryptable.encrypted_prop has. This isn't full fidelity, but we're trying to
 * straddle the line between complexity and usefulness. Specifically:
 *
 * Any `encrypted_prop` method call in a class body whose shape matches is considered.
 * The getter will return `T.nilable(String)` and the setter will take `String`.
 *
 * Any deviation from this expected shape stops the desugaring.
 *
 */
class MixinEncryptedProp final {
public:
    static std::vector<ast::TreePtr> run(core::MutableContext ctx, ast::Send *send);

    MixinEncryptedProp() = delete;
};

} // namespace sorbet::rewriter

#endif
