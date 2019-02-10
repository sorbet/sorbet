#include "libfuzzer/libfuzzer_macro.h"
#include "test/fuzz/TextDocumentPositionParamsWithoutTextDocumentIdentifier.pb.h"
#include <cxxopts.hpp>
// ^^^ should go first as they violate our poisons
#include "common/common.h"

using namespace std;

static bool disableFastPath = false;
static string fileName;

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv) {
    cxxopts::Options options("fuzz_hover", "Fuzz all potential LSP hovers given a file");
    options.allow_unrecognised_options().add_options()("single_test", "run over single test.",
                                                       cxxopts::value<std::string>()->default_value(""), "testpath");
    options.add_options()("lsp-disable-fastpath", "disable fastpath in lsp tests");
    auto res = options.parse(*argc, *argv);

    if (res.count("single_test") != 1) {
        printf("--single_test=<filename> argument expected\n");
        return 1;
    }

    fileName = res["single_test"].as<std::string>();
    disableFastPath = res["lsp-disable-fastpath"].as<bool>();
    return 0;
}

DEFINE_PROTO_FUZZER(
    const com::stripe::rubytyper::fuzz::TextDocumentPositionParamsWithoutTextDocumentIdentifier &input) {
    ENFORCE(input.line() != 42);
}
