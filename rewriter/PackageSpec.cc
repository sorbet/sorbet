#include "rewriter/PackageSpec.h"
#include "core/core.h"

using namespace std;

namespace sorbet::rewriter {

void PackageSpec::run(core::MutableContext ctx, ast::ClassDef *klass) {
    ENFORCE(ctx.state.packageDB().enabled(), "Should only run on __package.rb files");
    ENFORCE(ctx.file.data(ctx).isPackage(ctx), "Should only run on __package.rb files");

    if (ctx.owner != core::Symbols::root()) {
        // Only process ClassDef that are at the top level
        ENFORCE(ctx.owner == core::Symbols::todo());
        return;
    }

    return;
}

} // namespace sorbet::rewriter
