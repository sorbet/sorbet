#ifndef LSP_MESSAGES_GEN_HELPERS_H
#define LSP_MESSAGES_GEN_HELPERS_H

#include "common/common.h"
#include "main/lsp/json_enums.h"
#include "rapidjson/document.h"

namespace sorbet::realmain::lsp {

// The following block of methods are used from generated code and thus _have_ to share the same signature.

JSONNullObject tryConvertToNull(std::optional<const rapidjson::Value *> value, std::string_view name);

bool tryConvertToBoolean(std::optional<const rapidjson::Value *> value, std::string_view name);

int tryConvertToInt(std::optional<const rapidjson::Value *> value, std::string_view name);

double tryConvertToDouble(std::optional<const rapidjson::Value *> value, std::string_view name);

std::string tryConvertToString(std::optional<const rapidjson::Value *> value, std::string_view name);

std::string tryConvertToStringConstant(std::optional<const rapidjson::Value *> value, std::string_view constantValue,
                                       std::string_view name);

std::optional<const rapidjson::Value *> maybeGetJSONField(const rapidjson::Value &value, std::string_view name);

const rapidjson::Value &assertJSONField(std::optional<const rapidjson::Value *> maybeValue, std::string_view name);

} // namespace sorbet::realmain::lsp

#endif // LSP_MESSAGES_GEN_HELPERS_H
