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

TEST_CASE("RootPathBelowClientRoot") {
    // Sorbet runs in a subdirectory of the folder open in the editor.
    auto opts = makeOptions("/foo/bar");
    auto config = makeConfig(opts, "file:///foo");
    CHECK_EQ(config->localName2Remote("/foo/bar/baz.rb"), "file:///foo/bar/baz.rb");
    CHECK_EQ(config->remoteName2Local("file:///foo/bar/baz.rb"), "/foo/bar/baz.rb");
    CHECK(config->isUriInWorkspace("file:///foo/bar/baz.rb"));
    CHECK_FALSE(config->isUriInWorkspace("file:///foo/baz.rb"));
}

TEST_CASE("RootPathOutsideClientRoot") {
    // The client runs on another machine, with a different path for the same folder.
    auto opts = makeOptions("/remote/bar");
    auto config = makeConfig(opts, "file:///local/bar");
    CHECK_EQ(config->localName2Remote("/remote/bar/baz.rb"), "file:///local/bar/baz.rb");
    CHECK_EQ(config->remoteName2Local("file:///local/bar/baz.rb"), "/remote/bar/baz.rb");
}

TEST_CASE("RootPathSharingPrefixWithClientRoot") {
    // `/foo/barbaz` is not below `/foo/bar`.
    auto opts = makeOptions("/foo/barbaz");
    auto config = makeConfig(opts, "file:///foo/bar");
    CHECK_EQ(config->localName2Remote("/foo/barbaz/qux.rb"), "file:///foo/bar/qux.rb");
}

} // namespace sorbet::realmain::lsp::test
