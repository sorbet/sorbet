#include "common/JSON.h"

#include <iomanip>
#include <sstream>

namespace ruby_typer {
namespace core {

// https://stackoverflow.com/questions/7724448/simple-json-string-escape-for-c
std::string JSON::escape(std::string from) {
    std::ostringstream ss;
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
                    ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)ch;
                    break;
                }
                ss << ch;
                break;
        }
    }
    return ss.str();
}

std::string JSON::fromProto(const google::protobuf::Message &message) {
    std::string json_string;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    google::protobuf::util::MessageToJsonString(message, &json_string, options);
    return json_string;
}

} // namespace core
} // namespace ruby_typer
