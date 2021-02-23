#include "common/common.h"

#include <cxxopts.hpp>
#include <variant>

using namespace std;

namespace sorbet::sorbet_runtime::generate_call_validation {

struct Options {
    bool bindCall;
};

variant<Options, int> parseArgs(int argc, char **argv) {
    cxxopts::Options options("generate_call_validation");
    options.add_options()("bind-call", "Whether to use UnboundMethod#bind_call or not", cxxopts::value<bool>());
    options.add_options()("h,help", "Show this help");

    try {
        auto raw = options.parse(argc, argv);
        if (raw["help"].as<bool>()) {
            fmt::print(stderr, "{}", options.help());
            return 0;
        }

        if (raw["bind-call"].count() == 0) {
            fmt::print(stderr, "Missing required option: --bind-call\n\n{}", options.help());
            return 1;
        }

        return Options{
            raw["bind-call"].as<bool>(),
        };
    } catch (cxxopts::OptionParseException &e) {
        fmt::print(stderr, "{}\n\n{}", e.what(), options.help());
        return 1;
    }
}

int generateCallValidation(const Options &options) {
    fmt::print("# Hello, world! --bind-call={}\n", options.bindCall);
    return 0;
}

int realmain(int argc, char **argv) {
    auto options = parseArgs(argc, argv);
    if (holds_alternative<int>(options)) {
        return get<int>(options);
    }

    return generateCallValidation(get<Options>(options));
}

} // namespace sorbet::sorbet_runtime::generate_call_validation

int main(int argc, char **argv) {
    return sorbet::sorbet_runtime::generate_call_validation::realmain(argc, argv);
}
