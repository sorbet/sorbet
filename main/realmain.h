#ifndef RUBY_TYPER_REAL_MAIN_H
#include "common/Timer.h"
#include "common/common.h"
#include "spdlog/spdlog.h"
#include <cxxopts.hpp>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace ruby_typer {
namespace realmain {
int realmain(int argc, const char *argv[]);

// Terminate execution of ruby-typer with specific return code
class EarlyReturnWithCode : public SRubyException {
public:
    EarlyReturnWithCode(int returnCode);
    const int returnCode;
};

extern std::shared_ptr<spdlog::logger> console_err;
extern std::shared_ptr<spdlog::logger> tracer;
extern std::shared_ptr<spdlog::logger> console;

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
    bool CFG = false;
    bool CFGRaw = false;
    bool TypedSource = false;
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
    bool forceTyped = false;
    bool forceUntyped = false;
    bool showProgress = false;
    bool suggestTyped = false;
    bool silenceErrors = false;
    bool supressNonCriticalErrors = false;
    int threads = 0;
    std::string typedSource = "";
    std::string cacheDir = "";
    std::vector<std::string> configatronDirs;
    std::vector<std::string> configatronFiles;
    std::string storeState = "";
    bool enableCounters = false;
    std::vector<std::string> someCounters;

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
};

Options readOptions(int argc, const char *argv[]) throw(EarlyReturnWithCode);
} // namespace realmain
} // namespace ruby_typer
#endif // RUBY_TYPER_REAL_MAIN_H
