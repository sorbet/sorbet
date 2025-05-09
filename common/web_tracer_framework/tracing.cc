#include "common/FileOps.h"
#include "common/counters/Counters.h"

#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "common/JSON.h"
#include "common/counters/Counters_impl.h"
#include "common/strings/formatting.h"
#include "common/web_tracer_framework/tracing.h"
#include "rapidjson/writer.h"
#include "sorbet_version/sorbet_version.h"
#include <string>
#include <unistd.h>

using namespace std;

namespace sorbet::web_tracer_framework {

namespace {

void endLine(rapidjson::StringBuffer &result, rapidjson::Writer<rapidjson::StringBuffer> &writer) {
    // From the docs:
    //
    // > This function reset the writer with a new stream and default settings,
    // > in order to make a Writer object reusable for output multiple JSONs.
    //
    // We're using it so we can manage writing the top-level array ourselves
    // (one object per line, no closing ] and re-use previous file data).
    writer.Reset(result);

    result.Put(',');
    result.Put('\n');
}

} // namespace

string Tracing::stateToJSONL(const CounterState &counters, pid_t pid, microseconds now) {
    rapidjson::StringBuffer result;
    rapidjson::Writer<rapidjson::StringBuffer> writer(result);

    // header / meta information
    {
        writer.StartObject();

        writer.String("name");
        writer.String("process_name");

        writer.String("ph");
        writer.String("M");

        writer.String("pid");
        writer.Int(pid);

        writer.String("args");
        writer.StartObject();
        writer.String("name");
        writer.String(fmt::format("Sorbet v{}", sorbet_full_version_string));
        writer.EndObject();

        writer.EndObject();
        endLine(result, writer);
    }

    counters.counters->canonicalize();

    for (auto &[category, counters] : counters.counters->countersByCategory) {
        writer.StartObject();

        writer.String("name");
        writer.String(category);

        writer.String("ph");
        writer.String("C");

        writer.String("ts");
        auto ts = fmt::format("{:.3f}", now.usec * 1.0);
        writer.RawValue(ts.c_str(), ts.length(), rapidjson::Type::kNumberType);

        writer.String("pid");
        writer.Int(pid);

        writer.String("args");
        writer.StartObject();
        for (const auto &[counterName, value] : counters) {
            writer.String(counterName);
            writer.String(fmt::format("{}", value));
        }
        writer.EndObject();

        writer.EndObject();
        endLine(result, writer);
    }

    for (auto &[counterName, value] : counters.counters->counters) {
        writer.StartObject();

        writer.String("name");
        writer.String(counterName);

        writer.String("ph");
        writer.String("C");

        writer.String("ts");
        auto ts = fmt::format("{:.3f}", now.usec * 1.0);
        writer.RawValue(ts.c_str(), ts.length(), rapidjson::Type::kNumberType);

        writer.String("pid");
        writer.Int(pid);

        writer.String("args");
        writer.StartObject();
        writer.String("value");
        writer.String(fmt::format("{}", value));
        writer.EndObject();

        writer.EndObject();
        endLine(result, writer);
    }

    // For some reason, emitting all of our counters breaks flow event visualitaion.
    // @dmitry decided to not emit histograms

    for (const auto &timing : counters.counters->timings) {
        writer.StartObject();

        writer.String("name");
        writer.String(timing.measure);

        writer.String("ph");
        writer.String("X");

        writer.String("ts");
        auto ts = fmt::format("{:.3f}", timing.start.usec * 1.0);
        writer.RawValue(ts.c_str(), ts.length(), rapidjson::Type::kNumberType);

        writer.String("dur");
        auto dur = fmt::format("{:.3f}", (timing.end.usec - timing.start.usec) * 1.0);
        writer.RawValue(dur.c_str(), dur.length(), rapidjson::Type::kNumberType);

        writer.String("pid");
        writer.Int(pid);

        writer.String("tid");
        writer.Int(timing.threadId);

        if ((timing.args != nullptr && !timing.args->empty()) || (timing.tags != nullptr && !timing.tags->empty())) {
            writer.String("args");

            writer.StartObject();
            // Puts all tags and args in the same namespace, and does not check for overlaps.
            if (timing.args != nullptr && !timing.args->empty()) {
                for (const auto &[key, value] : *timing.args) {
                    writer.String(key);
                    writer.String(value);
                }
            }
            if (timing.tags != nullptr && !timing.tags->empty()) {
                for (const auto &[key, value] : *timing.tags) {
                    writer.String(key);
                    writer.String(value);
                }
            }
            writer.EndObject();
        }

        if (timing.self.id != 0) {
            ENFORCE(timing.prev.id == 0);

            writer.String("bind_id");
            writer.Int(timing.self.id);

            writer.String("flow_out");
            writer.Bool(true);
        } else if (timing.prev.id != 0) {
            writer.String("bind_id");
            writer.Int(timing.prev.id);

            writer.String("flow_in");
            writer.Bool(true);
        }

        writer.EndObject();
        endLine(result, writer);
    }

    return result.GetString();
}

string Tracing::jsonlToJSON(const string &jsonl, bool needsOpeningBracket, bool strictClosing) {
    string result;

    if (needsOpeningBracket) {
        result += "[\n";
    }

    result += jsonl;

    // Strict generation is useful when generating this file for non-Perfetto tools; non-strict
    // is useful for general forgetfulness and for doing tracing when LSP is active, as LSP will
    // continually append new entries to the file.
    if (strictClosing) {
        // Generating the JSONL will have appended ",\n"; we want to make `result` valid JSON,
        // so we need to strip the ",".
        if (absl::EndsWith(result, ",\n")) {
            result[result.size() - 2] = '\n';
            result[result.size() - 1] = ']';
        }
    }
    result += '\n';

    return result;
}

// Super rudimentary support for outputting trace files in Google's Trace Event Format
// https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/preview
bool Tracing::storeTraces(const CounterState &counters, const string &fileName, bool strict) {
    auto now = Timer::clock_gettime_coarse();
    auto pid = getpid();

    string jsonl = stateToJSONL(counters, pid, now);
    string result = jsonlToJSON(jsonl, !FileOps::exists(fileName), strict);

    FileOps::append(fileName, result);
    return true;
}
} // namespace sorbet::web_tracer_framework
