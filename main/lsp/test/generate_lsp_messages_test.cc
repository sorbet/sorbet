#include "gtest/gtest.h"

#include "common/common.h"
#include "common/sort.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/json_types.h"

using namespace std;
using namespace sorbet::realmain::lsp;

namespace sorbet::realmain::lsp::test {

template <typename T> using ParseTestLambda = function<void(std::unique_ptr<T> &)>;

template <typename T> unique_ptr<T> fromJSON(const string &jsonStr) {
    rapidjson::MemoryPoolAllocator<> alloc;
    rapidjson::Document d(&alloc);
    d.Parse(jsonStr);
    if (!d.IsObject()) {
        throw JSONTypeError("document root", "object", d);
    }
    return T::fromJSONValue(d.GetObject(), "root");
};

/**
 * Using jsonStr, creates two versions of the same document:
 * - One created by parsing jsonStr.
 * - One created by re-emitting JSON from the parsed jsonStr, and re-parsing it.
 * It then calls lambda with each, ensuring that any assertions it makes
 * passes on the parsed and re-parsed document.
 */
template <typename T> void parseTest(const string &jsonStr, ParseTestLambda<T> lambda) {
    auto original = fromJSON<T>(jsonStr);
    lambda(original);
    auto reparsed = fromJSON<T>(original->toJSON());
    lambda(reparsed);
};

const string SAMPLE_RANGE = "{\"start\": {\"line\": 0, \"character\": 1}, \"end\": {\"line\": 2, \"character\": 3}}";

// N.B.: Also tests integer fields.
TEST(GenerateLSPMessagesTest, Object) {
    parseTest<Range>(SAMPLE_RANGE, [](auto &range) -> void {
        ASSERT_EQ(range->start->line, 0);
        ASSERT_EQ(range->start->character, 1);
        ASSERT_EQ(range->end->line, 2);
        ASSERT_EQ(range->end->character, 3);
    });

    // Throws when missing a field.
    ASSERT_THROW(fromJSON<Range>("{\"start\": {\"line\": 0, \"character\": 1}, \"end\": {\"line\": 2}}"),
                 MissingFieldError);
    // Throws when not an object.
    ASSERT_THROW(fromJSON<Range>("4"), JSONTypeError);
    // Throws when field does not contain a number
    ASSERT_THROW(
        fromJSON<Range>("{\"start\": {\"line\": 0, \"character\": true}, \"end\": {\"line\": 2, \"character\": 3}}"),
        JSONTypeError);
    // Throws when field contains a double, not an int.
    ASSERT_THROW(
        fromJSON<Range>("{\"start\": {\"line\": 0, \"character\": 1.1}, \"end\": {\"line\": 2, \"character\": 3}}"),
        JSONTypeError);

    // Serialization: Throws if sub-objects are not initialized.
    auto badRange = make_unique<Range>(nullptr, nullptr);
    ASSERT_THROW(badRange->toJSON(), NullPtrError);
}

TEST(GenerateLSPMessagesTest, StringField) {
    const string expectedText = "Hello World!";
    parseTest<TextEdit>(fmt::format("{{\"range\": {}, \"newText\": \"{}\"}}", SAMPLE_RANGE, expectedText),
                        [&expectedText](auto &textEdit) -> void { ASSERT_EQ(textEdit->newText, expectedText); });

    // Throws when not a string
    ASSERT_THROW(fromJSON<TextEdit>(fmt::format("{{\"range\": {}, \"newText\": 4.0}}", SAMPLE_RANGE)), JSONTypeError);
}

TEST(GenerateLSPMessagesTest, StringEnumField) {
    const string markupKind = "markdown";
    parseTest<MarkupContent>(fmt::format("{{\"kind\": \"{}\", \"value\": \"Markup stuff\"}}", markupKind),
                             [](auto &markupContent) -> void { ASSERT_EQ(markupContent->kind, MarkupKind::Markdown); });

    // Throws when not a valid enum.
    ASSERT_THROW(fromJSON<MarkupContent>("{\"kind\": \"foobar\", \"value\": \"Hello\"}"), InvalidStringEnumError);
    // Throws when not a string.
    ASSERT_THROW(fromJSON<MarkupContent>("{\"kind\": 4, \"value\": \"Hello\"}"), JSONTypeError);

    // Create a C++ object with an invalid enum value and try to serialize.
    auto markupContent = make_unique<MarkupContent>((MarkupKind)1000, "hello");
    ASSERT_THROW(markupContent->toJSON(), InvalidEnumValueError);
}

TEST(GenerateLSPMessagesTest, NullField) {
    parseTest<VersionedTextDocumentIdentifier>(
        "{\"uri\": \"file://foo\", \"version\": null}", [](auto &versionedTextDocumentIdentifier) -> void {
            auto nullValue = get_if<JSONNullObject>(&(versionedTextDocumentIdentifier->version));
            // Should not be null; should point to an instance of JSONNullObject.
            ASSERT_NE(nullValue, nullptr);
        });
}

// N.B.: Also covers testing boolean types, which are treated as optional almost everywhere in the spec.
TEST(GenerateLSPMessagesTest, OptionalField) {
    parseTest<CreateOrRenameFileOptions>("{\"overwrite\": true}", [](auto &createOrRenameFileOptions) -> void {
        ASSERT_TRUE(createOrRenameFileOptions->overwrite.has_value());
        ASSERT_FALSE(createOrRenameFileOptions->ignoreIfExists.has_value());
        ASSERT_TRUE(*(createOrRenameFileOptions->overwrite));
    });

    parseTest<CreateOrRenameFileOptions>("{}", [](auto &createOrRenameFileOptions) -> void {
        ASSERT_FALSE(createOrRenameFileOptions->overwrite.has_value());
    });

    // Throws when not the correct type.
    ASSERT_THROW(fromJSON<CreateOrRenameFileOptions>("{\"overwrite\": 4}"), JSONTypeError);
}

struct ExceptionThrower {
    operator double() {
        throw runtime_error("Nope");
    }
};

TEST(GenerateLSPMessagesTest, DoubleField) {
    // Doubles can be ints or doubles.
    parseTest<Color>("{\"red\": 0, \"green\": 1.1, \"blue\": 2.0, \"alpha\": 3}", [](auto &color) -> void {
        ASSERT_EQ(0.0, color->red);
        ASSERT_EQ(1.1, color->green);
        ASSERT_EQ(2.0, color->blue);
        ASSERT_EQ(3.0, color->alpha);
    });
}

TEST(GenerateLSPMessagesTest, VariantField) {
    parseTest<CancelParams>("{\"id\": 4}", [](auto &cancelParamsNumber) -> void {
        auto numberId = get_if<int>(&cancelParamsNumber->id);
        ASSERT_NE(numberId, nullptr);
        ASSERT_EQ(*numberId, 4);
        ASSERT_EQ(get_if<std::string>(&cancelParamsNumber->id), nullptr);
    });

    parseTest<CancelParams>("{\"id\": \"iamanid\"}", [](auto &cancelParamsString) -> void {
        auto stringId = get_if<std::string>(&cancelParamsString->id);
        ASSERT_NE(stringId, nullptr);
        ASSERT_EQ(*stringId, "iamanid");
        ASSERT_EQ(get_if<int>(&cancelParamsString->id), nullptr);
    });

    // Throws when missing.
    ASSERT_THROW(fromJSON<CancelParams>("{}"), MissingFieldError);

    // Throws when not the correct type.
    ASSERT_THROW(fromJSON<CancelParams>("{\"id\": true}"), JSONTypeError);

    // Int types cannot be doubles.
    ASSERT_THROW(fromJSON<CancelParams>("{\"id\": 4.1}"), JSONTypeError);

    // Create CancelParams with a variant field in an erroneous state.
    // See https://en.cppreference.com/w/cpp/utility/variant/valueless_by_exception
    auto cancelParams = make_unique<CancelParams>(variant<int, std::string>());
    try {
        cancelParams->id.emplace<int>(ExceptionThrower());
    } catch (runtime_error e) {
    }
    ASSERT_THROW(cancelParams->toJSON(), MissingVariantValueError);
}

TEST(GenerateLSPMessagesTest, StringConstant) {
    parseTest<CreateFile>("{\"kind\": \"create\", \"uri\": \"file://foo\"}",
                          [](auto &createFile) -> void { ASSERT_EQ(createFile->kind, "create"); });

    // Throws when not the correct constant.
    ASSERT_THROW(fromJSON<CreateFile>("{\"kind\": \"delete\", \"uri\": \"file://foo\"}"), JSONConstantError);
    // Throws when not a string.
    ASSERT_THROW(fromJSON<CreateFile>("{\"kind\": 4, \"uri\": \"file://foo\"}"), JSONTypeError);

    // Throws during serialization if not set to proper constant value.
    auto createFile = make_unique<CreateFile>("delete", "file://foo");
    ASSERT_THROW(createFile->toJSON(), InvalidConstantValueError);
}

TEST(GenerateLSPMessagesTest, JSONArray) {
    parseTest<SymbolKindOptions>("{\"valueSet\": [1,2,3,4,5,6]}", [](auto &symbolKindOptions) -> void {
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
    ASSERT_THROW(fromJSON<SymbolKindOptions>("{\"valueSet\": {}}"), JSONTypeError);

    // Throws when a member of array has an invalid type.
    ASSERT_THROW(fromJSON<SymbolKindOptions>("{\"valueSet\": [1,2,true,4]}"), JSONTypeError);
}

TEST(GenerateLSPMessagesTest, IntEnums) {
    parseTest<SymbolKindOptions>(
        fmt::format("{{\"valueSet\": [{},{}]}}", (int)SymbolKind::Namespace, (int)SymbolKind::Null),
        [](auto &symbolKindOptions) -> void {
            auto &valueSetOptional = symbolKindOptions->valueSet;
            ASSERT_TRUE(valueSetOptional.has_value());
            auto &valueSet = *valueSetOptional;
            ASSERT_EQ(valueSet.size(), 2);
            ASSERT_EQ(valueSet.at(0), SymbolKind::Namespace);
            ASSERT_EQ(valueSet.at(1), SymbolKind::Null);
        });

    // Throws if enum is out of valid range.
    ASSERT_THROW(fromJSON<SymbolKindOptions>("{\"valueSet\": [1,2,-1,10]}"), InvalidEnumValueError);

    // Throws if enum is not the right type.
    ASSERT_THROW(fromJSON<SymbolKindOptions>("{\"valueSet\": [1,2.1]}"), JSONTypeError);

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
    auto request = make_unique<RequestMessage>("2.0", 1, LSPMethod::Shutdown, make_optional<JSONNullObject>());
    auto response = make_unique<ResponseMessage>("2.0", 1, LSPMethod::Shutdown);
    response->result = JSONNullObject();
    auto responseWithError = make_unique<ResponseMessage>("2.0", 1, LSPMethod::SorbetError);
    responseWithError->error = make_unique<ResponseError>(20, "Bad request");
    auto notification = make_unique<NotificationMessage>("2.0", LSPMethod::Exit, make_optional<JSONNullObject>());

    // For each, serialize as a JSON document to force LSPMessage to re-deserialize it.
    // Checks that LSPMessage recognizes each as the correct type of message.
    ASSERT_TRUE(LSPMessage(request->toJSON()).isRequest());
    ASSERT_TRUE(LSPMessage(response->toJSON()).isResponse());
    ASSERT_TRUE(LSPMessage(responseWithError->toJSON()).isResponse());
    ASSERT_TRUE(LSPMessage(notification->toJSON()).isNotification());
}

string makeRequestMessage(LSPMethod method, optional<string_view> params) {
    return fmt::format("{{\"jsonrpc\": \"2.0\", \"id\": 0, \"method\": \"{}\"{}}}", convertLSPMethodToString(method),
                       (params ? fmt::format(", \"params\": {}", *params) : ""));
}

// Serialize and deserialize various valid discriminated union values.
TEST(GenerateLSPMessagesTest, DiscriminatedUnionValidValues) {
    // Shutdown supports `null` and `nullopt`, but nothing else.
    parseTest<RequestMessage>(makeRequestMessage(LSPMethod::Shutdown, "null"), [](auto &msg) -> void {
        ASSERT_EQ(msg->method, LSPMethod::Shutdown);
        ASSERT_NO_THROW({
            auto maybeNull = get<optional<JSONNullObject>>(msg->params);
            // Null in an optional field is actually treated as a missing field for emacs compatibility.
            ASSERT_FALSE(maybeNull);
        });
    });
    parseTest<RequestMessage>(makeRequestMessage(LSPMethod::Shutdown, nullopt), [](auto &msg) -> void {
        ASSERT_EQ(msg->method, LSPMethod::Shutdown);
        ASSERT_NO_THROW({
            auto maybeNull = get<optional<JSONNullObject>>(msg->params);
            ASSERT_FALSE(maybeNull);
        });
    });
}

// Verify that serialization/deserialization code throws an exception when a discriminated union has an invalid
// parameter for the given discriminant
TEST(GenerateLSPMessagesTest, DiscriminatedUnionInvalidValues) {
    // Shutdown can't have a SorbetErrorParam.
    EXPECT_THROW(RequestMessage("2.0", 1, LSPMethod::Shutdown, make_unique<SorbetErrorParams>(1, "")).toJSON(),
                 InvalidDiscriminatedUnionValueError);
    // Shutdown can't have a string param.
    EXPECT_THROW(LSPMessage(makeRequestMessage(LSPMethod::Shutdown, "{\"code\": 1, \"message\": \"\"}")),
                 JSONTypeError);
    // TextDocumentDocumentSymbol must have a parameter.
    EXPECT_THROW(LSPMessage(makeRequestMessage(LSPMethod::TextDocumentDocumentSymbol, "null")), JSONTypeError);
}

// Verify that serialization/deserialization code throws an exception when a discriminated union has an invalid
// discriminant
TEST(GenerateLSPMessagesTest, DiscriminatedUnionInvalidDiscriminant) {
    // DidOpen is a notification.
    EXPECT_THROW(LSPMessage(makeRequestMessage(LSPMethod::TextDocumentDidOpen, "null")), InvalidDiscriminantValueError);
    EXPECT_THROW(RequestMessage("2.0", 1, LSPMethod::TextDocumentDidOpen, JSONNullObject()).toJSON(),
                 InvalidDiscriminantValueError);
}

TEST(GenerateLSPMessagesTest, RenamedFieldsWorkProperly) {
    parseTest<WatchmanQueryResponse>("{\"version\": \"versionstring\", \"clock\": \"clockvalue\", "
                                     "\"is_fresh_instance\": true, \"files\": [\"foo.rb\"]}",
                                     [](auto &watchmanQueryResponse) -> void {
                                         ASSERT_EQ(watchmanQueryResponse->version, "versionstring");
                                         ASSERT_EQ(watchmanQueryResponse->clock, "clockvalue");
                                         ASSERT_TRUE(watchmanQueryResponse->isFreshInstance);
                                         ASSERT_EQ(watchmanQueryResponse->files.size(), 1);
                                         ASSERT_EQ(watchmanQueryResponse->files.at(0), "foo.rb");
                                     });
}

TEST(GenerateLSPMessagesTest, AcceptsNullOnOptionalFields) {
    parseTest<ConfigurationItem>("{\"scopeUri\": null}", [](auto &item) -> void { ASSERT_FALSE(item->scopeUri); });
}

TEST(GenerateLSPMessagesTest, SorbetWorkspaceEditParamsMerge) {
    SorbetWorkspaceEditParams oldEdit;
    oldEdit.mergeCount = 10;
    oldEdit.sorbetCancellationExpected = true;
    oldEdit.epoch = 4;
    oldEdit.updates = {make_shared<core::File>("foo.rb", "foo", core::File::Type::Normal, 4)};

    SorbetWorkspaceEditParams newEdit;
    newEdit.mergeCount = 2;
    newEdit.sorbetCancellationExpected = false;
    newEdit.epoch = 5;
    newEdit.updates = {make_shared<core::File>("bar.rb", "bar", core::File::Type::Normal, 5)};

    // Merge merges the new edit into the old edit.
    oldEdit.merge(newEdit);

    EXPECT_EQ(13, oldEdit.mergeCount);
    EXPECT_TRUE(oldEdit.sorbetCancellationExpected);
    EXPECT_EQ(5, oldEdit.epoch);
    ASSERT_EQ(2, oldEdit.updates.size());

    fast_sort(oldEdit.updates, [](auto &a, auto &b) -> bool { return a->path().compare(b->path()) < 0; });

    ASSERT_EQ("bar.rb", oldEdit.updates.at(0)->path());
    ASSERT_EQ("foo.rb", oldEdit.updates.at(1)->path());
}

TEST(GenerateLSPMessagesTest, SorbetWorkspaceEditParamsMergeTakesNewerEdit) {
    SorbetWorkspaceEditParams oldEdit;
    oldEdit.mergeCount = 10;
    oldEdit.sorbetCancellationExpected = false;
    oldEdit.epoch = 4;
    oldEdit.updates = {make_shared<core::File>("foo.rb", "foo", core::File::Type::Normal, 4)};

    SorbetWorkspaceEditParams newEdit;
    newEdit.mergeCount = 0;
    newEdit.sorbetCancellationExpected = true;
    newEdit.epoch = 5;
    newEdit.updates = {make_shared<core::File>("foo.rb", "bar", core::File::Type::Normal, 5)};

    // Merge merges the new edit into the old edit.
    oldEdit.merge(newEdit);

    EXPECT_EQ(11, oldEdit.mergeCount);
    EXPECT_TRUE(oldEdit.sorbetCancellationExpected);
    EXPECT_EQ(5, oldEdit.epoch);
    ASSERT_EQ(1, oldEdit.updates.size());

    ASSERT_EQ("foo.rb", oldEdit.updates.at(0)->path());
    ASSERT_EQ("bar", oldEdit.updates.at(0)->source());
}
} // namespace sorbet::realmain::lsp::test
