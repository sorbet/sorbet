#ifndef RUBY_TYPER_OPTIONS_H
#define RUBY_TYPER_OPTIONS_H
#include "common/FileSystem.h"
#include "common/common.h"
#include "core/StrictLevel.h"
#include "spdlog/spdlog.h"

namespace sorbet::realmain::options {

// Terminate execution of sorbet with specific return code
class EarlyReturnWithCode : public SorbetException {
public:
    EarlyReturnWithCode(int returnCode);
    const int returnCode;
};

class PrinterConfig {
public:
    bool enabled = false;
    std::string outputPath;

    void print(spdlog::logger& logger, const std::string_view &contents) const;
    template <typename... Args> void fmt(spdlog::logger &logger, const std::string &msg, const Args &... args) const {
        print(logger, fmt::format(msg, args...));
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
    PrinterConfig Desugared;
    PrinterConfig DesugaredRaw;
    PrinterConfig DSLTree;
    PrinterConfig DSLTreeRaw;
    PrinterConfig NameTree;
    PrinterConfig NameTreeRaw;
    PrinterConfig SymbolTable;
    PrinterConfig SymbolTableRaw;
    PrinterConfig SymbolTableJson;
    PrinterConfig SymbolTableFull;
    PrinterConfig SymbolTableFullRaw;
    PrinterConfig FileTableJson;
    PrinterConfig ResolveTree;
    PrinterConfig ResolveTreeRaw;
    PrinterConfig MissingConstants;
    PrinterConfig CFG;
    // cfg-json format outputs a JSON object for each CFG, separated by newlines.
    // See https://en.wikipedia.org/wiki/JSON_streaming#Concatenated_JSON
    PrinterConfig CFGJson;
    // cfg-proto format outputs a binary MultiCFG for export to other tools.
    // See CFG.proto for details
    PrinterConfig CFGProto;
    PrinterConfig TypedSource;
    PrinterConfig Autogen;
    PrinterConfig AutogenMsgPack;
    PrinterConfig PluginGeneratedCode;
    // Ensure everything here is in PrinterConfig::printers().

    std::vector<std::reference_wrapper<PrinterConfig>> printers();
};

enum Phase {
    INIT,
    PARSER,
    DESUGARER,
    DSL,
    LOCAL_VARS,
    NAMER,
    RESOLVER,
    CFG,
    INFERENCER,
};

struct Options {
    Printers print;
    Phase stopAfterPhase = Phase::INFERENCER;
    bool noStdlib = false;

    // Should we monitor STDOUT for HUP and exit if it hangs up. This is a
    // workaround for https://bugzilla.mindrot.org/show_bug.cgi?id=2863
    bool stdoutHUPHack = false;

    core::StrictLevel forceMinStrict = core::StrictLevel::Ignore;
    core::StrictLevel forceMaxStrict = core::StrictLevel::Strong;

    bool showProgress = false;
    bool suggestTyped = false;
    bool silenceErrors = false;
    bool silenceDevMessage = false;
    bool suggestSig = false;
    bool supressNonCriticalErrors = false;
    bool runLSP = false;
    bool disableWatchman = false;
    std::string watchmanPath;
    bool stressIncrementalResolver = false;
    bool noErrorCount = false;
    bool autocorrect = false;
    bool waitForDebugger = false;
    bool skipDSLPasses = false;
    bool suggestRuntimeProfiledType = false;
    int threads = 0;
    int logLevel = 0; // number of time -v was passed
    int autogenVersion = 0;
    std::string typedSource = "";
    std::string cacheDir = "";
    std::vector<std::string> configatronDirs;
    std::vector<std::string> configatronFiles;
    UnorderedMap<std::string, core::StrictLevel> strictnessOverrides;
    UnorderedMap<std::string, std::string> dslPluginTriggers;
    std::vector<std::string> dslRubyExtraArgs;
    std::string storeState = "";
    bool enableCounters = false;
    std::vector<std::string> someCounters;
    std::string errorUrlBase;
    std::vector<int> errorCodeWhiteList;
    std::vector<int> errorCodeBlackList;
    /** Prefix to remove from all printed paths. */
    std::string pathPrefix;

    u4 reserveMemKiB;

    std::string statsdHost;
    std::string statsdPrefix;
    int statsdPort;

    std::string metricsFile;
    std::string metricsRepo;
    std::string metricsPrefix;
    std::string metricsBranch;
    std::string metricsSha;

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
    // Booleans enabling various experimental LSP features. Each will be removed once corresponding feature stabilizes.
    bool lspGoToDefinitionEnabled;
    bool lspFindReferencesEnabled;
    bool lspAutocompleteEnabled;
    bool lspWorkspaceSymbolsEnabled;
    bool lspDocumentSymbolEnabled;
    bool lspSignatureHelpEnabled;
    bool lspHoverEnabled;

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

void readOptions(Options &, int argc, char *argv[],
                 std::shared_ptr<spdlog::logger> logger) noexcept(false); // throw(EarlyReturnWithCode);

void flushPrinters(Options &);
} // namespace sorbet::realmain::options
#endif // RUBY_TYPER_OPTIONS_H
