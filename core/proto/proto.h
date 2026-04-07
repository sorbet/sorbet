#ifndef SORBET_CORE_PROTO_H
#define SORBET_CORE_PROTO_H
// have to go first as they violate our poisons
#include "proto/pay-server/SourceMetrics.pb.h"
#include <google/protobuf/json/json.h>

#include "core/core.h"

namespace sorbet::core {
class Proto {
public:
    Proto() = delete;

    static com::stripe::payserver::events::cibot::SourceMetrics toProto(const CounterState &counters,
                                                                        std::string_view prefix);

    static std::string toJSON(const google::protobuf::Message &message);
};
} // namespace sorbet::core

#endif
