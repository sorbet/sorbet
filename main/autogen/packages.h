#ifndef AUTOGEN_PACKAGES_H
#define AUTOGEN_PACKAGES_H

#include "main/autogen/data/definitions.h"

namespace sorbet::autogen {

struct Packages final {
    static Package extractPackage(core::Context ctx, ast::ParsedFile tree);
};

} // namespace sorbet::autogen
#endif // AUTOGEN_PACKAGES_H
