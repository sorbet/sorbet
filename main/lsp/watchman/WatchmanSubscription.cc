#include "main/lsp/watchman/WatchmanSubscription.h"
// common/common.h defines the ENFORCE that rapidjson's RAPIDJSON_ASSERT expands to, so it has to precede it.
#include "common/common.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace std;

namespace sorbet::realmain::lsp::watchman {

namespace {

using Writer = rapidjson::Writer<rapidjson::StringBuffer>;

// The files Sorbet wants to hear about, and the field it wants them named by. Shared by the subscription and by the
// catch-up query that covers for it, so that the two cannot drift apart.
void writeExpressionAndFields(Writer &w, const vector<string> &extensions, string_view watchmanNamespace) {
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

            writeExpressionAndFields(w, extensions, watchmanNamespace);

            // Note 2: `empty_on_fresh_instance` prevents Watchman from sending entire contents of folder if this
            // subscription starts the daemon / causes the daemon to watch this folder for the first time.
            w.String("empty_on_fresh_instance");
            w.Bool(true);

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

            writeExpressionAndFields(w, extensions, watchmanNamespace);

            w.String("since");
            w.String(sinceClock.data(), sinceClock.size());

            // Same as the subscription, and for the same reason. If watchman cannot honor this clock -- it restarted,
            // or the watch was recrawled past it -- there is no delta to be had, and it says so with
            // `is_fresh_instance`. Sorbet recovers from that flag by re-reading the workspace itself, so the whole-tree
            // file list watchman would otherwise attach is work nobody consumes: a multi-megabyte response to parse,
            // then a serial read of every path in it on the preprocessor thread. Worse, it could not do the whole job
            // anyway, because a list of what exists now cannot mention a file that was deleted while Sorbet was not
            // listening. See LSPIndexer::resyncAllFilesFromDisk, which walks the file table as well as the workspace.
            w.String("empty_on_fresh_instance");
            w.Bool(true);

            w.EndObject();
        }

        w.EndArray();
    }

    return string(buffer.GetString(), buffer.GetSize());
}

} // namespace sorbet::realmain::lsp::watchman
