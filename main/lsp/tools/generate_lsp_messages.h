#ifndef GENERATE_LSP_MESSAGES_H
#define GENERATE_LSP_MESSAGES_H
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "common/FileOps.h"
#include "common/JSON.h"
#include "common/common.h"
#include "common/strings/formatting.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace sorbet;

// rapidjson::Allocator variable, which is assumed to be available in serialization and deserialization methods.
// Used to copy things of type "any" from the JSON document into our C++ objects so we manage the memory.
const std::string ALLOCATOR_VAR = "alloc";

// How this type appears in raw JSON or C++.
// Primarily used to determine which types we can automatically discriminate in variant types.
enum class BaseKind {
    NullKind, // Note: In C++, it's not nullptr -- it's a JSONNullObject.
    BooleanKind,
    IntKind,
    DoubleKind,
    StringKind, // Covers string enums and strings
    ObjectKind,
    ArrayKind,
    // Catch-all for optional and variant types.
    ComplexKind,
};

using AssignLambda = std::function<void(fmt::memory_buffer &out, std::string_view)>;

class JSONType {
public:
    /**
     * Returns the C++ type for this specific type.
     */
    virtual std::string getCPPType() const = 0;

    /**
     * Returns the JSON/TypeScript type for this specific type.
     * Used to construct error messages.
     */
    virtual std::string getJSONType() const = 0;

    /**
     * This type's base kind on the C++ side.
     */
    virtual BaseKind getCPPBaseKind() const = 0;

    /**
     * This type's base kind on the JSON side.
     */
    virtual BaseKind getJSONBaseKind() const = 0;

    /**
     * Returns `true` if the underlying C++ type would prefer to be moved rather than
     * copied where possible.
     */
    virtual bool wantMove() const {
        return false;
    }

    /**
     * Writes the C++ statements needed to sanity check and retrieve a value
     * of this type from an `optional<const rapidjson::Value *>` object stored in `from` into
     * the struct. Call `assign` with the C++ code that returns the
     * deserialized value, and it'll return the C++ code for assigning
     * it to the struct.
     *
     * `fieldName` should be used to generate error messages.
     * If field is required and from is nullopt, code should throw a MissingFieldError.
     */
    virtual void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                                   std::string_view fieldName) = 0;

    /**
     * Writes the C++ statements needed to convert this type into a value
     * that can be assigned to a rapidjson::Value object into `out`.
     * A value of this type is currently stored in eval(`from`), and the the code
     * for writing it into the destination is produced by calling assign(codeThatProducesValue)
     */
    virtual void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                                 std::string_view fieldName) = 0;

protected:
    void simpleDeserialization(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                               std::string_view fieldName, std::string_view helperFunctionName) {
        assign(out, fmt::format("{}({}, \"{}\")", helperFunctionName, from, fieldName));
    }

    void simpleSerialization(fmt::memory_buffer &out, std::string_view from, AssignLambda assign) {
        assign(out, from);
    }
};

class JSONClassType : public JSONType {
protected:
    const std::string typeName;

    /**
     * Emit any declarations that should go in the header file.
     */
    virtual void emitDeclaration(fmt::memory_buffer &out) = 0;

    /**
     * Emit any definitions that should go in the class file.
     */
    virtual void emitDefinition(fmt::memory_buffer &out) = 0;

public:
    JSONClassType(std::string_view typeName) : typeName(std::string(typeName)) {}

    /**
     * Recursively emits definitions and declarations for this type and all
     * dependent class types. No-op if already emitted.
     */
    void emit(fmt::memory_buffer &headerBuffer, fmt::memory_buffer &classBuffer) {
        emitDeclaration(headerBuffer);
        emitDefinition(classBuffer);
    }
};

class JSONNullType final : public JSONType {
public:
    BaseKind getCPPBaseKind() const {
        return BaseKind::NullKind;
    }

    BaseKind getJSONBaseKind() const {
        return BaseKind::NullKind;
    }

    std::string getCPPType() const {
        return "JSONNullObject";
    }

    std::string getJSONType() const {
        return "null";
    }

    void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                           std::string_view fieldName) {
        simpleDeserialization(out, from, assign, fieldName, "tryConvertToNull");
    }

    void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                         std::string_view fieldName) {
        assign(out, "rapidjson::Value(rapidjson::kNullType)");
    }
};

class JSONBooleanType final : public JSONType {
public:
    BaseKind getCPPBaseKind() const {
        return BaseKind::BooleanKind;
    }

    BaseKind getJSONBaseKind() const {
        return BaseKind::BooleanKind;
    }

    std::string getCPPType() const {
        return "bool";
    }

    std::string getJSONType() const {
        return "boolean";
    }

    void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                           std::string_view fieldName) {
        simpleDeserialization(out, from, assign, fieldName, "tryConvertToBoolean");
    }

    void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                         std::string_view fieldName) {
        simpleSerialization(out, from, assign);
    }
};

class JSONIntType final : public JSONType {
public:
    BaseKind getCPPBaseKind() const {
        return BaseKind::IntKind;
    }

    BaseKind getJSONBaseKind() const {
        return BaseKind::IntKind;
    }

    std::string getCPPType() const {
        return "int";
    }

    std::string getJSONType() const {
        return "integer";
    }

    void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                           std::string_view fieldName) {
        simpleDeserialization(out, from, assign, fieldName, "tryConvertToInt");
    }

    void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                         std::string_view fieldName) {
        simpleSerialization(out, from, assign);
    }
};

class JSONDoubleType final : public JSONType {
public:
    BaseKind getCPPBaseKind() const {
        return BaseKind::DoubleKind;
    }

    BaseKind getJSONBaseKind() const {
        return BaseKind::DoubleKind;
    }

    std::string getCPPType() const {
        return "double";
    }

    std::string getJSONType() const {
        return "number";
    }

    void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                           std::string_view fieldName) {
        simpleDeserialization(out, from, assign, fieldName, "tryConvertToDouble");
    }

    void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                         std::string_view fieldName) {
        simpleSerialization(out, from, assign);
    }
};

class JSONStringType final : public JSONType {
public:
    static void serializeStringToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign) {
        // Copy into document so that it owns the string.
        // Create new scope for temp var.
        fmt::format_to(std::back_inserter(out), "{{\n");
        fmt::format_to(std::back_inserter(out), "rapidjson::Value strCopy;\n");
        fmt::format_to(std::back_inserter(out), "const std::string &tmpStr = {0};\n", from);
        fmt::format_to(std::back_inserter(out), "strCopy.SetString(tmpStr.c_str(), tmpStr.length(), {0});\n",
                       ALLOCATOR_VAR);
        assign(out, "strCopy");
        fmt::format_to(std::back_inserter(out), "}}\n");
    }

    bool wantMove() const {
        return true;
    }

    BaseKind getCPPBaseKind() const {
        return BaseKind::StringKind;
    }

    BaseKind getJSONBaseKind() const {
        return BaseKind::StringKind;
    }

    std::string getCPPType() const {
        return "std::string";
    }

    std::string getJSONType() const {
        return "string";
    }

    void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                           std::string_view fieldName) {
        simpleDeserialization(out, from, assign, fieldName, "tryConvertToString");
    }

    void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                         std::string_view fieldName) {
        serializeStringToJSONValue(out, from, assign);
    }
};

// TODO: Emit an actual constant.
class JSONStringConstantType final : public JSONType {
private:
    const std::string value;

public:
    JSONStringConstantType(std::string_view value) : value(std::string(value)) {}

    bool wantMove() const {
        return true;
    }

    BaseKind getCPPBaseKind() const {
        return BaseKind::StringKind;
    }

    BaseKind getJSONBaseKind() const {
        return BaseKind::StringKind;
    }

    std::string getCPPType() const {
        return "std::string";
    }

    std::string getJSONType() const {
        return fmt::format("\"{}\"", value);
    }

    void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                           std::string_view fieldName) {
        assign(out, fmt::format("tryConvertToStringConstant({}, \"{}\", \"{}\")", from, value, fieldName));
    }

    void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                         std::string_view fieldName) {
        fmt::format_to(std::back_inserter(out), "if ({} != \"{}\") {{\n", from, value);
        fmt::format_to(std::back_inserter(out), "throw InvalidConstantValueError(\"{}\", \"{}\", {});\n", fieldName,
                       value, from);
        fmt::format_to(std::back_inserter(out), "}}\n");
        JSONStringType::serializeStringToJSONValue(out, from, assign);
    }
};

class JSONArrayType final : public JSONType {
private:
    std::shared_ptr<JSONType> componentType;
    // Temp variable name used during serialization and deserialization.
    static const std::string arrayVar;

    static void AssignDeserializedElementValue(fmt::memory_buffer &out, std::string_view from) {
        fmt::format_to(std::back_inserter(out), "{}.push_back({});", arrayVar, from);
    }

    static void AssignSerializedElementValue(fmt::memory_buffer &out, std::string_view from) {
        fmt::format_to(std::back_inserter(out), "{}.PushBack({}, {});", arrayVar, from, ALLOCATOR_VAR);
    }

public:
    JSONArrayType(std::shared_ptr<JSONType> componentType) : componentType(componentType) {}

    BaseKind getCPPBaseKind() const {
        return BaseKind::ArrayKind;
    }

    BaseKind getJSONBaseKind() const {
        return BaseKind::ArrayKind;
    }

    std::string getCPPType() const {
        return fmt::format("std::vector<{}>", componentType->getCPPType());
    }

    std::string getJSONType() const {
        return fmt::format("Array<{}>", componentType->getJSONType());
    }

    bool wantMove() const {
        return componentType->wantMove();
    }

    void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                           std::string_view fieldName) {
        fmt::format_to(std::back_inserter(out), "{{\n");
        fmt::format_to(std::back_inserter(out), "auto &unwrappedVal = assertJSONField({}, \"{}\");", from, fieldName);
        fmt::format_to(std::back_inserter(out), "if (!unwrappedVal.IsArray()) {{\n");
        fmt::format_to(std::back_inserter(out), "throw JSONTypeError(\"{}\", \"array\", unwrappedVal);\n", fieldName,
                       from);
        // Use else branch so we operate in new scope to avoid ArrayVar conflicts.
        fmt::format_to(std::back_inserter(out), "}} else {{\n");
        fmt::format_to(std::back_inserter(out), "{} {};\n", getCPPType(), arrayVar, componentType->getCPPType());
        fmt::format_to(std::back_inserter(out), "for (auto &element : unwrappedVal.GetArray()) {{\n", from);
        // Note: All of these 'emitFromJSONValue' functions expect an optional<> type.
        fmt::format_to(std::back_inserter(out),
                       "auto maybeElement = std::make_optional<const rapidjson::Value *>(&element);\n");
        componentType->emitFromJSONValue(out, "maybeElement", AssignDeserializedElementValue, fieldName);
        fmt::format_to(std::back_inserter(out), "}}\n");
        assign(out, fmt::format("std::move({})", arrayVar));
        fmt::format_to(std::back_inserter(out), "}}\n");
        fmt::format_to(std::back_inserter(out), "}}\n");
    }

    void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                         std::string_view fieldName) {
        // Create new scope so our variable names don't conflict with other serialized arrays in same
        // context.
        fmt::format_to(std::back_inserter(out), "{{\n");
        fmt::format_to(std::back_inserter(out), "rapidjson::Value {}(rapidjson::kArrayType);\n", arrayVar);
        fmt::format_to(std::back_inserter(out), "for (auto &element : {}) {{\n", from);
        componentType->emitToJSONValue(out, "element", AssignSerializedElementValue, fieldName);
        fmt::format_to(std::back_inserter(out), "}}\n");
        assign(out, arrayVar);
        fmt::format_to(std::back_inserter(out), "}}\n");
    }
};

class JSONIntEnumType final : public JSONClassType {
private:
    std::vector<std::pair<const std::string, int>> enumValues;

    std::string enumVar(std::string_view value) {
        return fmt::format("{}::{}", typeName, value);
    }

public:
    JSONIntEnumType(std::string_view typeName, std::vector<std::pair<const std::string, int>> enumValues)
        : JSONClassType(typeName), enumValues(enumValues) {}

    BaseKind getCPPBaseKind() const {
        return BaseKind::IntKind;
    }

    BaseKind getJSONBaseKind() const {
        return BaseKind::IntKind;
    }

    std::string getCPPType() const {
        return typeName;
    }

    std::string getJSONType() const {
        return fmt::format("{}", fmt::map_join(enumValues, " | ", [](auto enumValue) -> std::string {
                               return std::to_string(enumValue.second);
                           }));
    }

    void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                           std::string_view fieldName) {
        assign(out, fmt::format("tryConvertTo{}(tryConvertToInt({}, \"{}\"))", typeName, from, fieldName));
    }

    void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                         std::string_view fieldName) {
        // Running tryConvertTo[typeName] will check that the enum value is valid.
        assign(out, fmt::format("(int)tryConvertTo{}((int){})", typeName, from));
    }

    void emitDeclaration(fmt::memory_buffer &out) {
        fmt::format_to(std::back_inserter(out), "enum class {} {{\n", typeName);
        for (auto &value : enumValues) {
            fmt::format_to(std::back_inserter(out), "{} = {},\n", value.first, value.second);
        }
        fmt::format_to(std::back_inserter(out), "}};\n");
        fmt::format_to(std::back_inserter(out), "{0} tryConvertTo{0}(int value);\n", typeName);
    }

    void emitDefinition(fmt::memory_buffer &out) {
        fmt::format_to(std::back_inserter(out), "{0} tryConvertTo{0}(int value) {{\n", typeName);
        fmt::format_to(std::back_inserter(out), "switch (({})value) {{\n", typeName);
        for (auto &value : enumValues) {
            fmt::format_to(std::back_inserter(out), "case {}:\n", enumVar(value.first));
            fmt::format_to(std::back_inserter(out), "return {}::{};\n", typeName, value.first);
        }
        fmt::format_to(std::back_inserter(out), "default:\n");
        fmt::format_to(std::back_inserter(out), "throw InvalidEnumValueError(\"{}\", value);\n", typeName);
        fmt::format_to(std::back_inserter(out), "}}\n");
        fmt::format_to(std::back_inserter(out), "}}\n");
    }
};

class JSONStringEnumType final : public JSONClassType {
private:
    std::vector<const std::string> enumValues;

    // Capitalizes the first character of the input string (e.g., foo => Foo),
    // strips {'.','_','/'}, and capitalizes first letter after those characters.
    static std::string toIdentifier(std::string_view val) {
        // Split string into components.
        auto components = absl::StrSplit(val, absl::ByAnyChar("._/"));
        // Capitalize each component.
        return fmt::format("{}", fmt::map_join(components, "", [](auto component) -> std::string {
                               if (component.length() == 0) {
                                   return "";
                               }
                               return fmt::format("{}{}", std::string(1, toupper(component[0])), component.substr(1));
                           }));
    }

    std::string enumStrVar(std::string_view value) {
        return fmt::format("{}_{}", typeName, toIdentifier(value));
    }

    std::string enumVar(std::string_view value) {
        return fmt::format("{}::{}", typeName, toIdentifier(value));
    }

public:
    JSONStringEnumType(std::string_view typeName, std::vector<const std::string> enumValues)
        : JSONClassType(typeName), enumValues(enumValues) {}

    BaseKind getCPPBaseKind() const {
        return BaseKind::IntKind;
    }

    BaseKind getJSONBaseKind() const {
        return BaseKind::StringKind;
    }

    std::string getCPPType() const {
        return typeName;
    }

    std::string getJSONType() const {
        return fmt::format("{}", fmt::join(enumValues, " | "));
    }

    /**
     * Retrieve the enum value for the given raw string. Performs sanity checking.
     */
    std::string getEnumValue(std::string_view rawString) {
        auto it = std::find(enumValues.begin(), enumValues.end(), rawString);
        if (it == enumValues.end()) {
            throw std::invalid_argument(fmt::format("Enum {} does not contain string `{}`", getCPPType(), rawString));
        }
        return enumVar(rawString);
    }

    void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                           std::string_view fieldName) {
        assign(out, fmt::format("get{}(tryConvertToString({}, \"{}\"))", typeName, from, fieldName));
    }

    void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                         std::string_view fieldName) {
        JSONStringType::serializeStringToJSONValue(out, fmt::format("convert{}ToString({})", typeName, from), assign);
    }

    void emitDeclaration(fmt::memory_buffer &out) {
        fmt::format_to(std::back_inserter(out), "enum class {} {{\n", typeName);
        for (std::string_view value : enumValues) {
            fmt::format_to(std::back_inserter(out), "{},\n", toIdentifier(value));
        }
        fmt::format_to(std::back_inserter(out), "}};\n");
        fmt::format_to(std::back_inserter(out), "{0} get{0}(std::string_view value);\n", typeName);
        fmt::format_to(std::back_inserter(out), "const std::string &convert{0}ToString({0} kind);", typeName);
    }

    void emitDefinition(fmt::memory_buffer &out) {
        for (std::string_view value : enumValues) {
            fmt::format_to(std::back_inserter(out), "static const std::string {} = \"{}\";\n", enumStrVar(value),
                           value);
        }
        // Map from str => enum value to facilitate conversion.
        fmt::format_to(std::back_inserter(out), "static const UnorderedMap<std::string, {0}> StringTo{0} = {{\n",
                       typeName);
        for (std::string_view value : enumValues) {
            fmt::format_to(std::back_inserter(out), "{{{}, {}}},\n", enumStrVar(value), enumVar(value));
        }
        fmt::format_to(std::back_inserter(out), "}};\n");
        fmt::format_to(std::back_inserter(out), "{0} get{0}(std::string_view value) {{\n", typeName);
        fmt::format_to(std::back_inserter(out), "auto it = StringTo{}.find(std::string(value));\n", typeName);
        fmt::format_to(std::back_inserter(out), "if (it == StringTo{}.end()) {{\n", typeName);
        fmt::format_to(std::back_inserter(out), "throw InvalidStringEnumError(\"{}\", value);\n", typeName);
        fmt::format_to(std::back_inserter(out), "}}\n");
        fmt::format_to(std::back_inserter(out), "return it->second;\n");
        fmt::format_to(std::back_inserter(out), "}}\n");
        fmt::format_to(std::back_inserter(out), "const std::string &convert{0}ToString({0} kind) {{\n", typeName);
        fmt::format_to(std::back_inserter(out), "switch (kind) {{\n");
        for (std::string_view value : enumValues) {
            fmt::format_to(std::back_inserter(out), "case {}:\n", enumVar(value));
            fmt::format_to(std::back_inserter(out), "return {};\n", enumStrVar(value));
        }
        fmt::format_to(std::back_inserter(out), "default:\n");
        fmt::format_to(std::back_inserter(out), "throw InvalidEnumValueError(\"{}\", (int) kind);\n", typeName);
        fmt::format_to(std::back_inserter(out), "}}\n");
        fmt::format_to(std::back_inserter(out), "}}\n");
    }
};

class FieldDef final {
public:
    const std::string jsonName;
    const std::string cppName;
    std::shared_ptr<JSONType> type;

    FieldDef(std::string_view name, std::shared_ptr<JSONType> type)
        : jsonName(std::string(name)), cppName(std::string(name)), type(type) {}
    FieldDef(std::string_view jsonName, std::string_view cppName, std::shared_ptr<JSONType> type)
        : jsonName(jsonName), cppName(cppName), type(type) {}

    void emitDeclaration(fmt::memory_buffer &out) const {
        fmt::format_to(std::back_inserter(out), "{} {};\n", type->getCPPType(), cppName);
    }
};

class JSONOptionalType final : public JSONType {
private:
    std::shared_ptr<JSONType> innerType;

public:
    JSONOptionalType(std::shared_ptr<JSONType> innerType) : innerType(innerType) {}

    BaseKind getCPPBaseKind() const {
        return BaseKind::ComplexKind;
    }

    BaseKind getJSONBaseKind() const {
        return BaseKind::ComplexKind;
    }

    std::string getCPPType() const {
        return fmt::format("std::optional<{}>", innerType->getCPPType());
    }

    std::string getJSONType() const {
        return fmt::format("({})?", innerType->getJSONType());
    }

    bool wantMove() const {
        return innerType->wantMove();
    }

    void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                           std::string_view fieldName) {
        // Check for presence of field.
        // N.B.: Treat null fields as missing. Emacs fills in optional fields with `null` values.
        fmt::format_to(std::back_inserter(out), "if ({0} && !(*{0})->IsNull()) {{\n", from);
        const std::string innerCPPType = innerType->getCPPType();
        AssignLambda assignOptional = [innerCPPType, assign](fmt::memory_buffer &out, std::string_view from) -> void {
            assign(out, fmt::format("std::make_optional<{}>({})", innerCPPType, from));
        };
        innerType->emitFromJSONValue(out, from, assignOptional, fieldName);
        fmt::format_to(std::back_inserter(out), "}} else {{\n");
        // Ensures that optional is assigned to correct variant slot on variant types, since optional<Foo> !=
        // optional<Bar>.
        assign(out, fmt::format("std::optional<{}>(std::nullopt)", innerCPPType));
        fmt::format_to(std::back_inserter(out), "}}\n");
    }

    void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                         std::string_view fieldName) {
        fmt::format_to(std::back_inserter(out), "if ({}.has_value()) {{\n", from);
        // N.B.: Mac OSX does not support .value() on std::optional yet.
        // Dereferencing does the same thing, but does not check if the value is present.
        // But since we explicitly check `has_value()`, we're good here.
        // See: https://stackoverflow.com/a/44244070
        innerType->emitToJSONValue(out, fmt::format("(*{})", from), assign, fieldName);
        fmt::format_to(std::back_inserter(out), "}}\n");
    }
};

class JSONObjectType final : public JSONClassType {
private:
    std::vector<std::string> extraMethodDefinitions;
    std::vector<std::shared_ptr<FieldDef>> fieldDefs;
    std::vector<std::shared_ptr<FieldDef>> getRequiredFields() {
        std::vector<std::shared_ptr<FieldDef>> reqFields;
        // Filter out optional fields.
        std::copy_if(fieldDefs.begin(), fieldDefs.end(), std::back_inserter(reqFields),
                     [](auto &fieldDef) { return !dynamic_cast<JSONOptionalType *>(fieldDef->type.get()); });
        return reqFields;
    }

public:
    JSONObjectType(std::string_view typeName, std::vector<std::shared_ptr<FieldDef>> fieldDefs,
                   std::vector<std::string> extraMethodDefinitions)
        : JSONClassType(typeName), extraMethodDefinitions(extraMethodDefinitions), fieldDefs(fieldDefs) {}

    BaseKind getCPPBaseKind() const {
        return BaseKind::ObjectKind;
    }

    BaseKind getJSONBaseKind() const {
        return BaseKind::ObjectKind;
    }

    std::string getCPPType() const {
        return fmt::format("std::unique_ptr<{}>", typeName);
    }

    std::string getJSONType() const {
        return typeName;
    }

    bool wantMove() const {
        return true;
    }

    void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                           std::string_view fieldName) {
        assign(out,
               fmt::format("{0}::fromJSONValue(assertJSONField({1}, \"{2}\"), \"{2}\")", typeName, from, fieldName));
    }

    void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                         std::string_view fieldName) {
        fmt::format_to(std::back_inserter(out), "if ({} == nullptr) {{\n", from);
        fmt::format_to(std::back_inserter(out), "throw NullPtrError(\"{}\");\n", fieldName);
        fmt::format_to(std::back_inserter(out), "}}\n");
        assign(out, fmt::format("*({}->toJSONValue({}))", from, ALLOCATOR_VAR));
    }

    void emitDeclaration(fmt::memory_buffer &out) {
        fmt::format_to(std::back_inserter(out), "class {} final : public JSONBaseType {{\n", typeName);
        fmt::format_to(std::back_inserter(out), "public:\n");
        fmt::format_to(std::back_inserter(out),
                       "static {} fromJSONValue(const rapidjson::Value &val, std::string_view fieldName = "
                       "JSONBaseType::defaultFieldName);\n",
                       getCPPType());
        for (std::shared_ptr<FieldDef> &fieldDef : fieldDefs) {
            fieldDef->emitDeclaration(out);
        }
        auto reqFields = getRequiredFields();
        if (reqFields.size() > 0) {
            // Constructor. Only accepts non-optional fields as arguments
            fmt::format_to(std::back_inserter(out), "{}({});\n", typeName,
                           fmt::map_join(getRequiredFields(), ", ", [](auto field) -> std::string {
                               return fmt::format("{} {}", field->type->getCPPType(), field->cppName);
                           }));
        }
        fmt::format_to(
            std::back_inserter(out),
            "std::unique_ptr<rapidjson::Value> toJSONValue(rapidjson::MemoryPoolAllocator<> &alloc) const;\n");
        fmt::format_to(std::back_inserter(out), "{}\n",
                       fmt::join(extraMethodDefinitions.begin(), extraMethodDefinitions.end(), "\n"));
        fmt::format_to(std::back_inserter(out), "}};\n");
    }

    void emitDefinition(fmt::memory_buffer &out) {
        auto reqFields = getRequiredFields();
        if (reqFields.size() > 0) {
            fmt::format_to(std::back_inserter(out), "{}::{}({}): {} {{\n", typeName, typeName,
                           fmt::map_join(reqFields, ", ",
                                         [](auto field) -> std::string {
                                             return fmt::format("{} {}", field->type->getCPPType(), field->cppName);
                                         }),
                           fmt::map_join(reqFields, ", ", [](auto field) -> std::string {
                               if (field->type->wantMove()) {
                                   return fmt::format("{}(move({}))", field->cppName, field->cppName);
                               }
                               return fmt::format("{}({})", field->cppName, field->cppName);
                           }));
            fmt::format_to(std::back_inserter(out), "}}\n");
        }
        fmt::format_to(std::back_inserter(out),
                       "{} {}::fromJSONValue(const rapidjson::Value &val, std::string_view fieldName) {{\n",
                       getCPPType(), typeName);
        fmt::format_to(std::back_inserter(out), "if (!val.IsObject()) {{\n");
        fmt::format_to(std::back_inserter(out), "throw JSONTypeError(fieldName, \"object\", val);\n");
        fmt::format_to(std::back_inserter(out), "}}\n");

        // Process required fields first.
        for (std::shared_ptr<FieldDef> &fieldDef : reqFields) {
            std::string fieldName = fmt::format("{}.{}", typeName, fieldDef->cppName);
            fmt::format_to(std::back_inserter(out), "auto rapidjson{} = maybeGetJSONField(val, \"{}\");\n",
                           fieldDef->cppName, fieldDef->jsonName);
            fmt::format_to(std::back_inserter(out), "{} {};\n", fieldDef->type->getCPPType(), fieldDef->cppName);
            AssignLambda assign = [&fieldDef](fmt::memory_buffer &out, std::string_view from) -> void {
                fmt::format_to(std::back_inserter(out), "{} = {};\n", fieldDef->cppName, from);
            };
            fieldDef->type->emitFromJSONValue(out, fmt::format("rapidjson{}", fieldDef->cppName), assign, fieldName);
        }
        fmt::format_to(std::back_inserter(out), "{} rv = std::make_unique<{}>({});\n", getCPPType(), typeName,
                       fmt::map_join(reqFields, ", ", [](auto field) -> std::string {
                           if (field->type->wantMove()) {
                               return fmt::format("move({})", field->cppName);
                           } else {
                               return field->cppName;
                           }
                       }));

        // Assign optionally specified fields.
        for (std::shared_ptr<FieldDef> &fieldDef : fieldDefs) {
            if (dynamic_cast<JSONOptionalType *>(fieldDef->type.get())) {
                std::string fieldName = fmt::format("{}.{}", typeName, fieldDef->cppName);
                fmt::format_to(std::back_inserter(out), "auto rapidjson{} = maybeGetJSONField(val, \"{}\");\n",
                               fieldDef->cppName, fieldDef->jsonName);
                AssignLambda assign = [&fieldDef](fmt::memory_buffer &out, std::string_view from) -> void {
                    fmt::format_to(std::back_inserter(out), "rv->{} = {};\n", fieldDef->cppName, from);
                };
                fieldDef->type->emitFromJSONValue(out, fmt::format("rapidjson{}", fieldDef->cppName), assign,
                                                  fieldName);
            }
        }
        fmt::format_to(std::back_inserter(out), "return rv;\n");
        fmt::format_to(std::back_inserter(out), "}}\n");

        fmt::format_to(std::back_inserter(out),
                       "std::unique_ptr<rapidjson::Value> {}::toJSONValue(rapidjson::MemoryPoolAllocator<> "
                       "&{}) const {{\n",
                       typeName, ALLOCATOR_VAR);
        fmt::format_to(std::back_inserter(out),
                       "auto rv = std::make_unique<rapidjson::Value>(rapidjson::kObjectType);\n");
        for (std::shared_ptr<FieldDef> &fieldDef : fieldDefs) {
            std::string fieldName = fmt::format("{}.{}", typeName, fieldDef->cppName);
            AssignLambda assign = [&fieldDef](fmt::memory_buffer &out, std::string_view from) -> void {
                fmt::format_to(std::back_inserter(out), "rv->AddMember(\"{}\", {}, {});\n", fieldDef->jsonName, from,
                               ALLOCATOR_VAR);
            };
            fieldDef->type->emitToJSONValue(out, fieldDef->cppName, assign, fieldName);
        }
        fmt::format_to(std::back_inserter(out), "return rv;\n");
        fmt::format_to(std::back_inserter(out), "}}\n");
    }

    /**
     * Add in a field post-definition. Used to support
     * object types that have fields of their own type.
     */
    void addField(std::shared_ptr<FieldDef> field) {
        fieldDefs.push_back(field);
    }
};

/**
 * Abstract class. Implements basic functionality for any field that can contain one or more different types of data.
 */
class JSONVariantType : public JSONType {
protected:
    std::vector<std::shared_ptr<JSONType>> variants;

public:
    JSONVariantType(std::vector<std::shared_ptr<JSONType>> variants) : variants(variants) {}

    BaseKind getCPPBaseKind() const {
        return BaseKind::ComplexKind;
    }

    BaseKind getJSONBaseKind() const {
        return BaseKind::ComplexKind;
    }

    std::string getCPPType() const {
        // Variants cannot contain duplicate types, so dedupe the CPP types.
        // Have order match order in `variants` (which matches declaration order) to avoid surprises.
        UnorderedSet<std::string> uniqueTypes;
        std::vector<std::string> emitOrder;
        for (auto &variant : variants) {
            auto cppType = variant->getCPPType();
            if (!uniqueTypes.contains(cppType)) {
                uniqueTypes.insert(cppType);
                emitOrder.push_back(cppType);
            }
        }
        return fmt::format("std::variant<{}>", fmt::join(emitOrder, ","));
    }

    std::string getJSONType() const {
        return fmt::format(
            "{}", fmt::map_join(variants, " | ", [](auto variant) -> std::string { return variant->getJSONType(); }));
    }

    bool wantMove() const {
        for (auto &variant : variants) {
            if (variant->wantMove()) {
                return true;
            }
        }
        return false;
    }
};

/**
 * A 'discriminated union' type is a variant type where some other field on the object
 * determines its true type.
 */
class JSONDiscriminatedUnionVariantType final : public JSONVariantType {
private:
    std::shared_ptr<FieldDef> fieldDef;
    const std::vector<std::pair<const std::string, std::shared_ptr<JSONType>>> variantsByDiscriminant;

    static std::vector<std::shared_ptr<JSONType>>
    getVariantTypes(const std::vector<std::pair<const std::string, std::shared_ptr<JSONType>>> &variants) {
        std::vector<std::shared_ptr<JSONType>> rv;
        rv.reserve(variants.size());
        for (auto &variant : variants) {
            rv.push_back(variant.second);
        }
        return rv;
    }

    JSONStringEnumType *getDiscriminantType() {
        auto enumType = dynamic_cast<JSONStringEnumType *>(fieldDef->type.get());
        if (!enumType) {
            throw std::invalid_argument("The discriminant for a discriminated union must be a string enum.");
        }
        return enumType;
    }

public:
    JSONDiscriminatedUnionVariantType(
        std::shared_ptr<FieldDef> fieldDef,
        const std::vector<std::pair<const std::string, std::shared_ptr<JSONType>>> variantsByDiscriminant)
        : JSONVariantType(getVariantTypes(variantsByDiscriminant)), fieldDef(fieldDef),
          variantsByDiscriminant(variantsByDiscriminant) {}

    void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                           std::string_view fieldName) {
        auto enumType = getDiscriminantType();
        fmt::format_to(std::back_inserter(out), "switch ({}) {{\n", fieldDef->cppName);
        for (auto &variant : variantsByDiscriminant) {
            // getEnumValue will throw if the discriminant value is not in the enum.
            fmt::format_to(std::back_inserter(out), "case {}:\n", enumType->getEnumValue(variant.first));
            variant.second->emitFromJSONValue(out, from, assign, fieldName);
            fmt::format_to(std::back_inserter(out), "break;\n");
        }
        fmt::format_to(std::back_inserter(out), "default:\n");
        fmt::format_to(std::back_inserter(out),
                       "throw InvalidDiscriminantValueError(\"{0}\", \"{1}\", convert{2}ToString({1}));\n", fieldName,
                       fieldDef->cppName, enumType->getCPPType());
        fmt::format_to(std::back_inserter(out), "}}\n");
    }

    void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                         std::string_view fieldName) {
        auto enumType = getDiscriminantType();
        fmt::format_to(std::back_inserter(out), "switch ({}) {{\n", fieldDef->cppName);
        for (auto &variant : variantsByDiscriminant) {
            // getEnumValue will throw if the discriminant value is not in the enum.
            fmt::format_to(std::back_inserter(out), "case {}:\n", enumType->getEnumValue(variant.first));
            fmt::format_to(std::back_inserter(out), "if (auto discVal = std::get_if<{}>(&{})) {{\n",
                           variant.second->getCPPType(), from);
            variant.second->emitToJSONValue(out, "(*discVal)", assign, fieldName);
            fmt::format_to(std::back_inserter(out), "}} else {{\n");
            fmt::format_to(
                std::back_inserter(out),
                "throw InvalidDiscriminatedUnionValueError(\"{0}\", \"{1}\", convert{2}ToString({1}), \"{3}\");\n",
                fieldName, fieldDef->cppName, enumType->getCPPType(), variant.second->getCPPType());
            fmt::format_to(std::back_inserter(out), "}}\n");
            fmt::format_to(std::back_inserter(out), "break;\n");
        }
        fmt::format_to(std::back_inserter(out), "default:\n");
        fmt::format_to(std::back_inserter(out),
                       "throw InvalidDiscriminantValueError(\"{0}\", \"{1}\", convert{2}ToString({1}));\n", fieldName,
                       fieldDef->cppName, enumType->getCPPType());
        fmt::format_to(std::back_inserter(out), "}}\n");
    }
};

class JSONBasicVariantType final : public JSONVariantType {
public:
    JSONBasicVariantType(std::vector<std::shared_ptr<JSONType>> variants) : JSONVariantType(variants) {
        // Check that we have at most one of every kind & do not have any complex types.
        UnorderedSet<BaseKind> cppKindSeen;
        UnorderedSet<BaseKind> jsonKindSeen;
        for (std::shared_ptr<JSONType> variant : variants) {
            if (variant->getCPPBaseKind() == BaseKind::ComplexKind ||
                variant->getJSONBaseKind() == BaseKind::ComplexKind) {
                throw std::invalid_argument("Invalid variant type: Complex are not supported.");
            }
            if (cppKindSeen.contains(variant->getCPPBaseKind())) {
                throw std::invalid_argument(
                    "Invalid variant type: Cannot discriminate between multiple types with same base C++ kind.");
            }
            cppKindSeen.insert(variant->getCPPBaseKind());
            if (jsonKindSeen.contains(variant->getJSONBaseKind())) {
                throw std::invalid_argument(
                    "Invalid variant type: Cannot discriminate between multiple types with same base JSON kind.");
            }
            jsonKindSeen.insert(variant->getJSONBaseKind());
        }
    }

    void emitFromJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                           std::string_view fieldName) {
        fmt::format_to(std::back_inserter(out), "{{\n");
        fmt::format_to(std::back_inserter(out), "auto &unwrappedValue = assertJSONField({}, \"{}\");", from, fieldName);
        bool first = true;
        for (std::shared_ptr<JSONType> variant : variants) {
            std::string checkMethod;
            switch (variant->getJSONBaseKind()) {
                case BaseKind::NullKind:
                    checkMethod = "IsNull";
                    break;
                case BaseKind::BooleanKind:
                    checkMethod = "IsBool";
                    break;
                case BaseKind::IntKind:
                    checkMethod = "IsInt";
                    break;
                case BaseKind::DoubleKind:
                    // N.B.: IsDouble() returns false for integers.
                    // We only care that the value is convertible to double, which is what IsNumber tests.
                    checkMethod = "IsNumber";
                    break;
                case BaseKind::StringKind:
                    checkMethod = "IsString";
                    break;
                case BaseKind::ObjectKind:
                    checkMethod = "IsObject";
                    break;
                case BaseKind::ArrayKind:
                    checkMethod = "IsArray";
                    break;
                default:
                    throw std::invalid_argument("Invalid kind for variant type.");
            }
            auto condition = fmt::format("unwrappedValue.{}()", checkMethod);
            if (first) {
                first = false;
                fmt::format_to(std::back_inserter(out), "if ({}) {{\n", condition);
            } else {
                fmt::format_to(std::back_inserter(out), "}} else if ({}) {{\n", condition);
            }
            variant->emitFromJSONValue(out, from, assign, fieldName);
        }
        fmt::format_to(std::back_inserter(out), "}} else {{\n");
        fmt::format_to(std::back_inserter(out), "throw JSONTypeError(\"{}\", \"{}\", unwrappedValue);\n", fieldName,
                       sorbet::JSON::escape(getJSONType()));
        fmt::format_to(std::back_inserter(out), "}}\n");
        fmt::format_to(std::back_inserter(out), "}}\n");
    }

    void emitToJSONValue(fmt::memory_buffer &out, std::string_view from, AssignLambda assign,
                         std::string_view fieldName) {
        bool first = true;
        for (std::shared_ptr<JSONType> variant : variants) {
            auto condition = fmt::format("auto val = std::get_if<{}>(&{})", variant->getCPPType(), from);
            if (first) {
                first = false;
                fmt::format_to(std::back_inserter(out), "if ({}) {{\n", condition);
            } else {
                fmt::format_to(std::back_inserter(out), "}} else if ({}) {{\n", condition);
            }
            variant->emitToJSONValue(out, "(*val)", assign, fieldName);
        }
        fmt::format_to(std::back_inserter(out), "}} else {{\n");
        fmt::format_to(std::back_inserter(out), "throw MissingVariantValueError(\"{}\");\n", fieldName);
        fmt::format_to(std::back_inserter(out), "}}\n");
    }
};

#endif // GENERATE_LSP_MESSAGES_H
