#ifndef SORBET_REWRITER_TYPEMEMBERS_H
#define SORBET_REWRITER_TYPEMEMBERS_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class does nothing but raise errors for and then delete duplicate type members
 */
class TypeMembers final {
public:
    static void run(core::MutableContext ctx, ast::ClassDef *cdef);

    TypeMembers() = delete;
};

} // namespace sorbet::rewriter

#endif
