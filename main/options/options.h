#ifndef RUBY_TYPER_OPTIONS_H
#define RUBY_TYPER_OPTIONS_H
#include "common/common.h"
#include "core/StrictLevel.h"
#include "spdlog/spdlog.h"
namespace sorbet {
namespace realmain {
namespace options {

// Terminate execution of sorbet with specific return code
class EarlyReturnWithCode : public SRubyException {
public:
    EarlyReturnWithCode(int returnCode);
    const int returnCode;
};

struct Printers {
    bool ParseTree = false;
    bool ParseTreeJSON = false;
    bool Desugared = false;
    bool DesugaredRaw = false;
    bool DSLTree = false;
    bool DSLTreeRaw = false;
    bool NameTree = false;
    bool NameTreeRaw = false;
    bool NameTable = false;
    bool NameTableJson = false;
    bool NameTableFull = false;
    bool FileTableJson = false;
    bool CFG = false;
    bool CFGRaw = false;
    bool TypedSource = false;
    bool ErrorFiles = false;
};

enum Phase {
    INIT,
    PARSER,
    DESUGARER,
    DSL,
    NAMER,
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

    core::StrictLevel forceMinStrict = core::StrictLevel::Stripe;
    core::StrictLevel forceMaxStrict = core::StrictLevel::Strong;

    bool showProgress = false;
    bool suggestTyped = false;
    bool silenceErrors = false;
    bool supressNonCriticalErrors = false;
    bool runLSP = false;
    bool noErrorCount = false;
    bool autocorrect = false;
    int threads = 0;
    int logLevel = 0; // number of time -v was passed
    std::string typedSource = "";
    std::string cacheDir = "";
    std::vector<std::string> configatronDirs;
    std::vector<std::string> configatronFiles;
    std::unordered_map<std::string, core::StrictLevel> strictnessOverrides;
    std::string storeState = "";
    bool enableCounters = false;
    std::vector<std::string> someCounters;

    u4 reserveMemKiB;

    std::string statsdHost;
    std::string statsdPrefix;
    int statsdPort;

    std::string metricsFile;
    std::string metricsRepo;
    std::string metricsPrefix;
    std::string metricsBranch;
    std::string metricsSha;

    std::vector<std::string> inputFileNames;
    std::string inlineInput; // passed via -e
    std::string debugLogFile;

    Options() = default;

    Options(const Options &) = delete;

    Options(Options &&) = default;

    const Options &operator=(const Options &) = delete;

    const Options &operator=(Options &&) = delete;
};

void readOptions(Options &, int argc, const char *argv[],
                 std::shared_ptr<spdlog::logger> logger) throw(EarlyReturnWithCode);
} // namespace options
} // namespace realmain
} // namespace sorbet
#endif // RUBY_TYPER_OPTIONS_H
