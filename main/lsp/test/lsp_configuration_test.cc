#include "doctest/doctest.h"
// has to go first as it violates our requirements

#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include "spdlog/sinks/null_sink.h"
#include "test/helpers/MockFileSystem.h"
#include <memory>

using namespace std;

namespace sorbet::realmain::lsp::test {

namespace {

auto nullSink = make_shared<spdlog::sinks::null_sink_mt>();
auto logger = make_shared<spdlog::logger>("console", nullSink);

constexpr string_view rootUri = "file:///workspace"sv;

options::Options makeMultiDirOptions() {
    options::Options opts;
    opts.rawInputDirNames.emplace_back(".");
    opts.rawInputDirNames.emplace_back("nested");
    opts.rawInputFileNames.emplace_back("also/included.rbi");
    opts.forciblySilenceLspMultipleDirError = true;
    opts.runLSP = true;
    opts.fs = make_shared<sorbet::test::MockFileSystem>(".");
    return opts;
}

options::Options makeSingleDirOptions(string_view rootPath) {
    options::Options opts;
    opts.rawInputDirNames.emplace_back(string(rootPath));
    opts.runLSP = true;
    opts.fs = make_shared<sorbet::test::MockFileSystem>(rootPath);
    return opts;
}

shared_ptr<LSPConfiguration> makeConfig(const options::Options &opts) {
    auto config = make_shared<LSPConfiguration>(opts, make_shared<LSPOutputToVector>(), logger, false);
    InitializeParams initParams(string(rootUri), make_unique<ClientCapabilities>());
    config->setClientConfig(make_shared<LSPClientConfiguration>(initParams));
    config->markInitialized();
    return config;
}

} // namespace

TEST_CASE("URIConversionWithMultipleInputDirs") {
    auto opts = makeMultiDirOptions();
    auto config = makeConfig(opts);

    // Files under the first input dir (".") are indexed with a "./" prefix.
    CHECK_EQ("file:///workspace/foo.rb", config->localName2Remote("./foo.rb"));
    CHECK_EQ("./foo.rb", config->remoteName2Local("file:///workspace/foo.rb"));

    // Files under the other input dirs are indexed relative to the working directory; the leading
    // path component must not be stripped (https://github.com/sorbet/sorbet/issues/10443).
    CHECK_EQ("file:///workspace/nested/bar.rb", config->localName2Remote("nested/bar.rb"));
    CHECK_EQ("nested/bar.rb", config->remoteName2Local("file:///workspace/nested/bar.rb"));

    // Files passed via --file behave like files from the other input dirs.
    CHECK_EQ("file:///workspace/also/included.rbi", config->localName2Remote("also/included.rbi"));
    CHECK_EQ("also/included.rbi", config->remoteName2Local("file:///workspace/also/included.rbi"));

    // "." must only match as a whole path component, not dotfiles.
    CHECK_EQ("file:///workspace/.rubocop.yml", config->localName2Remote("./.rubocop.yml"));
}

TEST_CASE("URIConversionWithSingleInputDir") {
    auto opts = makeSingleDirOptions("myroot");
    auto config = makeConfig(opts);

    CHECK_EQ("file:///workspace/foo.rb", config->localName2Remote("myroot/foo.rb"));
    CHECK_EQ("myroot/foo.rb", config->remoteName2Local("file:///workspace/foo.rb"));
}

} // namespace sorbet::realmain::lsp::test
