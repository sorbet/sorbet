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
    REQUIRE_FALSE(d.Parse(json.c_str(), json.size()).HasParseError());
    return d;
}

// Returns the second element of the `dirname` clause of an `allof` expression, or nullopt when there is none.
optional<string> dirnameClause(const rapidjson::Value &expression) {
    REQUIRE(expression.IsArray());
    REQUIRE_EQ(string(expression[0].GetString()), "allof");
    for (rapidjson::SizeType i = 1; i < expression.Size(); i++) {
        auto &clause = expression[i];
        if (clause.IsArray() && clause.Size() == 2 && string(clause[0].GetString()) == "dirname") {
            return string(clause[1].GetString());
        }
    }
    return nullopt;
}

TEST_CASE("buildSubscribeCommand subscribes to regular files with the given extensions") {
    auto command = buildSubscribeCommand("/repo", "ruby-typer-1", {"rb", "rbi"}, "");
    auto d = parse(command);

    REQUIRE(d.IsArray());
    REQUIRE_EQ(d.Size(), 4);
    CHECK_EQ(string(d[0].GetString()), "subscribe");
    CHECK_EQ(string(d[1].GetString()), "/repo");
    CHECK_EQ(string(d[2].GetString()), "ruby-typer-1");

    auto &options = d[3];
    REQUIRE(options.IsObject());
    CHECK_FALSE(options.HasMember("since"));
    REQUIRE(options.HasMember("expression"));
    CHECK_FALSE(dirnameClause(options["expression"]).has_value());
    REQUIRE(options.HasMember("empty_on_fresh_instance"));
    CHECK(options["empty_on_fresh_instance"].GetBool());
    REQUIRE(options.HasMember("fields"));
    REQUIRE_EQ(options["fields"].Size(), 1);
    CHECK_EQ(string(options["fields"][0].GetString()), "name");
}

TEST_CASE("buildSubscribeCommand scopes the expression to the namespace") {
    auto d = parse(buildSubscribeCommand("/repo", "ruby-typer-1", {"rb"}, "pay-server"));
    auto dirname = dirnameClause(d[3]["expression"]);
    REQUIRE(dirname.has_value());
    CHECK_EQ(*dirname, "pay-server");
}

TEST_CASE("buildChangesSinceQuery asks for the same files since a clock") {
    auto subscribe = parse(buildSubscribeCommand("/repo", "ruby-typer-1", {"rb", "rbi"}, "pay-server"));
    auto query = parse(buildChangesSinceQuery("/repo", {"rb", "rbi"}, "pay-server", "c:1:2:3:4"));

    REQUIRE(query.IsArray());
    REQUIRE_EQ(query.Size(), 3);
    CHECK_EQ(string(query[0].GetString()), "query");
    CHECK_EQ(string(query[1].GetString()), "/repo");

    auto &options = query[2];
    REQUIRE(options.IsObject());
    REQUIRE(options.HasMember("since"));
    CHECK_EQ(string(options["since"].GetString()), "c:1:2:3:4");
    REQUIRE(options.HasMember("empty_on_fresh_instance"));
    CHECK(options["empty_on_fresh_instance"].GetBool());
    REQUIRE(options.HasMember("fields"));
    REQUIRE_EQ(options["fields"].Size(), 1);
    CHECK_EQ(string(options["fields"][0].GetString()), "name");

    // The query and the subscription must agree on which files are interesting.
    CHECK(options["expression"] == subscribe[3]["expression"]);
}

TEST_CASE("watchmanRestartDelay backs off exponentially and caps at 8s") {
    CHECK_EQ(watchmanRestartDelay(1).count(), 1000);
    CHECK_EQ(watchmanRestartDelay(2).count(), 2000);
    CHECK_EQ(watchmanRestartDelay(3).count(), 4000);
    CHECK_EQ(watchmanRestartDelay(4).count(), 8000);
    CHECK_EQ(watchmanRestartDelay(5).count(), 8000);
    CHECK_EQ(watchmanRestartDelay(MAX_WATCHMAN_RESTARTS).count(), 8000);
}

} // namespace
} // namespace sorbet::realmain::lsp::watchman::test
