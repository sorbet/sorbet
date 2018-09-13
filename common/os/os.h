#ifndef SORBET_OS_H
#define SORBET_OS_H
#include <functional>
#include <memory>
#include <pthread.h>

std::string addr2line(const std::string program_name, void const *const *addr, int count);

std::string getProgramName();

class Joinable {
    friend std::unique_ptr<Joinable> runInAThread(std::function<void()> function);
    pthread_t handle;
    pthread_attr_t attr;
    std::function<void()> realFunction;

    static void *trampoline(void *);

public:
    ~Joinable() {
        void *status;
        pthread_join(handle, &status);
        pthread_attr_destroy(&attr);
    }

    Joinable() = default;
    Joinable(const Joinable &) = delete;
    Joinable(Joinable &&) = delete;
};

// run function in a thread. Return thread handle that you can join on
std::unique_ptr<Joinable> runInAThread(std::function<void()> function);

/** The should trigger debugger breakpoint if the debugger is attached, if no debugger is attach, it should do nothing
 *  This allows to:
 *   - have "persistent" break points in development loop, that survive line changes.
 *   - test the same executable outside of debugger without rebuilding.
 * */
bool stopInDebugger();
bool amIBeingDebugged();

#endif // SORBET_OS_H
