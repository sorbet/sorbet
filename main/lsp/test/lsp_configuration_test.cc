#include "doctest/doctest.h"
// has to go first as it violates our requirements

#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include "spdlog/sinks/null_sink.h"

using namespace std;

namespace sorbet::realmain::lsp::test {

namespace {

options::Options makeOptions(string_view rootPath) {
    options::Options opts;
    opts.rawInputDirNames.emplace_back(string(rootPath));
    return opts;
}

shared_ptr<LSPConfiguration> makeConfig(const options::Options &opts, string_view rootUri) {
    auto nullSink = make_shared<spdlog::sinks::null_sink_mt>();
    auto logger = make_shared<spdlog::logger>("console", nullSink);
    auto config = make_shared<LSPConfiguration>(opts, make_shared<LSPOutputToVector>(), logger, false);
    InitializeParams initParams(string(rootUri), make_unique<ClientCapabilities>());
    config->setClientConfig(make_shared<LSPClientConfiguration>(initParams));
    config->markInitialized();
    return config;
}

} // namespace

TEST_CASE("LocalName2RemoteFileAtRoot") {
    // When filePath == rootPath, the relative path is empty. localName2Remote must not index
    // into the empty string_view while stripping a leading slash.
    auto opts = makeOptions("/foo/bar");
    auto config = makeConfig(opts, "file:///foo/bar");
    CHECK_EQ(config->localName2Remote("/foo/bar"), "file:///foo/bar/");
}

TEST_CASE("LocalName2RemoteFileBelowRoot") {
    auto opts = makeOptions("/foo/bar");
    auto config = makeConfig(opts, "file:///foo/bar");
    CHECK_EQ(config->localName2Remote("/foo/bar/baz.rb"), "file:///foo/bar/baz.rb");
}

TEST_CASE("LocalName2RemoteEmptyRoot") {
    // Special case: root path is '' (current directory) and root uri is '' (happens in Monaco).
    // filePath == rootPath == "" yields an empty relative path, which must not throw.
    auto opts = makeOptions("");
    auto config = makeConfig(opts, "");
    CHECK_EQ(config->localName2Remote(""), "");
}

} // namespace sorbet::realmain::lsp::test
