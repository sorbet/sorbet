#include "compiler/Linker/Linker.h"
#include "common/Subprocess.h"
#include "common/Timer.h"

using namespace std;
namespace sorbet::compiler {
enum class OS { Lin, Darwin };
bool Linker::run(spdlog::logger &log, vector<string> objectFiles, string outputFile) {
    Timer timer("linking");
    OS os;
#if defined(__linux__)
    os = OS::Lin;
#elif defined(__APPLE__)
    os = OS::Darwin;
#else
    Exception::notImplemented();
#endif

    // this should probably be changed to direct dependency on lld source and fork+call to main of it.
    // https://github.com/llvm-mirror/lld/blob/64b024a57c56c3528d6be3d14be5e3da42614a6f/tools/lld/lld.cpp#L147 is what
    // to cargocult
    string executable = "ld";
    vector<string> commandLineArgs;
    switch (os) {
        case OS::Darwin:
            commandLineArgs = {
                "-bundle", "-o", outputFile + ".bundle", "-undefined", "dynamic_lookup", "-macosx_version_min",
                "10.14"};
            break;
        case OS::Lin:
            commandLineArgs = {"-shared", "-o", outputFile + ".so", "--allow-shlib-undefined"};
            break;
    }
    for (auto &of : objectFiles) {
        commandLineArgs.emplace_back(move(of));
    }
    auto ldOutput = Subprocess::spawn(executable, commandLineArgs);
    return ldOutput != nullopt;
};

} // namespace sorbet::compiler
