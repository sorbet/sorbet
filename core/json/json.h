#ifndef SORBET_CORE_JSON_H
#define SORBET_CORE_JSON_H

// has to go first as it defines ENFORCE which rapidjson uses via RAPIDJSON_ASSERT
#include "core/core.h"
// ^^ has to go first
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

namespace sorbet::core {

class JSON {
public:
    JSON() = delete;

    static void fileToJSON(rapidjson::PrettyWriter<rapidjson::StringBuffer> &writer, const GlobalState &gs,
                           FileRef file, long untypedUsages);

    static void symbolToJSON(rapidjson::PrettyWriter<rapidjson::StringBuffer> &writer, const GlobalState &gs,
                             SymbolRef sym, bool showFull);

    static std::string metricsToJSON(const CounterState &counters, std::string_view prefix, std::string_view repo,
                                     std::string_view branch, std::string_view sha, std::string_view status);
};

} // namespace sorbet::core

#endif
