#include "os.h"
#include "common/common.h"
#include <climits>
#include <pthread.h>

// 16 Megabytes
constexpr int REQUIRED_STACK_SIZE = 16 * 1024 * 1024;

void *Joinable::trampoline(void *ptr) {
    static_cast<Joinable *>(ptr)->realFunction();
    return ptr;
}

std::unique_ptr<Joinable> runInAThread(std::function<void()> function) {
    // AFAIK this should all be:
    //    - defined behaviour
    //    - available on all posix systems

    std::unique_ptr<Joinable> res = std::make_unique<Joinable>();
    res->realFunction = move(function);

    Joinable *joinablePTR = res.get();

    pthread_t &thread = res->handle;
    int err = 0;
    pthread_attr_t &attr = res->attr;
    size_t stackSize = 0;

    /*  Initialize the attribute */
    err = pthread_attr_init(&attr);
    if (err != 0) {
        ruby_typer::Error::raise("Failed to set create pthread_attr_t");
    };

    /* Get the default value */
    err = pthread_attr_getstacksize(&attr, &stackSize);
    if (err != 0) {
        ruby_typer::Error::raise("Failed to get stack size");
    }

    // This should be the default, but just to play it safe.

    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (err != 0) {
        ruby_typer::Error::raise("Failed make thread joinable");
    }

    // Mac default is too low
    if (stackSize < REQUIRED_STACK_SIZE) {
        err = pthread_attr_setstacksize(&attr, REQUIRED_STACK_SIZE);
        if (err != 0) {
            ruby_typer::Error::raise("Failed to set stack size");
        };
    }

    /*  Create the thread with our attribute */
    err = pthread_create(&thread, &attr, &Joinable::trampoline, joinablePTR);
    if (err != 0) {
        ruby_typer::Error::raise("Failed create thread");
    }
    return res;
}
