#include "main/lsp/watchman/WatchmanSubscription.h"
#include "common/common.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <algorithm>

using namespace std;

namespace sorbet::realmain::lsp::watchman {

namespace {

using Writer = rapidjson::Writer<rapidjson::StringBuffer>;

void writeString(Writer &w, string_view s) {
    w.String(s.data(), s.size());
}

// The `expression` shared by the subscription and the changes-since query: regular files with one of `extensions`,
// inside `watchmanNamespace` when set, excluding rsync tmpfiles.
void writeExpression(Writer &w, const vector<string> &extensions, string_view watchmanNamespace) {
    w.String("expression");
    w.StartArray();
    w.String("allof");
    {
        w.StartArray();
        w.String("type");
        w.String("f");
        w.EndArray();
    }

    if (!watchmanNamespace.empty()) {
        w.StartArray();
        w.String("dirname");
        writeString(w, watchmanNamespace);
        w.EndArray();
    }

    // Note: Newer versions of Watchman (post 4.9.0) support ["suffix", ["suffix1", "suffix2", ...]],
    // but Stripe laptops have 4.9.0. Thus, we use [ "anyof", [ "suffix", "suffix1" ], [ "suffix",
    // "suffix2" ], ... ].
    {
        w.StartArray();
        w.String("anyof");

        for (auto &extension : extensions) {
            w.StartArray();
            w.String("suffix");
            w.String(extension);
            w.EndArray();
        }

        w.EndArray();
    }

    // Exclude rsync tmpfiles
    {
        w.StartArray();
        w.String("not");
        {
            w.StartArray();
            w.String("match");
            w.String("**/.~tmp~/**");
            w.String("wholename");
            {
                w.StartObject();
                w.String("includedotfiles");
                w.Bool(true);
                w.EndObject();
            }
            w.EndArray();
        }
        w.EndArray();
    }

    w.EndArray();
}

void writeNameField(Writer &w) {
    w.String("fields");
    w.StartArray();
    w.String("name");
    w.EndArray();
}

} // namespace

string buildSubscribeCommand(string_view root, string_view subscriptionName, const vector<string> &extensions,
                             string_view watchmanNamespace) {
    rapidjson::StringBuffer buffer;
    Writer w(buffer);
    {
        w.StartArray();
        w.String("subscribe");
        writeString(w, root);
        writeString(w, subscriptionName);

        {
            w.StartObject();
            writeExpression(w, extensions, watchmanNamespace);
            writeNameField(w);

            // Note 2: `empty_on_fresh_instance` prevents Watchman from sending entire contents of folder if this
            // subscription starts the daemon / causes the daemon to watch this folder for the first time.
            w.String("empty_on_fresh_instance");
            w.Bool(true);

            w.EndObject();
        }

        w.EndArray();
    }

    return buffer.GetString();
}

string buildChangesSinceQuery(string_view root, const vector<string> &extensions, string_view watchmanNamespace,
                              string_view since) {
    rapidjson::StringBuffer buffer;
    Writer w(buffer);
    {
        w.StartArray();
        w.String("query");
        writeString(w, root);

        {
            w.StartObject();
            writeExpression(w, extensions, watchmanNamespace);
            writeNameField(w);

            w.String("since");
            writeString(w, since);

            // A clock from another watchman instance would otherwise list every file in the root.
            w.String("empty_on_fresh_instance");
            w.Bool(true);

            w.EndObject();
        }

        w.EndArray();
    }

    return buffer.GetString();
}

chrono::milliseconds watchmanRestartDelay(int attempt) {
    ENFORCE(attempt >= 1);
    auto exponent = min(attempt - 1, 3);
    return chrono::seconds(1 << exponent);
}

} // namespace sorbet::realmain::lsp::watchman
