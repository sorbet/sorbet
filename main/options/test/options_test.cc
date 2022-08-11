#include "doctest.h"
// has to go first as it violates our requirements
#include "spdlog/spdlog.h"
// has to go above null_sink.h; this comment prevents reordering.
#include "main/options/options.h"
#include "spdlog/sinks/null_sink.h"

// Default constructor of options should produce the same default option values as readOptions.
TEST_CASE("DefaultConstructorMatchesReadOptions") {
    std::vector<sorbet::pipeline::semantic_extension::SemanticExtensionProvider *> extensionProviders;
    std::vector<std::unique_ptr<sorbet::pipeline::semantic_extension::SemanticExtension>> extensions;
    sorbet::realmain::options::Options empty;
    sorbet::realmain::options::Options opts;
    const char *argv[] = {"sorbet", "-e", ""};
    auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("null", sink);
    sorbet::realmain::options::readOptions(opts, extensions, std::size(argv), const_cast<char **>(argv),
                                           extensionProviders, logger);

    // Check that both options objects have same field values.
    // NOTE: Add your field here if you add a new field to Options.
    CHECK_EQ(empty.stdoutHUPHack, opts.stdoutHUPHack);
    CHECK_EQ(empty.forceMinStrict, opts.forceMinStrict);
    CHECK_EQ(empty.forceMaxStrict, opts.forceMaxStrict);
    CHECK_EQ(empty.showProgress, opts.showProgress);
    CHECK_EQ(empty.suggestTyped, opts.suggestTyped);
    CHECK_EQ(empty.silenceErrors, opts.silenceErrors);
    CHECK_EQ(empty.silenceDevMessage, opts.silenceDevMessage);
    CHECK_EQ(empty.suggestSig, opts.suggestSig);
    CHECK_EQ(empty.suppressNonCriticalErrors, opts.suppressNonCriticalErrors);
    CHECK_EQ(empty.runLSP, opts.runLSP);
    CHECK_EQ(empty.disableWatchman, opts.disableWatchman);
    CHECK_EQ(empty.watchmanPath, opts.watchmanPath);
    CHECK_EQ(empty.stressIncrementalResolver, opts.stressIncrementalResolver);
    CHECK_EQ(empty.noErrorCount, opts.noErrorCount);
    CHECK_EQ(empty.autocorrect, opts.autocorrect);
    CHECK_EQ(empty.waitForDebugger, opts.waitForDebugger);
    CHECK_EQ(empty.threads, opts.threads);
    CHECK_EQ(empty.logLevel, opts.logLevel);
    CHECK_EQ(empty.autogenVersion, opts.autogenVersion);
    CHECK_EQ(empty.typedSource, opts.typedSource);
    CHECK_EQ(empty.cacheDir, opts.cacheDir);
    CHECK_EQ(empty.strictnessOverrides.size(), opts.strictnessOverrides.size());
    CHECK_EQ(empty.storeState, opts.storeState);
    CHECK_EQ(empty.enableCounters, opts.enableCounters);
    CHECK_EQ(empty.errorUrlBase, opts.errorUrlBase);
    CHECK_EQ(empty.isolateErrorCode, opts.isolateErrorCode);
    CHECK_EQ(empty.suppressErrorCode, opts.suppressErrorCode);
    CHECK_EQ(empty.pathPrefix, opts.pathPrefix);
    CHECK_EQ(empty.reserveUtf8NameTableCapacity, opts.reserveUtf8NameTableCapacity);
    CHECK_EQ(empty.reserveConstantNameTableCapacity, opts.reserveConstantNameTableCapacity);
    CHECK_EQ(empty.reserveUniqueNameTableCapacity, opts.reserveUniqueNameTableCapacity);
    CHECK_EQ(empty.reserveClassTableCapacity, opts.reserveClassTableCapacity);
    CHECK_EQ(empty.reserveMethodTableCapacity, opts.reserveMethodTableCapacity);
    CHECK_EQ(empty.reserveFieldTableCapacity, opts.reserveFieldTableCapacity);
    CHECK_EQ(empty.reserveTypeArgumentTableCapacity, opts.reserveTypeArgumentTableCapacity);
    CHECK_EQ(empty.reserveTypeMemberTableCapacity, opts.reserveTypeMemberTableCapacity);
    CHECK_EQ(empty.statsdHost, opts.statsdHost);
    CHECK_EQ(empty.statsdPrefix, opts.statsdPrefix);
    CHECK_EQ(empty.statsdPort, opts.statsdPort);
    CHECK_EQ(empty.metricsFile, opts.metricsFile);
    CHECK_EQ(empty.metricsRepo, opts.metricsRepo);
    CHECK_EQ(empty.metricsPrefix, opts.metricsPrefix);
    CHECK_EQ(empty.metricsBranch, opts.metricsBranch);
    CHECK_EQ(empty.metricsSha, opts.metricsSha);
    CHECK_EQ(empty.rawInputFileNames.size(), opts.rawInputFileNames.size());
    CHECK_EQ(empty.rawInputDirNames.size(), opts.rawInputDirNames.size());
    CHECK_EQ(empty.absoluteIgnorePatterns.size(), opts.absoluteIgnorePatterns.size());
    CHECK_EQ(empty.relativeIgnorePatterns.size(), opts.relativeIgnorePatterns.size());
    CHECK_EQ(empty.inputFileNames.size(), opts.inputFileNames.size());
    CHECK_EQ(empty.lspDocumentSymbolEnabled, opts.lspDocumentSymbolEnabled);
    CHECK_EQ(empty.lspDocumentHighlightEnabled, opts.lspDocumentHighlightEnabled);
    CHECK_EQ(empty.lspSignatureHelpEnabled, opts.lspSignatureHelpEnabled);
    CHECK_EQ(empty.lspDocumentFormatRubyfmtEnabled, opts.lspDocumentFormatRubyfmtEnabled);
    CHECK_EQ(empty.rubyfmtPath, opts.rubyfmtPath);
    CHECK_EQ(empty.inlineInput, opts.inlineInput);
    CHECK_EQ(empty.debugLogFile, opts.debugLogFile);
    CHECK_EQ(empty.webTraceFile, opts.webTraceFile);
    CHECK_EQ(empty.stripeMode, opts.stripeMode);
    CHECK_EQ(empty.stripePackages, opts.stripePackages);
    CHECK_EQ(empty.forceHashing, opts.forceHashing);
    CHECK_EQ(empty.lspErrorCap, opts.lspErrorCap);
}
