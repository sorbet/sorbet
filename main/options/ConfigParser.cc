#include "ConfigParser.h"
#include "absl/algorithm/container.h"
#include "absl/strings/str_split.h"
#include "common/FileOps.h"
#include "common/common.h"
#include "options.h"
#include <cctype> // for isspace

using namespace std;
namespace sorbet::realmain::options {

namespace {

// Returns true when the line starts with a '#', ignoring leading space.
//
// NOTE: we don't handle trailing comments because it would be difficult to handle the following case without a more
// complicated parser:
//
// > -e 'puts "hello" # this is a comment' -p parse-tree
bool isComment(string_view line) {
    auto hashPos = line.find('#');
    if (hashPos == string_view::npos) {
        return false;
    }

    auto prefix = line.substr(0, hashPos);
    return absl::c_all_of(prefix, [](auto c) { return isspace(c); });
}

} // namespace

void ConfigParser::readArgsFromFile(std::shared_ptr<spdlog::logger> logger, string_view filename,
                                    std::vector<std::string> &stringArgs) {
    try {
        string argsP = FileOps::read(string(filename));
        string_view argsPView = argsP;
        if (!argsPView.empty() && argsPView.back() == '\n') {
            argsPView = argsPView.substr(0, argsPView.size() - 1);
        }
        for (string_view arg : absl::StrSplit(argsPView, '\n')) {
            if (arg.size() == 0 || isComment(arg)) {
                // skip empty lines and comments
                continue;
            } else if (arg[0] == '@') {
                readArgsFromFile(logger, arg.substr(min(arg.find_first_not_of("@"), arg.size())), stringArgs);
            } else {
                stringArgs.emplace_back(string(arg));
            }
        }
    } catch (FileNotFoundException e) {
        logger->error("File Not Found: {}", filename);
        throw EarlyReturnWithCode(11);
    }
}

cxxopts::ParseResult ConfigParser::parseConfig(std::shared_ptr<spdlog::logger> logger, int &argc, char **&argv,
                                               cxxopts::Options options) {
    // Pointers into those args will be passed in argv
    std::vector<std::string> stringArgs;

    if (argc > 0) {
        // $0 / $PROGRAM_NAME should always be first
        stringArgs.emplace_back(argv[0]);
    }

    auto tmpArgc = argc;
    auto tmpArgv = argv;
    auto tmpOpts = options.parse(tmpArgc, tmpArgv);
    auto noConfigCount = tmpOpts.count("no-config");

    // Look for `sorbet/config` before all other args, only if `--no-config` was not specified
    if (noConfigCount == 0) {
        auto configFilename = "sorbet/config";
        if (FileOps::exists(configFilename)) {
            readArgsFromFile(logger, configFilename, stringArgs);
        }
    }

    // Override `sorbet/config` options by CLI and config files ones
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '@') {
            readArgsFromFile(logger, argv[i] + 1, stringArgs);
        } else {
            stringArgs.emplace_back(argv[i]);
        }
    }

    // Recompose the `argv` array from what we parsed previously
    std::vector<char *> args;
    args.reserve(stringArgs.size());
    for (auto &arg : stringArgs) {
        args.emplace_back(const_cast<char *>(arg.c_str()));
    }
    argc = args.size();
    argv = args.data();

    // Parse actual options
    auto opts = options.parse(argc, argv);
    if (opts.count("no-config") > noConfigCount) {
        logger->error("Option `--no-config` cannot be used inside the config file.");
        throw EarlyReturnWithCode(1);
    }

    return opts;
}
} // namespace sorbet::realmain::options
