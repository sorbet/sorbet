#ifndef SRUBY_DSL_COMMAND_H
#define SRUBY_DSL_COMMAND_H
#include "ast/ast.h"

namespace ruby_typer {
namespace dsl {

/**
 * This class implements Opus::Command by taking
 *
 *   class MyCommand < Opus::Command
 *     sig(...).returns(...)
 *     def call(...)
 *       ...
 *     end
 *   end
 *
 * and replicating the `sig` onto a new, empty, `self.call` version of the
 * method.
 *
 */
class Command final {
public:
    static void patchDSL(core::MutableContext ctx, ast::ClassDef *klass);

    Command() = delete;
};

} // namespace dsl
} // namespace ruby_typer

#endif
