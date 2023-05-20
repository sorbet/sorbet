#ifndef DSL_ANALYSIS_H
#define DSL_ANALYSIS_H

#include "main/autogen/crc_builder.h"
#include "main/autogen/data/definitions.h"
#include "main/options/options.h"

namespace sorbet::autogen {

class TokenPropAnalysis final {
public:
    static TokenPropAnalysisFile generate(core::Context ctx, ast::ParsedFile tree, const CRCBuilder &crcBuilder);
    TokenPropAnalysis() = delete;
};

} // namespace sorbet::autogen
#endif // AUTOGEN_H
