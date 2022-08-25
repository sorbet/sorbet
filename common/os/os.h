#ifndef SORBET_OS_H
#define SORBET_OS_H
#include <functional>
#include <memory>
#include <optional>
#include <pthread.h>
#include <string>

std::string addr2line(std::string_view programName, void const *const *addr, int count);

std::string getProgramName();

class Joinable {
    friend std::unique_ptr<Joinable> runInAThread(std::string_view threadName, std::function<void()> function,
                                                  std::optional<int> bindToCore);
    pthread_t handle;
    pthread_attr_t attr;
    std::function<void()> realFunction;
    std::string originalThreadName;

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

    void setPriorityLevel(int priority);
};

// run function in a thread. Return thread handle that you can join on
std::unique_ptr<Joinable> runInAThread(std::string_view threadName, std::function<void()> function,
                                       std::optional<int> bindToCore = std::nullopt);
bool setCurrentThreadName(std::string_view name);
bool bindThreadToCore(pthread_t handle, int coreId);

/** The should trigger debugger breakpoint if the debugger is attached, if no debugger is attach, it should do nothing
 *  This allows to:
 *   - have "persistent" break points in development loop, that survive line changes.
 *   - test the same executable outside of debugger without rebuilding.
 * */
bool stopInDebugger();
bool amIBeingDebugged();

void intentionallyLeakMemory(void *ptr);

void initializeSymbolizer(char *argv0);
#endif // SORBET_OS_H
