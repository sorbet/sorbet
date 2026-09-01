#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H

#include "common/common.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <string_view>
#include <vector>

namespace sorbet::realmain::lsp::watchman {

constexpr int MAX_WATCHMAN_RESTARTS = 5;

namespace subscription_detail {

using Writer = rapidjson::Writer<rapidjson::StringBuffer>;

inline void writeString(Writer &w, std::string_view s) {
    w.String(s.data(), s.size());
}

// Shared by both commands: a narrower delta than the subscription would silently lose files.
inline void writeExpression(Writer &w, const std::vector<std::string> &extensions, std::string_view watchmanNamespace) {
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

inline void writeNameField(Writer &w) {
    w.String("fields");
    w.StartArray();
    w.String("name");
    w.EndArray();
}

} // namespace subscription_detail

inline std::string buildSubscribeCommand(std::string_view root, std::string_view subscriptionName,
                                         const std::vector<std::string> &extensions,
                                         std::string_view watchmanNamespace) {
    rapidjson::StringBuffer buffer;
    subscription_detail::Writer w(buffer);
    {
        w.StartArray();
        w.String("subscribe");
        subscription_detail::writeString(w, root);
        subscription_detail::writeString(w, subscriptionName);

        {
            w.StartObject();
            subscription_detail::writeExpression(w, extensions, watchmanNamespace);
            subscription_detail::writeNameField(w);

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

// Matches the same files as the subscription, but synchronizes with the file system before answering.
inline std::string buildChangesSinceQuery(std::string_view root, const std::vector<std::string> &extensions,
                                          std::string_view watchmanNamespace, std::string_view since) {
    rapidjson::StringBuffer buffer;
    subscription_detail::Writer w(buffer);
    {
        w.StartArray();
        w.String("query");
        subscription_detail::writeString(w, root);

        {
            w.StartObject();
            subscription_detail::writeExpression(w, extensions, watchmanNamespace);
            subscription_detail::writeNameField(w);

            w.String("since");
            subscription_detail::writeString(w, since);

            // A clock from another watchman instance would otherwise list every file in the root.
            w.String("empty_on_fresh_instance");
            w.Bool(true);

            w.EndObject();
        }

        w.EndArray();
    }

    return buffer.GetString();
}

// 1-based, so the schedule is 1s, 2s, 4s, 8s, then 8s.
inline std::chrono::milliseconds watchmanRestartDelay(int attempt) {
    ENFORCE(attempt >= 1);
    auto exponent = std::min(attempt - 1, 3);
    return std::chrono::seconds(1 << exponent);
}

} // namespace sorbet::realmain::lsp::watchman

#endif // RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
