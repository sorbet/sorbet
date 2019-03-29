#include "FileFlatMapper.h"
#include "absl/strings/str_split.h"
#include "common/common.h"
#include "options.h"

using namespace std;
namespace sorbet::realmain::options {

void FileFlatMapper::readArgsFromFile(shared_ptr<spdlog::logger> logger, string_view filename) {
    try {
        string argsP = FileOps::read(filename);
        string_view argsPView = argsP;
        if (!argsPView.empty() && argsPView.back() == '\n') {
            argsPView = argsPView.substr(0, argsPView.size() - 1);
        }
        for (string_view arg : absl::StrSplit(argsPView, '\n')) {
            if (arg.size() == 0) {
                // skip empty line
                continue;
            } else if (arg[0] == '@') {
                readArgsFromFile(logger, arg.substr(min(arg.find_first_not_of("@"), arg.size())));
            } else {
                stringArgs.emplace_back(string(arg));
            }
        }
    } catch (FileNotFoundException e) {
        logger->error("File Not Found: {}", filename);
        throw EarlyReturnWithCode(11);
    }
}

FileFlatMapper::FileFlatMapper(int &argc, char **&argv, shared_ptr<spdlog::logger> logger)
    : origArgc(argc), origArgv(argv), argc(argc), argv(argv) {
    if (argc > 0) {
        // $0 / $PROGRAM_NAME should always be first
        stringArgs.emplace_back(argv[0]);
    }

    // Look for .sorbet/config before all other args, so that the CLI args can overwrite the config file
    auto configFilename = "sorbet/config";
    if (FileOps::exists(configFilename)) {
        // TODO(jez) Recurse upwards to find file in parent directory
        readArgsFromFile(logger, configFilename);
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '@') {
            readArgsFromFile(logger, argv[i] + 1);
        } else {
            stringArgs.emplace_back(argv[i]);
        }
    }
    args.reserve(stringArgs.size());
    for (auto &arg : stringArgs) {
        args.emplace_back(const_cast<char *>(arg.c_str()));
    }
    argc = args.size();
    argv = args.data();
}

FileFlatMapper::~FileFlatMapper() {
    argc = origArgc;
    argv = origArgv;
}
} // namespace sorbet::realmain::options
