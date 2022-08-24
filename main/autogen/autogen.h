#ifndef AUTOGEN_H
#define AUTOGEN_H

#include "main/autogen/crc_builder.h"
#include "main/autogen/data/definitions.h"
#include "main/options/options.h"

namespace sorbet::autogen {

class Autogen final {
public:
    static ParsedFile generate(core::Context ctx, ast::ParsedFile tree, const AutogenConfig &autogenCfg,
                               const CRCBuilder &crcBuilder);
    Autogen() = delete;
};

} // namespace sorbet::autogen
#endif // AUTOGEN_H
