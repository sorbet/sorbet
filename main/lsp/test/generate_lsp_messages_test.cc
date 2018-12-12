#include "gtest/gtest.h"

#include "common/common.h"
#include "main/lsp/json_types.h"

using namespace sorbet::realmain::lsp;

namespace sorbet::realmain::lsp::test {

template <typename T> using ParseTestLambda = std::function<void(std::unique_ptr<JSONDocument<T>> &)>;

/**
 * Using jsonStr, creates two versions of the same document:
 * - One created by parsing jsonStr.
 * - One created by re-emitting JSON from the parsed jsonStr, and re-parsing it.
 * It then calls lambda with each, ensuring that any assertions it makes
 * passes on the parsed and re-parsed document.
 */
template <typename T> void parseTest(const std::string &jsonStr, ParseTestLambda<T> lambda) {
    auto originalDoc = T::fromJSON(jsonStr);
    lambda(originalDoc);
    auto reparsedDoc = T::fromJSON(originalDoc->root->toJSON());
    lambda(reparsedDoc);
};

const std::string SAMPLE_RANGE =
    "{\"start\": {\"line\": 0, \"character\": 1}, \"end\": {\"line\": 2, \"character\": 3}}";

// N.B.: Also tests integer fields.
TEST(GenerateLSPMessagesTest, Object) {
    parseTest<Range>(SAMPLE_RANGE, [](auto &doc) -> void {
        auto &range = doc->root;
        ASSERT_EQ(range->start->line, 0);
        ASSERT_EQ(range->start->character, 1);
        ASSERT_EQ(range->end->line, 2);
        ASSERT_EQ(range->end->character, 3);
    });

    // Throws when missing a field.
    ASSERT_THROW(Range::fromJSON("{\"start\": {\"line\": 0, \"character\": 1}, \"end\": {\"line\": 2}}"),
                 MissingFieldError);
    // Throws when not an object.
    ASSERT_THROW(Range::fromJSON("4"), JSONTypeError);
    // Throws when field does not contain a number
    ASSERT_THROW(
        Range::fromJSON("{\"start\": {\"line\": 0, \"character\": true}, \"end\": {\"line\": 2, \"character\": 3}}"),
        JSONTypeError);
    // Throws when field contains a double, not an int.
    ASSERT_THROW(
        Range::fromJSON("{\"start\": {\"line\": 0, \"character\": 1.1}, \"end\": {\"line\": 2, \"character\": 3}}"),
        JSONTypeError);

    // Serialization: Throws if sub-objects are not initialized.
    auto badRange = std::make_unique<Range>();
    ASSERT_THROW(badRange->toJSON(), NullPtrError);
}

TEST(GenerateLSPMessagesTest, StringField) {
    const std::string expectedText = "Hello World!";
    parseTest<TextEdit>(fmt::format("{{\"range\": {}, \"newText\": \"{}\"}}", SAMPLE_RANGE, expectedText),
                        [&expectedText](auto &doc) -> void {
                            auto &textEdit = doc->root;
                            ASSERT_EQ(textEdit->newText, expectedText);
                        });

    // Throws when not a string
    ASSERT_THROW(TextEdit::fromJSON(fmt::format("{{\"range\": {}, \"newText\": 4.0}}", SAMPLE_RANGE)), JSONTypeError);
}

TEST(GenerateLSPMessagesTest, StringEnumField) {
    const std::string markupKind = "markdown";
    parseTest<MarkupContent>(fmt::format("{{\"kind\": \"{}\", \"value\": \"Markup stuff\"}}", markupKind),
                             [](auto &doc) -> void {
                                 auto &markupContent = doc->root;
                                 ASSERT_EQ(markupContent->kind, MarkupKind::Markdown);
                             });

    // Throws when not a valid enum.
    ASSERT_THROW(MarkupContent::fromJSON("{\"kind\": \"foobar\", \"value\": \"Hello\"}"), InvalidStringEnumError);
    // Throws when not a string.
    ASSERT_THROW(MarkupContent::fromJSON("{\"kind\": 4, \"value\": \"Hello\"}"), JSONTypeError);

    // Create a C++ object with an invalid enum value and try to serialize.
    auto markupContent = std::make_unique<MarkupContent>();
    markupContent->kind = (MarkupKind)1000;
    ASSERT_THROW(markupContent->toJSON(), InvalidEnumValueError);
}

TEST(GenerateLSPMessagesTest, NullField) {
    parseTest<VersionedTextDocumentIdentifier>("{\"uri\": \"file://foo\", \"version\": null}", [](auto &doc) -> void {
        auto &versionedTextDocumentIdentifier = doc->root;
        auto nullValue = std::get_if<JSONNullObject>(&(versionedTextDocumentIdentifier->version));
        // Should not be null; should point to an instance of JSONNullObject.
        ASSERT_NE(nullValue, nullptr);
    });
}

// N.B.: Also covers testing boolean types, which are treated as optional almost everywhere in the spec.
TEST(GenerateLSPMessagesTest, OptionalField) {
    parseTest<CreateOrRenameFileOptions>("{\"overwrite\": true}", [](auto &doc) -> void {
        auto &createOrRenameFileOptions = doc->root;
        ASSERT_TRUE(createOrRenameFileOptions->overwrite.has_value());
        ASSERT_FALSE(createOrRenameFileOptions->ignoreIfExists.has_value());
        ASSERT_TRUE(*(createOrRenameFileOptions->overwrite));
    });

    parseTest<CreateOrRenameFileOptions>("{}", [](auto &doc) -> void {
        auto &createOrRenameFileOptions = doc->root;
        ASSERT_FALSE(createOrRenameFileOptions->overwrite.has_value());
    });

    // Throws when not the correct type.
    ASSERT_THROW(CreateOrRenameFileOptions::fromJSON("{\"overwrite\": 4}"), JSONTypeError);
}

struct ExceptionThrower {
    operator double() {
        throw runtime_error("Nope");
    }
};

TEST(GenerateLSPMessagesTest, DoubleField) {
    // Doubles can be ints or doubles.
    parseTest<Color>("{\"red\": 0, \"green\": 1.1, \"blue\": 2.0, \"alpha\": 3}", [](auto &doc) -> void {
        auto &color = doc->root;
        ASSERT_EQ(0.0, color->red);
        ASSERT_EQ(1.1, color->green);
        ASSERT_EQ(2.0, color->blue);
        ASSERT_EQ(3.0, color->alpha);
    });
}

TEST(GenerateLSPMessagesTest, VariantField) {
    parseTest<CancelParams>("{\"id\": 4}", [](auto &docNumber) -> void {
        auto &cancelParamsNumber = docNumber->root;
        auto numberId = std::get_if<int>(&cancelParamsNumber->id);
        ASSERT_NE(numberId, nullptr);
        ASSERT_EQ(*numberId, 4);
        ASSERT_EQ(std::get_if<std::string>(&cancelParamsNumber->id), nullptr);
    });

    parseTest<CancelParams>("{\"id\": \"iamanid\"}", [](auto &docString) -> void {
        auto &cancelParamsString = docString->root;
        auto stringId = std::get_if<std::string>(&cancelParamsString->id);
        ASSERT_NE(stringId, nullptr);
        ASSERT_EQ(*stringId, "iamanid");
        ASSERT_EQ(std::get_if<int>(&cancelParamsString->id), nullptr);
    });

    // Throws when missing.
    ASSERT_THROW(CancelParams::fromJSON("{}"), MissingFieldError);

    // Throws when not the correct type.
    ASSERT_THROW(CancelParams::fromJSON("{\"id\": true}"), JSONTypeError);

    // Int types cannot be doubles.
    ASSERT_THROW(CancelParams::fromJSON("{\"id\": 4.1}"), JSONTypeError);

    // Create CancelParams with a variant field in an erroneous state.
    // See https://en.cppreference.com/w/cpp/utility/variant/valueless_by_exception
    auto cancelParams = std::make_unique<CancelParams>();
    try {
        cancelParams->id.emplace<int>(ExceptionThrower());
    } catch (runtime_error e) {
    }
    ASSERT_THROW(cancelParams->toJSON(), MissingVariantValueError);
}

TEST(GenerateLSPMessagesTest, AnyArray) {
    parseTest<Command>("{\"title\": \"\", \"command\": \"\", \"arguments\": [0, true, \"foo\"]}",
                       [](auto &doc) -> void {
                           auto &msg = doc->root;
                           auto &argsOptional = msg->arguments;
                           ASSERT_TRUE(argsOptional.has_value());
                           auto &args = *argsOptional;
                           ASSERT_EQ(args.size(), 3);
                           ASSERT_TRUE(args.at(0)->IsNumber());
                           ASSERT_TRUE(args.at(1)->IsBool());
                           ASSERT_TRUE(args.at(2)->IsString());
                       });

    // Must be an array.
    ASSERT_THROW(Command::fromJSON("{\"title\": \"\", \"command\": \"\", \"arguments\": {}}"), JSONTypeError);
}

std::string makeNotificationMessage(const std::string &params) {
    return fmt::format("{{\"jsonrpc\": \"2.0\", \"method\": \"blah\", \"params\": {}}}", params);
}

TEST(GenerateLSPMessagesTest, AnyObject) {
    parseTest<NotificationMessage>(makeNotificationMessage("{\"jim\": \"henson\"}"), [](auto &doc) -> void {
        auto &msg = doc->root;
        auto &paramsOptional = msg->params;
        ASSERT_TRUE(paramsOptional.has_value());
        auto &paramsVariant = *paramsOptional;
        auto paramsPtr = std::get_if<std::unique_ptr<rapidjson::Value>>(&paramsVariant);
        ASSERT_NE(paramsPtr, nullptr);
        auto &params = *paramsPtr;
        ASSERT_TRUE(params->IsObject());
    });

    // Deserialization: Must be an object.
    ASSERT_THROW(NotificationMessage::fromJSON(makeNotificationMessage("true")), JSONTypeError);

    // Serialization: Must be an object.
    // Null pointer case
    auto notificationMessage = std::make_unique<NotificationMessage>();
    notificationMessage->jsonrpc = "2.0";
    notificationMessage->params = nullptr;
    ASSERT_THROW(notificationMessage->toJSON(), NullPtrError);

    // Non-object case
    notificationMessage->params = std::make_unique<rapidjson::Value>(rapidjson::kNullType);
    ASSERT_THROW(notificationMessage->toJSON(), InvalidTypeError);

    // New object case -- doesn't throw and stresses supported APIs for making values.
    auto doc = NotificationMessage::fromJSON(makeNotificationMessage("{}"));
    auto range = std::make_unique<Position>();
    doc->root->params = range->toJSONValue(doc);
    ASSERT_NO_THROW(doc->root->toJSON());
}

TEST(GenerateLSPMessagesTest, StringConstant) {
    parseTest<CreateFile>("{\"kind\": \"create\", \"uri\": \"file://foo\"}", [](auto &doc) -> void {
        auto &createFile = doc->root;
        ASSERT_EQ(createFile->kind, "create");
    });

    // Throws when not the correct constant.
    ASSERT_THROW(CreateFile::fromJSON("{\"kind\": \"delete\", \"uri\": \"file://foo\"}"), JSONConstantError);
    // Throws when not a string.
    ASSERT_THROW(CreateFile::fromJSON("{\"kind\": 4, \"uri\": \"file://foo\"}"), JSONTypeError);

    // Throws during serialization if not set to proper constant value.
    auto createFile = std::make_unique<CreateFile>();
    createFile->kind = "delete";
    ASSERT_THROW(createFile->toJSON(), InvalidConstantValueError);
}

TEST(GenerateLSPMessagesTest, JSONArray) {
    parseTest<SymbolKindOptions>("{\"valueSet\": [1,2,3,4,5,6]}", [](auto &doc) -> void {
        auto &symbolKindOptions = doc->root;
        auto &valueSetOptional = symbolKindOptions->valueSet;
        ASSERT_TRUE(valueSetOptional.has_value());
        auto &valueSetUniquePtr = *valueSetOptional;
        int start = 1;
        for (const SymbolKind &value : valueSetUniquePtr) {
            ASSERT_EQ((const int &)value, start);
            start += 1;
        }
    });

    // Throws when not an array.
    ASSERT_THROW(SymbolKindOptions::fromJSON("{\"valueSet\": null}"), JSONTypeError);

    // Throws when a member of array has an invalid type.
    ASSERT_THROW(SymbolKindOptions::fromJSON("{\"valueSet\": [1,2,true,4]}"), JSONTypeError);
}

TEST(GenerateLSPMessagesTest, IntEnums) {
    parseTest<SymbolKindOptions>(
        fmt::format("{{\"valueSet\": [{},{}]}}", (int)SymbolKind::Namespace, (int)SymbolKind::Null),
        [](auto &doc) -> void {
            auto &symbolKindOptions = doc->root;
            auto &valueSetOptional = symbolKindOptions->valueSet;
            ASSERT_TRUE(valueSetOptional.has_value());
            auto &valueSet = *valueSetOptional;
            ASSERT_EQ(valueSet.size(), 2);
            ASSERT_EQ(valueSet.at(0), SymbolKind::Namespace);
            ASSERT_EQ(valueSet.at(1), SymbolKind::Null);
        });

    // Throws if enum is out of valid range.
    ASSERT_THROW(SymbolKindOptions::fromJSON("{\"valueSet\": [1,2,-1,10]}"), InvalidEnumValueError);

    // Throws if enum is not the right type.
    ASSERT_THROW(SymbolKindOptions::fromJSON("{\"valueSet\": [1,2.1]}"), JSONTypeError);

    // Throws during serialization if enum is out of valid range.
    auto symbolKind = std::make_unique<SymbolKindOptions>();
    auto symbols = std::vector<SymbolKind>();
    symbols.push_back(SymbolKind::Namespace);
    symbols.push_back((SymbolKind)-1);
    symbolKind->valueSet = std::make_optional<std::vector<SymbolKind>>(std::move(symbols));
    ASSERT_THROW(symbolKind->toJSON(), InvalidEnumValueError);
}

} // namespace sorbet::realmain::lsp::test