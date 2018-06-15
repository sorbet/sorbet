#include "common/JSON.h"

#include <iomanip>
#include <sstream>

using namespace std;

namespace sorbet {
namespace core {

// https://stackoverflow.com/questions/7724448/simple-json-string-escape-for-c
string JSON::escape(string from) {
    ostringstream ss;
    for (auto ch : from) {
        switch (ch) {
            case '\\':
                ss << "\\\\";
                break;
            case '"':
                ss << "\\\"";
                break;
            case '\b':
                ss << "\\b";
                break;
            case '\f':
                ss << "\\f";
                break;
            case '\n':
                ss << "\\n";
                break;
            case '\r':
                ss << "\\r";
                break;
            case '\t':
                ss << "\\t";
                break;
            default:
                if (ch <= 0x1f) {
                    ss << "\\u" << hex << setw(4) << setfill('0') << (int)ch;
                    break;
                }
                ss << ch;
                break;
        }
    }
    return ss.str();
}

} // namespace core
} // namespace sorbet
