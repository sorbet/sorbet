#ifndef SORBET_DSL_RAILS_H
#define SORBET_DSL_RAILS_H
#include "ast/ast.h"

namespace sorbet::dsl {

/**
 * This class is meant to support some of the missing methods for rails. It is
 * in no way comprehensive, but will grow over time.
 *
 * It desugars things of the form
 *
 *   class Foo < ActiveRecord::Migration[5.2]
 *
 * into
 *
 *   class Foo < ActiveRecord::Migration::Compatibility::V5_2
 *
 */
class Rails final {
public:
    static void patchDSL(core::MutableContext ctx, ast::ClassDef *cdef);

    Rails() = delete;
};

} // namespace sorbet::dsl

#endif
