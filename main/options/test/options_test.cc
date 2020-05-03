#include "gtest/gtest.h"
// has to go first as it violates our requirements
#include "spdlog/spdlog.h"
// has to go above null_sink.h; this comment prevents reordering.
#include "main/options/options.h"
#include "spdlog/sinks/null_sink.h"

// Default constructor of options should produce the same default option values as readOptions.
TEST(OptionsTest, DefaultConstructorMatchesReadOptions) {
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
    EXPECT_EQ(empty.stdoutHUPHack, opts.stdoutHUPHack);
    EXPECT_EQ(empty.forceMinStrict, opts.forceMinStrict);
    EXPECT_EQ(empty.forceMaxStrict, opts.forceMaxStrict);
    EXPECT_EQ(empty.showProgress, opts.showProgress);
    EXPECT_EQ(empty.suggestTyped, opts.suggestTyped);
    EXPECT_EQ(empty.silenceErrors, opts.silenceErrors);
    EXPECT_EQ(empty.silenceDevMessage, opts.silenceDevMessage);
    EXPECT_EQ(empty.suggestSig, opts.suggestSig);
    EXPECT_EQ(empty.supressNonCriticalErrors, opts.supressNonCriticalErrors);
    EXPECT_EQ(empty.runLSP, opts.runLSP);
    EXPECT_EQ(empty.disableWatchman, opts.disableWatchman);
    EXPECT_EQ(empty.watchmanPath, opts.watchmanPath);
    EXPECT_EQ(empty.stressIncrementalResolver, opts.stressIncrementalResolver);
    EXPECT_EQ(empty.noErrorCount, opts.noErrorCount);
    EXPECT_EQ(empty.autocorrect, opts.autocorrect);
    EXPECT_EQ(empty.waitForDebugger, opts.waitForDebugger);
    EXPECT_EQ(empty.skipRewriterPasses, opts.skipRewriterPasses);
    EXPECT_EQ(empty.suggestRuntimeProfiledType, opts.suggestRuntimeProfiledType);
    EXPECT_EQ(empty.threads, opts.threads);
    EXPECT_EQ(empty.logLevel, opts.logLevel);
    EXPECT_EQ(empty.autogenVersion, opts.autogenVersion);
    EXPECT_EQ(empty.typedSource, opts.typedSource);
    EXPECT_EQ(empty.cacheDir, opts.cacheDir);
    EXPECT_EQ(empty.configatronDirs.size(), opts.configatronDirs.size());
    EXPECT_EQ(empty.configatronFiles.size(), opts.configatronFiles.size());
    EXPECT_EQ(empty.strictnessOverrides.size(), opts.strictnessOverrides.size());
    EXPECT_EQ(empty.dslPluginTriggers.size(), opts.dslPluginTriggers.size());
    EXPECT_EQ(empty.dslRubyExtraArgs.size(), opts.dslRubyExtraArgs.size());
    EXPECT_EQ(empty.storeState, opts.storeState);
    EXPECT_EQ(empty.enableCounters, opts.enableCounters);
    EXPECT_EQ(empty.errorUrlBase, opts.errorUrlBase);
    EXPECT_EQ(empty.errorCodeWhiteList, opts.errorCodeWhiteList);
    EXPECT_EQ(empty.errorCodeBlackList, opts.errorCodeBlackList);
    EXPECT_EQ(empty.pathPrefix, opts.pathPrefix);
    EXPECT_EQ(empty.reserveMemKiB, opts.reserveMemKiB);
    EXPECT_EQ(empty.statsdHost, opts.statsdHost);
    EXPECT_EQ(empty.statsdPrefix, opts.statsdPrefix);
    EXPECT_EQ(empty.statsdPort, opts.statsdPort);
    EXPECT_EQ(empty.metricsFile, opts.metricsFile);
    EXPECT_EQ(empty.metricsRepo, opts.metricsRepo);
    EXPECT_EQ(empty.metricsPrefix, opts.metricsPrefix);
    EXPECT_EQ(empty.metricsBranch, opts.metricsBranch);
    EXPECT_EQ(empty.metricsSha, opts.metricsSha);
    EXPECT_EQ(empty.rawInputFileNames.size(), opts.rawInputFileNames.size());
    EXPECT_EQ(empty.rawInputDirNames.size(), opts.rawInputDirNames.size());
    EXPECT_EQ(empty.absoluteIgnorePatterns.size(), opts.absoluteIgnorePatterns.size());
    EXPECT_EQ(empty.relativeIgnorePatterns.size(), opts.relativeIgnorePatterns.size());
    EXPECT_EQ(empty.inputFileNames.size(), opts.inputFileNames.size());
    EXPECT_EQ(empty.lspDocumentSymbolEnabled, opts.lspDocumentSymbolEnabled);
    EXPECT_EQ(empty.lspDocumentHighlightEnabled, opts.lspDocumentHighlightEnabled);
    EXPECT_EQ(empty.lspSignatureHelpEnabled, opts.lspSignatureHelpEnabled);
    EXPECT_EQ(empty.inlineInput, opts.inlineInput);
    EXPECT_EQ(empty.debugLogFile, opts.debugLogFile);
    EXPECT_EQ(empty.webTraceFile, opts.webTraceFile);
}
