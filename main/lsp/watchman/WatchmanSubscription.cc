#include "main/lsp/watchman/WatchmanSubscription.h"
// common/common.h defines the ENFORCE that rapidjson's RAPIDJSON_ASSERT expands to, so it has to precede it.
#include "common/common.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace std;

namespace sorbet::realmain::lsp::watchman {

namespace {

using Writer = rapidjson::Writer<rapidjson::StringBuffer>;

// Everything the subscription and the catch-up query that covers for it have in common, so that the two cannot drift
// apart: which files Sorbet wants to hear about, what to call them, and that a fresh instance must not come with a
// listing of the whole tree.
void writeSharedQueryBody(Writer &w, const vector<string> &extensions, string_view watchmanNamespace) {
    w.String("expression");
    {
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
            w.String(watchmanNamespace.data(), watchmanNamespace.size());
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

    w.String("fields");
    {
        w.StartArray();
        w.String("name");
        w.EndArray();
    }

    // Sorbet reads the tree itself at startup, and recovers from a fresh instance by re-reading it again, so the
    // listing of every matching file that watchman would otherwise attach is work nobody consumes. It could not do the
    // whole job anyway: what exists now cannot name a file deleted while Sorbet was not listening. See
    // LSPIndexer::resyncAllFilesFromDisk.
    w.String("empty_on_fresh_instance");
    w.Bool(true);
}

} // namespace

string buildSubscribeCommand(string_view root, string_view subscriptionName, const vector<string> &extensions,
                             string_view watchmanNamespace) {
    rapidjson::StringBuffer buffer;
    Writer w(buffer);
    {
        w.StartArray();
        w.String("subscribe");
        w.String(root.data(), root.size());
        w.String(subscriptionName.data(), subscriptionName.size());

        {
            w.StartObject();

            writeSharedQueryBody(w, extensions, watchmanNamespace);

            w.EndObject();
        }

        w.EndArray();
    }

    return string(buffer.GetString(), buffer.GetSize());
}

string buildCatchUpQueryCommand(string_view root, const vector<string> &extensions, string_view watchmanNamespace,
                                string_view sinceClock) {
    rapidjson::StringBuffer buffer;
    Writer w(buffer);
    {
        w.StartArray();
        w.String("query");
        w.String(root.data(), root.size());

        {
            w.StartObject();

            writeSharedQueryBody(w, extensions, watchmanNamespace);

            w.String("since");
            w.String(sinceClock.data(), sinceClock.size());

            w.EndObject();
        }

        w.EndArray();
    }

    return string(buffer.GetString(), buffer.GetSize());
}

} // namespace sorbet::realmain::lsp::watchman
