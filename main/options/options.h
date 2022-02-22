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
    template <typename... Args> void fmt(fmt::format_string<Args...> msg, Args &&...args) const {
        auto contents = fmt::format(msg, std::forward<Args>(args)...);
        print(contents);
    }
    void flush();
    std::string flushToString();

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
    PrinterConfig FileTableFullJson;
    PrinterConfig FileTableFullProto;
    PrinterConfig FileTableFullMessagePack;
    PrinterConfig MissingConstants;
    PrinterConfig Autogen;
    PrinterConfig AutogenMsgPack;
    PrinterConfig AutogenAutoloader;
    PrinterConfig AutogenSubclasses;
    PrinterConfig Packager;
    PrinterConfig MinimizeRBI;
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
    PACKAGER,
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
    bool traceLexer = false;
    bool traceParser = false;
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
    std::string stripePackagesHint = "";
    std::vector<std::string> extraPackageFilesDirectoryPrefixes;
    std::vector<std::string> secondaryTestPackageNamespaces;
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

    uint32_t reserveClassTableCapacity = 0;
    uint32_t reserveMethodTableCapacity = 0;
    uint32_t reserveFieldTableCapacity = 0;
    uint32_t reserveTypeArgumentTableCapacity = 0;
    uint32_t reserveTypeMemberTableCapacity = 0;
    uint32_t reserveUtf8NameTableCapacity = 0;
    uint32_t reserveConstantNameTableCapacity = 0;
    uint32_t reserveUniqueNameTableCapacity = 0;

    /* The maximum number of files that are permitted to typecheck on the fast path concurrently. Not exposed on CLI.
     * Placed on Options for convenience so tests can override. */
    uint32_t lspMaxFilesOnFastPath = 50;
    /* The maximum number of errors to report to the client when in LSP mode. Prevents editor UI slowdown
     * related to large error lists. 0 means no limit. */
    uint32_t lspErrorCap = 1000;

    std::string statsdHost;
    std::string statsdPrefix = "ruby_typer.unknown";
    int statsdPort = 8200;

    std::string metricsFile;
    std::string metricsRepo = "none";
    std::string metricsPrefix = "ruby_typer.unknown.";
    std::string metricsBranch = "none";
    std::string metricsSha = "none";
    std::map<std::string, std::string> metricsExtraTags; // be super careful with cardinality here

    std::string dumpPackageInfo = "";
    std::string singlePackage = "";
    std::string packageRBIOutput = "";

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

    // Path to an RBI whose contents should be minimized by subtracting parts that Sorbet already
    // has a record of in its own GlobalState.
    //
    // Used primarily by missing_methods.rb / `srb rbi hiddden-definitions` to minimize an RBI file
    // generated by requiring a codebase and inspecting it via reflection.
    std::string minimizeRBI;

    std::shared_ptr<FileSystem> fs = std::make_shared<OSFileSystem>();

    void flushPrinters();

    Options clone() {
        Options options(*this);
        return options;
    };

    Options() = default;

    Options(Options &&) = default;

    Options &operator=(Options &&) = delete;

private:
    Options(const Options &) = default;
    Options &operator=(const Options &) = default;
};

void readOptions(
    Options &, std::vector<std::unique_ptr<pipeline::semantic_extension::SemanticExtension>> &configuredExtensions,
    int argc, char *argv[],
    const std::vector<pipeline::semantic_extension::SemanticExtensionProvider *> &semanticExtensionProviders,
    std::shared_ptr<spdlog::logger> logger) noexcept(false); // throw(EarlyReturnWithCode);

void flushPrinters(Options &);
} // namespace sorbet::realmain::options
#endif // RUBY_TYPER_OPTIONS_H
