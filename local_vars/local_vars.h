#ifndef SORBET_LOCAL_VARS_H
#define SORBET_LOCAL_VARS_H
#include "ast/ast.h"
#include <memory>

namespace sorbet::local_vars {

class LocalVars final {
public:
    static ast::ParsedFile run(core::GlobalState &gs, ast::ParsedFile tree);

    LocalVars() = delete;
};

} // namespace sorbet::local_vars

#endif
