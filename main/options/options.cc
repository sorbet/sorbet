// have to go first as they violate our poisons
#include "rang.hpp"
#include "yaml-cpp/yaml.h"
#include <cxxopts.hpp>

#include "absl/strings/str_split.h"
#include "common/FileOps.h"
#include "common/Timer.h"
#include "common/formatting.h"
#include "common/sort.h"
#include "core/Error.h"
#include "core/errors/infer.h"
#include "main/options/ConfigParser.h"
#include "main/options/options.h"
#include "options.h"
#include "sorbet_version/sorbet_version.h"
#include "sys/stat.h"
#include "third_party/licenses/licenses.h"

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
    {"parse-tree-json-with-locs", &Printers::ParseTreeJsonWithLocs},
    {"parse-tree-whitequark", &Printers::ParseTreeWhitequark},
    {"desugar-tree", &Printers::DesugarTree},
    {"desugar-tree-raw", &Printers::DesugarTreeRaw},
    {"rewrite-tree", &Printers::RewriterTree},
    {"rewrite-tree-raw", &Printers::RewriterTreeRaw},
    {"index-tree", &Printers::IndexTree, true},
    {"index-tree-raw", &Printers::IndexTreeRaw, true},
    {"name-tree", &Printers::NameTree, true},
    {"name-tree-raw", &Printers::NameTreeRaw, true},
    {"resolve-tree", &Printers::ResolveTree, true},
    {"resolve-tree-raw", &Printers::ResolveTreeRaw, true},
    {"flatten-tree", &Printers::FlattenTree, true},
    {"flatten-tree-raw", &Printers::FlattenTreeRaw, true},
    {"ast", &Printers::AST, true},
    {"ast-raw", &Printers::ASTRaw, true},
    {"cfg", &Printers::CFG, true},
    {"cfg-raw", &Printers::CFGRaw, true},
    {"cfg-text", &Printers::CFGText, true},
    {"symbol-table", &Printers::SymbolTable, true},
    {"symbol-table-raw", &Printers::SymbolTableRaw, true},
    {"symbol-table-json", &Printers::SymbolTableJson, true},
    {"symbol-table-proto", &Printers::SymbolTableProto, true},
    {"symbol-table-messagepack", &Printers::SymbolTableMessagePack, true, false},
    {"symbol-table-full", &Printers::SymbolTableFull, true},
    {"symbol-table-full-raw", &Printers::SymbolTableFullRaw, true},
    {"symbol-table-full-json", &Printers::SymbolTableFullJson, true},
    {"symbol-table-full-proto", &Printers::SymbolTableFullProto, true},
    {"symbol-table-full-messagepack", &Printers::SymbolTableFullMessagePack, true, false},
    {"file-table-json", &Printers::FileTableJson, true},
    {"file-table-proto", &Printers::FileTableProto, true},
    {"file-table-messagepack", &Printers::FileTableMessagePack, true, false},
    {"file-table-full-json", &Printers::FileTableFullJson, true},
    {"file-table-full-proto", &Printers::FileTableFullProto, true},
    {"file-table-full-messagepack", &Printers::FileTableFullMessagePack, true, false},
    {"missing-constants", &Printers::MissingConstants, true},
    {"autogen", &Printers::Autogen, true},
    {"autogen-msgpack", &Printers::AutogenMsgPack, true},
    {"autogen-classlist", &Printers::AutogenClasslist, true},
    {"autogen-autoloader", &Printers::AutogenAutoloader, true, false},
    {"autogen-subclasses", &Printers::AutogenSubclasses, true},
    {"package-tree", &Printers::Packager},
});

PrinterConfig::PrinterConfig() : state(make_shared<GuardedState>()){};

void PrinterConfig::print(const string_view &contents) const {
    if (outputPath.empty()) {
        // TODO (gdritter): thread a logger through instead of just printing to stdout
        fmt::print("{}", contents);
    } else {
        absl::MutexLock lck(&state->mutex);
        fmt::format_to(std::back_inserter(state->buf), "{}", contents);
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
        ParseTreeJsonWithLocs,
        ParseTreeWhitequark,
        DesugarTree,
        DesugarTreeRaw,
        RewriterTree,
        RewriterTreeRaw,
        IndexTree,
        IndexTreeRaw,
        NameTree,
        NameTreeRaw,
        ResolveTree,
        ResolveTreeRaw,
        FlattenTree,
        FlattenTreeRaw,
        AST,
        ASTRaw,
        CFG,
        CFGText,
        CFGRaw,
        SymbolTable,
        SymbolTableRaw,
        SymbolTableProto,
        SymbolTableMessagePack,
        SymbolTableJson,
        SymbolTableFull,
        SymbolTableFullProto,
        SymbolTableFullMessagePack,
        SymbolTableFullJson,
        SymbolTableFullRaw,
        FileTableJson,
        FileTableProto,
        FileTableMessagePack,
        MissingConstants,
        Autogen,
        AutogenMsgPack,
        AutogenClasslist,
        AutogenAutoloader,
        AutogenSubclasses,
        Packager,
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
    {"rewriter", Phase::REWRITER},
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

    fmt::format_to(std::back_inserter(all_prints), "Print: [{}]",
                   fmt::map_join(
                       print_options, ", ", [](const auto &pr) -> auto { return pr.option; }));
    fmt::format_to(std::back_inserter(all_stop_after), "Stop After: [{}]",
                   fmt::map_join(
                       stop_after_options, ", ", [](const auto &pr) -> auto { return pr.option; }));

    // Advanced options
    options.add_options("advanced")("dir", "Input directory", cxxopts::value<vector<string>>());
    options.add_options("advanced")("file", "Input file", cxxopts::value<vector<string>>());
    options.add_options("advanced")("allowed-extension", "Allowed extension", cxxopts::value<vector<string>>());
    options.add_options("advanced")("web-trace-file", "Web trace file. For use with chrome about://tracing",
                                    cxxopts::value<string>()->default_value(empty.webTraceFile), "file");
    options.add_options("advanced")("debug-log-file", "Path to debug log file",
                                    cxxopts::value<string>()->default_value(empty.debugLogFile), "file");
    options.add_options("advanced")(
        "reserve-class-table-capacity", "Preallocate the specified number of entries in the class and modules table",
        cxxopts::value<u4>()->default_value(fmt::format("{}", empty.reserveClassTableCapacity)));
    options.add_options("advanced")(
        "reserve-method-table-capacity", "Preallocate the specified number of entries in the method table",
        cxxopts::value<u4>()->default_value(fmt::format("{}", empty.reserveMethodTableCapacity)));
    options.add_options("advanced")(
        "reserve-field-table-capacity", "Preallocate the specified number of entries in the field table",
        cxxopts::value<u4>()->default_value(fmt::format("{}", empty.reserveFieldTableCapacity)));
    options.add_options("advanced")(
        "reserve-type-argument-table-capacity",
        "Preallocate the specified number of entries in the type argument table",
        cxxopts::value<u4>()->default_value(fmt::format("{}", empty.reserveTypeArgumentTableCapacity)));
    options.add_options("advanced")(
        "reserve-type-member-table-capacity", "Preallocate the specified number of entries in the type member table",
        cxxopts::value<u4>()->default_value(fmt::format("{}", empty.reserveTypeMemberTableCapacity)));
    options.add_options("advanced")(
        "reserve-utf8-name-table-capacity", "Preallocate the specified number of entries in the UTF8 name table",
        cxxopts::value<u4>()->default_value(fmt::format("{}", empty.reserveUtf8NameTableCapacity)));
    options.add_options("advanced")(
        "reserve-constant-name-table-capacity",
        "Preallocate the specified number of entries in the constant name table",
        cxxopts::value<u4>()->default_value(fmt::format("{}", empty.reserveConstantNameTableCapacity)));
    options.add_options("advanced")(
        "reserve-unique-name-table-capacity", "Preallocate the specified number of entries in the unique name table",
        cxxopts::value<u4>()->default_value(fmt::format("{}", empty.reserveUniqueNameTableCapacity)));
    options.add_options("advanced")("stdout-hup-hack", "Monitor STDERR for HUP and exit on hangup");
    options.add_options("advanced")("remove-path-prefix",
                                    "Remove the provided path prefix from all printed paths. Defaults to the input "
                                    "directory passed to Sorbet, if any.",
                                    cxxopts::value<string>()->default_value(empty.pathPrefix), "prefix");
    options.add_options("advanced")("a,autocorrect", "Auto-correct source files with suggested fixes");
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
    options.add_options("advanced")("enable-experimental-lsp-document-symbol",
                                    "Enable experimental LSP feature: Document Symbol");
    options.add_options("advanced")("enable-experimental-lsp-document-formatting-rubyfmt",
                                    "Enable experimental LSP feature: Document Formatting with Rubyfmt");
    options.add_options("advanced")("enable-experimental-lsp-document-highlight",
                                    "Enable experimental LSP feature: Document Highlight");
    options.add_options("advanced")("enable-experimental-lsp-signature-help",
                                    "Enable experimental LSP feature: Signature Help");
    options.add_options("advanced")("enable-experimental-requires-ancestor",
                                    "Enable experimental `requires_ancestor` annotation");

    options.add_options("advanced")(
        "enable-all-experimental-lsp-features",
        "Enable every experimental LSP feature. (WARNING: can be crashy; for developer use only. "
        "End users should prefer to use `--enable-all-beta-lsp-features`, instead.)");
    options.add_options("advanced")("enable-all-beta-lsp-features",
                                    "Enable (expected-to-be-non-crashy) early-access LSP features.");
    options.add_options("advanced")("lsp-error-cap",
                                    "Caps the maximum number of errors that LSP reports to the editor. Can prevent "
                                    "editor slowdown triggered by large error lists. A cap of 0 means 'no cap'.",
                                    cxxopts::value<int>()->default_value(to_string(empty.lspErrorCap)), "cap");
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
    options.add_options("advanced")("stripe-packages", "Enable support for Stripe's internal Ruby package system",
                                    cxxopts::value<bool>());
    options.add_options("dev")("extra-package-files-directory-prefix",
                               "Extra parent directories which contain package files. "
                               "This option must be used in conjunction with --stripe-packages",
                               cxxopts::value<vector<string>>(), "string");
    options.add_options("dev")(
        "secondary-test-package-namespaces",
        "Secondary top-level namespaces which contain test code (in addition to Test, which is primary). "
        "This option must be used in conjunction with --stripe-packages",
        cxxopts::value<vector<string>>(), "string");

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
    options.add_options("advanced")("autogen-registry-module", "Name of Ruby module used for autoloader registry",
                                    cxxopts::value<string>()->default_value("Opus::Require"));
    options.add_options("advanced")("autogen-root-object",
                                    "Name of Ruby object on which root autoloads should be installed",
                                    cxxopts::value<string>()->default_value("Object"));
    options.add_options("advanced")("autogen-autoloader-samefile",
                                    "Modules that should never be collapsed into their parent. This helps break cycles "
                                    "in certain cases. (e.g. Foo::Bar::Baz)",
                                    cxxopts::value<vector<string>>());
    options.add_options("advanced")("autogen-autoloader-strip-prefix",
                                    "Prefixes to strip from file output paths. "
                                    "If path does not start with prefix, nothing is stripped",
                                    cxxopts::value<vector<string>>());
    options.add_options("advanced")("autogen-autoloader-packaged", "Generate the packaged autoloader files",
                                    cxxopts::value<bool>());

    options.add_options("advanced")("error-url-base",
                                    "Error URL base string. If set, error URLs are generated by prefixing the "
                                    "error code with this string.",
                                    cxxopts::value<string>()->default_value(empty.errorUrlBase), "url-base");
    options.add_options("advanced")("ruby3-keyword-args", "Enforce use of new (Ruby 3.0-style) keyword arguments",
                                    cxxopts::value<bool>());

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
    options.add_options("dev")("minimize-rbi", "TODO(jez)", cxxopts::value<std::string>()->default_value(""),
                               "<file.rbi>");
    options.add_options("dev")("skip-rewriter-passes", "Do not run Rewriter passess");
    options.add_options("dev")("wait-for-dbg", "Wait for debugger on start");
    options.add_options("dev")("stress-incremental-resolver",
                               "Force incremental updates to discover resolver & namer bugs");
    options.add_options("dev")("sleep-in-slow-path", "Add some sleeps to slow path to artificially slow it down");
    options.add_options("dev")("simulate-crash", "Crash on start");
    options.add_options("dev")("silence-dev-message", "Silence \"You are running a development build\" message");
    options.add_options("dev")("censor-for-snapshot-tests",
                               "When printing raw location information, don't show line numbers");
    options.add_options("dev")("isolate-error-code",
                               "Error code to include in reporting. "
                               "Errors not mentioned will be silenced. "
                               "This option can be passed multiple times.",
                               cxxopts::value<vector<int>>(), "errorCode");
    options.add_options("dev")("suppress-error-code",
                               "Error code to exclude from reporting. "
                               "Errors mentioned will be silenced. "
                               "This option can be passed multiple times.",
                               cxxopts::value<vector<int>>(), "errorCode");
    options.add_options("dev")("error-white-list",
                               "(DEPRECATED) Alias for --isolate-error-code. Will be removed in a later release.",
                               cxxopts::value<vector<int>>(), "errorCode");
    options.add_options("dev")("error-black-list",
                               "(DEPRECATED) Alias for --suppress-error-code. Will be removed in a later release.",
                               cxxopts::value<vector<int>>(), "errorCode");
    options.add_options("dev")("no-error-sections", "Do not print error sections.");
    options.add_options("dev")("typed", "Force all code to specified strictness level",
                               cxxopts::value<string>()->default_value("auto"), "{false,true,strict,strong,[auto]}");
    options.add_options("dev")("typed-override", "Yaml config that overrides strictness levels on files",
                               cxxopts::value<string>()->default_value(""), "filepath.yaml");
    options.add_options("dev")("store-state", "Store state into file",
                               cxxopts::value<string>()->default_value(empty.storeState), "file");
    options.add_options("dev")("cache-dir", "Use the specified folder to cache data",
                               cxxopts::value<string>()->default_value(empty.cacheDir), "dir");
    options.add_options("dev")("suppress-non-critical", "Exit 0 unless there was a critical error");

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
    options.add_options("dev")("suggest-unsafe",
                               "In as many errors as possible, suggest autocorrects to wrap problem code with "
                               "<method>. Omit the =<method> to default to wrapping with T.unsafe. "
                               "This supercedes certain autocorrects, especially T.must.",
                               cxxopts::value<std::string>()->implicit_value("T.unsafe"), "<method>");
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
    options.add_options("dev")("metrics-extra-tags", "Extra tags to report, comma separated",
                               cxxopts::value<string>()->default_value(""), "key1=value1,key2=value2");
    options.add_options("dev")(
        "force-hashing", "Forces Sorbet to calculate file hashes when run from CLI. Useful for profiling purposes.");

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
    cfg.registryModule = raw["autogen-registry-module"].as<string>();
    cfg.rootDir = stripTrailingSlashes(raw["autogen-autoloader-root"].as<string>());
    cfg.packagedAutoloader = raw["autogen-autoloader-packaged"].as<bool>();
    if (cfg.packagedAutoloader) {
        // TODO(ngroman) Multi-package autoloader runtime support
        logger->error("Flag --autogen-autoloader-packaged is currently not supported.");
        throw EarlyReturnWithCode(1);
    }
    cfg.rootObject = raw["autogen-root-object"].as<string>();
    return true;
}

void addFilesFromDir(Options &opts, string_view dir, shared_ptr<spdlog::logger> logger) {
    auto fileNormalized = stripTrailingSlashes(dir);
    opts.rawInputDirNames.emplace_back(fileNormalized);
    // Expand directory into list of files.
    vector<string> containedFiles;
    try {
        containedFiles = opts.fs->listFilesInDir(fileNormalized, opts.allowedExtensions, true,
                                                 opts.absoluteIgnorePatterns, opts.relativeIgnorePatterns);
    } catch (sorbet::FileNotFoundException) {
        logger->error("Directory `{}` not found", dir);
        throw EarlyReturnWithCode(1);
    } catch (sorbet::FileNotDirException) {
        logger->error("Path `{}` is not a directory", dir);
        throw EarlyReturnWithCode(1);
    }
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

        if (raw.count("allowed-extension") > 0) {
            auto exts = raw["allowed-extension"].as<vector<string>>();
            opts.allowedExtensions.insert(exts.begin(), exts.end());
        } else {
            opts.allowedExtensions.emplace(".rb");
            opts.allowedExtensions.emplace(".rbi");
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
                    addFilesFromDir(opts, file, logger);
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
                addFilesFromDir(opts, dir, logger);
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

        opts.requiresAncestorEnabled = raw["enable-experimental-requires-ancestor"].as<bool>();

        bool enableAllLSPFeatures = raw["enable-all-experimental-lsp-features"].as<bool>();
        opts.lspAllBetaFeaturesEnabled = enableAllLSPFeatures || raw["enable-all-beta-lsp-features"].as<bool>();
        opts.lspDocumentSymbolEnabled =
            enableAllLSPFeatures || raw["enable-experimental-lsp-document-symbol"].as<bool>();
        opts.lspDocumentHighlightEnabled =
            enableAllLSPFeatures || raw["enable-experimental-lsp-document-highlight"].as<bool>();
        opts.lspSignatureHelpEnabled = enableAllLSPFeatures || raw["enable-experimental-lsp-signature-help"].as<bool>();
        opts.lspDocumentFormatRubyfmtEnabled =
            enableAllLSPFeatures || raw["enable-experimental-lsp-document-formatting-rubyfmt"].as<bool>();

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

        opts.lspErrorCap = raw["lsp-error-cap"].as<int>();

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
        opts.minimizeRBI = raw["minimize-rbi"].as<string>();
        opts.stdoutHUPHack = raw["stdout-hup-hack"].as<bool>();
        opts.storeState = raw["store-state"].as<string>();
        opts.forceHashing = raw["force-hashing"].as<bool>();

        opts.threads = (opts.runLSP || !opts.storeState.empty())
                           ? raw["max-threads"].as<int>()
                           : min(raw["max-threads"].as<int>(), int(opts.inputFileNames.size() / 2));

        if (raw["h"].as<bool>()) {
            logger->info("{}", options.help({""}));
            throw EarlyReturnWithCode(0);
        }
        if (raw["help"].as<bool>()) {
            logger->info("{}", options.help(options.groups()));
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
        opts.skipRewriterPasses = raw["skip-rewriter-passes"].as<bool>();
        opts.suggestTyped = raw["suggest-typed"].as<bool>();
        if (raw.count("suggest-unsafe") > 0) {
            opts.suggestUnsafe = raw["suggest-unsafe"].as<string>();
        }
        opts.waitForDebugger = raw["wait-for-dbg"].as<bool>();
        opts.stressIncrementalResolver = raw["stress-incremental-resolver"].as<bool>();
        opts.sleepInSlowPath = raw["sleep-in-slow-path"].as<bool>();
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
        {
            // parse extra sfx/datadog tags
            auto stringToParse = raw["metrics-extra-tags"].as<string>();
            if (stringToParse != "") {
                for (absl::string_view sp : absl::StrSplit(stringToParse, ',')) {
                    opts.metricsExtraTags.insert(absl::StrSplit(sp, absl::MaxSplits('=', 1)));
                }
            }
        }
        opts.reserveUtf8NameTableCapacity = raw["reserve-utf8-name-table-capacity"].as<u4>();
        opts.reserveConstantNameTableCapacity = raw["reserve-constant-name-table-capacity"].as<u4>();
        opts.reserveUniqueNameTableCapacity = raw["reserve-unique-name-table-capacity"].as<u4>();
        opts.reserveClassTableCapacity = raw["reserve-class-table-capacity"].as<u4>();
        opts.reserveMethodTableCapacity = raw["reserve-method-table-capacity"].as<u4>();
        opts.reserveFieldTableCapacity = raw["reserve-field-table-capacity"].as<u4>();
        opts.reserveTypeArgumentTableCapacity = raw["reserve-type-argument-table-capacity"].as<u4>();
        opts.reserveTypeMemberTableCapacity = raw["reserve-type-member-table-capacity"].as<u4>();
        if (raw.count("autogen-version") > 0) {
            if (!opts.print.AutogenMsgPack.enabled) {
                logger->error("`{}` must also include `{}`", "--autogen-version", "-p autogen-msgpack");
                throw EarlyReturnWithCode(1);
            }
            opts.autogenVersion = raw["autogen-version"].as<int>();
        }
        opts.stripeMode = raw["stripe-mode"].as<bool>();
        opts.stripePackages = raw["stripe-packages"].as<bool>();
        if (raw.count("extra-package-files-directory-prefix")) {
            if (!opts.stripePackages) {
                logger->error("--extra-package-files-directory-prefix can only be specified in --stripe-packages mode");
                throw EarlyReturnWithCode(1);
            }
            for (const string &dirName : raw["extra-package-files-directory-prefix"].as<vector<string>>()) {
                if (dirName.back() != '/') {
                    logger->error(
                        "--extra-package-files-directory-prefix directory path must have slash (/) at the end");
                    throw EarlyReturnWithCode(1);
                }
                opts.extraPackageFilesDirectoryPrefixes.emplace_back(dirName);
            }
        }
        if (raw.count("secondary-test-package-namespaces")) {
            if (!opts.stripePackages) {
                logger->error("--secondary-test-package-namespaces can only be specified in --stripe-packages mode");
                throw EarlyReturnWithCode(1);
            }
            std::regex nsValid("[A-Z][a-zA-Z0-9]+");
            for (const string &ns : raw["secondary-test-package-namespaces"].as<vector<string>>()) {
                if (!std::regex_match(ns, nsValid)) {
                    logger->error("--secondary-test-package-namespaces must contain items that start with a capital "
                                  "letter and are alphanumeric.");
                    throw EarlyReturnWithCode(1);
                }
                opts.secondaryTestPackageNamespaces.emplace_back(ns);
            }
        }

        extractAutoloaderConfig(raw, opts, logger);
        opts.errorUrlBase = raw["error-url-base"].as<string>();
        opts.noErrorSections = raw["no-error-sections"].as<bool>();
        opts.ruby3KeywordArgs = raw["ruby3-keyword-args"].as<bool>();
        if (raw.count("error-white-list") > 0) {
            logger->error("`{}` is deprecated; please use `{}` instead", "--error-white-list", "--isolate-error-code");
            auto rawList = raw["error-white-list"].as<vector<int>>();
            opts.isolateErrorCode.insert(rawList.begin(), rawList.end());
        }
        if (raw.count("isolate-error-code") > 0) {
            auto rawList = raw["isolate-error-code"].as<vector<int>>();
            opts.isolateErrorCode.insert(rawList.begin(), rawList.end());
        }

        if (raw.count("error-black-list") > 0) {
            logger->error("`{}` is deprecated; please use `{}` instead", "--error-black-list", "--suppress-error-code");
            auto rawList = raw["error-black-list"].as<vector<int>>();
            opts.suppressErrorCode.insert(rawList.begin(), rawList.end());
        }
        if (raw.count("suppress-error-code") > 0) {
            auto rawList = raw["suppress-error-code"].as<vector<int>>();
            opts.suppressErrorCode.insert(rawList.begin(), rawList.end());
        }

        if (!opts.isolateErrorCode.empty() && !opts.suppressErrorCode.empty()) {
            logger->error("You can't pass both `{}` and `{}`", "--isolate-error-code", "--suppress-error-code");
            throw EarlyReturnWithCode(1);
        }

        if (sorbet::debug_mode) {
            opts.suggestSig = raw["suggest-sig"].as<bool>();
        }

        if (raw.count("e") == 0 && opts.inputFileNames.empty() && !raw["version"].as<bool>() && !opts.runLSP &&
            opts.storeState.empty()) {
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
            if (opts.isolateErrorCode != set<int>{core::errors::Infer::SuggestTyped.code} &&
                raw["typed"].as<string>() != "strict") {
                logger->error("--suggest-typed must also include `{}`",
                              fmt::format("{}{}", "--typed=strict --isolate-error-code=",
                                          core::errors::Infer::SuggestTyped.code));
                throw EarlyReturnWithCode(1);
            }
            if (opts.isolateErrorCode != set<int>{core::errors::Infer::SuggestTyped.code}) {
                logger->error("--suggest-typed must also include `{}`",
                              fmt::format("{}{}", "--isolate-error-code=", core::errors::Infer::SuggestTyped.code));
                throw EarlyReturnWithCode(1);
            }
            if (!opts.suppressErrorCode.empty()) {
                logger->error("--suggest-typed can't include `{}`", "--suppress-error-code");
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

        for (auto &provider : semanticExtensionProviders) {
            auto maybeExtension = provider->readOptions(raw);
            if (maybeExtension) {
                configuredExtensions.emplace_back(move(maybeExtension));
            }
        }

        // Allow semanticExtensionProviders to print something when --version is given before we throw.
        if (raw["version"].as<bool>()) {
            fmt::print("Sorbet typechecker {}\n", sorbet_full_version_string);
            throw EarlyReturnWithCode(0);
        }
    } catch (cxxopts::OptionParseException &e) {
        logger->info("{}. To see all available options pass `--help`.", e.what());
        throw EarlyReturnWithCode(1);
    }
}

} // namespace sorbet::realmain::options
