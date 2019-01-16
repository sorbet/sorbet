#ifndef LSP_MESSAGES_GEN_HELPERS_H
#define LSP_MESSAGES_GEN_HELPERS_H

#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {

// The following block of methods are used from generated code and thus _have_ to share the same signature.

JSONNullObject tryConvertToNull(const rapidjson::Value &value, std::string_view name);

bool tryConvertToBoolean(const rapidjson::Value &value, std::string_view name);

int tryConvertToInt(const rapidjson::Value &value, std::string_view name);

double tryConvertToDouble(const rapidjson::Value &value, std::string_view name);

std::string tryConvertToString(const rapidjson::Value &value, std::string_view name);

std::string tryConvertToStringConstant(const rapidjson::Value &value, std::string_view constantValue,
                                       std::string_view name);

std::unique_ptr<rapidjson::Value> tryConvertToAny(rapidjson::MemoryPoolAllocator<> &alloc,
                                                  const rapidjson::Value &value, std::string_view name = "");

std::unique_ptr<rapidjson::Value> tryConvertToAnyObject(rapidjson::MemoryPoolAllocator<> &alloc,
                                                        const rapidjson::Value &value, std::string_view name);

template <typename Representation, typename Extractor>
inline Representation tryConvertToEnum(const rapidjson::Value &value, std::string name,
                                       Extractor tryConvertToEnumFunction) {
    return tryConvertToEnumFunction(tryConvertToString(value, name));
};

} // namespace sorbet::realmain::lsp

#endif // LSP_MESSAGES_GEN_HELPERS_H
