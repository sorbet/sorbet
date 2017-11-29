#ifdef __APPLE__
#include "common/common.h"
#include <assert.h>
#include <cstdio>
#include <mach-o/dyld.h> /* _NSGetExecutablePath */
#include <stdbool.h>
#include <string>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

string exec(string cmd);

string addr2line(const string program_name, void const *const *addr, int count) {
    auto addr2line_cmd = strprintf("atos -o %.256s -p %d", program_name.c_str(), (int)getpid());
    for (int i = 3; i < count; ++i) {
        addr2line_cmd = strprintf("%s %p", addr2line_cmd.c_str(), addr[i]);
    }

    //    printf("%s\n", addr2line_cmd.c_str());

    return exec(addr2line_cmd);
}
string getProgramName() {
    char buf[512];
    uint32_t sz = 512;
    _NSGetExecutablePath(buf, &sz);
    string res(buf);
    return res;
}

/** taken from https://developer.apple.com/library/content/qa/qa1361/_index.html */
static bool amIBeingDebugged()
// Returns true if the current process is being debugged (either
// running under the debugger or has a debugger attached post facto).
{
    int junk;
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
    junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
    assert(junk == 0);

    // We're being debugged if the P_TRACED flag is set.

    return ((info.kp_proc.p_flag & P_TRACED) != 0);
}

bool stopInDebugger() {
    if (amIBeingDebugged()) {
        __asm__("int $3");
        return true;
    }
    return false;
}

#endif
