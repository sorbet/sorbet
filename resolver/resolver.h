#ifndef SORBET_RESOLVER_RESOLVER_H
#define SORBET_RESOLVER_RESOLVER_H

#include "ast/ast.h"
#include <memory>

namespace sorbet {
namespace resolver {

class Resolver final {
public:
    static std::vector<std::unique_ptr<ast::Expression>> run(core::MutableContext ctx,
                                                             std::vector<std::unique_ptr<ast::Expression>> trees);
    Resolver() = delete;

private:
    static void finalizeResolution(core::GlobalState &gs);
};

} // namespace resolver
} // namespace sorbet

#endif
