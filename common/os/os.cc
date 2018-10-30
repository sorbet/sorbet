#include "os.h"
#include "common/common.h"
#include <climits>
#include <pthread.h>
#if __has_feature(address_sanitizer)
#include <sanitizer/lsan_interface.h>
#endif

using namespace std;

// When changing this, you should also update the -stack-size flag in main/BUILD.
// 64 Megabytes
constexpr int REQUIRED_STACK_SIZE = 64 * 1024 * 1024;

void *Joinable::trampoline(void *ptr) {
    ENFORCE(ptr != nullptr);
    Joinable &self = *static_cast<Joinable *>(ptr);
    setCurrentThreadName(self.originalThreadName);
    self.realFunction();
    return ptr;
}

unique_ptr<Joinable> runInAThread(std::string_view threadName, function<void()> function) {
#ifdef EMSCRIPTEN
    sorbet::Error::raise("Creating threads in unsupported in EMSCRIPTEN");
#endif
    // AFAIK this should all be:
    //    - defined behaviour
    //    - available on all posix systems

    unique_ptr<Joinable> res = make_unique<Joinable>();
    res->realFunction = move(function);
    res->originalThreadName = string(threadName);

    Joinable *joinablePTR = res.get();

    pthread_t &thread = res->handle;
    int err = 0;
    pthread_attr_t &attr = res->attr;
    size_t stackSize = 0;

    /*  Initialize the attribute */
    err = pthread_attr_init(&attr);
    if (err != 0) {
        sorbet::Exception::raise("Failed to set create pthread_attr_t");
    };

    /* Get the default value */
    err = pthread_attr_getstacksize(&attr, &stackSize);
    if (err != 0) {
        sorbet::Exception::raise("Failed to get stack size");
    }

    // This should be the default, but just to play it safe.

    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (err != 0) {
        sorbet::Exception::raise("Failed make thread joinable");
    }

    // Mac default is too low
    if (stackSize < REQUIRED_STACK_SIZE) {
        err = pthread_attr_setstacksize(&attr, REQUIRED_STACK_SIZE);
        if (err != 0) {
            sorbet::Exception::raise("Failed to set stack size");
        };
    }

    /*  Create the thread with our attribute */
    err = pthread_create(&thread, &attr, &Joinable::trampoline, joinablePTR);
    if (err != 0) {
        sorbet::Exception::raise("Failed create thread");
    }
    return res;
}

void intentionallyLeakMemory(void *ptr) {
#if __has_feature(address_sanitizer)
    __lsan_ignore_object(ptr);
#endif
}
