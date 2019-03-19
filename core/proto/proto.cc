// have to be included first as they violate our poisons
#include "core/proto/proto.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/type_resolver_util.h>

#include "absl/strings/str_cat.h"
#include "common/Counters_impl.h"
#include "common/Random.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::core {

com::stripe::rubytyper::Name Proto::toProto(const GlobalState &gs, NameRef name) {
    com::stripe::rubytyper::Name protoName;
    protoName.set_name(name.show(gs));
    switch (name.data(gs)->kind) {
        case UTF8:
            protoName.set_kind(com::stripe::rubytyper::Name::UTF8);
            break;
        case UNIQUE:
            protoName.set_kind(com::stripe::rubytyper::Name::UNIQUE);
            break;
        case CONSTANT:
            protoName.set_kind(com::stripe::rubytyper::Name::CONSTANT);
            break;
    }
    return protoName;
}

com::stripe::rubytyper::Symbol Proto::toProto(const GlobalState &gs, SymbolRef sym) {
    com::stripe::rubytyper::Symbol symbolProto;
    const auto data = sym.data(gs);

    symbolProto.set_id(sym._id);
    *symbolProto.mutable_name() = toProto(gs, data->name);

    if (data->isClass()) {
        symbolProto.set_kind(com::stripe::rubytyper::Symbol::CLASS);
    } else if (data->isStaticField()) {
        symbolProto.set_kind(com::stripe::rubytyper::Symbol::STATIC_FIELD);
    } else if (data->isField()) {
        symbolProto.set_kind(com::stripe::rubytyper::Symbol::FIELD);
    } else if (data->isMethod()) {
        symbolProto.set_kind(com::stripe::rubytyper::Symbol::METHOD);
    } else if (data->isMethodArgument()) {
        symbolProto.set_kind(com::stripe::rubytyper::Symbol::ARGUMENT);
    } else if (data->isTypeMember()) {
        symbolProto.set_kind(com::stripe::rubytyper::Symbol::TYPE_MEMBER);
    } else if (data->isTypeArgument()) {
        symbolProto.set_kind(com::stripe::rubytyper::Symbol::TYPE_ARGUMENT);
    }

    if (data->isClass() || data->isMethod()) {
        if (data->isClass()) {
            for (auto thing : data->mixins()) {
                symbolProto.add_mixins(thing._id);
            }
        } else {
            for (auto thing : data->arguments()) {
                symbolProto.add_arguments(thing._id);
            }
        }

        if (data->isClass() && data->superClass().exists()) {
            symbolProto.set_superclass(data->superClass()._id);
        }
    }

    if (data->isStaticField()) {
        if (auto type = core::cast_type<core::AliasType>(data->resultType.get())) {
            symbolProto.set_aliasto(type->symbol._id);
        }
    }

    for (auto pair : data->membersStableOrderSlow(gs)) {
        if (pair.first == Names::singleton() || pair.first == Names::attached() ||
            pair.first == Names::classMethods()) {
            continue;
        }

        if (!pair.second.exists()) {
            continue;
        }

        *symbolProto.add_children() = toProto(gs, pair.second);
    }

    return symbolProto;
}

com::stripe::rubytyper::Loc Proto::toProto(const GlobalState &gs, Loc loc) {
    com::stripe::rubytyper::Loc protoLoc;
    auto position = protoLoc.mutable_position();
    auto start = position->mutable_start();
    auto end = position->mutable_end();

    if (!loc.exists()) {
        protoLoc.set_path("???");
    } else {
        auto path = loc.file().data(gs).path();
        protoLoc.set_path(string(path));

        auto pos = loc.position(gs);
        start->set_line(pos.first.line);
        start->set_column(pos.first.column);
        end->set_line(pos.second.line);
        end->set_column(pos.second.column);
    }

    return protoLoc;
}

com::stripe::payserver::events::cibot::SourceMetrics Proto::toProto(const CounterState &counters, string_view prefix) {
    com::stripe::payserver::events::cibot::SourceMetrics metrics;
    auto unix_timestamp = chrono::seconds(time(nullptr));
    metrics.set_timestamp(unix_timestamp.count());

    // UUID version 1 as specified in RFC 4122
    string uuid = fmt::format(
        "{:#08x}-{:#04x}-{:#04x}-{:#04x}-{:#08x}{:#04x}",
        (unsigned long long)Random::uniformU8(), // Generates a 64-bit Hex number
        Random::uniformU4(),                     // Generates a 32-bit Hex number
        Random::uniformU4(0, 0x0fff) |
            0x4000, // Generates a 32-bit Hex number of the form 4xxx (4 indicates the UUID version)
        Random::uniformU4(0, 0x3fff) | 0x8000,            // Generates a 32-bit Hex number in the range [0x8000, 0xbfff]
        (unsigned long long)Random::uniformU8(), rand()); // Generates a 96-bit Hex number

    metrics.set_uuid(uuid);

    counters.counters->canonicalize();
    for (auto &cat : counters.counters->countersByCategory) {
        CounterImpl::CounterType sum = 0;
        for (auto &e : cat.second) {
            sum += e.second;
            com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
            metric->set_name(absl::StrCat(prefix, ".", cat.first, ".", e.first));
            metric->set_value(e.second);
        }

        com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", cat.first, ".total"));
        metric->set_value(sum);
    }

    for (auto &hist : counters.counters->histograms) {
        CounterImpl::CounterType sum = 0;
        int histMin = hist.second.begin()->first;
        int histMax = hist.second.begin()->first;
        for (auto &e : hist.second) {
            sum += e.second;
            histMin = min(histMin, e.first);
            histMax = max(histMin, e.first);
        }
        CounterImpl::CounterType running = 0;
        vector<pair<int, bool>> percentiles = {{25, false}, {50, false}, {75, false}, {90, false}};
        for (auto &e : hist.second) {
            running += e.second;
            for (auto &pct : percentiles) {
                if (pct.second) {
                    continue;
                }
                if (running >= sum * pct.first / 100) {
                    pct.second = true;
                    com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric =
                        metrics.add_metrics();
                    metric->set_name(absl::StrCat(prefix, ".", hist.first, ".p", pct.first));
                    metric->set_value(e.first);
                }
            }
        }
        com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", hist.first, ".min"));
        metric->set_value(histMin);

        metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", hist.first, ".max"));
        metric->set_value(histMax);

        metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", hist.first, ".total"));
        metric->set_value(sum);
    }

    for (auto &e : counters.counters->counters) {
        com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", e.first));
        metric->set_value(e.second);
    }

    for (auto &e : counters.counters->timings) {
        if (e.second.size() == 1) {
            com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
            metric->set_name(absl::StrCat(prefix, ".timings.", e.first, ".value"));
            metric->set_value(e.second[0]);
            continue;
        }
        auto histMin = *absl::c_min_element(e.second);
        auto histMax = *absl::c_max_element(e.second);
        auto avg = absl::c_accumulate(e.second, 0) / e.second.size();
        com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".timings.", e.first, ".min"));
        metric->set_value(histMin);

        metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".timings.", e.first, ".max"));
        metric->set_value(histMax);

        metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".timings.", e.first, ".avg"));
        metric->set_value(avg);
    }

    return metrics;
}

com::stripe::rubytyper::File::StrictLevel strictToProto(core::StrictLevel strict) {
    switch (strict) {
        case core::StrictLevel::None:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_None;
        case core::StrictLevel::Internal:
            // we should never attempt to serialize any state that had internal errors
            Exception::raise("Should never happen");
        case core::StrictLevel::Ignore:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_Ignore;
        case core::StrictLevel::Stripe:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_Stripe;
        case core::StrictLevel::Typed:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_Typed;
        case core::StrictLevel::Strict:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_Strict;
        case core::StrictLevel::Strong:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_Strong;
        case core::StrictLevel::Max:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_Max;
        case core::StrictLevel::Autogenerated:
            return com::stripe::rubytyper::File::StrictLevel::File_StrictLevel_Autogenerated;
        default:
            ENFORCE(false, "bad strict level: {}", (int)strict);
    }
}

com::stripe::rubytyper::FileTable Proto::filesToProto(const GlobalState &gs) {
    com::stripe::rubytyper::FileTable files;
    for (int i = 1; i < gs.filesUsed(); ++i) {
        core::FileRef file(i);
        auto *entry = files.add_files();
        auto path_view = file.data(gs).path();
        string path(path_view.begin(), path_view.end());
        entry->set_path(path);
        entry->set_sigil(strictToProto(file.data(gs).originalSigil));
        entry->set_strict(strictToProto(file.data(gs).strictLevel));
        entry->set_min_error_level(strictToProto(file.data(gs).minErrorLevel()));
    }
    return files;
}

string Proto::toJSON(const google::protobuf::Message &message) {
    string jsonString;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = false;
    options.preserve_proto_field_names = true;
    google::protobuf::util::MessageToJsonString(message, &jsonString, options);
    return jsonString;
}

const char *kTypeUrlPrefix = "type.googleapis.com";

void Proto::toJSON(const google::protobuf::Message &message, ostream &out) {
    string binaryProto;
    message.SerializeToString(&binaryProto);

    google::protobuf::io::ArrayInputStream istream(binaryProto.data(), binaryProto.size());
    google::protobuf::io::OstreamOutputStream ostream(&out);

    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = false;
    options.preserve_proto_field_names = true;

    const google::protobuf::DescriptorPool *pool = message.GetDescriptor()->file()->pool();
    unique_ptr<google::protobuf::util::TypeResolver> resolver(
        google::protobuf::util::NewTypeResolverForDescriptorPool(kTypeUrlPrefix, pool));

    string url = absl::StrCat(kTypeUrlPrefix, "/", message.GetDescriptor()->full_name());
    auto status = google::protobuf::util::BinaryToJsonStream(resolver.get(), url, &istream, &ostream, options);
    if (!status.ok()) {
        cerr << "error converting to proto json: " << status.error_message() << '\n';
        abort();
    }
}

} // namespace sorbet::core
