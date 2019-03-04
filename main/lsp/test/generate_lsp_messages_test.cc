#include "gtest/gtest.h"

#include "common/common.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/json_types.h"

using namespace std;
using namespace sorbet::realmain::lsp;

namespace sorbet::realmain::lsp::test {

template <typename T> using ParseTestLambda = function<void(std::unique_ptr<T> &)>;

/**
 * Using jsonStr, creates two versions of the same document:
 * - One created by parsing jsonStr.
 * - One created by re-emitting JSON from the parsed jsonStr, and re-parsing it.
 * It then calls lambda with each, ensuring that any assertions it makes
 * passes on the parsed and re-parsed document.
 */
template <typename T>
void parseTest(rapidjson::MemoryPoolAllocator<> &alloc, const string &jsonStr, ParseTestLambda<T> lambda) {
    auto original = T::fromJSON(alloc, jsonStr);
    lambda(original);
    auto reparsed = T::fromJSON(alloc, original->toJSON());
    lambda(reparsed);
};

const string SAMPLE_RANGE = "{\"start\": {\"line\": 0, \"character\": 1}, \"end\": {\"line\": 2, \"character\": 3}}";
rapidjson::MemoryPoolAllocator<> alloc;

// N.B.: Also tests integer fields.
TEST(GenerateLSPMessagesTest, Object) {
    parseTest<Range>(alloc, SAMPLE_RANGE, [](auto &range) -> void {
        ASSERT_EQ(range->start->line, 0);
        ASSERT_EQ(range->start->character, 1);
        ASSERT_EQ(range->end->line, 2);
        ASSERT_EQ(range->end->character, 3);
    });

    // Throws when missing a field.
    ASSERT_THROW(Range::fromJSON(alloc, "{\"start\": {\"line\": 0, \"character\": 1}, \"end\": {\"line\": 2}}"),
                 MissingFieldError);
    // Throws when not an object.
    ASSERT_THROW(Range::fromJSON(alloc, "4"), JSONTypeError);
    // Throws when field does not contain a number
    ASSERT_THROW(
        Range::fromJSON(alloc,
                        "{\"start\": {\"line\": 0, \"character\": true}, \"end\": {\"line\": 2, \"character\": 3}}"),
        JSONTypeError);
    // Throws when field contains a double, not an int.
    ASSERT_THROW(Range::fromJSON(
                     alloc, "{\"start\": {\"line\": 0, \"character\": 1.1}, \"end\": {\"line\": 2, \"character\": 3}}"),
                 JSONTypeError);

    // Serialization: Throws if sub-objects are not initialized.
    auto badRange = make_unique<Range>(nullptr, nullptr);
    ASSERT_THROW(badRange->toJSON(), NullPtrError);
}

TEST(GenerateLSPMessagesTest, StringField) {
    const string expectedText = "Hello World!";
    parseTest<TextEdit>(alloc, fmt::format("{{\"range\": {}, \"newText\": \"{}\"}}", SAMPLE_RANGE, expectedText),
                        [&expectedText](auto &textEdit) -> void { ASSERT_EQ(textEdit->newText, expectedText); });

    // Throws when not a string
    ASSERT_THROW(TextEdit::fromJSON(alloc, fmt::format("{{\"range\": {}, \"newText\": 4.0}}", SAMPLE_RANGE)),
                 JSONTypeError);
}

TEST(GenerateLSPMessagesTest, StringEnumField) {
    const string markupKind = "markdown";
    parseTest<MarkupContent>(alloc, fmt::format("{{\"kind\": \"{}\", \"value\": \"Markup stuff\"}}", markupKind),
                             [](auto &markupContent) -> void { ASSERT_EQ(markupContent->kind, MarkupKind::Markdown); });

    // Throws when not a valid enum.
    ASSERT_THROW(MarkupContent::fromJSON(alloc, "{\"kind\": \"foobar\", \"value\": \"Hello\"}"),
                 InvalidStringEnumError);
    // Throws when not a string.
    ASSERT_THROW(MarkupContent::fromJSON(alloc, "{\"kind\": 4, \"value\": \"Hello\"}"), JSONTypeError);

    // Create a C++ object with an invalid enum value and try to serialize.
    auto markupContent = make_unique<MarkupContent>((MarkupKind)1000, "hello");
    ASSERT_THROW(markupContent->toJSON(), InvalidEnumValueError);
}

TEST(GenerateLSPMessagesTest, NullField) {
    parseTest<VersionedTextDocumentIdentifier>(
        alloc, "{\"uri\": \"file://foo\", \"version\": null}", [](auto &versionedTextDocumentIdentifier) -> void {
            auto nullValue = get_if<JSONNullObject>(&(versionedTextDocumentIdentifier->version));
            // Should not be null; should point to an instance of JSONNullObject.
            ASSERT_NE(nullValue, nullptr);
        });
}

// N.B.: Also covers testing boolean types, which are treated as optional almost everywhere in the spec.
TEST(GenerateLSPMessagesTest, OptionalField) {
    parseTest<CreateOrRenameFileOptions>(alloc, "{\"overwrite\": true}", [](auto &createOrRenameFileOptions) -> void {
        ASSERT_TRUE(createOrRenameFileOptions->overwrite.has_value());
        ASSERT_FALSE(createOrRenameFileOptions->ignoreIfExists.has_value());
        ASSERT_TRUE(*(createOrRenameFileOptions->overwrite));
    });

    parseTest<CreateOrRenameFileOptions>(alloc, "{}", [](auto &createOrRenameFileOptions) -> void {
        ASSERT_FALSE(createOrRenameFileOptions->overwrite.has_value());
    });

    // Throws when not the correct type.
    ASSERT_THROW(CreateOrRenameFileOptions::fromJSON(alloc, "{\"overwrite\": 4}"), JSONTypeError);
}

struct ExceptionThrower {
    operator double() {
        throw runtime_error("Nope");
    }
};

TEST(GenerateLSPMessagesTest, DoubleField) {
    // Doubles can be ints or doubles.
    parseTest<Color>(alloc, "{\"red\": 0, \"green\": 1.1, \"blue\": 2.0, \"alpha\": 3}", [](auto &color) -> void {
        ASSERT_EQ(0.0, color->red);
        ASSERT_EQ(1.1, color->green);
        ASSERT_EQ(2.0, color->blue);
        ASSERT_EQ(3.0, color->alpha);
    });
}

TEST(GenerateLSPMessagesTest, VariantField) {
    parseTest<CancelParams>(alloc, "{\"id\": 4}", [](auto &cancelParamsNumber) -> void {
        auto numberId = get_if<int>(&cancelParamsNumber->id);
        ASSERT_NE(numberId, nullptr);
        ASSERT_EQ(*numberId, 4);
        ASSERT_EQ(get_if<std::string>(&cancelParamsNumber->id), nullptr);
    });

    parseTest<CancelParams>(alloc, "{\"id\": \"iamanid\"}", [](auto &cancelParamsString) -> void {
        auto stringId = get_if<std::string>(&cancelParamsString->id);
        ASSERT_NE(stringId, nullptr);
        ASSERT_EQ(*stringId, "iamanid");
        ASSERT_EQ(get_if<int>(&cancelParamsString->id), nullptr);
    });

    // Throws when missing.
    ASSERT_THROW(CancelParams::fromJSON(alloc, "{}"), MissingFieldError);

    // Throws when not the correct type.
    ASSERT_THROW(CancelParams::fromJSON(alloc, "{\"id\": true}"), JSONTypeError);

    // Int types cannot be doubles.
    ASSERT_THROW(CancelParams::fromJSON(alloc, "{\"id\": 4.1}"), JSONTypeError);

    // Create CancelParams with a variant field in an erroneous state.
    // See https://en.cppreference.com/w/cpp/utility/variant/valueless_by_exception
    auto cancelParams = make_unique<CancelParams>(variant<int, std::string>());
    try {
        cancelParams->id.emplace<int>(ExceptionThrower());
    } catch (runtime_error e) {
    }
    ASSERT_THROW(cancelParams->toJSON(), MissingVariantValueError);
}

TEST(GenerateLSPMessagesTest, AnyArray) {
    parseTest<Command>(alloc, "{\"title\": \"\", \"command\": \"\", \"arguments\": [0, true, \"foo\"]}",
                       [](auto &msg) -> void {
                           auto &argsOptional = msg->arguments;
                           ASSERT_TRUE(argsOptional.has_value());
                           auto &args = *argsOptional;
                           ASSERT_EQ(args.size(), 3);
                           ASSERT_TRUE(args.at(0)->IsNumber());
                           ASSERT_TRUE(args.at(1)->IsBool());
                           ASSERT_TRUE(args.at(2)->IsString());
                       });

    // Must be an array.
    ASSERT_THROW(Command::fromJSON(alloc, "{\"title\": \"\", \"command\": \"\", \"arguments\": {}}"), JSONTypeError);
}

string makeNotificationMessage(string_view params) {
    return fmt::format("{{\"jsonrpc\": \"2.0\", \"method\": \"blah\", \"params\": {}}}", params);
}

TEST(GenerateLSPMessagesTest, AnyObject) {
    parseTest<NotificationMessage>(alloc, makeNotificationMessage("{\"jim\": \"henson\"}"), [](auto &msg) -> void {
        auto &paramsOptional = msg->params;
        ASSERT_TRUE(paramsOptional.has_value());
        auto &paramsVariant = *paramsOptional;
        auto paramsPtr = get_if<std::unique_ptr<rapidjson::Value>>(&paramsVariant);
        ASSERT_NE(paramsPtr, nullptr);
        auto &params = *paramsPtr;
        ASSERT_TRUE(params->IsObject());
    });

    // Deserialization: Must be an object.
    ASSERT_THROW(NotificationMessage::fromJSON(alloc, makeNotificationMessage("true")), JSONTypeError);

    // Serialization: Must be an object.
    // Null pointer case
    auto notificationMessage = make_unique<NotificationMessage>("2.0", "foo");
    notificationMessage->params = nullptr;
    ASSERT_THROW(notificationMessage->toJSON(), NullPtrError);

    // Non-object case
    notificationMessage->params = make_unique<rapidjson::Value>(rapidjson::kNullType);
    ASSERT_THROW(notificationMessage->toJSON(), InvalidTypeError);

    // New object case -- doesn't throw and stresses supported APIs for making values.
    auto notifMsg = NotificationMessage::fromJSON(alloc, makeNotificationMessage("{}"));
    auto range = make_unique<Position>(0, 0);
    notifMsg->params = range->toJSONValue(alloc);
    ASSERT_NO_THROW(notifMsg->toJSON());
}

TEST(GenerateLSPMessagesTest, StringConstant) {
    parseTest<CreateFile>(alloc, "{\"kind\": \"create\", \"uri\": \"file://foo\"}",
                          [](auto &createFile) -> void { ASSERT_EQ(createFile->kind, "create"); });

    // Throws when not the correct constant.
    ASSERT_THROW(CreateFile::fromJSON(alloc, "{\"kind\": \"delete\", \"uri\": \"file://foo\"}"), JSONConstantError);
    // Throws when not a string.
    ASSERT_THROW(CreateFile::fromJSON(alloc, "{\"kind\": 4, \"uri\": \"file://foo\"}"), JSONTypeError);

    // Throws during serialization if not set to proper constant value.
    auto createFile = make_unique<CreateFile>("delete", "file://foo");
    ASSERT_THROW(createFile->toJSON(), InvalidConstantValueError);
}

TEST(GenerateLSPMessagesTest, JSONArray) {
    parseTest<SymbolKindOptions>(alloc, "{\"valueSet\": [1,2,3,4,5,6]}", [](auto &symbolKindOptions) -> void {
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
    ASSERT_THROW(SymbolKindOptions::fromJSON(alloc, "{\"valueSet\": null}"), JSONTypeError);

    // Throws when a member of array has an invalid type.
    ASSERT_THROW(SymbolKindOptions::fromJSON(alloc, "{\"valueSet\": [1,2,true,4]}"), JSONTypeError);
}

TEST(GenerateLSPMessagesTest, IntEnums) {
    parseTest<SymbolKindOptions>(
        alloc, fmt::format("{{\"valueSet\": [{},{}]}}", (int)SymbolKind::Namespace, (int)SymbolKind::Null),
        [](auto &symbolKindOptions) -> void {
            auto &valueSetOptional = symbolKindOptions->valueSet;
            ASSERT_TRUE(valueSetOptional.has_value());
            auto &valueSet = *valueSetOptional;
            ASSERT_EQ(valueSet.size(), 2);
            ASSERT_EQ(valueSet.at(0), SymbolKind::Namespace);
            ASSERT_EQ(valueSet.at(1), SymbolKind::Null);
        });

    // Throws if enum is out of valid range.
    ASSERT_THROW(SymbolKindOptions::fromJSON(alloc, "{\"valueSet\": [1,2,-1,10]}"), InvalidEnumValueError);

    // Throws if enum is not the right type.
    ASSERT_THROW(SymbolKindOptions::fromJSON(alloc, "{\"valueSet\": [1,2.1]}"), JSONTypeError);

    // Throws during serialization if enum is out of valid range.
    auto symbolKind = make_unique<SymbolKindOptions>();
    auto symbols = vector<SymbolKind>();
    symbols.push_back(SymbolKind::Namespace);
    symbols.push_back((SymbolKind)-1);
    symbolKind->valueSet = make_optional<std::vector<SymbolKind>>(std::move(symbols));
    ASSERT_THROW(symbolKind->toJSON(), InvalidEnumValueError);
}

// Ensures that LSPMessage parses ResultMessage/ResultMessageWithError/RequestMessage/NotificationMessage properly.
TEST(GenerateLSPMessagesTest, DifferentLSPMessageTypes) {
    auto request = make_unique<RequestMessage>("2.0", 1, "foobar");
    auto response = make_unique<ResponseMessage>("2.0", 1);
    // Null result.
    response->result = make_unique<rapidjson::Value>();
    auto responseWithError = make_unique<ResponseMessage>("2.0", 1);
    responseWithError->error = make_unique<ResponseError>(20, "Bad request");
    auto notification = make_unique<NotificationMessage>("2.0", "foobar");

    // For each, serialize as a JSON document to force LSPMessage to re-deserialize it.
    // Checks that LSPMessage recognizes each as the correct type of message.
    ASSERT_TRUE(LSPMessage(alloc, request->toJSON()).isRequest());
    ASSERT_TRUE(LSPMessage(alloc, response->toJSON()).isResponse());
    ASSERT_TRUE(LSPMessage(alloc, responseWithError->toJSON()).isResponse());
    ASSERT_TRUE(LSPMessage(alloc, notification->toJSON()).isNotification());
}

} // namespace sorbet::realmain::lsp::test
