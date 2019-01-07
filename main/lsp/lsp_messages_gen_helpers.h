#ifndef LSP_MESSAGES_GEN_HELPERS_H
#define LSP_MESSAGES_GEN_HELPERS_H

#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {

// The following block of methods are used from generated code and thus _have_ to share the same signature.

JSONNullObject tryConvertToNull(const rapidjson::Value &value, const std::string &name);

bool tryConvertToBoolean(const rapidjson::Value &value, const std::string &name);

int tryConvertToInt(const rapidjson::Value &value, const std::string &name);

double tryConvertToDouble(const rapidjson::Value &value, const std::string &name);

std::string tryConvertToString(const rapidjson::Value &value, const std::string &name);

std::string tryConvertToStringConstant(const rapidjson::Value &value, const std::string &constantValue,
                                       const std::string &name);

std::unique_ptr<rapidjson::Value> tryConvertToAny(rapidjson::MemoryPoolAllocator<> &alloc,
                                                  const rapidjson::Value &value, const std::string &name = "");

std::unique_ptr<rapidjson::Value> tryConvertToAnyObject(rapidjson::MemoryPoolAllocator<> &alloc,
                                                        const rapidjson::Value &value, const std::string &name);

template <typename Representation, typename Extractor>
inline Representation tryConvertToEnum(const rapidjson::Value &value, std::string name,
                                       Extractor tryConvertToEnumFunction) {
    return tryConvertToEnumFunction(tryConvertToString(value, name));
};

template <typename T> inline std::unique_ptr<JSONDocument<T>> fromJSONInternal(const std::string &json) {
    std::unique_ptr<rapidjson::Document> d = std::make_unique<rapidjson::Document>();
    d->Parse(json);
    if (!d->IsObject()) {
        throw JSONTypeError("document root", "object", *d);
    }
    auto rootObject = T::fromJSONValue(d->GetAllocator(), d->GetObject(), "root");
    return std::make_unique<JSONDocument<T>>(d, rootObject);
};

} // namespace sorbet::realmain::lsp

#endif // LSP_MESSAGES_GEN_HELPERS_H
