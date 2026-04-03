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
};

} // namespace sorbet::core

#endif
