#include "main/lsp/lsp_messages_gen_helpers.h"

using namespace std;

namespace sorbet::realmain::lsp {

JSONNullObject tryConvertToNull(optional<const rapidjson::Value *> value, string_view name) {
    auto &realValue = assertJSONField(value, name);
    if (!realValue.IsNull()) {
        throw JSONTypeError(name, "null", realValue);
    }
    return JSONNullObject();
}

bool tryConvertToBoolean(optional<const rapidjson::Value *> value, string_view name) {
    auto &realValue = assertJSONField(value, name);
    if (!realValue.IsBool()) {
        throw JSONTypeError(name, "boolean", realValue);
    }
    return realValue.GetBool();
}

int tryConvertToInt(optional<const rapidjson::Value *> value, string_view name) {
    auto &realValue = assertJSONField(value, name);
    if (!realValue.IsInt()) {
        throw JSONTypeError(name, "int", realValue);
    }
    return realValue.GetInt();
}

double tryConvertToDouble(optional<const rapidjson::Value *> value, string_view name) {
    auto &realValue = assertJSONField(value, name);
    if (!realValue.IsNumber()) {
        throw JSONTypeError(name, "number", realValue);
    }
    return realValue.GetDouble();
}

string tryConvertToString(optional<const rapidjson::Value *> value, string_view name) {
    auto &realValue = assertJSONField(value, name);
    if (!realValue.IsString()) {
        throw JSONTypeError(name, "string", realValue);
    }
    return realValue.GetString();
}

string tryConvertToStringConstant(optional<const rapidjson::Value *> value, string_view constantValue,
                                  string_view name) {
    string strValue = tryConvertToString(value, name);
    if (strValue != constantValue) {
        throw JSONConstantError(name, constantValue, **value);
    }
    return strValue;
}

optional<const rapidjson::Value *> maybeGetJSONField(const rapidjson::Value &value, const string &name) {
    if (value.HasMember(name)) {
        return &value[name];
    }
    return nullopt;
}

const rapidjson::Value &assertJSONField(optional<const rapidjson::Value *> maybeValue, string_view name) {
    if (!maybeValue) {
        throw MissingFieldError(name);
    }
    return **maybeValue;
}

} // namespace sorbet::realmain::lsp
