#include "common/opentelemetry/opentelemetry.h"

#include "opentelemetry/sdk/trace/simple_processor.h"
// #include "opentelemetry/sdk/trace/tracer_provider.h"
// #include "opentelemetry/trace/provider.h"

// // Using an exporter that simply dumps span data to stdout.
// #ifdef BAZEL_BUILD
// #  include "examples/common/foo_library/foo_library.h"
// #else
// #  include "foo_library/foo_library.h"
// #endif
// #include "opentelemetry/exporters/ostream/span_exporter.h"

// namespace trace_api = opentelemetry::trace;
// namespace trace_sdk = opentelemetry::sdk::trace;
// namespace nostd     = opentelemetry::nostd;
// namespace
// {
// void initTracer()
// {
//   auto exporter = std::unique_ptr<trace_sdk::SpanExporter>(
//       new opentelemetry::exporter::trace::OStreamSpanExporter);
//   auto processor = std::unique_ptr<trace_sdk::SpanProcessor>(
//       new trace_sdk::SimpleSpanProcessor(std::move(exporter)));
//   auto provider = nostd::shared_ptr<trace_api::TracerProvider>(
//       new trace_sdk::TracerProvider(std::move(processor)));

//   // Set the global trace provider
//   trace_api::Provider::SetTracerProvider(provider);
// }
// }  // namespace

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
}

} // namespace sorbet
