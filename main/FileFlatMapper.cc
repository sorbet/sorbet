#include "main/FileFlatMapper.h"
#include "common/common.h"
#include "main/realmain.h"

using namespace std;
namespace ruby_typer {
namespace realmain {
vector<string> split(const string &s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

FileFlatMapper::FileFlatMapper(int &argc, const char **&argv) : origArgc(argc), origArgv(argv), argc(argc), argv(argv) {
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '@') {
            try {
                string argsP = FileOps::read(argv[i] + 1);
                for (string arg : split(argsP, '\n')) {
                    char *c_arg = (char *)malloc(arg.size() + 1);
                    memcpy(c_arg, arg.c_str(), arg.size() + 1);
                    args.push_back(c_arg);
                }
            } catch (FileNotFoundException e) {
                console_err->error("File Not Found: {}", argv[i]);
                returnCode = 11;
                continue;
            }
        } else {
            int length = strlen(argv[i]);
            char *c_arg = (char *)malloc(length + 1);
            memcpy(c_arg, argv[i], length);
            c_arg[length] = '\0';
            args.push_back(c_arg);
        }
    }
    argc = args.size();
    argv = args.data();
}

FileFlatMapper::~FileFlatMapper() {
    argc = origArgc;
    argv = origArgv;
    for (const char *c : args) {
        free((void *)c);
    }
}

} // namespace realmain
} // namespace ruby_typer
