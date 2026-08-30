#ifndef RUBY_TYPER_REAL_MAIN_H
#define RUBY_TYPER_REAL_MAIN_H
#include "spdlog/spdlog.h"

namespace sorbet::realmain {
int realmain(int argc, char *argv[]);

extern std::shared_ptr<spdlog::logger> logger;

// Called once, before any significant allocation, when realmain knows it is running a batch typecheck rather than the
// language server. The `sorbet` binary installs a function here that tunes its allocator for a short-lived process
// (see main/main.cc); other programs that link realmain leave it null.
extern void (*batchAllocatorHook)();
} // namespace sorbet::realmain
#endif // RUBY_TYPER_REAL_MAIN_H
