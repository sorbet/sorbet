#ifndef RUBY_TYPER_REAL_MAIN_H
#define RUBY_TYPER_REAL_MAIN_H
#include "common/ProgressIndicator.h"
#include "common/Timer.h"
#include "main/options/options.h"
#include "spdlog/spdlog.h"

namespace sorbet {
namespace realmain {
int realmain(int argc, const char *argv[]);

extern std::shared_ptr<spdlog::logger> logger;
} // namespace realmain
} // namespace sorbet
#endif // RUBY_TYPER_REAL_MAIN_H
