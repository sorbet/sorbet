#ifndef RUBY_TYPER_REAL_MAIN_H
#define RUBY_TYPER_REAL_MAIN_H
#include "common/Timer.h"
#include "main/options/options.h"
#include "spdlog/spdlog.h"

namespace sorbet::realmain {
int realmain(int argc, char *argv[]);

extern std::shared_ptr<spdlog::logger> logger;
} // namespace sorbet::realmain
#endif // RUBY_TYPER_REAL_MAIN_H
