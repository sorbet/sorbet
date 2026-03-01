// have to go first as they violate our poisons
#include "rang.hpp"
#include "yaml-cpp/yaml.h"
#include <cxxopts.hpp>

#include "absl/algorithm/container.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "common/FileOps.h"
#include "common/concurrency/WorkerPool.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "common/timers/Timer.h"
#include "core/Error.h"
#include "core/errors/infer.h"
#include "main/options/ConfigParser.h"
#include "main/options/options.h"
#include "options.h"
#include "sorbet_version/sorbet_version.h"
#include "third_party/licenses/licenses.h"
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

namespace sorbet::realmain::options {

namespace {

enum class Group {
    INPUT,
    OUTPUT,
    AUTOCORRECT,
    ERROR,
    METRIC,
    LSP,
    LSP_FEATURE,
    PERFORMANCE,
    SORBET_PACKAGES_MODE,
    STRIPE_AUTOGEN,
    DEBUGGING,
    INTERNAL,
    EXPERIMENTAL,
    OTHER,
};

string groupToString(Group group) {
    switch (group) {
        case Group::INPUT:
            return "INPUT";
        case Group::OUTPUT:
            return "OUTPUT";
        case Group::AUTOCORRECT:
            return "AUTOCORRECT";
        case Group::ERROR:
            return "ERROR";
        case Group::METRIC:
            return "METRIC";
        case Group::LSP:
            return "LSP";
        case Group::LSP_FEATURE:
            return "LSP FEATURE";
        case Group::PERFORMANCE:
            return "PERFORMANCE";
        case Group::SORBET_PACKAGES_MODE:
            return "SORBET PACKAGES MODE";
        case Group::STRIPE_AUTOGEN:
            return "STRIPE AUTOGEN";
        case Group::DEBUGGING:
            return "DEBUGGING";
        case Group::INTERNAL:
            return "INTERNAL";
        case Group::EXPERIMENTAL:
            return "EXPERIMENTAL";
        case Group::OTHER:
            return "OTHER";
    }
}

// All sections, in order.
// Otherwise, in the help output cxxopts will sort the sections by name.
const vector<string> groupSections{
    groupToString(Group::INPUT),          groupToString(Group::OUTPUT),      groupToString(Group::AUTOCORRECT),
    groupToString(Group::ERROR),          groupToString(Group::METRIC),      groupToString(Group::LSP),
    groupToString(Group::LSP_FEATURE),    groupToString(Group::PERFORMANCE), groupToString(Group::SORBET_PACKAGES_MODE),
    groupToString(Group::STRIPE_AUTOGEN), groupToString(Group::DEBUGGING),   groupToString(Group::INTERNAL),
    groupToString(Group::EXPERIMENTAL),   groupToString(Group::OTHER),
};

struct PrintOptions {
    string option;
    PrinterConfig Printers::*config;

    // Whether the option is compatible with --cache-dir.
    // (Some printers only run in a phase of pipeline.cc that runs if a tree is not cached)
    bool supportsCaching = true;

    // If false, printer is responsible for flushing its own output.
    // Otherwise, just using opts.print.MyPrinter.print(...) with a string will be enough.
    bool supportsFlush = true;

    // Whether users can consider this output stable, or whether it's for internal-use only.
    bool stable = false;
};

const vector<PrintOptions> print_options({
    {"parse-tree", &Printers::ParseTree, false},
    {"parse-tree-json", &Printers::ParseTreeJson, false},
    {"parse-tree-json-with-locs", &Printers::ParseTreeJsonWithLocs, false},
    {"parse-tree-whitequark", &Printers::ParseTreeWhitequark, false},
    {"rbs-rewrite-tree", &Printers::RBSRewriteTree, false},
    {"desugar-tree", &Printers::DesugarTree, false},
    {"desugar-tree-raw", &Printers::DesugarTreeRaw, false},
    {"desugar-tree-raw-with-locs", &Printers::DesugarTreeRawWithLocs, false},
    {"rewrite-tree", &Printers::RewriterTree, false},
    {"rewrite-tree-raw", &Printers::RewriterTreeRaw, false},
    {"index-tree", &Printers::IndexTree},
    {"index-tree-raw", &Printers::IndexTreeRaw},
    {"name-tree", &Printers::NameTree},
    {"name-tree-raw", &Printers::NameTreeRaw},
    {"resolve-tree", &Printers::ResolveTree},
    {"resolve-tree-raw", &Printers::ResolveTreeRaw},
    {"flatten-tree", &Printers::FlattenTree},
    {"flatten-tree-raw", &Printers::FlattenTreeRaw},
    {"ast", &Printers::AST},
    {"ast-raw", &Printers::ASTRaw},
    {"cfg", &Printers::CFG},
    {"cfg-raw", &Printers::CFGRaw},
    {"cfg-text", &Printers::CFGText},
    {"symbol-table", &Printers::SymbolTable},
    {"symbol-table-raw", &Printers::SymbolTableRaw},
    {"symbol-table-json", &Printers::SymbolTableJson},
    {"symbol-table-proto", &Printers::SymbolTableProto},
    {"symbol-table-messagepack", &Printers::SymbolTableMessagePack, true, false},
    {"symbol-table-full", &Printers::SymbolTableFull},
    {"symbol-table-full-raw", &Printers::SymbolTableFullRaw},
    {"symbol-table-full-json", &Printers::SymbolTableFullJson},
    {"symbol-table-full-proto", &Printers::SymbolTableFullProto},
    {"symbol-table-full-messagepack", &Printers::SymbolTableFullMessagePack, true, false},
    {"file-table-json", &Printers::FileTableJson, true, true, true},
    {"file-table-proto", &Printers::FileTableProto, true, true, true},
    {"file-table-messagepack", &Printers::FileTableMessagePack, true, false, true},
    {"file-table-full-json", &Printers::FileTableFullJson, true, true, true},
    {"file-table-full-proto", &Printers::FileTableFullProto, true, true, true},
    {"file-table-full-messagepack", &Printers::FileTableFullMessagePack, true, false, true},
    {"missing-constants", &Printers::MissingConstants, true, true, true},
    {"autogen", &Printers::Autogen},
    {"autogen-msgpack", &Printers::AutogenMsgPack},
    {"autogen-subclasses", &Printers::AutogenSubclasses},
    {"package-tree", &Printers::Packager, false},
    {"minimized-rbi", &Printers::MinimizeRBI},
    {"payload-sources", &Printers::PayloadSources, true, true, true},
    {"untyped-blame", &Printers::UntypedBlame, true, true, true},
});

struct ParserOptions {
    std::string option;
    Parser flag;
};

const vector<ParserOptions> parser_options({
    {"original", Parser::ORIGINAL},
    {"prism", Parser::PRISM},
});

} // namespace

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
    FileOps::write(outputPath, string_view(state->buf.data(), state->buf.size()));
};

string PrinterConfig::flushToString() {
    absl::MutexLock lck(&state->mutex);
    return to_string(state->buf);
};

vector<reference_wrapper<PrinterConfig>> Printers::printers() {
    return vector<reference_wrapper<PrinterConfig>>({
        ParseTree,
        ParseTreeJson,
        ParseTreeJsonWithLocs,
        ParseTreeWhitequark,
        RBSRewriteTree,
        DesugarTree,
        DesugarTreeRaw,
        DesugarTreeRawWithLocs,
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
        AutogenSubclasses,
        Packager,
        MinimizeRBI,
        PayloadSources,
        UntypedBlame,
    });
}

namespace {

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

core::TrackUntyped text2TrackUntyped(string_view key, spdlog::logger &logger) {
    if (key == "") {
        return core::TrackUntyped::Everywhere;
    } else if (key == "nowhere") {
        return core::TrackUntyped::Nowhere;
    } else if (key == "everywhere-but-tests") {
        return core::TrackUntyped::EverywhereButTests;
    } else if (key == "everywhere") {
        return core::TrackUntyped::Everywhere;
    } else {
        logger.error("Unknown --track-untyped option: `{}`", key);
        throw EarlyReturnWithCode(1);
    }
}

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

UnorderedMap<string, core::StrictLevel> extractStrictnessOverrides(string fileName, shared_ptr<spdlog::logger> logger) {
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

    cxxopts::Options options("sorbet", "Sorbet: A fast, powerful typechecker designed for Ruby");
    string section;

    struct winsize w;
    unsigned short defaultCols = 100;
    if (ioctl(STDERR_FILENO, TIOCGWINSZ, &w) != -1) {
        defaultCols = std::min(defaultCols, w.ws_col);
    }
    options.set_width(defaultCols);

    // TODO(neil): we should mention how vector options work/can be used.
    // (ie. that they can be passed as both `--arg 1 --arg 2` and `--arg 1,2`)
    // ----- INPUT -------------------------------------------------------- {{{
    section = groupToString(Group::INPUT);
    options.add_options(section)("files", "Input files", cxxopts::value<vector<string>>());
    options.parse_positional("files");
    options.custom_help("[options]");
    options.positional_help("[[--] <path>...]");

    options.add_options(section)("e",
                                 "Treat <string> as if it were the contents of a Ruby file passed on the command line",
                                 cxxopts::value<string>()->default_value(empty.inlineInput), "<string>");
    options.add_options(section)("e-rbi", "Like `-e`, but treat <string> as an RBI file",
                                 cxxopts::value<string>()->default_value(empty.inlineRBIInput), "<string>");
    options.add_options(section)("file",
                                 "Run over the contents of <path>\n"
                                 "(Equivalent to passing <path> as a positional argument)",
                                 cxxopts::value<vector<string>>(), "<path>");
    options.add_options(section)("dir",
                                 "Run over all Ruby and RBI files in <path>, recursively\n"
                                 "(Equivalent to passing <path> as a positional argument)",
                                 cxxopts::value<vector<string>>(), "<path>");
    options.add_options(section)(
        "allowed-extension",
        "Use these extensions to determine which file types Sorbet should discover inside directories.",
        cxxopts::value<vector<string>>()->default_value(".rb,.rbi"), "<ext>[,<ext>...]");
    options.add_options(section)(
        "ignore",
        "Ignores input files that contain <pattern> in their paths (relative to the input path passed to Sorbet).\n"
        "When <pattern> starts with `/` it matches against the prefix of these relative paths; others match anywhere.\n"
        "Matches must be against whole path segments, so `foo` matches `foo/bar.rb` and `bar/foo/baz.rb` but not "
        "`foo.rb` or `foo2/bar.rb`.",
        cxxopts::value<vector<string>>(), "<pattern>");
    options.add_options(section)("no-config",
                                 "Do not load the content of the `sorbet/config` file.\n"
                                 "Otherwise, Sorbet reads the `sorbet/config` file and treats each line as if it were "
                                 "passed on the command line, unless the line starts with `#`.\n"
                                 "To load a <file> as if it were a config file, pass `@<file>` as a positional arg");
    options.add_options(section)(
        "typed",
        "Force all code to specified strictness level, disregarding all `# typed:` sigils. For `auto`, "
        "uses the `# typed:` sigil in the file or `false` for files without a sigil.",
        cxxopts::value<string>()->default_value("auto"), "{false,true,strict,strong,[auto]}");
    options.add_options(section)("typed-override",
                                 "Read <filepath.yaml> to override the strictness of individual files.\n"
                                 "Contents must be a map of `<strictness>: ['path1.rb', ...]` pairs.\n"
                                 "Can be used to enable type checking for certain files temporarily without having "
                                 "to add a comment to every file.",
                                 cxxopts::value<string>()->default_value(""), "<filepath.yaml>");
    // }}}

    // ----- OUTPUT ------------------------------------------------------- {{{
    section = groupToString(Group::OUTPUT);
    // TODO(jez) What is critical error? How does this affect the exit code?
    options.add_options(section)("q,quiet", "Silence all non-critical errors");
    options.add_options(section)("P,progress", "Draw progressbar");
    options.add_options(section)("color", "Use color output. For `auto`: use color if stderr is a tty",
                                 cxxopts::value<string>()->default_value("auto"), "{always,never,[auto]}");
    options.add_options(section)("no-error-count", "Do not print the `Errors: <N>` summary line");
    options.add_options(section)(
        "no-error-sections", "Only print the first line of every error message (suppress any additional information "
                             "below an error). Can provide substantial speedups when dealing with many errors");
    options.add_options(section)(
        "error-url-base",
        "Every error message includes a link created by prefixing that error's code with <url-base>. Can be used to "
        "maintain docs on Sorbet error codes which are more relevant to a specific project or company.",
        cxxopts::value<string>()->default_value(empty.errorUrlBase), "<url-base>");
    options.add_options(section)("remove-path-prefix",
                                 "Remove the provided <prefix> from all printed paths. Defaults to the input "
                                 "directory passed to Sorbet, if any.",
                                 cxxopts::value<string>()->default_value(empty.pathPrefix), "<prefix>");
    options.add_options(section)("gen-packages", "Generate package information", cxxopts::value<bool>());
    // }}}

    // ----- AUTOCORRECTS ------------------------------------------------- {{{
    section = groupToString(Group::AUTOCORRECT);
    options.add_options(section)(
        "a,autocorrect", "Auto-correct source files with suggested fixes.\n"
                         "Use the `--{isolate,suppress}-error-code` options to control which corrections to apply");
    options.add_options(section)(
        "suggest-unsafe",
        "Include 'unsafe' autocorrects, e.g. those which can be fixed by wrapping code in `T.unsafe` or using "
        "`T.untyped`. Provide a custom <method> to wrap with `<method>(...)` instead of `T.unsafe(...)`. This "
        "supersedes certain autocorrects, especially T.must.",
        cxxopts::value<string>()->implicit_value("T.unsafe"), "<method>");
    options.add_options(section)("did-you-mean",
                                 "Whether to include 'Did you mean' suggestions in autocorrects. For large codemods, "
                                 "it's usually better to avoid spurious changes by setting this to false",
                                 cxxopts::value<bool>()->default_value("true"), "{[true],false}");
    options.add_options(section)("suggest-typed",
                                 "Emit autocorrects to set the `# typed:` sigil in every file to the highest possible "
                                 "level where no errors would be reported in that file. Will downgrade the `# typed:` "
                                 "sigil for any files with errors.");
    // }}}

    // ----- ERRORS ------------------------------------------------------- {{{
    section = groupToString(Group::ERROR);
    options.add_options(section)("typed-super", "Enable typechecking of `super` calls when possible",
                                 cxxopts::value<bool>()->default_value("true"));
    options.add_options(section)("check-out-of-order-constant-references",
                                 "Detect when a constant is referenced early in a file, but defined later in that "
                                 "file. Does not detect out-of-order references across file boundaries.");
    options.add_options(section)("isolate-error-code",
                                 "Which error(s) to include in reporting. This option can be passed multiple times. "
                                 "All errors not mentioned will be silenced.",
                                 cxxopts::value<vector<int>>(), "<error-code>");
    options.add_options(section)("suppress-error-code",
                                 "Which error(s) to exclude from reporting. This option can be passed multiple times.",
                                 cxxopts::value<vector<int>>(), "<error-code>");
    options.add_options(section)("suppress-payload-superclass-redefinition-for",
                                 "Explicitly suppress the superclass redefinition error for the specified class "
                                 "defined in Sorbet's payload. May be repeated.",
                                 cxxopts::value<vector<string>>(), "Fully::Qualified::ClassName");
    options.add_options(section)("experimental-ruby3-keyword-args",
                                 "Enforce use of new (Ruby 3.0-style) keyword arguments. (incomplete and experimental)",
                                 cxxopts::value<bool>());
    options.add_options(section)("uniquely-defined-behavior",
                                 "Ensure that every class and module only defines 'behavior' in one file. Ensures "
                                 "that every class or module can be autoloaded by loading exactly one file.",
                                 cxxopts::value<bool>());
    fmt::memory_buffer all_stop_after;
    fmt::format_to(
        std::back_inserter(all_stop_after),
        "Stop after completing <phase>. Can be useful for debugging. Also useful when overwhelmed with errors, because "
        "errors from earlier phases (like resolver) can cause errors downstream (in inferencer).\n"
        "Phases: [{}]",
        fmt::map_join(stop_after_options, ", ", [](const auto &pr) { return pr.option; }));
    options.add_options(section)("stop-after", to_string(all_stop_after),
                                 cxxopts::value<string>()->default_value("inferencer"), "<phase>");
    // }}}

    // ----- METRICS ------------------------------------------------------ {{{
    section = groupToString(Group::METRIC);
    options.add_options(section)("counters", "Print all internal counters");
    options.add_options(section)("counter", "Print internal counter for <counter> (repeatable)",
                                 cxxopts::value<vector<string>>(), "<counter>");
    options.add_options(section)(
        "track-untyped",
        "Include a per-file counter of untyped usages in the `--print=file-table-<format>` output. "
        "This is in addition to the codebase-wide `types.input.untyped.usages` counter.",
        cxxopts::value<string>()->implicit_value("everywhere"), "{[nowhere],everywhere,everywhere-but-tests}");
    options.add_options(section)("metrics-file", "Report counters and some timers to <file>, in JSON format.",
                                 cxxopts::value<string>()->default_value(empty.metricsFile), "<file>");
    options.add_options(section)("metrics-prefix", "String to prefix all metrics with, e.g. `my_org.my_repo`.",
                                 cxxopts::value<string>()->default_value(empty.metricsPrefix), "<string>");
    options.add_options(section)("metrics-branch", "Branch to report in metrics export.",
                                 cxxopts::value<string>()->default_value(empty.metricsBranch), "<branch>");
    options.add_options(section)("metrics-sha", "Set the `sha` field to <sha> in the `--metrics-file` output.",
                                 cxxopts::value<string>()->default_value(empty.metricsSha), "<sha>");
    options.add_options(section)("metrics-repo", "Set the `repo` field to <repo> in the `--metrics-file` output.",
                                 cxxopts::value<string>()->default_value(empty.metricsRepo), "<repo>");
    options.add_options(section)("statsd-host", "StatsD sever hostname",
                                 cxxopts::value<string>()->default_value(empty.statsdHost), "<host>");
    options.add_options(section)("statsd-prefix", "StatsD prefix",
                                 cxxopts::value<string>()->default_value(empty.statsdPrefix), "<prefix>");
    options.add_options(section)("statsd-port", "StatsD server port",
                                 cxxopts::value<int>()->default_value(fmt::format("{}", empty.statsdPort)), "<port>");
    options.add_options(section)("metrics-extra-tags", "Extra tags to include in every statsd metric, comma separated.",
                                 cxxopts::value<string>()->default_value(""), "<key1>=<value1>,<key2>=<value2>");
    // }}}

    // ----- LSP ---------------------------------------------------------- {{{
    section = groupToString(Group::LSP);
    options.add_options(section)("lsp", "Start in language server protocol mode (LSP mode)");
    options.add_options(section)(
        "lsp-error-cap",
        "Reports no more than <cap> diagnostics (e.g. errors and informations) to the language client, like VS Code. "
        "Can prevent slowdown triggered by large diagnostic lists. A <cap> of 0 means no limit.",
        cxxopts::value<int>()->default_value(to_string(empty.lspErrorCap)), "<cap>");
    options.add_options(section)("disable-watchman", "When in LSP mode, disable file watching via Watchman");
    options.add_options(section)("watchman-path",
                                 "Path to watchman executable. Will search on `PATH` if <path> contains no slashes.",
                                 cxxopts::value<string>()->default_value(empty.watchmanPath), "<path>");
    options.add_options(section)("watchman-pause-state-name",
                                 "Name of watchman state that halts processing for its duration",
                                 cxxopts::value<string>()->default_value(empty.watchmanPauseStateName), "<state>");
    options.add_options(section)("watchman-namespace", "Namespace for watchman",
                                 cxxopts::value<string>()->default_value(empty.watchmanNamespace), "<namespace>");
    options.add_options(section)(
        "lsp-directories-missing-from-client",
        "Directory prefixes that only exist where the LSP server is running, not on the client. "
        "Useful when running Sorbet via an `ssh` connection to a remote server, where the remote server has generated "
        "files that do not exist on the client. References to files in these directories will be sent "
        "as `sorbet:` URIs to clients that understand them.",
        cxxopts::value<vector<string>>(), "<path>");
    options.add_options(section)(
        "sorbet-root",
        "Relative path from the project/repository root to Sorbet's input directory. "
        "Set this when the editor's workspace root is an ancestor of Sorbet's input directory "
        "(e.g., if the editor opens `/proj` but Sorbet runs against `/proj/Library/Homebrew`, "
        "pass `--sorbet-root=Library/Homebrew`).",
        cxxopts::value<string>()->default_value(empty.sorbetRoot), "<path>");
    options.add_options(section)(
        "forcibly-silence-lsp-multiple-dir-error",
        "Allow the LSP to start with multiple `--dir` options by silencing the error. (WARNING: This flag does not "
        "address the known issues with multiple directory support in LSP mode. You are likely to encounter unexpected "
        "behavior.)");
    // }}}

    // ----- LSP FEATURES ------------------------------------------------- {{{
    section = groupToString(Group::LSP_FEATURE);
    options.add_options(section)("enable-experimental-lsp-document-formatting-rubyfmt",
                                 "Enable experimental LSP feature: Document Formatting with Rubyfmt");
    options.add_options(section)("rubyfmt-path",
                                 "Path to the rubyfmt executable used for document formatting. Will search on `PATH` "
                                 "if <path> contains no slashes.",
                                 cxxopts::value<string>()->default_value(empty.rubyfmtPath), "<path>");
    options.add_options(section)("enable-experimental-lsp-document-highlight",
                                 "Enable experimental LSP feature: Document Highlight");
    options.add_options(section)("enable-experimental-lsp-signature-help",
                                 "Enable experimental LSP feature: Signature Help");

    options.add_options(section)("enable-experimental-lsp-extract-to-variable",
                                 "Enable experimental LSP feature: Extract To Variable");
    options.add_options(section)(
        "enable-all-experimental-lsp-features",
        "Enable every experimental LSP feature. (WARNING: can be crashy; for developer use only. "
        "End users should prefer to use `--enable-all-beta-lsp-features`, instead.)");
    options.add_options(section)("enable-all-beta-lsp-features",
                                 "Enable (expected-to-be-non-crashy) early-access LSP features.");
    // }}}

    // ----- PERFORMANCE -------------------------------------------------- {{{
    section = groupToString(Group::PERFORMANCE);
    options.add_options(section)("web-trace-file",
                                 "Generate a trace into <file> in the Trace Event Format (used by chrome://tracing)",
                                 cxxopts::value<string>()->default_value(empty.webTraceFile), "<file>");
    options.add_options(section)("web-trace-file-strict", "Whether to close the toplevel array in `--web-trace-file`",
                                 cxxopts::value<bool>());
    options.add_options(section)("cache-dir", "Use <dir> to cache certain data. Will create <dir> if it does not exist",
                                 cxxopts::value<string>()->default_value(empty.cacheDir), "<dir>");
    options.add_options(section)("max-cache-size-bytes",
                                 "Must be a multiple of OS page size (usually 4096). Subject to restrictions on "
                                 "mdb_env_set_mapsize function in LMDB API docs.",
                                 cxxopts::value<size_t>()->default_value(to_string(empty.maxCacheSizeBytes)),
                                 "<bytes>");
    int defaultThreads = thread::hardware_concurrency();
    if (defaultThreads == 0) {
        defaultThreads = 2;
    }
    options.add_options(section)("max-threads",
                                 "Set number of threads to use for fork/join worker pools. LSP will <n> threads plus "
                                 "some extra threads to manage the connection with the client, watchman, etc.",
                                 cxxopts::value<int>()->default_value(to_string(defaultThreads)), "<n>");
    options.add_options(section)(
        "reserve-class-table-capacity", "Preallocate <n> slots in the class and modules table",
        cxxopts::value<uint32_t>()->default_value(fmt::format("{}", empty.reserveClassTableCapacity)), "<n>");
    options.add_options(section)(
        "reserve-method-table-capacity", "Preallocate <n> slots in the method table",
        cxxopts::value<uint32_t>()->default_value(fmt::format("{}", empty.reserveMethodTableCapacity)), "<n>");
    options.add_options(section)(
        "reserve-field-table-capacity", "Preallocate <n> slots in the field table",
        cxxopts::value<uint32_t>()->default_value(fmt::format("{}", empty.reserveFieldTableCapacity)), "<n>");
    options.add_options(section)(
        "reserve-type-argument-table-capacity", "Preallocate <n> slots in the type argument table",
        cxxopts::value<uint32_t>()->default_value(fmt::format("{}", empty.reserveTypeParameterTableCapacity)), "<n>");
    options.add_options(section)(
        "reserve-type-member-table-capacity", "Preallocate <n> slots in the type member table",
        cxxopts::value<uint32_t>()->default_value(fmt::format("{}", empty.reserveTypeMemberTableCapacity)), "<n>");
    options.add_options(section)(
        "reserve-utf8-name-table-capacity", "Preallocate <n> slots in the UTF8 name table",
        cxxopts::value<uint32_t>()->default_value(fmt::format("{}", empty.reserveUtf8NameTableCapacity)), "<n>");
    options.add_options(section)(
        "reserve-constant-name-table-capacity", "Preallocate <n> slots in the constant name table",
        cxxopts::value<uint32_t>()->default_value(fmt::format("{}", empty.reserveConstantNameTableCapacity)), "<n>");
    options.add_options(section)(
        "reserve-unique-name-table-capacity", "Preallocate <n> slots in the unique name table",
        cxxopts::value<uint32_t>()->default_value(fmt::format("{}", empty.reserveUniqueNameTableCapacity)), "<n>");
    // }}}

    // ----- STRIPE PACKAGES ---------------------------------------------- {{{
    section = groupToString(Group::SORBET_PACKAGES_MODE);
    options.add_options(section)("stripe-packages", "Enable support for Sorbet's experimental Ruby package system",
                                 cxxopts::value<bool>());
    options.add_options(section)("stripe-packages-hint-message",
                                 "Optional hint message to add to all packaging related errors",
                                 cxxopts::value<string>()->default_value(""));
    options.add_options(section)("sorbet-packages", "Enable support for Sorbet's experimental Ruby package system",
                                 cxxopts::value<bool>());
    options.add_options(section)("sorbet-packages-hint-message",
                                 "Optional hint message to add to all packaging related errors",
                                 cxxopts::value<string>()->default_value(""));
    options.add_options(section)("extra-package-files-directory-prefix-underscore",
                                 "Extra parent directories which contain package files. Files are associated to a "
                                 "package using a package's namespace joined by underscores. That is, files in "
                                 "<dir>/Project_FooBar/ belong to Project::FooBar",
                                 cxxopts::value<vector<string>>(), "<dir>");
    options.add_options(section)("extra-package-files-directory-prefix-slash-deprecated",
                                 "Extra parent directories which contain package files. Files are associated to a "
                                 "package using slash munging conventions. That is, files in "
                                 "<dir>/Project/Foo_bar/ belong to Project::FooBar. This is deprecated in favor of "
                                 "--extra-package-files-directory-prefix-slash since the paths are more readable",
                                 cxxopts::value<vector<string>>(), "<dir>");
    options.add_options(section)("extra-package-files-directory-prefix-slash",
                                 "Extra parent directories which contain package files. Files are associated to a "
                                 "package using a package's namespace join by slashes. That is, files in "
                                 "<dir>/Project/FooBar/ belong to Project::FooBar",
                                 cxxopts::value<vector<string>>(), "<dir>");
    options.add_options(section)("allow-relaxed-packager-checks-for",
                                 "Packages which are allowed to ignore the restrictions set by `visible_to` "
                                 "and `export` directives",
                                 cxxopts::value<vector<string>>(), "<name>");
    options.add_options(section)("gen-packages-update-visibility-for",
                                 "Packages for which to generate `visible_to` autocorrects in --gen-packages mode",
                                 cxxopts::value<vector<string>>(), "<name>");
    options.add_options(section)(
        "packager-layers",
        "Valid layer names for packages, ordered lowest to highest. Passing this flag also enables layering checks.",
        cxxopts::value<vector<string>>()->implicit_value("library,application"), "<layer-name>");
    options.add_options(section)("package-skip-rbi-export-enforcement",
                                 "Constants defined in RBIs in these directories (and no others) can be exported",
                                 cxxopts::value<vector<string>>(), "<dir>");
    options.add_options(section)("package-attributed-errors",
                                 "Attribute errors to their enclosing package in the error output",
                                 cxxopts::value<bool>());
    options.add_options(section)("experimental-package-directed",
                                 "Enable support for checking by package, instead of processing all files at once",
                                 cxxopts::value<bool>());
    options.add_options(section)("experimental-test-packages", "Enable support for tests as their own packages",
                                 cxxopts::value<bool>());
    // }}}

    // ----- STRIPE AUTOGEN ----------------------------------------------- {{{
    section = groupToString(Group::STRIPE_AUTOGEN);
    options.add_options(section)("autogen-version", "Autogen version to output", cxxopts::value<int>());
    options.add_options(section)("autogen-subclasses-parent",
                                 "Parent classes for which generate a list of subclasses. "
                                 "This option must be used in conjunction with -p autogen-subclasses",
                                 cxxopts::value<vector<string>>(), "string");
    options.add_options(section)("autogen-subclasses-ignore",
                                 "Like --ignore, but it only affects `-p autogen-subclasses`.",
                                 cxxopts::value<vector<string>>(), "string");
    options.add_options(section)("autogen-behavior-allowed-in-rbi-files-paths",
                                 "RBI files defined in these paths can be considered by autogen as behavior-defining.",
                                 cxxopts::value<vector<string>>(), "string");
    options.add_options(section)("autogen-msgpack-skip-reference-metadata",
                                 "Skip serializing extra metadata on references when printing msgpack in autogen",
                                 cxxopts::value<bool>());
    // }}}

    // ----- DEBUGGING ---------------------------------------------------- {{{
    section = groupToString(Group::DEBUGGING);
    options.add_options(section)("v,verbose", "Verbosity level [0-3]");
    options.add_options(section)("debug-log-file", "Path to debug log file",
                                 cxxopts::value<string>()->default_value(empty.debugLogFile), "<file>");
    options.add_options(section)("wait-for-dbg", "Wait for debugger to attach after starting. Especially useful to "
                                                 "attach to a Sorbet process launched by a language client");
    options.add_options(section)("sleep-in-slow-path", "Add some sleeps to slow path to artificially slow it down",
                                 cxxopts::value<int>()->implicit_value("3"));
    options.add_options(section)("stress-incremental-resolver",
                                 "Simulate updates which tend to expose namer and resolver bugs");
    options.add_options(section)("simulate-crash", "Raise an uncaught C++ exception on startup to simulate a crash");
    options.add_options(section)("force-hashing",
                                 "Force Sorbet to calculate file hashes, even from the CLI. Useful for profiling.");
    options.add_options(section)("trace-lexer", "Emit the lexer's token stream in a debug format");
    options.add_options(section)("trace-parser", "Enable bison's parser trace functionality");
    auto partitioned_print_options = print_options;
    auto stableEnd = absl::c_stable_partition(partitioned_print_options, [](const auto &po) { return po.stable; });
    fmt::memory_buffer print_help;
    fmt::format_to(std::back_inserter(print_help),
                   "Print various internal data structures.\n"
                   "By default, the output is to stdout. To send the data to a file, use\n"
                   "--print=<format>:<file>\n"
                   "Most of these formats are unstable, for internal-use only.\n\n"
                   "Stable: [");
    fmt::format_to(
        std::back_inserter(print_help), "{}",
        fmt::map_join(partitioned_print_options.begin(), stableEnd, ", ", [](const auto &it) { return it.option; }));
    fmt::format_to(std::back_inserter(print_help), "]\n\n"
                                                   "Unstable: [");
    fmt::format_to(
        std::back_inserter(print_help), "{}",
        fmt::map_join(stableEnd, partitioned_print_options.end(), ", ", [](const auto &it) { return it.option; }));
    options.add_options(section)("p,print", to_string(print_help), cxxopts::value<vector<string>>(), "<format>");
    // }}}

    // ----- INTERNAL ----------------------------------------------------- {{{
    section = groupToString(Group::INTERNAL);
    // TODO(jez) I'm pretty sure that `--quiet` and `--suppress-non-critical` are the same?
    options.add_options(section)("suppress-non-critical",
                                 "Exit 0 unless there was a critical error (i.e., an uncaught exception)");
    options.add_options(section)("no-stdlib",
                                 "Do not load Sorbet's payload which defines RBI files for the Ruby standard library");
    options.add_options(section)(
        "store-state", "Store state into three files, separated by commas: <symbol-table>,<name-table>,<file-table>",
        cxxopts::value<string>()->default_value(""), "file");
    options.add_options(section)("silence-dev-message", "Silence \"You are running a development build\" message");
    options.add_options(section)("censor-for-snapshot-tests",
                                 "When printing raw location information, don't show line numbers");
    // }}}

    // ----- EXPERIMENTAL ------------------------------------------------- {{{
    section = groupToString(Group::EXPERIMENTAL);

    options.add_options(section)("enable-experimental-requires-ancestor",
                                 "Enable experimental `requires_ancestor` annotation");

    options.add_options(section)("enable-experimental-rbs-signatures",
                                 "Enable experimental support for RBS signatures as inline comments");
    options.add_options(section)("enable-experimental-rbs-assertions",
                                 "Enable experimental support for RBS assertions as inline comments");
    options.add_options(section)("enable-experimental-rbs-comments",
                                 "Enable experimental support for RBS signatures and assertions as inline comments");

    options.add_options(section)(
        "enable-experimental-rspec",
        "Enables experimental support for RSpec. "
        "There are many RSpec constructs that are impossible for Sorbet to handle. "
        "As a result, there is no path to this flag ever being stable: RSpec support is best effort.");
    // }}}

    // ----- OTHER -------------------------------------------------------- {{{
    section = groupToString(Group::OTHER);
    options.add_options(section)("version", "Show Sorbet's version");
    options.add_options(section)("license", "Show Sorbet's license, and licenses of its dependencies");
    options.add_options(section)("stdout-hup-hack",
                                 "Monitor STDERR for HUP and exit on hangup to work around OpenSSH bug");
    options.add_options(section)("minimize-to-rbi",
                                 "[experimental] Output a minimal RBI containing the diff between Sorbet's view of a "
                                 "codebase and the definitions present in this file",
                                 cxxopts::value<string>()->default_value(""), "<file.rbi>");
    options.add_options(section)("h,help",
                                 "Show help. Can pass an optional SECTION to show help for only one section instead of "
                                 "the default of all sections",
                                 cxxopts::value<vector<string>>()->implicit_value("all"), "SECTION");
    options.add_options(section)("parser",
                                 "Which parser to use. Prism support is experimental and still under active "
                                 "development. Correct code should still parse correctly, but error diagnostics "
                                 "and auto-corrections are a work-in-progress.",
                                 cxxopts::value<string>()->default_value("original"), "{[original], prism}");

    // }}}

    for (auto &provider : semanticExtensionProviders) {
        provider->injectOptions(options);
    }

    return options;
}

bool extractPrinters(cxxopts::ParseResult &raw, Options &opts, shared_ptr<spdlog::logger> logger) {
    if (raw.count("print") == 0) {
        return true;
    }
    const vector<string> &printOpts = raw["print"].as<vector<string>>();
    for (string_view opt : printOpts) {
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
                if (known.option == "desugar-tree-raw-with-locs") {
                    logger->warn("Option `--print=desugar-tree-raw-with-locs` is a temporary option and will be "
                                 "removed in a future version of Sorbet.");
                }
                if (!known.supportsCaching) {
                    if (!opts.cacheDir.empty()) {
                        logger->error("--print={} is incompatible with --cache-dir. Ignoring cache", opt);
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

} // namespace

std::optional<Parser> extractParser(std::string_view opt, std::shared_ptr<spdlog::logger> logger) {
    for (auto &known : parser_options) {
        if (known.option == opt) {
            return known.flag;
        }
    }
    vector<string_view> allOptions;
    for (auto &known : parser_options) {
        allOptions.emplace_back(known.option);
    }

    logger->error("Unknown --parser option: {}\nValid values: {}", opt, fmt::join(allOptions, ", "));
    return nullopt;
}

void Options::flushPrinters() {
    for (PrinterConfig &cfg : print.printers()) {
        cfg.flush();
    }
}

namespace {

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

void addFilesFromDir(Options &opts, string_view dir, WorkerPool &workerPool, shared_ptr<spdlog::logger> logger) {
    auto fileNormalized = stripTrailingSlashes(dir);
    opts.rawInputDirNames.emplace_back(fileNormalized);
    // Expand directory into list of files.
    vector<string> containedFiles;
    try {
        containedFiles = opts.fs->listFilesInDir(fileNormalized, opts.allowedExtensions, workerPool, true,
                                                 opts.absoluteIgnorePatterns, opts.relativeIgnorePatterns);
    } catch (sorbet::FileNotFoundException &e) {
        logger->error("{}", e.what());
        throw EarlyReturnWithCode(1);
    } catch (sorbet::FileNotDirException) {
        logger->error("Path `{}` is not a directory", dir);
        throw EarlyReturnWithCode(1);
    }
    if (opts.inputFileNames.size() == 0) {
        opts.inputFileNames = move(containedFiles);
    } else {
        opts.inputFileNames.reserve(opts.inputFileNames.size() + containedFiles.size());
        opts.inputFileNames.insert(opts.inputFileNames.end(), std::make_move_iterator(containedFiles.begin()),
                                   std::make_move_iterator(containedFiles.end()));
    }
}

} // namespace

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
        opts.waitForDebugger = raw["wait-for-dbg"].as<bool>();
        while (opts.waitForDebugger && !stopInDebugger()) {
            // spin
        }

        if (raw.count("allowed-extension") > 0) {
            // Any use of `--allowed-extension` overrides the default.
            opts.allowedExtensions.clear();

            const auto &exts = raw["allowed-extension"].as<vector<string>>();
            opts.allowedExtensions.insert(exts.begin(), exts.end());
        }

        if (raw.count("ignore") > 0) {
            const auto &rawIgnorePatterns = raw["ignore"].as<vector<string>>();
            parseIgnorePatterns(rawIgnorePatterns, opts.absoluteIgnorePatterns, opts.relativeIgnorePatterns);
        }

        {
            int maxInputFileThreads = raw["max-threads"].as<int>();
            auto workerPool = WorkerPool::create(maxInputFileThreads, *logger);

            opts.pathPrefix = raw["remove-path-prefix"].as<string>();
            if (raw.count("files") > 0) {
                const auto &rawFiles = raw["files"].as<vector<string>>();
                struct stat s;
                for (auto &file : rawFiles) {
                    if (stat(file.c_str(), &s) == 0 && s.st_mode & S_IFDIR) {
                        addFilesFromDir(opts, file, *workerPool, logger);
                    } else {
                        opts.rawInputFileNames.push_back(file);
                        opts.inputFileNames.push_back(file);
                    }
                }
            }

            if (raw.count("file") > 0) {
                const auto &files = raw["file"].as<vector<string>>();
                opts.rawInputFileNames.insert(opts.rawInputFileNames.end(), files.begin(), files.end());
                opts.inputFileNames.insert(opts.inputFileNames.end(), files.begin(), files.end());
            }

            if (raw.count("dir") > 0) {
                const auto &rawDirs = raw["dir"].as<vector<string>>();
                for (auto &dir : rawDirs) {
                    // Since we don't stat here, we're unsure if the directory exists / is a directory.
                    addFilesFromDir(opts, dir, *workerPool, logger);
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

        opts.cacheDir = raw["cache-dir"].as<string>();

        // Enable experimental support for RBS signatures
        opts.cacheSensitiveOptions.rbsEnabled = raw["enable-experimental-rbs-comments"].as<bool>();
        if (raw["enable-experimental-rbs-signatures"].as<bool>() ||
            raw["enable-experimental-rbs-assertions"].as<bool>()) {
            logger->warn(
                "Options `--enable-experimental-rbs-signatures` and `--enable-experimental-rbs-assertions` have "
                "been combined into the `--enable-experimental-rbs-comments` option. Please update your Sorbet config "
                "to use `--enable-experimental-rbs-comments` instead.");
            opts.cacheSensitiveOptions.rbsEnabled = true;
        }

        opts.cacheSensitiveOptions.requiresAncestorEnabled = raw["enable-experimental-requires-ancestor"].as<bool>();
        opts.cacheSensitiveOptions.rspecRewriterEnabled = raw["enable-experimental-rspec"].as<bool>();

        bool enableAllLSPFeatures = raw["enable-all-experimental-lsp-features"].as<bool>();
        opts.lspAllBetaFeaturesEnabled = enableAllLSPFeatures || raw["enable-all-beta-lsp-features"].as<bool>();
        opts.lspExtractToVariableEnabled =
            opts.lspAllBetaFeaturesEnabled || raw["enable-experimental-lsp-extract-to-variable"].as<bool>();
        opts.lspDocumentHighlightEnabled =
            enableAllLSPFeatures || raw["enable-experimental-lsp-document-highlight"].as<bool>();
        opts.lspSignatureHelpEnabled = enableAllLSPFeatures || raw["enable-experimental-lsp-signature-help"].as<bool>();
        opts.forciblySilenceLspMultipleDirError = raw["forcibly-silence-lsp-multiple-dir-error"].as<bool>();
        opts.rubyfmtPath = raw["rubyfmt-path"].as<string>();
        if (enableAllLSPFeatures || raw["enable-experimental-lsp-document-formatting-rubyfmt"].as<bool>()) {
            if (!FileOps::exists(opts.rubyfmtPath)) {
                logger->error("`{}` does not exist, LSP rubyfmt integration will not be enabled", opts.rubyfmtPath);
            } else {
                opts.lspDocumentFormatRubyfmtEnabled = true;
            }
        }
        opts.outOfOrderReferenceChecksEnabled = raw["check-out-of-order-constant-references"].as<bool>();
        if (raw.count("track-untyped") > 0) {
            opts.trackUntyped = text2TrackUntyped(raw["track-untyped"].as<string>(), *logger);
        }

        if (raw.count("lsp-directories-missing-from-client") > 0) {
            const auto &lspDirsMissingFromClient = raw["lsp-directories-missing-from-client"].as<vector<string>>();
            // Convert all of these dirs into absolute ignore patterns that begin with '/'.
            for (auto &dir : lspDirsMissingFromClient) {
                string pNormalized = dir;
                if (dir.at(0) != '/') {
                    pNormalized = '/' + dir;
                }
                opts.lspDirsMissingFromClient.push_back(pNormalized);
            }
        }

        opts.sorbetRoot = raw["sorbet-root"].as<string>();
        // Normalize: strip any trailing slashes
        while (!opts.sorbetRoot.empty() && opts.sorbetRoot.back() == '/') {
            opts.sorbetRoot.pop_back();
        }

        opts.lspErrorCap = raw["lsp-error-cap"].as<int>();

        opts.maxCacheSizeBytes = raw["max-cache-size-bytes"].as<size_t>();
        if (!extractPrinters(raw, opts, logger)) {
            throw EarlyReturnWithCode(1);
        }
        opts.stopAfterPhase = extractStopAfter(raw, logger);

        auto parser = extractParser(raw["parser"].as<string>(), logger).value_or(Parser::ORIGINAL);
        opts.cacheSensitiveOptions.usePrismParser = (parser == Parser::PRISM);
        opts.silenceErrors = raw["quiet"].as<bool>();
        opts.autocorrect = raw["autocorrect"].as<bool>();
        opts.didYouMean = raw["did-you-mean"].as<bool>();
        opts.inlineInput = raw["e"].as<string>();
        opts.inlineRBIInput = raw["e-rbi"].as<string>();
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
        opts.watchmanPauseStateName = raw["watchman-pause-state-name"].as<string>();
        if (!opts.watchmanPauseStateName.empty() && !opts.disableWatchman) {
            logger->error("watchman-pause-state-name must be used with watchman enabled");
            throw EarlyReturnWithCode(1);
        }
        opts.watchmanNamespace = raw["watchman-namespace"].as<string>();

        // Certain features only need certain passes
        auto isAutogen =
            opts.print.Autogen.enabled || opts.print.AutogenMsgPack.enabled || opts.print.AutogenSubclasses.enabled;
        if (isAutogen) {
            if (opts.stopAfterPhase != Phase::NAMER) {
                logger->error("-p autogen{-msgpack,-classlist,-subclasses} must also include --stop-after=namer");
                throw EarlyReturnWithCode(1);
            }

            opts.cacheSensitiveOptions.runningUnderAutogen = isAutogen;
        }

        if (raw.count("autogen-version") > 0) {
            if (!opts.print.AutogenMsgPack.enabled) {
                logger->error("`{}` must also include `{}`", "--autogen-version", "-p autogen-msgpack");
                throw EarlyReturnWithCode(1);
            }
            opts.autogenVersion = raw["autogen-version"].as<int>();
        }

        if (raw.count("autogen-subclasses-parent")) {
            if (!opts.print.AutogenSubclasses.enabled) {
                logger->error("autogen-subclasses-parent must be used with -p autogen-subclasses");
                throw EarlyReturnWithCode(1);
            }
            for (const string &parentClassName : raw["autogen-subclasses-parent"].as<vector<string>>()) {
                opts.autogenSubclassesParents.emplace_back(parentClassName);
            }
        }

        if (raw.count("autogen-subclasses-ignore") > 0) {
            const auto &rawIgnorePatterns = raw["autogen-subclasses-ignore"].as<vector<string>>();
            for (auto &p : rawIgnorePatterns) {
                string_view pNormalized = stripTrailingSlashes(p);
                if (p.at(0) == '/') {
                    opts.autogenSubclassesAbsoluteIgnorePatterns.emplace_back(pNormalized);
                } else {
                    opts.autogenSubclassesRelativeIgnorePatterns.emplace_back(fmt::format("/{}", pNormalized));
                }
            }
        }

        if (raw.count("autogen-behavior-allowed-in-rbi-files-paths") > 0) {
            if (!opts.cacheSensitiveOptions.runningUnderAutogen) {
                logger->error("autogen-behavior-allowed-in-rbi-files-paths can only be used with -p autogen or -p "
                              "autogen-msgpack");
                throw EarlyReturnWithCode(1);
            }
            opts.autogenBehaviorAllowedInRBIFilesPaths =
                raw["autogen-behavior-allowed-in-rbi-files-paths"].as<vector<string>>();
        }

        opts.autogenMsgpackSkipReferenceMetadata = raw["autogen-msgpack-skip-reference-metadata"].as<bool>();
        if (opts.autogenMsgpackSkipReferenceMetadata) {
            if (!opts.print.AutogenMsgPack.enabled) {
                logger->error("autogen-skip-reference-metadata can only be used with -p autogen-msgpack");
                throw EarlyReturnWithCode(1);
            }

            if (opts.autogenVersion < 6) {
                logger->error(
                    "autogen-skip-reference-metadata can only be used with autogen msgpack version 6 or above");
                throw EarlyReturnWithCode(1);
            }
        }

        if (opts.print.UntypedBlame.enabled && opts.trackUntyped == core::TrackUntyped::Nowhere) {
            logger->error("-p untyped-blame:<output-path> must also include --track-untyped");
            throw EarlyReturnWithCode(1);
        }

        opts.noErrorCount = raw["no-error-count"].as<bool>();

        opts.cacheSensitiveOptions.noStdlib = raw["no-stdlib"].as<bool>();

        opts.minimizeRBI = raw["minimize-to-rbi"].as<string>();
        if (!opts.minimizeRBI.empty() && !opts.print.MinimizeRBI.enabled) {
            logger->error("--minimize-to-rbi must also include --print=minimized-rbi");
            throw EarlyReturnWithCode(1);
        }
        if (!opts.minimizeRBI.empty() && opts.autocorrect) {
            logger->error("--minimize-to-rbi plus --autocorrect is not implemented");
            throw EarlyReturnWithCode(1);
        }
        opts.stdoutHUPHack = raw["stdout-hup-hack"].as<bool>();
        auto storeStateRaw = raw["store-state"].as<string>();
        if (!storeStateRaw.empty()) {
            opts.storeState = absl::StrSplit(storeStateRaw, ',');
            if (!opts.storeState.empty() && opts.storeState.size() != 3) {
                logger->error("--store-state must be given three paths, separated by commas");
                throw EarlyReturnWithCode(1);
            }
        }

        opts.forceHashing = raw["force-hashing"].as<bool>();

        opts.threads = (opts.runLSP || !opts.storeState.empty())
                           ? raw["max-threads"].as<int>()
                           : min(raw["max-threads"].as<int>(), int(opts.inputFileNames.size() / 2));

        if (raw.count("h") > 0) {
            auto helpSections = raw["h"].as<vector<string>>();
            if (helpSections.size() == 1 && helpSections[0] == "all") {
                helpSections = groupSections;
            }
            logger->info("{}", options.help(helpSections));
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
        opts.suggestTyped = raw["suggest-typed"].as<bool>();
        if (raw.count("suggest-unsafe") > 0) {
            opts.suggestUnsafe = raw["suggest-unsafe"].as<string>();
        }
        opts.traceLexer = raw["trace-lexer"].as<bool>();
        opts.traceParser = raw["trace-parser"].as<bool>();
        opts.stressIncrementalResolver = raw["stress-incremental-resolver"].as<bool>();
        if (raw.count("sleep-in-slow-path") > 0) {
            opts.sleepInSlowPathSeconds = raw["sleep-in-slow-path"].as<int>();
        }
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
        opts.webTraceFileStrict = raw["web-trace-file-strict"].as<bool>();
        {
            // parse extra sfx/datadog tags
            auto stringToParse = raw["metrics-extra-tags"].as<string>();
            if (stringToParse != "") {
                for (absl::string_view sp : absl::StrSplit(stringToParse, ',')) {
                    opts.metricsExtraTags.insert(absl::StrSplit(sp, absl::MaxSplits('=', 1)));
                }
            }
        }
        opts.reserveUtf8NameTableCapacity = raw["reserve-utf8-name-table-capacity"].as<uint32_t>();
        opts.reserveConstantNameTableCapacity = raw["reserve-constant-name-table-capacity"].as<uint32_t>();
        opts.reserveUniqueNameTableCapacity = raw["reserve-unique-name-table-capacity"].as<uint32_t>();
        opts.reserveClassTableCapacity = raw["reserve-class-table-capacity"].as<uint32_t>();
        opts.reserveMethodTableCapacity = raw["reserve-method-table-capacity"].as<uint32_t>();
        opts.reserveFieldTableCapacity = raw["reserve-field-table-capacity"].as<uint32_t>();
        opts.reserveTypeParameterTableCapacity = raw["reserve-type-argument-table-capacity"].as<uint32_t>();
        opts.reserveTypeMemberTableCapacity = raw["reserve-type-member-table-capacity"].as<uint32_t>();
        opts.uniquelyDefinedBehavior = raw["uniquely-defined-behavior"].as<bool>();
        opts.cacheSensitiveOptions.sorbetPackages =
            raw["sorbet-packages"].as<bool>() || raw["stripe-packages"].as<bool>();

        opts.packageAttributedErrors = raw["package-attributed-errors"].as<bool>();
        if (opts.packageAttributedErrors && !opts.cacheSensitiveOptions.sorbetPackages) {
            logger->error("--package-attributed-errors can only be specified in --sorbet-packages mode");
            throw EarlyReturnWithCode(1);
        }
        opts.testPackages = raw["experimental-test-packages"].as<bool>();
        if (opts.testPackages && !opts.cacheSensitiveOptions.sorbetPackages) {
            logger->error("--experimental-test-packages can only be specified in --sorbet-packages mode");
            throw EarlyReturnWithCode(1);
        }

        opts.packageDirected = raw["experimental-package-directed"].as<bool>();
        if (opts.packageDirected && !opts.cacheSensitiveOptions.sorbetPackages) {
            logger->error("--experimental-package-directed can only be specified in --sorbet-packages mode");
            throw EarlyReturnWithCode(1);
        }

        if (raw.count("extra-package-files-directory-prefix-underscore")) {
            for (const string &dirName : raw["extra-package-files-directory-prefix-underscore"].as<vector<string>>()) {
                if (dirName.back() != '/') {
                    logger->error("--extra-package-files-directory-prefix-underscore directory path must have slash "
                                  "(/) at the end");
                    throw EarlyReturnWithCode(1);
                }
                opts.extraPackageFilesDirectoryUnderscorePrefixes.emplace_back(dirName);
            }
        }

        if (raw.count("extra-package-files-directory-prefix-slash-deprecated")) {
            for (const string &dirName :
                 raw["extra-package-files-directory-prefix-slash-deprecated"].as<vector<string>>()) {
                if (dirName.back() != '/') {
                    logger->error(
                        "--extra-package-files-directory-prefix-slash-deprecated directory path must have slash "
                        "(/) at the end");
                    throw EarlyReturnWithCode(1);
                }
                opts.extraPackageFilesDirectorySlashDeprecatedPrefixes.emplace_back(dirName);
            }
        }

        if (raw.count("extra-package-files-directory-prefix-slash")) {
            for (const string &dirName : raw["extra-package-files-directory-prefix-slash"].as<vector<string>>()) {
                if (dirName.back() != '/') {
                    logger->error("--extra-package-files-directory-prefix-slash directory path must have slash "
                                  "(/) at the end");
                    throw EarlyReturnWithCode(1);
                }
                opts.extraPackageFilesDirectorySlashPrefixes.emplace_back(dirName);
            }
        }

        opts.genPackages = raw["gen-packages"].as<bool>();
        if (opts.genPackages && !opts.cacheSensitiveOptions.sorbetPackages) {
            logger->error("--gen-packages can only be can only be used in --sorbet-packages mode");
            throw EarlyReturnWithCode(1);
        }
        if (opts.genPackages && opts.runLSP) {
            logger->error("--gen-packages can not be used when --lsp is also enabled");
            throw EarlyReturnWithCode(1);
        }
        if (raw.count("allow-relaxed-packager-checks-for")) {
            if (!opts.cacheSensitiveOptions.sorbetPackages) {
                logger->error("--allow-relaxed-packager-checks-for can only be specified in --sorbet-packages mode");
                throw EarlyReturnWithCode(1);
            }
            // TODO(neil): should this also be "[A-Z][a-zA-Z0-9:]*" similar to update-visiblity-for?
            std::regex nsValid("[A-Z][a-zA-Z0-9:]+");
            for (const string &ns : raw["allow-relaxed-packager-checks-for"].as<vector<string>>()) {
                if (!std::regex_match(ns, nsValid)) {
                    logger->error("--allow-relaxed-packager-checks-for must contain items that start with a capital "
                                  "letter and are alphanumeric.");
                    throw EarlyReturnWithCode(1);
                }
                opts.allowRelaxedPackagerChecksFor.emplace_back(ns);
            }
        }

        if (raw.count("gen-packages-update-visibility-for")) {
            if (!opts.cacheSensitiveOptions.sorbetPackages) {
                logger->error("--gen-packages-update-visibility-for can only be specified in --sorbet-packages mode");
                throw EarlyReturnWithCode(1);
            }
            if (!opts.genPackages) {
                logger->error("--gen-packages-update-visibility-for can only be specified in --gen-packages mode");
                throw EarlyReturnWithCode(1);
            }
            std::regex nsValid("[A-Z][a-zA-Z0-9:]*");
            for (const string &ns : raw["gen-packages-update-visibility-for"].as<vector<string>>()) {
                if (!std::regex_match(ns, nsValid)) {
                    logger->error("--gen-packages-update-visibility-for must contain items that start with a capital "
                                  "letter and are alphanumeric.");
                    throw EarlyReturnWithCode(1);
                }
                opts.updateVisibilityFor.emplace_back(ns);
            }
        }

        if (raw.count("packager-layers")) {
            if (opts.cacheSensitiveOptions.sorbetPackages) {
                // TODO(neil): This regex was picked on a whim, so open to changing to be more or less restrictive based
                // on feedback/usecases.
                std::regex layerValid("[a-zA-Z0-9]+");
                for (const string &layer : raw["packager-layers"].as<vector<string>>()) {
                    if (!std::regex_match(layer, layerValid)) {
                        logger->error("--packager-layers must contain items that are alphanumeric.");
                        throw EarlyReturnWithCode(1);
                    }
                    opts.packagerLayers.emplace_back(layer);
                }
            } else {
                logger->error("--packager-layers can only be specified in --sorbet-packages mode");
                throw EarlyReturnWithCode(1);
            }
        }

        opts.sorbetPackagesHint = raw["sorbet-packages-hint-message"].as<string>();
        if (opts.sorbetPackagesHint.empty()) {
            opts.sorbetPackagesHint = raw["stripe-packages-hint-message"].as<string>();
        }
        if (!opts.sorbetPackagesHint.empty() && !opts.cacheSensitiveOptions.sorbetPackages) {
            if (!opts.cacheSensitiveOptions.sorbetPackages) {
                logger->error("--sorbet-packages-hint-message can only be specified in --sorbet-packages mode");
                throw EarlyReturnWithCode(1);
            }
        }

        if (raw.count("package-skip-rbi-export-enforcement")) {
            if (!opts.cacheSensitiveOptions.sorbetPackages) {
                logger->error("--package-skip-rbi-export-enforcement can only be specified in --sorbet-packages mode");
                throw EarlyReturnWithCode(1);
            }
            for (const string &ns : raw["package-skip-rbi-export-enforcement"].as<vector<string>>()) {
                opts.packageSkipRBIExportEnforcementDirs.emplace_back(ns);
            }
        }

        opts.errorUrlBase = raw["error-url-base"].as<string>();
        opts.noErrorSections = raw["no-error-sections"].as<bool>();
        opts.ruby3KeywordArgs = raw["experimental-ruby3-keyword-args"].as<bool>();
        opts.cacheSensitiveOptions.typedSuper = raw["typed-super"].as<bool>();

        if (raw.count("suppress-payload-superclass-redefinition-for") > 0) {
            for (const auto &childClassName :
                 raw["suppress-payload-superclass-redefinition-for"].as<vector<string>>()) {
                opts.suppressPayloadSuperclassRedefinitionFor.emplace_back(childClassName);
            }
        }

        if (raw.count("isolate-error-code") > 0) {
            const auto &rawList = raw["isolate-error-code"].as<vector<int>>();
            opts.isolateErrorCode.insert(rawList.begin(), rawList.end());
        }

        if (raw.count("suppress-error-code") > 0) {
            const auto &rawList = raw["suppress-error-code"].as<vector<int>>();
            opts.suppressErrorCode.insert(rawList.begin(), rawList.end());
        }

        if (!opts.isolateErrorCode.empty() && !opts.suppressErrorCode.empty()) {
            logger->error("You can't pass both `{}` and `{}`", "--isolate-error-code", "--suppress-error-code");
            throw EarlyReturnWithCode(1);
        }

        if (raw.count("e") == 0 && opts.inputFileNames.empty() && !raw["version"].as<bool>() && !opts.runLSP &&
            opts.storeState.empty() && !opts.print.PayloadSources.enabled) {
            logger->error("You must pass either `{}` or at least one folder or ruby file.\n\n{}", "-e",
                          options.help({groupToString(Group::INPUT)}));
            throw EarlyReturnWithCode(1);
        }

        if (opts.print.PayloadSources.enabled) {
            if (opts.cacheSensitiveOptions.noStdlib) {
                logger->error("You can't pass both `{}` and `{}`.", "--print=payload-sources", "--no-stdlib");
                throw EarlyReturnWithCode(1);
            } else if (raw.count("e") > 0) {
                logger->error("You can't pass both `{}` and `{}`.", "--print=payload-sources", "-e");
                throw EarlyReturnWithCode(1);
            } else if (!opts.inputFileNames.empty()) {
                logger->error("You can't pass both `{}` and paths to typecheck.", "--print=payload-sources");
                throw EarlyReturnWithCode(1);
            }
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

        opts.suppressNonCriticalErrors = raw["suppress-non-critical"].as<bool>();
        if (!raw["typed-override"].as<string>().empty()) {
            opts.strictnessOverrides = extractStrictnessOverrides(raw["typed-override"].as<string>(), logger);
        }

        for (auto &provider : semanticExtensionProviders) {
            auto maybeExtension = provider->readOptions(raw);
            if (maybeExtension) {
                configuredExtensions.emplace_back(move(maybeExtension));
            }
        }

        if (opts.print.RBSRewriteTree.enabled && !opts.cacheSensitiveOptions.rbsEnabled) {
            logger->error("--print=rbs-rewrite-tree must also include `{}`", "--enable-experimental-rbs-comments");
            throw EarlyReturnWithCode(1);
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
// vim:fdm=marker
