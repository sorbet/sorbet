#include "common/opentelemetry/opentelemetry.h"
#include "common/Counters_impl.h"
#include "opentelemetry/exporters/ostream/span_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"
#include "sorbet_version/sorbet_version.h"

namespace {
void initTracer() {
    auto exporter = std::unique_ptr<opentelemetry::sdk::trace::SpanExporter>(
        new opentelemetry::exporter::trace::OStreamSpanExporter);
    auto processor = std::unique_ptr<opentelemetry::sdk::trace::SpanProcessor>(
        new opentelemetry::sdk::trace::SimpleSpanProcessor(std::move(exporter)));
    auto provider = opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>(
        new opentelemetry::sdk::trace::TracerProvider(std::move(processor)));

    // Set the global trace provider
    opentelemetry::trace::Provider::SetTracerProvider(provider);
}
} // namespace

// int main()
// {
//   // Removing this line will leave the default noop TracerProvider in place.
//   initTracer();

//   foo_library();
// }

using namespace std;

namespace sorbet {

void OpenTelemetry::submitTimers(const CounterState &counters) {
    fmt::print("Hello, world!\n");

    // TODO(jez) Probably only need to call this once, globally? Maybe do it earlier in realmain,
    // common to both LSP and non-LSP?
    initTracer();

    auto provider = opentelemetry::trace::Provider::GetTracerProvider();
    // TODO(jez) Where does this string show up? What should we pick?
    auto libraryName = "sorbet";
    auto libraryVersion = sorbet_full_version_string;
    auto tracer = provider->GetTracer(libraryName, libraryVersion);

    // auto now = Timer::clock_gettime_coarse();

    for (const auto &timing : counters.counters->timings) {
        auto startTimestamp = opentelemetry::common::SteadyTimestamp{chrono::microseconds{timing.start.usec}};
        auto endTimestamp = opentelemetry::common::SteadyTimestamp{chrono::microseconds{timing.end.usec}};

        auto startOptions = opentelemetry::trace::StartSpanOptions{};
        startOptions.start_steady_time = startTimestamp;
        auto span = tracer->StartSpan(timing.measure, startOptions);

        span->End(opentelemetry::trace::EndSpanOptions{endTimestamp});
    }
}

} // namespace sorbet
