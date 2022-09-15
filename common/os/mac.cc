#ifdef __APPLE__
#include "absl/debugging/symbolize.h"
#include "common/common.h"
#include <cassert>
#include <cstdio>
#include <mach-o/dyld.h> /* _NSGetExecutablePath */

#import <mach/thread_act.h>
#include <string>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

string exec(string cmd);

string addr2line(string_view programName, void const *const *addr, int count) {
    auto addr2lineCmd = fmt::format("atos -o {} -p {}", programName, (int)getpid());
    for (int i = 3; i < count; ++i) {
        addr2lineCmd = fmt::format("{} {}", addr2lineCmd, addr[i]);
    }

    //    printf("%s\n", addr2lineCmd.c_str());

    return exec(addr2lineCmd);
}
string getProgramName() {
    char buf[512];
    uint32_t sz = 512;
    _NSGetExecutablePath(buf, &sz);
    string res(buf);
    return res;
}

/** taken from https://developer.apple.com/library/content/qa/qa1361/_index.html */
bool amIBeingDebugged()
// Returns true if the current process is being debugged (either
// running under the debugger or has a debugger attached post facto).
{
    int junk __attribute__((unused));
    int mib[4];
    struct kinfo_proc info;
    size_t size;

    // Initialize the flags so that, if sysctl fails for some bizarre
    // reason, we get a predictable result.

    info.kp_proc.p_flag = 0;

    // Initialize mib, which tells sysctl the info we want, in this case
    // we're looking for information about a specific process ID.

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    // Call sysctl.

    size = sizeof(info);
    junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0);
    assert(junk == 0);

    // We're being debugged if the P_TRACED flag is set.

    return ((info.kp_proc.p_flag & P_TRACED) != 0);
}

bool stopInDebugger() {
    if (amIBeingDebugged()) {
        __builtin_debugtrap();
        return true;
    }
    return false;
}

bool setCurrentThreadName(string_view name) {
    const size_t maxLen = 16 - 1; // Pthreads limits it to 16 bytes including trailing '\0'
    auto truncatedName = string(name.substr(0, maxLen));
    auto retCode = ::pthread_setname_np(truncatedName.c_str());
    return retCode == 0;
}

bool bindThreadToCore(pthread_t handle, int coreId) {
    thread_affinity_policy_data_t policy = {coreId};
    thread_port_t mach_thread = pthread_mach_thread_np(handle);
    auto ret = thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);
    return ret == 0;
}

void initializeSymbolizer(char *argv0) {
    absl::InitializeSymbolizer(argv0);
}
#endif
