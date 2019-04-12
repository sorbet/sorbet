#ifndef LSP_MESSAGES_GEN_HELPERS_H
#define LSP_MESSAGES_GEN_HELPERS_H

#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {

// The following block of methods are used from generated code and thus _have_ to share the same signature.

JSONNullObject tryConvertToNull(std::optional<const rapidjson::Value *> value, std::string_view name);

bool tryConvertToBoolean(std::optional<const rapidjson::Value *> value, std::string_view name);

int tryConvertToInt(std::optional<const rapidjson::Value *> value, std::string_view name);

double tryConvertToDouble(std::optional<const rapidjson::Value *> value, std::string_view name);

std::string tryConvertToString(std::optional<const rapidjson::Value *> value, std::string_view name);

std::string tryConvertToStringConstant(std::optional<const rapidjson::Value *> value, std::string_view constantValue,
                                       std::string_view name);

std::unique_ptr<rapidjson::Value> tryConvertToAny(rapidjson::MemoryPoolAllocator<> &alloc,
                                                  std::optional<const rapidjson::Value *> value,
                                                  std::string_view name = "");

std::unique_ptr<rapidjson::Value> tryConvertToAnyObject(rapidjson::MemoryPoolAllocator<> &alloc,
                                                        std::optional<const rapidjson::Value *> value,
                                                        std::string_view name);

// N.B.: Uses a string reference since rapidjson APIs require a C string.
std::optional<const rapidjson::Value *> maybeGetJSONField(const rapidjson::Value &value, const std::string &name);

const rapidjson::Value &assertJSONField(std::optional<const rapidjson::Value *> maybeValue, std::string_view name);

} // namespace sorbet::realmain::lsp

#endif // LSP_MESSAGES_GEN_HELPERS_H
