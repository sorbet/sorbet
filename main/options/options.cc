// have to go first as they violate our poisons
#include "rang.hpp"
#include "yaml-cpp/yaml.h"
#include <cxxopts.hpp>

#include "core/Error.h"
#include "core/errors/infer.h"
#include "main/options/FileFlatMapper.h"
#include "main/options/options.h"
#include "options.h"
#include "sys/stat.h"
#include "third_party/licences/licences.h"
#include "version/version.h"

namespace spd = spdlog;
using namespace std;

namespace sorbet::realmain::options {
struct PrintOptions {
    string option;
    bool Printers::*flag;
    bool supportsCaching = false;
};

const vector<PrintOptions> print_options({
    {"parse-tree", &Printers::ParseTree},
    {"parse-tree-json", &Printers::ParseTreeJson},
    {"ast", &Printers::Desugared},
    {"ast-raw", &Printers::DesugaredRaw},
    {"dsl-tree", &Printers::DSLTree, true},
    {"dsl-tree-raw", &Printers::DSLTreeRaw, true},
    {"symbol-table", &Printers::SymbolTable, true},
    {"symbol-table-raw", &Printers::SymbolTableRaw, true},
    {"symbol-table-json", &Printers::SymbolTableJson, true},
    {"symbol-table-full", &Printers::SymbolTableFull, true},
    {"symbol-table-full-raw", &Printers::SymbolTableFullRaw, true},
    {"name-tree", &Printers::NameTree, true},
    {"name-tree-raw", &Printers::NameTreeRaw, true},
    {"file-table-json", &Printers::FileTableJson, true},
    {"resolve-tree", &Printers::ResolveTree, true},
    {"resolve-tree-raw", &Printers::ResolveTreeRaw, true},
    {"missing-constants", &Printers::MissingConstants, true},
    {"cfg", &Printers::CFG, true},
    {"autogen", &Printers::Autogen, true},
    {"autogen-msgpack", &Printers::AutogenMsgPack, true},
    {"plugin-generated-code", &Printers::PluginGeneratedCode, true},
});

struct StopAfterOptions {
    string option;
    Phase flag;
};

const vector<StopAfterOptions> stop_after_options({
    {"init", Phase::INIT},
    {"parser", Phase::PARSER},
    {"desugarer", Phase::DESUGARER},
    {"dsl", Phase::DSL},
    {"namer", Phase::NAMER},
    {"resolver", Phase::RESOLVER},
    {"cfg", Phase::CFG},
    {"inferencer", Phase::INFERENCER},
});

core::StrictLevel text2StrictLevel(string_view key, shared_ptr<spdlog::logger> logger) {
    if (key == "ignore") {
        return core::StrictLevel::Ignore;
    } else if (key == "false") {
        return core::StrictLevel::False;
    } else if (key == "true") {
        return core::StrictLevel::Typed;
    } else if (key == "strict") {
        return core::StrictLevel::Strict;
    } else if (key == "strong") {
        return core::StrictLevel::Strong;
    } else if (key == "autogenerated") {
        return core::StrictLevel::Autogenerated;
    } else {
        logger->error("Unknown strictness level: `{}`", key);
        throw EarlyReturnWithCode(1);
    }
}

UnorderedMap<string, core::StrictLevel> extractStricnessOverrides(string fileName, shared_ptr<spdlog::logger> logger) {
    UnorderedMap<string, core::StrictLevel> result;
    try {
        YAML::Node config = YAML::LoadFile(fileName);
        switch (config.Type()) {
            case YAML::NodeType::Map:
                for (const auto &child : config) {
                    auto key = child.first.as<string>();
                    core::StrictLevel level = text2StrictLevel(key, logger);
                    switch (child.second.Type()) {
                        case YAML::NodeType::Sequence:
                            for (const auto &file : child.second) {
                                if (file.IsScalar()) {
                                    result[file.as<string>()] = level;
                                } else {
                                    logger->error("Cannot parse strictness override format. Invalid file name.");
                                    throw EarlyReturnWithCode(1);
                                }
                            }
                            break;
                        default:
                            logger->error("Cannot parse strictness override format. File names should be specified as "
                                          "a sequence.");
                            throw EarlyReturnWithCode(1);
                    }
                }
                break;
            default:
                logger->error("Cannot parse strictness override format. Map is expected on top level.");
                throw EarlyReturnWithCode(1);
        }
    } catch (YAML::BadFile) {
        logger->error("Failed to read strictness override file \"{}\". Does it exist?", fileName);
        throw EarlyReturnWithCode(1);
    }
    return result;
}

static vector<string> extractExtraSubprocessOptions(const YAML::Node &config, const string &filePath,
                                                    shared_ptr<spdlog::logger> logger) {
    auto extraArgNode = config["ruby_extra_args"];
    vector<string> extraArgs;
    if (!extraArgNode) {
        return extraArgs;
    }
    if (extraArgNode.IsSequence()) {
        for (const auto &arg : extraArgNode) {
            if (arg.IsScalar()) {
                extraArgs.emplace_back(arg.as<string>());
            } else {
                logger->error("{}: An element of `ruby_extra_args` is not a string", filePath);
                throw EarlyReturnWithCode(1);
            }
        }
        return extraArgs;
    } else {
        logger->error("{}: `ruby_extra_args` must be an array of strings", filePath);
        throw EarlyReturnWithCode(1);
    }
}

struct DslConfiguration {
    UnorderedMap<string, string> triggers;
    vector<string> rubyExtraArgs;
};

DslConfiguration extractDslPlugins(string filePath, shared_ptr<spdlog::logger> logger) {
    bool good = true;
    YAML::Node config;
    try {
        config = YAML::LoadFile(filePath);
    } catch (YAML::BadFile) {
        logger->error("Failed to open DSL specification file \"{}\"", filePath);
        throw EarlyReturnWithCode(1);
    }
    if (!config.IsMap()) {
        logger->error("{}: Cannot parse DSL plugin format. Map is expected at top level", filePath);
        throw EarlyReturnWithCode(1);
    }
    UnorderedMap<string, string> triggers;
    if (auto triggersNode = config["triggers"]; triggersNode.IsMap()) {
        for (const auto &child : triggersNode) {
            auto key = child.first.as<string>();
            if (child.second.Type() == YAML::NodeType::Scalar) {
                auto value = child.second.as<string>();
                auto [_, inserted] = triggers.emplace(move(key), move(value));
                if (!inserted) {
                    logger->error("{}: Duplicate plugin trigger \"{}\"", filePath, key);
                    good = false;
                }
            } else {
                logger->error("{}: Plugin trigger \"{}\" must map to a command that is a string", filePath, key);
                good = false;
            }
        }
    } else {
        logger->error("{}: Required key `triggers` must be a map", filePath);
        good = false;
    }
    if (!good) {
        throw EarlyReturnWithCode(1);
    }
    return {triggers, extractExtraSubprocessOptions(config, filePath, logger)};
}

cxxopts::Options buildOptions() {
    cxxopts::Options options("sorbet", "Typechecker for Ruby");

    // Common user options in order of use
    options.add_options()("e", "Parse an inline ruby string", cxxopts::value<string>()->default_value(""), "string");
    options.add_options()("files", "Input files", cxxopts::value<vector<string>>());
    options.add_options()("q,quiet", "Silence all non-critical errors");
    options.add_options()("v,verbose", "Verbosity level [0-3]");
    options.add_options()("h", "Show short help");
    options.add_options()("help", "Show long help");
    options.add_options()("version", "Show version");

    fmt::memory_buffer all_prints;
    fmt::memory_buffer all_stop_after;

    fmt::format_to(all_prints, "Print: [{}]",
                   fmt::map_join(
                       print_options, ", ", [](const auto &pr) -> auto { return pr.option; }));
    fmt::format_to(all_stop_after, "Stop After: [{}]",
                   fmt::map_join(
                       stop_after_options, ", ", [](const auto &pr) -> auto { return pr.option; }));

    // Advanced options
    options.add_options("advanced")("configatron-dir", "Path to configatron yaml folders",
                                    cxxopts::value<vector<string>>(), "path");
    options.add_options("advanced")("configatron-file", "Path to configatron yaml files",
                                    cxxopts::value<vector<string>>(), "path");
    options.add_options("advanced")("web-trace-file", "Web trace file. For use with chrome about://tracing",
                                    cxxopts::value<string>()->default_value(""), "file");
    options.add_options("advanced")("debug-log-file", "Path to debug log file",
                                    cxxopts::value<string>()->default_value(""), "file");
    options.add_options("advanced")("reserve-mem-kb",
                                    "Preallocate the specified amount of memory for symbol+name tables",
                                    cxxopts::value<u8>()->default_value("0"));
    options.add_options("advanced")("stdout-hup-hack", "Monitor STDERR for HUP and exit on hangup");
    options.add_options("advanced")("remove-path-prefix",
                                    "Remove the provided path prefix from all printed paths. Defaults to the input "
                                    "directory passed to Sorbet, if any.",
                                    cxxopts::value<string>()->default_value(""), "prefix");
    options.add_options("advanced")("a,autocorrect", "Auto-correct source files with suggested fixes");
    options.add_options("advanced")(
        "suggest-runtime-profiled",
        "When suggesting signatures in `typed: strict` mode, suggest `::T::Utils::RuntimeProfiled`");
    options.add_options("advanced")("P,progress", "Draw progressbar");
    options.add_options("advanced")("licence", "Show licence");
    options.add_options("advanced")("color", "Use color output", cxxopts::value<string>()->default_value("auto"),
                                    "{always,never,[auto]}");
    options.add_options("advanced")("lsp", "Start in language-server-protocol mode");
    options.add_options("advanced")("disable-watchman",
                                    "When in language-server-protocol mode, disable file watching via Watchman");
    options.add_options("advanced")("watchman-path",
                                    "Path to watchman executable. Defaults to using `watchman` on your PATH.",
                                    cxxopts::value<string>()->default_value("watchman"));
    options.add_options("advanced")("enable-lsp-hover", "Enable experimental LSP feature: Hover");
    options.add_options("advanced")("enable-lsp-go-to-definition", "Enable experimental LSP feature: Go-to-definition");
    options.add_options("advanced")("enable-lsp-find-references", "Enable experimental LSP feature: Find References");
    options.add_options("advanced")("enable-lsp-autocomplete", "Enable experimental LSP feature: Autocomplete");
    options.add_options("advanced")("enable-lsp-workspace-symbols",
                                    "Enable experimental LSP feature: Workspace Symbols");
    options.add_options("advanced")("enable-lsp-document-symbol", "Enable experimental LSP feature: Document Symbol");
    options.add_options("advanced")("enable-lsp-signature-help", "Enable experimental LSP feature: Signature Help");
    options.add_options("advanced")("enable-lsp-all", "Enable every experimental LSP feature.");
    options.add_options("advanced")(
        "ignore",
        "Ignores input files that contain the given string in their paths (relative to the input path passed to "
        "Sorbet). Strings beginning with / match against the prefix of these relative paths; others are substring "
        "matchs. Matches must be against whole folder and file names, so `foo` matches `/foo/bar.rb` and "
        "`/bar/foo/baz.rb` but not `/foo.rb` or `/foo2/bar.rb`.",
        cxxopts::value<vector<string>>(), "string");
    options.add_options("advanced")("no-error-count", "Do not print the error count summary line");
    options.add_options("advanced")("autogen-version", "Autogen version to output", cxxopts::value<int>());
    options.add_options("advanced")("error-url-base",
                                    "Error URL base string. If set, error URLs are generated by prefixing the "
                                    "error code with this string.",
                                    cxxopts::value<string>()->default_value("https://sorbet.org/docs/error-reference#"),
                                    "url-base");
    // Developer options
    options.add_options("dev")("p,print", to_string(all_prints), cxxopts::value<vector<string>>(), "type");
    options.add_options("dev")("stop-after", to_string(all_stop_after),
                               cxxopts::value<string>()->default_value("inferencer"), "phase");
    options.add_options("dev")("no-stdlib", "Do not load included rbi files for stdlib");
    options.add_options("dev")("skip-dsl-passes", "Do not run DSL passess");
    options.add_options("dev")("wait-for-dbg", "Wait for debugger on start");
    options.add_options("dev")("stress-incremental-resolver",
                               "Force incremental updates to discover resolver & namer bugs");
    options.add_options("dev")("simulate-crash", "Crash on start");
    options.add_options("dev")("silence-dev-message", "Silence \"You are running a development build\" message");
    options.add_options("dev")("error-white-list", "Whitelist of errors to be reported", cxxopts::value<vector<int>>(),
                               "errorCodes");
    options.add_options("dev")("error-black-list", "Blacklist of errors to be reported", cxxopts::value<vector<int>>(),
                               "errorCodes");
    options.add_options("dev")("typed", "Force all code to specified strictness level",
                               cxxopts::value<string>()->default_value("auto"), "{false,true,strict,strong,[auto]}");
    options.add_options("dev")("typed-override", "Yaml config that overrides strictness levels on files",
                               cxxopts::value<string>()->default_value(""), "filepath.yaml");
    options.add_options("dev")("store-state", "Store state into file", cxxopts::value<string>()->default_value(""),
                               "file");
    options.add_options("dev")("cache-dir", "Use the specified folder to cache data",
                               cxxopts::value<string>()->default_value(""), "dir");
    options.add_options("dev")("suppress-non-critical", "Exit 0 unless there was a critical error");
    options.add_options("dev")("dsl-plugins", "YAML config that configures external DSL plugins",
                               cxxopts::value<string>()->default_value(""), "filepath.yaml");

    int defaultThreads = thread::hardware_concurrency();
    if (defaultThreads == 0) {
        defaultThreads = 2;
    }

    options.add_options("dev")("max-threads", "Set number of threads",
                               cxxopts::value<int>()->default_value(to_string(defaultThreads)), "int");
    options.add_options("dev")("counter", "Print internal counter", cxxopts::value<vector<string>>(), "counter");
    options.add_options("dev")("statsd-host", "StatsD sever hostname", cxxopts::value<string>()->default_value(""),
                               "host");
    options.add_options("dev")("counters", "Print all internal counters");
    if (sorbet::debug_mode) {
        options.add_options("dev")("suggest-sig", "Report typing candidates. Only supported in debug builds");
    }
    options.add_options("dev")("suggest-typed", "Suggest which typed: sigils to add or upgrade");
    options.add_options("dev")("statsd-prefix", "StatsD prefix",
                               cxxopts::value<string>()->default_value("ruby_typer.unknown"), "prefix");
    options.add_options("dev")("statsd-port", "StatsD sever port", cxxopts::value<int>()->default_value("8200"),
                               "port");
    options.add_options("dev")("metrics-file", "File to export metrics to", cxxopts::value<string>()->default_value(""),
                               "file");
    options.add_options("dev")("metrics-prefix", "Prefix to use in metrics",
                               cxxopts::value<string>()->default_value("ruby_typer.unknown."), "file");
    options.add_options("dev")("metrics-branch", "Branch to report in metrics export",
                               cxxopts::value<string>()->default_value("none"), "branch");
    options.add_options("dev")("metrics-sha", "Sha1 to report in metrics export",
                               cxxopts::value<string>()->default_value("none"), "sha1");
    options.add_options("dev")("metrics-repo", "Repo to report in metrics export",
                               cxxopts::value<string>()->default_value("none"), "repo");

    // Positional params
    options.parse_positional("files");
    options.positional_help("<path 1> <path 2> ...");
    return options;
}

bool extractPrinters(cxxopts::ParseResult &raw, Options &opts, shared_ptr<spdlog::logger> logger) {
    if (raw.count("print") == 0) {
        return true;
    }
    vector<string> printOpts = raw["print"].as<vector<string>>();
    for (auto opt : printOpts) {
        bool found = false;
        for (auto &known : print_options) {
            if (known.option == opt) {
                opts.print.*(known.flag) = true;
                if (!known.supportsCaching) {
                    if (!opts.cacheDir.empty()) {
                        logger->error("--print={} is incompatible with --cacheDir. Ignoring cache", opt);
                        opts.cacheDir = "";
                    }
                }
                found = true;
                break;
            }
        }
        if (!found) {
            vector<string_view> allOptions;
            for (auto &known : print_options) {
                allOptions.emplace_back(known.option);
            }
            logger->error("Unknown --print option: {}\nValid values: {}", opt, fmt::join(allOptions, ", "));
            return false;
        }
    }
    return true;
}

Phase extractStopAfter(cxxopts::ParseResult &raw, shared_ptr<spdlog::logger> logger) {
    string opt = raw["stop-after"].as<string>();
    for (auto &known : stop_after_options) {
        if (known.option == opt) {
            return known.flag;
        }
    }
    vector<string_view> allOptions;
    for (auto &known : stop_after_options) {
        allOptions.emplace_back(known.option);
    }

    logger->error("Unknown --stop-after option: {}\nValid values: {}", opt, fmt::join(allOptions, ", "));
    return Phase::INIT;
}

// Given a path, strips any trailing forward slashes (/) at the end of the path.
string_view stripTrailingSlashes(string_view path) {
    while (path.back() == '/') {
        path = path.substr(0, path.length() - 1);
    }
    return path;
}

void readOptions(Options &opts, int argc, char *argv[],
                 shared_ptr<spdlog::logger> logger) noexcept(false) { // throw(EarlyReturnWithCode)
    FileFlatMapper flatMapper(argc, argv, logger);

    cxxopts::Options options = buildOptions();
    try {
        cxxopts::ParseResult raw = options.parse(argc, argv);
        if (raw["simulate-crash"].as<bool>()) {
            Exception::raise("simulated crash");
        }

        if (raw.count("ignore") > 0) {
            auto rawIgnorePatterns = raw["ignore"].as<vector<string>>();
            for (auto &p : rawIgnorePatterns) {
                string_view pNormalized = stripTrailingSlashes(p);
                if (p.at(0) == '/') {
                    opts.absoluteIgnorePatterns.emplace_back(pNormalized);
                } else {
                    opts.relativeIgnorePatterns.push_back(fmt::format("/{}", pNormalized));
                }
            }
        }

        opts.pathPrefix = raw["remove-path-prefix"].as<string>();
        if (raw.count("files") > 0) {
            auto rawFiles = raw["files"].as<vector<string>>();
            UnorderedSet<string> acceptableExtensions = {".rb", ".rbi"};
            struct stat s;
            for (auto &file : rawFiles) {
                if (stat(file.c_str(), &s) == 0 && s.st_mode & S_IFDIR) {
                    auto fileNormalized = stripTrailingSlashes(file);
                    if (opts.pathPrefix == "" && rawFiles.size() == 1) {
                        // If Sorbet is provided with a single input directory, the
                        // default path prefix is that directory.
                        opts.pathPrefix = fmt::format("{}/", fileNormalized);
                    }
                    opts.rawInputDirNames.emplace_back(fileNormalized);
                    // Expand directory into list of files.
                    auto containedFiles =
                        opts.fs->listFilesInDir(fileNormalized, acceptableExtensions, true, opts.absoluteIgnorePatterns,
                                                opts.relativeIgnorePatterns);
                    opts.inputFileNames.reserve(opts.inputFileNames.size() + containedFiles.size());
                    opts.inputFileNames.insert(opts.inputFileNames.end(),
                                               std::make_move_iterator(containedFiles.begin()),
                                               std::make_move_iterator(containedFiles.end()));
                } else {
                    opts.rawInputFileNames.push_back(file);
                    opts.inputFileNames.push_back(file);
                }
            }
        }

        bool enableAllLSPFeatures = raw["enable-lsp-all"].as<bool>();
        opts.lspAutocompleteEnabled = enableAllLSPFeatures || raw["enable-lsp-autocomplete"].as<bool>();
        opts.lspGoToDefinitionEnabled = enableAllLSPFeatures || raw["enable-lsp-go-to-definition"].as<bool>();
        opts.lspFindReferencesEnabled = enableAllLSPFeatures || raw["enable-lsp-find-references"].as<bool>();
        opts.lspWorkspaceSymbolsEnabled = enableAllLSPFeatures || raw["enable-lsp-workspace-symbols"].as<bool>();
        opts.lspDocumentSymbolEnabled = enableAllLSPFeatures || raw["enable-lsp-document-symbol"].as<bool>();
        opts.lspSignatureHelpEnabled = enableAllLSPFeatures || raw["enable-lsp-signature-help"].as<bool>();
        opts.lspHoverEnabled = enableAllLSPFeatures || raw["enable-lsp-hover"].as<bool>();

        opts.cacheDir = raw["cache-dir"].as<string>();
        if (!extractPrinters(raw, opts, logger)) {
            throw EarlyReturnWithCode(1);
        }
        opts.stopAfterPhase = extractStopAfter(raw, logger);

        opts.autocorrect = raw["autocorrect"].as<bool>();
        opts.skipDSLPasses = raw["skip-dsl-passes"].as<bool>();

        opts.runLSP = raw["lsp"].as<bool>();
        if (opts.runLSP && !opts.cacheDir.empty()) {
            logger->error("lsp mode does not yet support caching.");
            throw EarlyReturnWithCode(1);
        }
        opts.disableWatchman = raw["disable-watchman"].as<bool>();
        opts.watchmanPath = raw["watchman-path"].as<string>();
        if ((opts.print.Autogen || opts.print.AutogenMsgPack) && (opts.stopAfterPhase != Phase::NAMER)) {
            logger->error("-p autogen{} must also include --stop-after=namer",
                          opts.print.AutogenMsgPack ? "-msgpack" : "");
            throw EarlyReturnWithCode(1);
        }

        opts.noErrorCount = raw["no-error-count"].as<bool>();
        opts.noStdlib = raw["no-stdlib"].as<bool>();
        opts.stdoutHUPHack = raw["stdout-hup-hack"].as<bool>();

        opts.threads = opts.runLSP ? raw["max-threads"].as<int>()
                                   : min(raw["max-threads"].as<int>(), int(opts.inputFileNames.size() / 2));

        if (raw["h"].as<bool>()) {
            logger->info("{}", options.help({""}));
            throw EarlyReturnWithCode(0);
        }
        if (raw["help"].as<bool>()) {
            logger->info("{}", options.help({"", "advanced", "dev"}));
            throw EarlyReturnWithCode(0);
        }
        if (raw["version"].as<bool>()) {
            fmt::print("Sorbet typechecker {}\n", Version::full_version_string);
            throw EarlyReturnWithCode(0);
        }
        if (raw["licence"].as<bool>()) {
            fmt::print(
                "Sorbet typechecker is licenced under Apache License Version 2.0.\n\nSorbet is built on top of:\n{}",
                fmt::map_join(third_party::licences::all(), "\n\n", [](const auto &pair) { return pair.second; }));
            throw EarlyReturnWithCode(0);
        }

        string typed = raw["typed"].as<string>();
        opts.logLevel = raw.count("v");
        if (typed != "auto") {
            opts.forceMinStrict = opts.forceMaxStrict = text2StrictLevel(typed, logger);
        }

        opts.showProgress = raw["P"].as<bool>();
        if (raw.count("configatron-dir") > 0) {
            opts.configatronDirs = raw["configatron-dir"].as<vector<string>>();
        }
        if (raw.count("configatron-file")) {
            opts.configatronFiles = raw["configatron-file"].as<vector<string>>();
        }
        opts.storeState = raw["store-state"].as<string>();
        opts.suggestTyped = raw["suggest-typed"].as<bool>();
        opts.waitForDebugger = raw["wait-for-dbg"].as<bool>();
        opts.stressIncrementalResolver = raw["stress-incremental-resolver"].as<bool>();
        opts.silenceErrors = raw["quiet"].as<bool>();
        opts.suggestRuntimeProfiledType = raw["suggest-runtime-profiled"].as<bool>();
        opts.enableCounters = raw["counters"].as<bool>();
        opts.silenceDevMessage = raw["silence-dev-message"].as<bool>();
        opts.statsdHost = raw["statsd-host"].as<string>();
        opts.statsdPort = raw["statsd-port"].as<int>();
        opts.statsdPrefix = raw["statsd-prefix"].as<string>();
        opts.metricsSha = raw["metrics-sha"].as<string>();
        opts.metricsFile = raw["metrics-file"].as<string>();
        opts.metricsRepo = raw["metrics-repo"].as<string>();
        opts.metricsBranch = raw["metrics-branch"].as<string>();
        opts.metricsPrefix = raw["metrics-prefix"].as<string>();
        opts.debugLogFile = raw["debug-log-file"].as<string>();
        opts.webTraceFile = raw["web-trace-file"].as<string>();
        opts.reserveMemKiB = raw["reserve-mem-kb"].as<u8>();
        if (raw.count("autogen-version") > 0) {
            if (!opts.print.AutogenMsgPack) {
                logger->error("`{}` must also include `{}`", "--autogen-version", "-p autogen-msgpack");
                throw EarlyReturnWithCode(1);
            }
            opts.autogenVersion = raw["autogen-version"].as<int>();
        }
        opts.errorUrlBase = raw["error-url-base"].as<string>();
        if (raw.count("error-white-list") > 0) {
            opts.errorCodeWhiteList = raw["error-white-list"].as<vector<int>>();
        }
        if (raw.count("error-black-list") > 0) {
            if (raw.count("error-white-list") > 0) {
                logger->error("You can't pass both `{}` and `{}`", "--error-black-list", "--error-white-list");
                throw EarlyReturnWithCode(1);
            }
            opts.errorCodeBlackList = raw["error-black-list"].as<vector<int>>();
        }
        if (sorbet::debug_mode) {
            opts.suggestSig = raw["suggest-sig"].as<bool>();
        }

        if (raw.count("e") == 0 && opts.inputFileNames.empty() && !opts.runLSP && opts.storeState.empty()) {
            logger->error("You must pass either `{}` or at least one folder or ruby file.\n\n{}", "-e",
                          options.help({""}));
            throw EarlyReturnWithCode(1);
        }

        if ((raw["color"].as<string>() == "never") || opts.runLSP) {
            core::ErrorColors::disableColors();
        } else if (raw["color"].as<string>() == "auto") {
            if (rang::rang_implementation::isTerminal(cerr.rdbuf())) {
                core::ErrorColors::enableColors();
            }
        } else if (raw["color"].as<string>() == "always") {
            core::ErrorColors::enableColors();
        }

        if (opts.suggestTyped) {
            if (opts.errorCodeWhiteList != vector<int>{core::errors::Infer::SuggestTyped.code} &&
                raw["typed"].as<string>() != "strict") {
                logger->error(
                    "--suggest-typed must also include `{}`",
                    fmt::format("{}{}", "--typed=strict --error-white-list=", core::errors::Infer::SuggestTyped.code));
                throw EarlyReturnWithCode(1);
            }
            if (opts.errorCodeWhiteList != vector<int>{core::errors::Infer::SuggestTyped.code}) {
                logger->error("--suggest-typed must also include `{}`",
                              fmt::format("{}{}", "--error-white-list=", core::errors::Infer::SuggestTyped.code));
                throw EarlyReturnWithCode(1);
            }
            if (!opts.errorCodeBlackList.empty()) {
                logger->error("--suggest-typed can't include `{}`", "--error-black-list");
                throw EarlyReturnWithCode(1);
            }
            if (raw["typed"].as<string>() != "strict") {
                logger->error("--suggest-typed must also include `{}`", "--typed=strict");
                throw EarlyReturnWithCode(1);
            }
        }

        opts.inlineInput = raw["e"].as<string>();
        opts.supressNonCriticalErrors = raw["suppress-non-critical"].as<bool>();
        if (!raw["typed-override"].as<string>().empty()) {
            opts.strictnessOverrides = extractStricnessOverrides(raw["typed-override"].as<string>(), logger);
        }
        if (!raw["dsl-plugins"].as<string>().empty()) {
            auto dslConfig = extractDslPlugins(raw["dsl-plugins"].as<string>(), logger);
            opts.dslPluginTriggers = std::move(dslConfig.triggers);
            opts.dslRubyExtraArgs = std::move(dslConfig.rubyExtraArgs);
        }
    } catch (cxxopts::OptionParseException &e) {
        logger->info("{}\n\n{}", e.what(), options.help({"", "advanced", "dev"}));
        throw EarlyReturnWithCode(1);
    }
}

EarlyReturnWithCode::EarlyReturnWithCode(int returnCode)
    : SorbetException("early return with code " + to_string(returnCode)), returnCode(returnCode){};

} // namespace sorbet::realmain::options
