#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "common/common.h"
#include "main/lsp/watchman/WatchmanSubscription.h"
#include "rapidjson/document.h"
#include <string>
#include <vector>

using namespace std;

namespace sorbet::realmain::lsp::watchman::test {
namespace {

rapidjson::Document parse(const string &json) {
    rapidjson::Document d;
    d.Parse(json.c_str(), json.size());
    REQUIRE_FALSE(d.HasParseError());
    return d;
}

TEST_CASE("buildSubscribeCommand renders the subscribe verb, root, name and file filter") {
    auto d = parse(buildSubscribeCommand("/repo/pay-server", "ruby-typer-1", {"rb", "rbi"}, "", {}));

    REQUIRE(d.IsArray());
    REQUIRE_EQ(d.Size(), 4);
    CHECK_EQ(string(d[0].GetString()), "subscribe");
    CHECK_EQ(string(d[1].GetString()), "/repo/pay-server");
    CHECK_EQ(string(d[2].GetString()), "ruby-typer-1");

    auto &options = d[3];
    REQUIRE(options.IsObject());
    CHECK(options["empty_on_fresh_instance"].GetBool());
    REQUIRE_EQ(options["fields"].Size(), 1);
    CHECK_EQ(string(options["fields"][0].GetString()), "name");

    // ["allof", ["type", "f"], ["anyof", ["suffix", "rb"], ["suffix", "rbi"]], ["not", ...]]
    auto &expression = options["expression"];
    REQUIRE_EQ(expression.Size(), 4);
    CHECK_EQ(string(expression[0].GetString()), "allof");
    CHECK_EQ(string(expression[1][0].GetString()), "type");
    CHECK_EQ(string(expression[1][1].GetString()), "f");
    auto &suffixes = expression[2];
    CHECK_EQ(string(suffixes[0].GetString()), "anyof");
    REQUIRE_EQ(suffixes.Size(), 3);
    CHECK_EQ(string(suffixes[1][1].GetString()), "rb");
    CHECK_EQ(string(suffixes[2][1].GetString()), "rbi");
    CHECK_EQ(string(expression[3][0].GetString()), "not");
    CHECK_EQ(string(expression[3][1][1].GetString()), "**/.~tmp~/**");

    CHECK_FALSE(options.HasMember("defer"));
}

TEST_CASE("buildSubscribeCommand restricts to the namespace directory when one is given") {
    auto d = parse(buildSubscribeCommand("/repo", "ruby-typer-1", {"rb"}, "pay-server", {}));

    auto &expression = d[3]["expression"];
    REQUIRE_EQ(expression.Size(), 5);
    CHECK_EQ(string(expression[2][0].GetString()), "dirname");
    CHECK_EQ(string(expression[2][1].GetString()), "pay-server");
}

TEST_CASE("buildSubscribeCommand defers on every configured pause state") {
    auto d = parse(buildSubscribeCommand("/repo", "ruby-typer-1", {"rb"}, "", {"autogen-run", "rsync-update"}));

    auto &options = d[3];
    REQUIRE(options.HasMember("defer"));
    REQUIRE(options["defer"].IsArray());
    REQUIRE_EQ(options["defer"].Size(), 2);
    CHECK_EQ(string(options["defer"][0].GetString()), "autogen-run");
    CHECK_EQ(string(options["defer"][1].GetString()), "rsync-update");
    // The rest of the subscription is unchanged by the defer list.
    CHECK(options["empty_on_fresh_instance"].GetBool());
    CHECK_EQ(options["expression"].Size(), 4);
}

} // namespace
} // namespace sorbet::realmain::lsp::watchman::test
