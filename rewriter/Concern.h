#ifndef SORBET_REWRITER_CONCERN_H
#define SORBET_REWRITER_CONCERN_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars things of the form
 *
 *    module Foo
 *      extend ActiveSupport::Concern
 *      class_methods do
 *        def a_class_method
 *        end
 *      end
 *    end
 *
 * into
 *
 *    module Foo
 *      module ClassMethods
 *        def a_class_method
 *        end
 *      end
 *      <Magic>.mixes_in_class_methods(ClassMethods)
 *    end
 */
class Concern final {
public:
    static void run(core::MutableContext ctx, ast::ClassDef *klass);

    Concern() = delete;
};

} // namespace sorbet::rewriter

#endif
