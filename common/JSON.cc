#include "common/JSON.h"
#include "spdlog/spdlog.h"
// spdlog.h must be included before fmt.h https://github.com/sorbet/sorbet/pull/2839
#include "spdlog/fmt/fmt.h"

using namespace std;

namespace sorbet {

string JSON::escape(string from) {
    fmt::memory_buffer buf;
    int firstUnusedChar = 0;
    int currentChar = -1;
    for (auto ch : from) {
        currentChar++;
        string_view special;
        switch (ch) {
            case '\\':
                special = "\\\\"sv;
                break;
            case '"':
                special = "\\\""sv;
                break;
            case '\b':
                special = "\\b"sv;
                break;
            case '\f':
                special = "\\f"sv;
                break;
            case '\n':
                special = "\\n"sv;
                break;
            case '\r':
                special = "\\r"sv;
                break;
            case '\t':
                special = "\\t"sv;
                break;
            default:
                if (0x00 <= ch && ch <= 0x1f) {
                    string_view toAdd(from.data() + firstUnusedChar, currentChar - firstUnusedChar);
                    firstUnusedChar = currentChar + 1;
                    fmt::format_to(std::back_inserter(buf), "{}\\u{:04x}", toAdd, ch);
                }
                continue;
        }
        string_view toAdd(from.data() + firstUnusedChar, currentChar - firstUnusedChar);
        firstUnusedChar = currentChar + 1;
        fmt::format_to(std::back_inserter(buf), "{}{}", toAdd, special);
    }
    string_view toAdd(from.data() + firstUnusedChar, from.size() - firstUnusedChar);
    fmt::format_to(std::back_inserter(buf), "{}", toAdd);

    return to_string(buf);
}

} // namespace sorbet
