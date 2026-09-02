#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "common/common.h"
#include "main/lsp/watchman/WatchmanSubscription.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <string>
#include <vector>

using namespace std;

namespace sorbet::realmain::lsp::watchman::test {
namespace {

const vector<string> EXTENSIONS = {"rb", "rbi"};
const string CLOCK = "c:1787952234:224531:1:441511";

rapidjson::Document parseCommand(const string &command, const string &expectedVerb, unsigned int expectedSize) {
    rapidjson::Document d;
    d.Parse(command.c_str(), command.size());
    REQUIRE_FALSE(d.HasParseError());
    REQUIRE(d.IsArray());
    REQUIRE(d.Size() == expectedSize);
    REQUIRE(string(d[0].GetString()) == expectedVerb);
    return d;
}

// ["subscribe", <root>, <name>, <options>]
rapidjson::Document parseSubscribe(const string &command) {
    return parseCommand(command, "subscribe", 4);
}

// ["query", <root>, <options>]
rapidjson::Document parseQuery(const string &command) {
    return parseCommand(command, "query", 3);
}

// Returns the clause of the top level `allof` expression at `index`, e.g. ["type", "f"].
const rapidjson::Value &expressionClause(const rapidjson::Value &options, int index) {
    const auto &expression = options["expression"];
    REQUIRE(expression.IsArray());
    REQUIRE(string(expression[0].GetString()) == "allof");
    REQUIRE(expression.Size() > static_cast<unsigned int>(index));
    return expression[index];
}

TEST_CASE("buildSubscribeCommand names the root and the subscription") {
    auto d = parseSubscribe(buildSubscribeCommand("/pay/src", "ruby-typer-1234", EXTENSIONS, ""));

    CHECK(string(d[1].GetString()) == "/pay/src");
    CHECK(string(d[2].GetString()) == "ruby-typer-1234");
    CHECK(d[3]["fields"].Size() == 1);
    CHECK(string(d[3]["fields"][0].GetString()) == "name");
}

// Sorbet reads the tree itself at startup, so a fresh instance listing every file would tell it nothing, and asking a
// cold watchman daemon for one is expensive.
TEST_CASE("buildSubscribeCommand suppresses the fresh instance listing") {
    auto d = parseSubscribe(buildSubscribeCommand("/pay/src", "ruby-typer-1234", EXTENSIONS, ""));

    REQUIRE(d[3].HasMember("empty_on_fresh_instance"));
    CHECK(d[3]["empty_on_fresh_instance"].GetBool());
}

TEST_CASE("buildSubscribeCommand matches every watched extension") {
    auto d = parseSubscribe(buildSubscribeCommand("/pay/src", "ruby-typer-1234", EXTENSIONS, ""));

    // ["allof", ["type", "f"], ["anyof", ...], ["not", ...]]
    const auto &anyof = expressionClause(d[3], 2);
    REQUIRE(string(anyof[0].GetString()) == "anyof");
    REQUIRE(anyof.Size() == 3);
    CHECK(string(anyof[1][0].GetString()) == "suffix");
    CHECK(string(anyof[1][1].GetString()) == "rb");
    CHECK(string(anyof[2][1].GetString()) == "rbi");
}

TEST_CASE("buildSubscribeCommand scopes the expression to the namespace directory") {
    auto d = parseSubscribe(buildSubscribeCommand("/pay/src", "ruby-typer-1234", EXTENSIONS, "pay-server"));

    // ["allof", ["type", "f"], ["dirname", "pay-server"], ["anyof", ...], ["not", ...]]
    const auto &dirname = expressionClause(d[3], 2);
    CHECK(string(dirname[0].GetString()) == "dirname");
    CHECK(string(dirname[1].GetString()) == "pay-server");
    CHECK(string(expressionClause(d[3], 3)[0].GetString()) == "anyof");
}

TEST_CASE("buildCatchUpQueryCommand asks for the changes since the given clock") {
    auto d = parseQuery(buildCatchUpQueryCommand("/pay/src", EXTENSIONS, "", CLOCK));

    CHECK(string(d[1].GetString()) == "/pay/src");
    REQUIRE(d[2].HasMember("since"));
    CHECK(string(d[2]["since"].GetString()) == CLOCK);
}

// Both commands suppress it, because `is_fresh_instance` means the same thing on either: there is no delta to be had,
// and the recovery is for Sorbet to re-read the workspace itself rather than to work through a list watchman attaches.
// That list could not do the job anyway, since what exists now cannot mention a file deleted while Sorbet was not
// listening. See LSPIndexer::resyncAllFilesFromDisk.
TEST_CASE("buildCatchUpQueryCommand suppresses the fresh instance listing") {
    auto d = parseQuery(buildCatchUpQueryCommand("/pay/src", EXTENSIONS, "", CLOCK));

    REQUIRE(d[2].HasMember("empty_on_fresh_instance"));
    CHECK(d[2]["empty_on_fresh_instance"].GetBool());
}

// The query stands in for the subscription over the window it cannot cover, so it has to select the same files. If the
// two drifted apart, the gap would be invisible: Sorbet would simply never hear about whatever the query left out.
TEST_CASE("buildCatchUpQueryCommand selects the same files as the subscription") {
    auto subscribe = parseSubscribe(buildSubscribeCommand("/pay/src", "ruby-typer-1234", EXTENSIONS, "pay-server"));
    auto query = parseQuery(buildCatchUpQueryCommand("/pay/src", EXTENSIONS, "pay-server", CLOCK));

    rapidjson::StringBuffer subscribeExpression;
    rapidjson::Writer<rapidjson::StringBuffer> subscribeWriter(subscribeExpression);
    subscribe[3]["expression"].Accept(subscribeWriter);

    rapidjson::StringBuffer queryExpression;
    rapidjson::Writer<rapidjson::StringBuffer> queryWriter(queryExpression);
    query[2]["expression"].Accept(queryWriter);

    CHECK(string(subscribeExpression.GetString()) == string(queryExpression.GetString()));
    CHECK(string(subscribe[3]["fields"][0].GetString()) == string(query[2]["fields"][0].GetString()));
}

} // namespace
} // namespace sorbet::realmain::lsp::watchman::test
