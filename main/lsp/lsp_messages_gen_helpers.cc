#include "main/lsp/lsp_messages_gen_helpers.h"

using namespace std;

namespace sorbet::realmain::lsp {

JSONNullObject tryConvertToNull(const rapidjson::Value &value, string_view name) {
    if (!value.IsNull()) {
        throw JSONTypeError(name, "null", value);
    }
    return JSONNullObject();
}

bool tryConvertToBoolean(const rapidjson::Value &value, string_view name) {
    if (!value.IsBool()) {
        throw JSONTypeError(name, "boolean", value);
    }
    return value.GetBool();
}

int tryConvertToInt(const rapidjson::Value &value, string_view name) {
    if (!value.IsInt()) {
        throw JSONTypeError(name, "int", value);
    }
    return value.GetInt();
}

double tryConvertToDouble(const rapidjson::Value &value, string_view name) {
    if (!value.IsNumber()) {
        throw JSONTypeError(name, "number", value);
    }
    return value.GetDouble();
}

string tryConvertToString(const rapidjson::Value &value, string_view name) {
    if (!value.IsString()) {
        throw JSONTypeError(name, "string", value);
    }
    return value.GetString();
}

string tryConvertToStringConstant(const rapidjson::Value &value, string_view constantValue, string_view name) {
    string strValue = tryConvertToString(value, name);
    if (strValue != constantValue) {
        throw JSONConstantError(name, constantValue, value);
    }
    return strValue;
}

unique_ptr<rapidjson::Value> tryConvertToAny(rapidjson::MemoryPoolAllocator<> &alloc, const rapidjson::Value &value,
                                             string_view name) {
    return make_unique<rapidjson::Value>(value, alloc);
}

unique_ptr<rapidjson::Value> tryConvertToAnyObject(rapidjson::MemoryPoolAllocator<> &alloc,
                                                   const rapidjson::Value &value, string_view name) {
    if (!value.IsObject()) {
        throw JSONTypeError(name, "object", value);
    }
    return make_unique<rapidjson::Value>(value, alloc);
}

} // namespace sorbet::realmain::lsp
