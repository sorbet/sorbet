#ifndef DSL_ANALYSIS_H
#define DSL_ANALYSIS_H

#include "main/autogen/crc_builder.h"
#include "main/autogen/data/definitions.h"
#include "main/options/options.h"

namespace sorbet::autogen {

class DSLAnalysis final {
public:
    static DSLAnalysisFile generate(core::Context ctx, ast::ParsedFile tree, const CRCBuilder &crcBuilder);
    DSLAnalysis() = delete;
};

} // namespace sorbet::autogen
#endif // AUTOGEN_H

