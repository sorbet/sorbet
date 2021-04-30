#ifndef RUBY_TYPER_OPTIONS_H
#define RUBY_TYPER_OPTIONS_H
#include "common/ConstExprStr.h"
#include "common/EarlyReturnWithCode.h"
#include "common/FileSystem.h"
#include "common/common.h"
#include "core/StrictLevel.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include "spdlog/spdlog.h"
#include <optional>

namespace sorbet::realmain::options {

class PrinterConfig {
public:
    bool enabled = false;
    std::string outputPath;
    bool supportsFlush = false;

    void print(const std::string_view &contents) const;
    template <typename... Args> void fmt(const ConstExprStr msg, Args &&...args) const {
        print(fmt::format(msg.str, std::forward<Args>(args)...));
    }
    void flush();

    PrinterConfig();
    PrinterConfig(const PrinterConfig &) = default;
    PrinterConfig(PrinterConfig &&) = default;
    PrinterConfig &operator=(const PrinterConfig &) = default;
    PrinterConfig &operator=(PrinterConfig &&) = default;

private:
    struct GuardedState {
        fmt::memory_buffer buf;
        absl::Mutex mutex;
    };
    std::shared_ptr<GuardedState> state;
};

struct Printers {
    PrinterConfig ParseTree;
    PrinterConfig ParseTreeJson;
    PrinterConfig ParseTreeJsonWithLocs;
    PrinterConfig ParseTreeWhitequark;
    PrinterConfig DesugarTree;
    PrinterConfig DesugarTreeRaw;
    PrinterConfig RewriterTree;
    PrinterConfig RewriterTreeRaw;
    PrinterConfig IndexTree;
    PrinterConfig IndexTreeRaw;
    PrinterConfig NameTree;
    PrinterConfig NameTreeRaw;
    PrinterConfig ResolveTree;
    PrinterConfig ResolveTreeRaw;
    PrinterConfig FlattenTree;
    PrinterConfig FlattenTreeRaw;
    PrinterConfig AST;
    PrinterConfig ASTRaw;
    PrinterConfig CFG;
    PrinterConfig CFGText;
    PrinterConfig CFGRaw;
    PrinterConfig TypedSource;
    PrinterConfig SymbolTable;
    PrinterConfig SymbolTableRaw;
    PrinterConfig SymbolTableProto;
    PrinterConfig SymbolTableMessagePack;
    PrinterConfig SymbolTableJson;
    PrinterConfig SymbolTableFull;
    PrinterConfig SymbolTableFullRaw;
    PrinterConfig SymbolTableFullProto;
    PrinterConfig SymbolTableFullMessagePack;
    PrinterConfig SymbolTableFullJson;
    PrinterConfig FileTableJson;
    PrinterConfig FileTableProto;
    PrinterConfig FileTableMessagePack;
    PrinterConfig MissingConstants;
    PrinterConfig Autogen;
    PrinterConfig AutogenMsgPack;
    PrinterConfig AutogenClasslist;
    PrinterConfig AutogenAutoloader;
    PrinterConfig AutogenSubclasses;
    PrinterConfig Packager;
    // Ensure everything here is in PrinterConfig::printers().

    std::vector<std::reference_wrapper<PrinterConfig>> printers();
    bool isAutogen() const;
};

enum class Phase {
    INIT,
    PARSER,
    DESUGARER,
    REWRITER,
    LOCAL_VARS,
    NAMER,
    RESOLVER,
    CFG,
    INFERENCER,
};

struct AutoloaderConfig {
    // Top-level modules to include in autoloader output
    std::vector<std::string> modules;
    std::string rootDir;
    std::string preamble;
    std::string registryModule;
    std::string rootObject;
    std::vector<std::string> requireExcludes;
    std::vector<std::vector<std::string>> sameFileModules;
    std::vector<std::string> stripPrefixes;

    std::vector<std::string> absoluteIgnorePatterns;
    std::vector<std::string> relativeIgnorePatterns;

    bool packagedAutoloader;
};

struct Options {
    Printers print;
    AutoloaderConfig autoloaderConfig;
    Phase stopAfterPhase = Phase::INFERENCER;
    bool noStdlib = false;

    // Should we monitor STDOUT for HUP and exit if it hangs up. This is a
    // workaround for https://bugzilla.mindrot.org/show_bug.cgi?id=2863
    bool stdoutHUPHack = false;

    core::StrictLevel forceMinStrict = core::StrictLevel::Ignore;
    core::StrictLevel forceMaxStrict = core::StrictLevel::Strong;

    bool showProgress = false;
    bool suggestTyped = false;
    std::optional<std::string> suggestUnsafe = std::nullopt;
    bool silenceErrors = false;
    bool silenceDevMessage = false;
    bool suggestSig = false;
    bool supressNonCriticalErrors = false;
    bool runLSP = false;
    bool disableWatchman = false;
    std::string watchmanPath = "watchman";
    bool stressIncrementalResolver = false;
    bool sleepInSlowPath = false;
    bool noErrorCount = false;
    bool autocorrect = false;
    bool waitForDebugger = false;
    bool skipRewriterPasses = false;
    bool censorForSnapshotTests = false;
    bool forceHashing = false;
    int threads = 0;
    int logLevel = 0; // number of time -v was passed
    int autogenVersion = 0;
    bool stripeMode = false;
    bool stripePackages = false;
    std::string typedSource = "";
    std::string cacheDir = "";
    UnorderedMap<std::string, core::StrictLevel> strictnessOverrides;
    std::string storeState = "";
    bool enableCounters = false;
    std::string errorUrlBase = "https://srb.help/";
    bool ruby3KeywordArgs = false;
    std::set<int> isolateErrorCode;
    std::set<int> suppressErrorCode;
    bool noErrorSections = false;
    /** Prefix to remove from all printed paths. */
    std::string pathPrefix;

    u4 reserveClassTableCapacity = 0;
    u4 reserveMethodTableCapacity = 0;
    u4 reserveFieldTableCapacity = 0;
    u4 reserveTypeArgumentTableCapacity = 0;
    u4 reserveTypeMemberTableCapacity = 0;
    u4 reserveUtf8NameTableCapacity = 0;
    u4 reserveConstantNameTableCapacity = 0;
    u4 reserveUniqueNameTableCapacity = 0;

    /* The maximum number of files that are permitted to typecheck on the fast path concurrently. Not exposed on CLI.
     * Placed on Options for convenience so tests can override. */
    u4 lspMaxFilesOnFastPath = 50;

    std::string statsdHost;
    std::string statsdPrefix = "ruby_typer.unknown";
    int statsdPort = 8200;

    std::string metricsFile;
    std::string metricsRepo = "none";
    std::string metricsPrefix = "ruby_typer.unknown.";
    std::string metricsBranch = "none";
    std::string metricsSha = "none";
    std::map<std::string, std::string> metricsExtraTags; // be super careful with cardinality here

    // Contains the allowed extensions Sorbet can parse.
    UnorderedSet<std::string> allowedExtensions;
    // Contains the file names passed in to Sorbet.
    std::vector<std::string> rawInputFileNames;
    // Contains the directory names passed in to Sorbet.
    std::vector<std::string> rawInputDirNames;
    // Ignore patterns beginning from the root of an input folder.
    std::vector<std::string> absoluteIgnorePatterns;
    // Ignore patterns that can occur anywhere in a file's path from an input folder.
    std::vector<std::string> relativeIgnorePatterns;
    // Contains the expanded list of all Ruby file inputs (rawInputFileNames + all Ruby files in rawInputDirNames)
    std::vector<std::string> inputFileNames;
    // A list of parent classes to be used in `-p autogen-subclasses`
    std::vector<std::string> autogenSubclassesParents;
    // Ignore patterns beginning from the root of an input folder.
    std::vector<std::string> autogenSubclassesAbsoluteIgnorePatterns;
    // Ignore patterns that can occur anywhere in a file's path from an input folder.
    std::vector<std::string> autogenSubclassesRelativeIgnorePatterns;
    // List of directories not available editor-side. References to files in these directories should be sent via
    // sorbet: URIs to clients that support them.
    std::vector<std::string> lspDirsMissingFromClient;
    // Enable stable-but-not-yet-shipped features suitable for late-stage beta-testing.
    bool lspAllBetaFeaturesEnabled = false;
    // Booleans enabling various experimental LSP features. Each will be removed once corresponding feature stabilizes.
    bool lspDocumentHighlightEnabled = false;
    bool lspDocumentSymbolEnabled = false;
    bool lspDocumentFormatRubyfmtEnabled = false;
    bool lspSignatureHelpEnabled = false;

    // Experimental feature `requires_ancestor`
    bool requiresAncestorEnabled = false;

    std::string inlineInput; // passed via -e
    std::string debugLogFile;
    std::string webTraceFile;

    std::shared_ptr<FileSystem> fs = std::make_shared<OSFileSystem>();

    void flushPrinters();

    Options() = default;

    Options(const Options &) = delete;

    Options(Options &&) = default;

    Options &operator=(const Options &) = delete;

    Options &operator=(Options &&) = delete;
};

void readOptions(
    Options &, std::vector<std::unique_ptr<pipeline::semantic_extension::SemanticExtension>> &configuredExtensions,
    int argc, char *argv[],
    const std::vector<pipeline::semantic_extension::SemanticExtensionProvider *> &semanticExtensionProviders,
    std::shared_ptr<spdlog::logger> logger) noexcept(false); // throw(EarlyReturnWithCode);

void flushPrinters(Options &);
} // namespace sorbet::realmain::options
#endif // RUBY_TYPER_OPTIONS_H
