// have to go first as they violate our poisons
#include "rang.hpp"
#include "yaml-cpp/yaml.h"
#include <cxxopts.hpp>

#include "absl/strings/str_split.h"
#include "common/FileOps.h"
#include "common/Timer.h"
#include "core/Error.h"
#include "core/errors/infer.h"
#include "main/options/ConfigParser.h"
#include "main/options/options.h"
#include "options.h"
#include "sys/stat.h"
#include "third_party/licenses/licenses.h"
#include "version/version.h"

namespace spd = spdlog;
using namespace std;

namespace sorbet::realmain::options {
struct PrintOptions {
    string option;
    PrinterConfig Printers::*config;
    bool supportsCaching = false;
    bool supportsFlush = true; // If false, printer is responsible for flushing its own output.
};

const vector<PrintOptions> print_options({
    {"parse-tree", &Printers::ParseTree},
    {"parse-tree-json", &Printers::ParseTreeJson},
    {"parse-tree-whitequark", &Printers::ParseTreeWhitequark},
    {"ast", &Printers::Desugared},
    {"ast-raw", &Printers::DesugaredRaw},
    {"dsl-tree", &Printers::DSLTree},
    {"dsl-tree-raw", &Printers::DSLTreeRaw},
    {"index-tree", &Printers::IndexTree, true},
    {"index-tree-raw", &Printers::IndexTreeRaw, true},
    {"symbol-table", &Printers::SymbolTable, true},
    {"symbol-table-raw", &Printers::SymbolTableRaw, true},
    {"symbol-table-json", &Printers::SymbolTableJson, true},
    {"symbol-table-full", &Printers::SymbolTableFull, true},
    {"symbol-table-full-raw", &Printers::SymbolTableFullRaw, true},
    {"symbol-table-full-json", &Printers::SymbolTableFullJson, true},
    {"name-tree", &Printers::NameTree, true},
    {"name-tree-raw", &Printers::NameTreeRaw, true},
    {"file-table-json", &Printers::FileTableJson, true},
    {"resolve-tree", &Printers::ResolveTree, true},
    {"resolve-tree-raw", &Printers::ResolveTreeRaw, true},
    {"missing-constants", &Printers::MissingConstants, true},
    {"flattened-tree", &Printers::FlattenedTree, true},
    {"flattened-tree-raw", &Printers::FlattenedTreeRaw, true},
    {"cfg", &Printers::CFG, true},
    {"cfg-raw", &Printers::CFGRaw, true},
    {"cfg-json", &Printers::CFGJson, true},
    {"cfg-proto", &Printers::CFGProto, true},
    {"autogen", &Printers::Autogen, true},
    {"autogen-msgpack", &Printers::AutogenMsgPack, true},
    {"autogen-classlist", &Printers::AutogenClasslist, true},
    {"autogen-autoloader", &Printers::AutogenAutoloader, true, false},
    {"autogen-subclasses", &Printers::AutogenSubclasses, true},
    {"plugin-generated-code", &Printers::PluginGeneratedCode, true},
});

PrinterConfig::PrinterConfig() : state(make_shared<GuardedState>()){};

void PrinterConfig::print(const string_view &contents) const {
    if (outputPath.empty()) {
        // TODO (gdritter): thread a logger through instead of just printing to stdout
        fmt::print("{}", contents);
    } else {
        absl::MutexLock lck(&state->mutex);
        fmt::format_to(state->buf, "{}", contents);
    }
};

void PrinterConfig::flush() {
    if (!enabled || !supportsFlush || outputPath.empty()) {
        return;
    }
    absl::MutexLock lck(&state->mutex);
    FileOps::write(outputPath, to_string(state->buf));
};

vector<reference_wrapper<PrinterConfig>> Printers::printers() {
    return vector<reference_wrapper<PrinterConfig>>({
        ParseTree,
        ParseTreeJson,
        ParseTreeWhitequark,
        Desugared,
        DesugaredRaw,
        DSLTree,
        DSLTreeRaw,
        IndexTree,
        IndexTreeRaw,
        SymbolTable,
        SymbolTableRaw,
        SymbolTableJson,
        SymbolTableFull,
        SymbolTableFullRaw,
        NameTree,
        NameTreeRaw,
        FileTableJson,
        ResolveTree,
        ResolveTreeRaw,
        MissingConstants,
        FlattenedTree,
        FlattenedTreeRaw,
        CFG,
        CFGRaw,
        CFGJson,
        CFGProto,
        Autogen,
        AutogenMsgPack,
        AutogenClasslist,
        AutogenAutoloader,
        AutogenSubclasses,
        PluginGeneratedCode,
    });
}

bool Printers::isAutogen() const {
    return Autogen.enabled || AutogenMsgPack.enabled || AutogenClasslist.enabled || AutogenSubclasses.enabled ||
           AutogenAutoloader.enabled;
}

struct StopAfterOptions {
    string option;
    Phase flag;
};

const vector<StopAfterOptions> stop_after_options({
    {"init", Phase::INIT},
    {"parser", Phase::PARSER},
    {"desugarer", Phase::DESUGARER},
    {"dsl", Phase::DSL},
    {"local-vars", Phase::LOCAL_VARS},
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
        return core::StrictLevel::True;
    } else if (key == "strict") {
        return core::StrictLevel::Strict;
    } else if (key == "strong") {
        return core::StrictLevel::Strong;
    } else if (key == "autogenerated") {
        return core::StrictLevel::Autogenerated;
    } else if (key == "__STDLIB_INTERNAL") {
        return core::StrictLevel::Stdlib;
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
                                    string key = file.as<string>();
                                    if (!absl::StartsWith(key, "/") && !absl::StartsWith(key, "./")) {
                                        logger->error("All relative file names in \"{}\" should start with ./",
                                                      fileName);
                                        throw EarlyReturnWithCode(1);
                                    }
                                    result[key] = level;
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

cxxopts::Options
buildOptions(const vector<pipeline::semantic_extension::SemanticExtensionProvider *> &semanticExtensionProviders) {
    // Used to populate default options.
    Options empty;

    cxxopts::Options options("sorbet", "Typechecker for Ruby");

    // Common user options in order of use
    options.add_options()("e", "Parse an inline ruby string",
                          cxxopts::value<string>()->default_value(empty.inlineInput), "string");
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
    options.add_options("advanced")("dir", "Input directory", cxxopts::value<vector<string>>());
    options.add_options("advanced")("file", "Input file", cxxopts::value<vector<string>>());
    options.add_options("advanced")("configatron-dir", "Path to configatron yaml folders",
                                    cxxopts::value<vector<string>>(), "path");
    options.add_options("advanced")("configatron-file", "Path to configatron yaml files",
                                    cxxopts::value<vector<string>>(), "path");
    options.add_options("advanced")("web-trace-file", "Web trace file. For use with chrome about://tracing",
                                    cxxopts::value<string>()->default_value(empty.webTraceFile), "file");
    options.add_options("advanced")("debug-log-file", "Path to debug log file",
                                    cxxopts::value<string>()->default_value(empty.debugLogFile), "file");
    options.add_options("advanced")("reserve-mem-kb",
                                    "Preallocate the specified amount of memory for symbol+name tables",
                                    cxxopts::value<u8>()->default_value(fmt::format("{}", empty.reserveMemKiB)));
    options.add_options("advanced")("stdout-hup-hack", "Monitor STDERR for HUP and exit on hangup");
    options.add_options("advanced")("remove-path-prefix",
                                    "Remove the provided path prefix from all printed paths. Defaults to the input "
                                    "directory passed to Sorbet, if any.",
                                    cxxopts::value<string>()->default_value(empty.pathPrefix), "prefix");
    options.add_options("advanced")("a,autocorrect", "Auto-correct source files with suggested fixes");
    options.add_options("advanced")(
        "suggest-runtime-profiled",
        "When suggesting signatures in `typed: strict` mode, suggest `::T::Utils::RuntimeProfiled`");
    options.add_options("advanced")("P,progress", "Draw progressbar");
    options.add_options("advanced")("license", "Show license");
    options.add_options("advanced")("color", "Use color output", cxxopts::value<string>()->default_value("auto"),
                                    "{always,never,[auto]}");
    options.add_options("advanced")("lsp", "Start in language-server-protocol mode");
    options.add_options("advanced")("no-config", "Do not load the content of the `sorbet/config` file");
    options.add_options("advanced")("disable-watchman",
                                    "When in language-server-protocol mode, disable file watching via Watchman");
    options.add_options("advanced")("watchman-path",
                                    "Path to watchman executable. Defaults to using `watchman` on your PATH.",
                                    cxxopts::value<string>()->default_value(empty.watchmanPath));
    options.add_options("advanced")("enable-experimental-lsp-autocomplete",
                                    "Enable experimental LSP feature: Autocomplete");
    options.add_options("advanced")("enable-experimental-lsp-workspace-symbols",
                                    "Enable experimental LSP feature: Workspace Symbols");
    options.add_options("advanced")("enable-experimental-lsp-document-symbol",
                                    "Enable experimental LSP feature: Document Symbol");
    options.add_options("advanced")("enable-experimental-lsp-signature-help",
                                    "Enable experimental LSP feature: Signature Help");
    options.add_options("advanced")("enable-experimental-lsp-quick-fix", "Enable experimental LSP feature: Quick Fix");
    options.add_options("advanced")("enable-all-experimental-lsp-features", "Enable every experimental LSP feature.");
    options.add_options("advanced")(
        "ignore",
        "Ignores input files that contain the given string in their paths (relative to the input path passed to "
        "Sorbet). Strings beginning with / match against the prefix of these relative paths; others are substring "
        "matchs. Matches must be against whole folder and file names, so `foo` matches `/foo/bar.rb` and "
        "`/bar/foo/baz.rb` but not `/foo.rb` or `/foo2/bar.rb`.",
        cxxopts::value<vector<string>>(), "string");
    options.add_options("advanced")(
        "lsp-directories-missing-from-client",
        "Directory prefixes that are not accessible editor-side. References to files in these directories will be sent "
        "as sorbet: URIs to clients that understand them.",
        cxxopts::value<vector<string>>(), "string");
    options.add_options("advanced")("no-error-count", "Do not print the error count summary line");
    options.add_options("advanced")("autogen-version", "Autogen version to output", cxxopts::value<int>());
    options.add_options("advanced")("stripe-mode", "Enable Stripe specific error enforcement", cxxopts::value<bool>());

    options.add_options("advanced")(
        "autogen-autoloader-exclude-require",
        "Names that should be excluded from top-level require statements in autoloader output. (e.g. 'pry')",
        cxxopts::value<vector<string>>());
    options.add_options("advanced")("autogen-autoloader-ignore",
                                    "Input files to exclude from autoloader output. (See --ignore for formatting.)",
                                    cxxopts::value<vector<string>>());
    options.add_options("advanced")("autogen-autoloader-modules", "Top-level modules to include in autoloader output",
                                    cxxopts::value<vector<string>>());
    options.add_options("advanced")("autogen-autoloader-preamble", "Preamble to add to each autoloader file",
                                    cxxopts::value<string>()->default_value(""));
    options.add_options("advanced")("autogen-autoloader-root", "Root directory for autoloader output",
                                    cxxopts::value<string>()->default_value("autoloader"));
    options.add_options("advanced")("autogen-autoloader-samefile",
                                    "Modules that should never be collapsed into their parent. This helps break cycles "
                                    "in certain cases. (e.g. Foo::Bar::Baz)",
                                    cxxopts::value<vector<string>>());
    options.add_options("advanced")("autogen-autoloader-strip-prefix",
                                    "Prefixes to strip from file output paths. "
                                    "If path does not start with prefix, nothing is stripped",
                                    cxxopts::value<vector<string>>());

    options.add_options("advanced")("error-url-base",
                                    "Error URL base string. If set, error URLs are generated by prefixing the "
                                    "error code with this string.",
                                    cxxopts::value<string>()->default_value(empty.errorUrlBase), "url-base");
    // Developer options
    options.add_options("dev")("p,print", to_string(all_prints), cxxopts::value<vector<string>>(), "type");
    options.add_options("dev")("autogen-subclasses-parent",
                               "Parent classes for which generate a list of subclasses. "
                               "This option must be used in conjunction with -p autogen-subclasses",
                               cxxopts::value<vector<string>>(), "string");
    options.add_options("dev")("autogen-subclasses-ignore",
                               "Like --ignore, but it only affects `-p autogen-subclasses`.",
                               cxxopts::value<vector<string>>(), "string");
    options.add_options("dev")("stop-after", to_string(all_stop_after),
                               cxxopts::value<string>()->default_value("inferencer"), "phase");
    options.add_options("dev")("no-stdlib", "Do not load included rbi files for stdlib");
    options.add_options("dev")("skip-dsl-passes", "Do not run DSL passess");
    options.add_options("dev")("wait-for-dbg", "Wait for debugger on start");
    options.add_options("dev")("stress-incremental-resolver",
                               "Force incremental updates to discover resolver & namer bugs");
    options.add_options("dev")("simulate-crash", "Crash on start");
    options.add_options("dev")("silence-dev-message", "Silence \"You are running a development build\" message");
    options.add_options("dev")("censor-for-snapshot-tests",
                               "When printing raw location information, don't show line numbers");
    options.add_options("dev")("error-white-list",
                               "Error code to whitelist into reporting. "
                               "Errors not mentioned will be silenced. "
                               "This option can be passed multiple times.",
                               cxxopts::value<vector<int>>(), "errorCode");
    options.add_options("dev")("error-black-list",
                               "Error code to blacklist from reporting. "
                               "Errors mentioned will be silenced. "
                               "This option can be passed multiple times.",
                               cxxopts::value<vector<int>>(), "errorCode");
    options.add_options("dev")("typed", "Force all code to specified strictness level",
                               cxxopts::value<string>()->default_value("auto"), "{false,true,strict,strong,[auto]}");
    options.add_options("dev")("typed-override", "Yaml config that overrides strictness levels on files",
                               cxxopts::value<string>()->default_value(""), "filepath.yaml");
    options.add_options("dev")("store-state", "Store state into file",
                               cxxopts::value<string>()->default_value(empty.storeState), "file");
    options.add_options("dev")("cache-dir", "Use the specified folder to cache data",
                               cxxopts::value<string>()->default_value(empty.cacheDir), "dir");
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
    options.add_options("dev")("statsd-host", "StatsD sever hostname",
                               cxxopts::value<string>()->default_value(empty.statsdHost), "host");
    options.add_options("dev")("counters", "Print all internal counters");
    if (sorbet::debug_mode) {
        options.add_options("dev")("suggest-sig", "Report typing candidates. Only supported in debug builds");
    }
    options.add_options("dev")("suggest-typed", "Suggest which typed: sigils to add or upgrade");
    options.add_options("dev")("statsd-prefix", "StatsD prefix",
                               cxxopts::value<string>()->default_value(empty.statsdPrefix), "prefix");
    options.add_options("dev")("statsd-port", "StatsD server port",
                               cxxopts::value<int>()->default_value(fmt::format("{}", empty.statsdPort)), "port");
    options.add_options("dev")("metrics-file", "File to export metrics to",
                               cxxopts::value<string>()->default_value(empty.metricsFile), "file");
    options.add_options("dev")("metrics-prefix", "Prefix to use in metrics",
                               cxxopts::value<string>()->default_value(empty.metricsPrefix), "file");
    options.add_options("dev")("metrics-branch", "Branch to report in metrics export",
                               cxxopts::value<string>()->default_value(empty.metricsBranch), "branch");
    options.add_options("dev")("metrics-sha", "Sha1 to report in metrics export",
                               cxxopts::value<string>()->default_value(empty.metricsSha), "sha1");
    options.add_options("dev")("metrics-repo", "Repo to report in metrics export",
                               cxxopts::value<string>()->default_value(empty.metricsRepo), "repo");

    for (auto &provider : semanticExtensionProviders) {
        provider->injectOptions(options);
    }

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
        string outPath;
        auto pos = opt.find(":");
        if (pos != string::npos) {
            outPath = opt.substr(pos + 1);
            opt = opt.substr(0, pos);
        }
        bool found = false;
        for (auto &known : print_options) {
            if (known.option == opt) {
                auto &cfg = opts.print.*(known.config);
                if (cfg.enabled && cfg.outputPath != outPath) {
                    logger->error("--print={} specified multiple times with inconsistent output options", opt);
                    throw EarlyReturnWithCode(1);
                }
                cfg.enabled = true;
                cfg.outputPath = outPath;
                cfg.supportsFlush = known.supportsFlush;
                if (!known.supportsCaching) {
                    if (!opts.cacheDir.empty()) {
                        logger->error("--print={} is incompatible with --cache-dir. Ignoring cache", opt);
                        opts.cacheDir = "";
                    }
                }
                if (opt == "autogen-autoloader" && outPath.empty()) {
                    logger->error("--print={} requires an output path to be specified", opt);
                    throw EarlyReturnWithCode(1);
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

void Options::flushPrinters() {
    for (PrinterConfig &cfg : print.printers()) {
        cfg.flush();
    }
}

void parseIgnorePatterns(const vector<string> &rawIgnorePatterns, vector<string> &absoluteIgnorePatterns,
                         vector<string> &relativeIgnorePatterns) {
    for (auto &p : rawIgnorePatterns) {
        string_view pNormalized = stripTrailingSlashes(p);
        if (p.at(0) == '/') {
            absoluteIgnorePatterns.emplace_back(pNormalized);
        } else {
            relativeIgnorePatterns.push_back(fmt::format("/{}", pNormalized));
        }
    }
}

bool extractAutoloaderConfig(cxxopts::ParseResult &raw, Options &opts, shared_ptr<spdlog::logger> logger) {
    AutoloaderConfig &cfg = opts.autoloaderConfig;
    if (raw.count("autogen-autoloader-exclude-require") > 0) {
        cfg.requireExcludes = raw["autogen-autoloader-exclude-require"].as<vector<string>>();
    }
    if (raw.count("autogen-autoloader-ignore") > 0) {
        auto rawIgnorePatterns = raw["autogen-autoloader-ignore"].as<vector<string>>();
        parseIgnorePatterns(rawIgnorePatterns, cfg.absoluteIgnorePatterns, cfg.relativeIgnorePatterns);
    }
    if (raw.count("autogen-autoloader-modules") > 0) {
        cfg.modules = raw["autogen-autoloader-modules"].as<vector<string>>();
    }
    if (raw.count("autogen-autoloader-strip-prefix") > 0) {
        cfg.stripPrefixes = raw["autogen-autoloader-strip-prefix"].as<vector<string>>();
    }
    if (raw.count("autogen-autoloader-samefile") > 0) {
        for (auto &fullName : raw["autogen-autoloader-samefile"].as<vector<string>>()) {
            cfg.sameFileModules.emplace_back(absl::StrSplit(fullName, "::"));
        }
    }
    cfg.preamble = raw["autogen-autoloader-preamble"].as<string>();
    cfg.rootDir = stripTrailingSlashes(raw["autogen-autoloader-root"].as<string>());
    return true;
}

void addFilesFromDir(Options &opts, string_view dir) {
    auto fileNormalized = stripTrailingSlashes(dir);
    opts.rawInputDirNames.emplace_back(fileNormalized);
    // Expand directory into list of files.
    auto containedFiles = opts.fs->listFilesInDir(fileNormalized, {".rb", ".rbi"}, true, opts.absoluteIgnorePatterns,
                                                  opts.relativeIgnorePatterns);
    opts.inputFileNames.reserve(opts.inputFileNames.size() + containedFiles.size());
    opts.inputFileNames.insert(opts.inputFileNames.end(), std::make_move_iterator(containedFiles.begin()),
                               std::make_move_iterator(containedFiles.end()));
}

void readOptions(Options &opts,
                 vector<unique_ptr<pipeline::semantic_extension::SemanticExtension>> &configuredExtensions, int argc,
                 char *argv[],
                 const vector<pipeline::semantic_extension::SemanticExtensionProvider *> &semanticExtensionProviders,
                 shared_ptr<spdlog::logger> logger) noexcept(false) { // throw(EarlyReturnWithCode)
    Timer timeit(*logger, "readOptions");
    cxxopts::Options options = buildOptions(semanticExtensionProviders);
    try {
        cxxopts::ParseResult raw = ConfigParser::parseConfig(logger, argc, argv, options);
        if (raw["simulate-crash"].as<bool>()) {
            Exception::raise("simulated crash");
        }

        if (raw.count("ignore") > 0) {
            auto rawIgnorePatterns = raw["ignore"].as<vector<string>>();
            parseIgnorePatterns(rawIgnorePatterns, opts.absoluteIgnorePatterns, opts.relativeIgnorePatterns);
        }

        opts.pathPrefix = raw["remove-path-prefix"].as<string>();
        if (raw.count("files") > 0) {
            auto rawFiles = raw["files"].as<vector<string>>();
            struct stat s;
            for (auto &file : rawFiles) {
                if (stat(file.c_str(), &s) == 0 && s.st_mode & S_IFDIR) {
                    addFilesFromDir(opts, file);
                } else {
                    opts.rawInputFileNames.push_back(file);
                    opts.inputFileNames.push_back(file);
                }
            }
        }

        if (raw.count("file") > 0) {
            auto files = raw["file"].as<vector<string>>();
            opts.rawInputFileNames.insert(opts.rawInputFileNames.end(), files.begin(), files.end());
            opts.inputFileNames.insert(opts.inputFileNames.end(), files.begin(), files.end());
        }

        if (raw.count("dir") > 0) {
            auto rawDirs = raw["dir"].as<vector<string>>();
            for (auto &dir : rawDirs) {
                // Since we don't stat here, we're unsure if the directory exists / is a directory.
                try {
                    addFilesFromDir(opts, dir);
                } catch (sorbet::FileNotFoundException) {
                    logger->error("Directory `{}` not found", dir);
                    throw EarlyReturnWithCode(1);
                } catch (sorbet::FileNotDirException) {
                    logger->error("Path `{}` is not a directory", dir);
                    throw EarlyReturnWithCode(1);
                }
            }
        }

        if (opts.pathPrefix.empty() && opts.rawInputDirNames.size() == 1 && opts.rawInputFileNames.size() == 0) {
            // If Sorbet is provided with a single input directory, the
            // default path prefix is that directory.
            opts.pathPrefix = fmt::format("{}/", opts.rawInputDirNames.at(0));
        }
        fast_sort(opts.inputFileNames);
        opts.inputFileNames.erase(unique(opts.inputFileNames.begin(), opts.inputFileNames.end()),
                                  opts.inputFileNames.end());

        bool enableAllLSPFeatures = raw["enable-all-experimental-lsp-features"].as<bool>();
        opts.lspAutocompleteEnabled = enableAllLSPFeatures || raw["enable-experimental-lsp-autocomplete"].as<bool>();
        opts.lspQuickFixEnabled = enableAllLSPFeatures || raw["enable-experimental-lsp-quick-fix"].as<bool>();
        opts.lspWorkspaceSymbolsEnabled =
            enableAllLSPFeatures || raw["enable-experimental-lsp-workspace-symbols"].as<bool>();
        opts.lspDocumentSymbolEnabled =
            enableAllLSPFeatures || raw["enable-experimental-lsp-document-symbol"].as<bool>();
        opts.lspSignatureHelpEnabled = enableAllLSPFeatures || raw["enable-experimental-lsp-signature-help"].as<bool>();

        if (raw.count("lsp-directories-missing-from-client") > 0) {
            auto lspDirsMissingFromClient = raw["lsp-directories-missing-from-client"].as<vector<string>>();
            // Convert all of these dirs into absolute ignore patterns that begin with '/'.
            for (auto &dir : lspDirsMissingFromClient) {
                string pNormalized = dir;
                if (dir.at(0) != '/') {
                    pNormalized = '/' + dir;
                }
                opts.lspDirsMissingFromClient.push_back(pNormalized);
            }
        }

        opts.cacheDir = raw["cache-dir"].as<string>();
        if (!extractPrinters(raw, opts, logger)) {
            throw EarlyReturnWithCode(1);
        }
        opts.stopAfterPhase = extractStopAfter(raw, logger);

        opts.silenceErrors = raw["quiet"].as<bool>();
        opts.autocorrect = raw["autocorrect"].as<bool>();
        opts.inlineInput = raw["e"].as<string>();
        if (opts.autocorrect && opts.silenceErrors) {
            logger->error("You may not use autocorrect when silencing errors.");
            throw EarlyReturnWithCode(1);
        }
        if (opts.autocorrect && !opts.inlineInput.empty()) {
            logger->error("You may not use autocorrect with inline input.");
            throw EarlyReturnWithCode(1);
        }

        opts.runLSP = raw["lsp"].as<bool>();
        if (opts.runLSP && !opts.cacheDir.empty()) {
            logger->error("lsp mode does not yet support caching.");
            throw EarlyReturnWithCode(1);
        }
        opts.disableWatchman = raw["disable-watchman"].as<bool>();
        opts.watchmanPath = raw["watchman-path"].as<string>();
        // Certain features only need certain passes
        if (opts.print.isAutogen() && (opts.stopAfterPhase != Phase::NAMER)) {
            logger->error(
                "-p autogen{-msgpack,-classlist,-subclasses,-autoloader} must also include --stop-after=namer");
            throw EarlyReturnWithCode(1);
        }

        if (raw.count("autogen-subclasses-parent")) {
            if (!opts.print.AutogenSubclasses.enabled) {
                logger->error("autogen-subclasses-parent must be used with -p autogen-subclasses");
                throw EarlyReturnWithCode(1);
            }
            for (string parentClassName : raw["autogen-subclasses-parent"].as<vector<string>>()) {
                opts.autogenSubclassesParents.emplace_back(parentClassName);
            }
        }

        if (raw.count("autogen-subclasses-ignore") > 0) {
            auto rawIgnorePatterns = raw["autogen-subclasses-ignore"].as<vector<string>>();
            for (auto &p : rawIgnorePatterns) {
                string_view pNormalized = stripTrailingSlashes(p);
                if (p.at(0) == '/') {
                    opts.autogenSubclassesAbsoluteIgnorePatterns.emplace_back(pNormalized);
                } else {
                    opts.autogenSubclassesRelativeIgnorePatterns.emplace_back(fmt::format("/{}", pNormalized));
                }
            }
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
            logger->info("{}", options.help(options.groups()));
            throw EarlyReturnWithCode(0);
        }
        if (raw["version"].as<bool>()) {
            fmt::print("Sorbet typechecker {}\n", Version::full_version_string);
            throw EarlyReturnWithCode(0);
        }
        if (raw["license"].as<bool>()) {
            fmt::print(
                "Sorbet typechecker is licensed under Apache License Version 2.0.\n\nSorbet is built on top of:\n{}",
                fmt::map_join(third_party::licenses::all(), "\n\n", [](const auto &pair) { return pair.second; }));
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
        opts.skipDSLPasses = raw["skip-dsl-passes"].as<bool>();
        opts.storeState = raw["store-state"].as<string>();
        opts.suggestTyped = raw["suggest-typed"].as<bool>();
        opts.waitForDebugger = raw["wait-for-dbg"].as<bool>();
        opts.stressIncrementalResolver = raw["stress-incremental-resolver"].as<bool>();
        opts.suggestRuntimeProfiledType = raw["suggest-runtime-profiled"].as<bool>();
        opts.enableCounters = raw["counters"].as<bool>();
        opts.silenceDevMessage = raw["silence-dev-message"].as<bool>();
        opts.censorForSnapshotTests = raw["censor-for-snapshot-tests"].as<bool>();
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
            if (!opts.print.AutogenMsgPack.enabled) {
                logger->error("`{}` must also include `{}`", "--autogen-version", "-p autogen-msgpack");
                throw EarlyReturnWithCode(1);
            }
            opts.autogenVersion = raw["autogen-version"].as<int>();
        }
        opts.stripeMode = raw["stripe-mode"].as<bool>();
        extractAutoloaderConfig(raw, opts, logger);
        opts.errorUrlBase = raw["error-url-base"].as<string>();
        if (raw.count("error-white-list") > 0) {
            auto rawList = raw["error-white-list"].as<vector<int>>();
            opts.errorCodeWhiteList = set<int>(rawList.begin(), rawList.end());
        }
        if (raw.count("error-black-list") > 0) {
            if (raw.count("error-white-list") > 0) {
                logger->error("You can't pass both `{}` and `{}`", "--error-black-list", "--error-white-list");
                throw EarlyReturnWithCode(1);
            }
            auto rawList = raw["error-black-list"].as<vector<int>>();
            opts.errorCodeBlackList = set<int>(rawList.begin(), rawList.end());
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
            if (opts.errorCodeWhiteList != set<int>{core::errors::Infer::SuggestTyped.code} &&
                raw["typed"].as<string>() != "strict") {
                logger->error(
                    "--suggest-typed must also include `{}`",
                    fmt::format("{}{}", "--typed=strict --error-white-list=", core::errors::Infer::SuggestTyped.code));
                throw EarlyReturnWithCode(1);
            }
            if (opts.errorCodeWhiteList != set<int>{core::errors::Infer::SuggestTyped.code}) {
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

        opts.supressNonCriticalErrors = raw["suppress-non-critical"].as<bool>();
        if (!raw["typed-override"].as<string>().empty()) {
            opts.strictnessOverrides = extractStricnessOverrides(raw["typed-override"].as<string>(), logger);
        }
        if (!raw["dsl-plugins"].as<string>().empty()) {
            auto dslConfig = extractDslPlugins(raw["dsl-plugins"].as<string>(), logger);
            opts.dslPluginTriggers = std::move(dslConfig.triggers);
            opts.dslRubyExtraArgs = std::move(dslConfig.rubyExtraArgs);
        }

        for (auto &provider : semanticExtensionProviders) {
            auto maybeExtension = provider->readOptions(raw);
            if (maybeExtension) {
                configuredExtensions.emplace_back(move(maybeExtension));
            }
        }
    } catch (cxxopts::OptionParseException &e) {
        logger->info("{}. To see all available options pass `--help`.", e.what());
        throw EarlyReturnWithCode(1);
    }
}

EarlyReturnWithCode::EarlyReturnWithCode(int returnCode)
    : SorbetException("early return with code " + to_string(returnCode)), returnCode(returnCode){};

} // namespace sorbet::realmain::options
