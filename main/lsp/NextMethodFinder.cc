#include "NextMethodFinder.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::realmain::lsp {

unique_ptr<ast::MethodDef> NextMethodFinder::preTransformMethodDef(core::Context ctx,
                                                                   unique_ptr<ast::MethodDef> methodDef) {
    ENFORCE(methodDef->symbol.exists());
    ENFORCE(methodDef->symbol != core::Symbols::todo());

    auto currentMethod = methodDef->symbol;

    auto &currentMethodLocs = currentMethod.data(ctx)->locs();
    auto inFileOfQuery = [&](const auto &loc) { return loc.file() == this->queryLoc.file(); };
    auto maybeCurrentLoc = absl::c_find_if(currentMethodLocs, inFileOfQuery);
    if (maybeCurrentLoc == currentMethodLocs.end()) {
        return methodDef;
    }

    auto currentLoc = *maybeCurrentLoc;
    if (!currentLoc.exists()) {
        // Defensive in case location information is disabled (e.g., certain fuzzer modes)
        return methodDef;
    }

    ENFORCE(currentLoc.file() == this->queryLoc.file());

    if (currentLoc.beginPos() < queryLoc.beginPos()) {
        // Current method is before query, not after.
        return methodDef;
    }

    // Current method starts at or after query loc. Starting 'at' is fine, because it can happen in cases like this:
    //   |def foo; end

    if (this->result_.exists()) {
        // We've already found a result after, so current is not the first.
        return methodDef;
    }

    this->result_ = currentMethod;

    return methodDef;
}

const core::SymbolRef NextMethodFinder::result() const {
    return this->result_;
}

}; // namespace sorbet::realmain::lsp
