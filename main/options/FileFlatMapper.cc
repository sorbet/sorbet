#include "FileFlatMapper.h"
#include "absl/strings/str_split.h"
#include "common/common.h"
#include "options.h"

using namespace std;
namespace sorbet::realmain::options {

FileFlatMapper::FileFlatMapper(int &argc, char **&argv, shared_ptr<spdlog::logger> logger)
    : origArgc(argc), origArgv(argv), argc(argc), argv(argv) {
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '@') {
            try {
                string argsP = FileOps::read(argv[i] + 1);
                string_view argsPView = argsP;
                if (!argsPView.empty() && argsPView.back() == '\n') {
                    argsPView = argsPView.substr(0, argsPView.size() - 1);
                }
                for (string_view arg : absl::StrSplit(argsPView, '\n')) {
                    stringArgs.emplace_back((string)arg);
                }
            } catch (FileNotFoundException e) {
                logger->error("File Not Found: {}", argv[i]);
                throw EarlyReturnWithCode(11);
            }
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
