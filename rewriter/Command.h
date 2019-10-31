#ifndef SORBET_REWRITER_COMMAND_H
#define SORBET_REWRITER_COMMAND_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class implements Opus::Command by taking
 *
 *   class MyCommand < Opus::Command
 *     sig {params(...).returns(...)}
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
    static void run(core::MutableContext ctx, ast::ClassDef *klass);

    Command() = delete;
};

} // namespace sorbet::rewriter

#endif
