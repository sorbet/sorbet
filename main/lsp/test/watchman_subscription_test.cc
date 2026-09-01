#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "common/common.h"
#include "main/lsp/watchman/WatchmanSubscription.h"
#include "rapidjson/document.h"
#include <string>

using namespace std;

namespace sorbet::realmain::lsp::watchman::test {
namespace {

rapidjson::Document parse(const string &json) {
    rapidjson::Document d;
    d.Parse(json.c_str());
    REQUIRE_FALSE(d.HasParseError());
    return d;
}

string str(const rapidjson::Value &v) {
    REQUIRE(v.IsString());
    return v.GetString();
}

TEST_CASE("buildSubscribeCommand renders the subscribe verb, root, name and file filter") {
    auto d = parse(buildSubscribeCommand("/repo/pay-server", "ruby-typer-1", {"rb", "rbi"}, "", {}));

    REQUIRE(d.IsArray());
    REQUIRE_EQ(d.Size(), 4);
    CHECK_EQ(str(d[0]), "subscribe");
    CHECK_EQ(str(d[1]), "/repo/pay-server");
    CHECK_EQ(str(d[2]), "ruby-typer-1");

    auto &options = d[3];
    REQUIRE(options.IsObject());
    CHECK(options["empty_on_fresh_instance"].GetBool());
    REQUIRE_EQ(options["fields"].Size(), 1);
    CHECK_EQ(str(options["fields"][0]), "name");
    CHECK_FALSE(options.HasMember("defer"));

    auto &expression = options["expression"];
    REQUIRE_EQ(expression.Size(), 4);
    CHECK_EQ(str(expression[0]), "allof");
    CHECK_EQ(str(expression[1][0]), "type");
    CHECK_EQ(str(expression[1][1]), "f");
    auto &suffixes = expression[2];
    REQUIRE_EQ(suffixes.Size(), 3);
    CHECK_EQ(str(suffixes[0]), "anyof");
    CHECK_EQ(str(suffixes[1][1]), "rb");
    CHECK_EQ(str(suffixes[2][1]), "rbi");
    CHECK_EQ(str(expression[3][0]), "not");
    CHECK_EQ(str(expression[3][1][1]), "**/.~tmp~/**");
}

TEST_CASE("buildSubscribeCommand restricts to the namespace directory when one is given") {
    auto d = parse(buildSubscribeCommand("/repo", "ruby-typer-1", {"rb"}, "pay-server", {}));

    auto &expression = d[3]["expression"];
    REQUIRE_EQ(expression.Size(), 5);
    CHECK_EQ(str(expression[2][0]), "dirname");
    CHECK_EQ(str(expression[2][1]), "pay-server");
}

TEST_CASE("buildSubscribeCommand defers on every configured pause state") {
    auto d = parse(buildSubscribeCommand("/repo", "ruby-typer-1", {"rb"}, "", {"autogen-run", "rsync-update"}));

    auto &options = d[3];
    REQUIRE(options.HasMember("defer"));
    auto &defer = options["defer"];
    REQUIRE_EQ(defer.Size(), 2);
    CHECK_EQ(str(defer[0]), "autogen-run");
    CHECK_EQ(str(defer[1]), "rsync-update");
    CHECK(options["empty_on_fresh_instance"].GetBool());
    CHECK_EQ(options["expression"].Size(), 4);
}

} // namespace
} // namespace sorbet::realmain::lsp::watchman::test
