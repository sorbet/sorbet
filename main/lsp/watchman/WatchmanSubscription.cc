#include "main/lsp/watchman/WatchmanSubscription.h"
#include "common/common.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace std;

namespace sorbet::realmain::lsp::watchman {

string buildSubscribeCommand(string_view root, string_view subscriptionName, const vector<string> &extensions,
                             string_view watchmanNamespace, const vector<string> &deferStates) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> w(buffer);

    w.StartArray();
    w.String("subscribe");
    w.String(root.data(), root.size());
    w.String(subscriptionName.data(), subscriptionName.size());

    w.StartObject();

    w.String("expression");
    w.StartArray();
    w.String("allof");
    w.StartArray();
    w.String("type");
    w.String("f");
    w.EndArray();

    if (!watchmanNamespace.empty()) {
        w.StartArray();
        w.String("dirname");
        w.String(watchmanNamespace.data(), watchmanNamespace.size());
        w.EndArray();
    }

    // Note: Newer versions of Watchman (post 4.9.0) support ["suffix", ["suffix1", "suffix2", ...]],
    // but Stripe laptops have 4.9.0. Thus, we use [ "anyof", [ "suffix", "suffix1" ], [ "suffix",
    // "suffix2" ], ... ].
    w.StartArray();
    w.String("anyof");
    for (auto &extension : extensions) {
        w.StartArray();
        w.String("suffix");
        w.String(extension.data(), extension.size());
        w.EndArray();
    }
    w.EndArray();

    // Exclude rsync tmpfiles
    w.StartArray();
    w.String("not");
    w.StartArray();
    w.String("match");
    w.String("**/.~tmp~/**");
    w.String("wholename");
    w.StartObject();
    w.String("includedotfiles");
    w.Bool(true);
    w.EndObject();
    w.EndArray();
    w.EndArray();

    w.EndArray(); // allof

    w.String("fields");
    w.StartArray();
    w.String("name");
    w.EndArray();

    // Note 2: `empty_on_fresh_instance` prevents Watchman from sending entire contents of folder if this
    // subscription starts the daemon / causes the daemon to watch this folder for the first time.
    w.String("empty_on_fresh_instance");
    w.Bool(true);

    if (!deferStates.empty()) {
        w.String("defer");
        w.StartArray();
        for (auto &state : deferStates) {
            w.String(state.data(), state.size());
        }
        w.EndArray();
    }

    w.EndObject();
    w.EndArray();

    return string(buffer.GetString(), buffer.GetSize());
}

} // namespace sorbet::realmain::lsp::watchman
