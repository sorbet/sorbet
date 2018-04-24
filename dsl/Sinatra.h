#ifndef SRUBY_DSL_SINATRA_H
#define SRUBY_DSL_SINATRA_H
#include "ast/ast.h"

namespace ruby_typer {
namespace dsl {

/**
 * This class desugars things of the form
 *
 *   module MyRoutes
 *     def self.registered(app)
 *       app.helpers Helpers
 *       app.get "/route/" do
 *         params
 *       end
 *     end
 *     module Helpers
 *     end
 *   end
 *
 * into
 *
 *   module MyRoutes
 *     include Sinatra::Base
 *     include Helpers
 *     def <instance_registered>(app)
 *       app.get "/route/" do
 *         params
 *       end
 *     end
 *     module Helpers
 *     end
 *   end
 *
 * which is a big lie, but emulates the scoping rules that the block passed to
 * app.get will have as well as pull in the methods from the app.helpers.
 */
class Sinatra final {
public:
    static std::vector<std::unique_ptr<ast::Expression>> replaceDSL(core::MutableContext ctx, ast::MethodDef *send);

    Sinatra() = delete;
};

} // namespace dsl
} // namespace ruby_typer

#endif
