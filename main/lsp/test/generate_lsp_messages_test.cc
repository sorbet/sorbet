#include "doctest/doctest.h"

#include "common/common.h"
#include "common/sort/sort.h"
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
TEST_CASE("Object") {
    parseTest<Range>(SAMPLE_RANGE, [](auto &range) -> void {
        REQUIRE_EQ(range->start->line, 0);
        REQUIRE_EQ(range->start->character, 1);
        REQUIRE_EQ(range->end->line, 2);
        REQUIRE_EQ(range->end->character, 3);
    });

    // Throws when missing a field.
    REQUIRE_THROWS_AS(fromJSON<Range>("{\"start\": {\"line\": 0, \"character\": 1}, \"end\": {\"line\": 2}}"),
                      MissingFieldError);
    // Throws when not an object.
    REQUIRE_THROWS_AS(fromJSON<Range>("4"), JSONTypeError);
    // Throws when field does not contain a number
    REQUIRE_THROWS_AS(
        fromJSON<Range>("{\"start\": {\"line\": 0, \"character\": true}, \"end\": {\"line\": 2, \"character\": 3}}"),
        JSONTypeError);
    // Throws when field contains a double, not an int.
    REQUIRE_THROWS_AS(
        fromJSON<Range>("{\"start\": {\"line\": 0, \"character\": 1.1}, \"end\": {\"line\": 2, \"character\": 3}}"),
        JSONTypeError);

    // Serialization: Throws if sub-objects are not initialized.
    auto badRange = make_unique<Range>(nullptr, nullptr);
    REQUIRE_THROWS_AS(badRange->toJSON(), NullPtrError);
}

TEST_CASE("StringField") {
    const string expectedText = "Hello World!";
    parseTest<TextEdit>(fmt::format("{{\"range\": {}, \"newText\": \"{}\"}}", SAMPLE_RANGE, expectedText),
                        [&expectedText](auto &textEdit) -> void { REQUIRE_EQ(textEdit->newText, expectedText); });

    // Throws when not a string
    REQUIRE_THROWS_AS(fromJSON<TextEdit>(fmt::format("{{\"range\": {}, \"newText\": 4.0}}", SAMPLE_RANGE)),
                      JSONTypeError);
}

TEST_CASE("StringEnumField") {
    const string markupKind = "markdown";
    parseTest<MarkupContent>(
        fmt::format("{{\"kind\": \"{}\", \"value\": \"Markup stuff\"}}", markupKind),
        [](auto &markupContent) -> void { REQUIRE_EQ(markupContent->kind, MarkupKind::Markdown); });

    // Throws when not a valid enum.
    REQUIRE_THROWS_AS(fromJSON<MarkupContent>("{\"kind\": \"foobar\", \"value\": \"Hello\"}"), InvalidStringEnumError);
    // Throws when not a string.
    REQUIRE_THROWS_AS(fromJSON<MarkupContent>("{\"kind\": 4, \"value\": \"Hello\"}"), JSONTypeError);

    // Create a C++ object with an invalid enum value and try to serialize.
    auto markupContent = make_unique<MarkupContent>((MarkupKind)1000, "hello");
    REQUIRE_THROWS_AS(markupContent->toJSON(), InvalidEnumValueError);
}

TEST_CASE("NullField") {
    parseTest<VersionedTextDocumentIdentifier>(
        "{\"uri\": \"file://foo\", \"version\": null}", [](auto &versionedTextDocumentIdentifier) -> void {
            auto nullValue = get_if<JSONNullObject>(&(versionedTextDocumentIdentifier->version));
            // Should not be null; should point to an instance of JSONNullObject.
            REQUIRE_NE(nullValue, nullptr);
        });
}

// N.B.: Also covers testing boolean types, which are treated as optional almost everywhere in the spec.
TEST_CASE("OptionalField") {
    parseTest<CreateOrRenameFileOptions>("{\"overwrite\": true}", [](auto &createOrRenameFileOptions) -> void {
        REQUIRE(createOrRenameFileOptions->overwrite.has_value());
        REQUIRE_FALSE(createOrRenameFileOptions->ignoreIfExists.has_value());
        REQUIRE(*(createOrRenameFileOptions->overwrite));
    });

    parseTest<CreateOrRenameFileOptions>("{}", [](auto &createOrRenameFileOptions) -> void {
        REQUIRE_FALSE(createOrRenameFileOptions->overwrite.has_value());
    });

    // Throws when not the correct type.
    REQUIRE_THROWS_AS(fromJSON<CreateOrRenameFileOptions>("{\"overwrite\": 4}"), JSONTypeError);
}

struct ExceptionThrower {
    operator double() {
        throw runtime_error("Nope");
    }
};

TEST_CASE("DoubleField") {
    // Doubles can be ints or doubles.
    parseTest<Color>("{\"red\": 0, \"green\": 1.1, \"blue\": 2.0, \"alpha\": 3}", [](auto &color) -> void {
        REQUIRE_EQ(0.0, color->red);
        REQUIRE_EQ(1.1, color->green);
        REQUIRE_EQ(2.0, color->blue);
        REQUIRE_EQ(3.0, color->alpha);
    });
}

TEST_CASE("VariantField") {
    parseTest<CancelParams>("{\"id\": 4}", [](auto &cancelParamsNumber) -> void {
        auto numberId = get_if<int>(&cancelParamsNumber->id);
        REQUIRE_NE(numberId, nullptr);
        REQUIRE_EQ(*numberId, 4);
        REQUIRE_EQ(get_if<std::string>(&cancelParamsNumber->id), nullptr);
    });

    parseTest<CancelParams>("{\"id\": \"iamanid\"}", [](auto &cancelParamsString) -> void {
        auto stringId = get_if<std::string>(&cancelParamsString->id);
        REQUIRE_NE(stringId, nullptr);
        REQUIRE_EQ(*stringId, "iamanid");
        REQUIRE_EQ(get_if<int>(&cancelParamsString->id), nullptr);
    });

    // Throws when missing.
    REQUIRE_THROWS_AS(fromJSON<CancelParams>("{}"), MissingFieldError);

    // Throws when not the correct type.
    REQUIRE_THROWS_AS(fromJSON<CancelParams>("{\"id\": true}"), JSONTypeError);

    // Int types cannot be doubles.
    REQUIRE_THROWS_AS(fromJSON<CancelParams>("{\"id\": 4.1}"), JSONTypeError);

    // Create CancelParams with a variant field in an erroneous state.
    // See https://en.cppreference.com/w/cpp/utility/variant/valueless_by_exception
    auto cancelParams = make_unique<CancelParams>(variant<int, std::string>());
    try {
        cancelParams->id.emplace<int>(ExceptionThrower());
    } catch (runtime_error e) {
    }
    REQUIRE_THROWS_AS(cancelParams->toJSON(), MissingVariantValueError);
}

TEST_CASE("StringConstant") {
    parseTest<CreateFile>("{\"kind\": \"create\", \"uri\": \"file://foo\"}",
                          [](auto &createFile) -> void { REQUIRE_EQ(createFile->kind, "create"); });

    // Throws when not the correct constant.
    REQUIRE_THROWS_AS(fromJSON<CreateFile>("{\"kind\": \"delete\", \"uri\": \"file://foo\"}"), JSONConstantError);
    // Throws when not a string.
    REQUIRE_THROWS_AS(fromJSON<CreateFile>("{\"kind\": 4, \"uri\": \"file://foo\"}"), JSONTypeError);

    // Throws during serialization if not set to proper constant value.
    auto createFile = make_unique<CreateFile>("delete", "file://foo");
    REQUIRE_THROWS_AS(createFile->toJSON(), InvalidConstantValueError);
}

TEST_CASE("JSONArray") {
    parseTest<SymbolKindOptions>("{\"valueSet\": [1,2,3,4,5,6]}", [](auto &symbolKindOptions) -> void {
        auto &valueSetOptional = symbolKindOptions->valueSet;
        REQUIRE(valueSetOptional.has_value());
        auto &valueSetUniquePtr = *valueSetOptional;
        int start = 1;
        for (const SymbolKind &value : valueSetUniquePtr) {
            REQUIRE_EQ((const int &)value, start);
            start += 1;
        }
    });

    // Throws when not an array.
    REQUIRE_THROWS_AS(fromJSON<SymbolKindOptions>("{\"valueSet\": {}}"), JSONTypeError);

    // Throws when a member of array has an invalid type.
    REQUIRE_THROWS_AS(fromJSON<SymbolKindOptions>("{\"valueSet\": [1,2,true,4]}"), JSONTypeError);
}

TEST_CASE("IntEnums") {
    parseTest<SymbolKindOptions>(
        fmt::format("{{\"valueSet\": [{},{}]}}", (int)SymbolKind::Namespace, (int)SymbolKind::Null),
        [](auto &symbolKindOptions) -> void {
            auto &valueSetOptional = symbolKindOptions->valueSet;
            REQUIRE(valueSetOptional.has_value());
            auto &valueSet = *valueSetOptional;
            REQUIRE_EQ(valueSet.size(), 2);
            REQUIRE_EQ(valueSet.at(0), SymbolKind::Namespace);
            REQUIRE_EQ(valueSet.at(1), SymbolKind::Null);
        });

    // Throws if enum is out of valid range.
    REQUIRE_THROWS_AS(fromJSON<SymbolKindOptions>("{\"valueSet\": [1,2,-1,10]}"), InvalidEnumValueError);

    // Throws if enum is not the right type.
    REQUIRE_THROWS_AS(fromJSON<SymbolKindOptions>("{\"valueSet\": [1,2.1]}"), JSONTypeError);

    // Throws during serialization if enum is out of valid range.
    auto symbolKind = make_unique<SymbolKindOptions>();
    auto symbols = vector<SymbolKind>();
    symbols.push_back(SymbolKind::Namespace);
    symbols.push_back((SymbolKind)-1);
    symbolKind->valueSet = make_optional<std::vector<SymbolKind>>(std::move(symbols));
    REQUIRE_THROWS_AS(symbolKind->toJSON(), InvalidEnumValueError);
}

// Ensures that LSPMessage parses ResultMessage/ResultMessageWithError/RequestMessage/NotificationMessage properly.
TEST_CASE("DifferentLSPMessageTypes") {
    auto request = make_unique<RequestMessage>("2.0", 1, LSPMethod::Shutdown, make_optional<JSONNullObject>());
    auto response = make_unique<ResponseMessage>("2.0", 1, LSPMethod::Shutdown);
    response->result = JSONNullObject();
    auto responseWithError = make_unique<ResponseMessage>("2.0", 1, LSPMethod::SorbetError);
    responseWithError->error = make_unique<ResponseError>(20, "Bad request");
    auto notification = make_unique<NotificationMessage>("2.0", LSPMethod::Exit, make_optional<JSONNullObject>());

    // For each, serialize as a JSON document to force LSPMessage to re-deserialize it.
    // Checks that LSPMessage recognizes each as the correct type of message.
    REQUIRE(LSPMessage(request->toJSON()).isRequest());
    REQUIRE(LSPMessage(response->toJSON()).isResponse());
    REQUIRE(LSPMessage(responseWithError->toJSON()).isResponse());
    REQUIRE(LSPMessage(notification->toJSON()).isNotification());
}

string makeRequestMessage(LSPMethod method, optional<string_view> params) {
    return fmt::format("{{\"jsonrpc\": \"2.0\", \"id\": 0, \"method\": \"{}\"{}}}", convertLSPMethodToString(method),
                       (params ? fmt::format(", \"params\": {}", *params) : ""));
}

// Serialize and deserialize various valid discriminated union values.
TEST_CASE("DiscriminatedUnionValidValues") {
    // Shutdown supports `null` and `nullopt`, but nothing else.
    parseTest<RequestMessage>(makeRequestMessage(LSPMethod::Shutdown, "null"), [](auto &msg) -> void {
        REQUIRE_EQ(msg->method, LSPMethod::Shutdown);
        REQUIRE_NOTHROW({
            auto maybeNull = get<optional<JSONNullObject>>(msg->params);
            // Null in an optional field is actually treated as a missing field for emacs compatibility.
            REQUIRE_FALSE(maybeNull);
        });
    });
    parseTest<RequestMessage>(makeRequestMessage(LSPMethod::Shutdown, nullopt), [](auto &msg) -> void {
        REQUIRE_EQ(msg->method, LSPMethod::Shutdown);
        REQUIRE_NOTHROW({
            auto maybeNull = get<optional<JSONNullObject>>(msg->params);
            REQUIRE_FALSE(maybeNull);
        });
    });
}

// Verify that serialization/deserialization code throws an exception when a discriminated union has an invalid
// parameter for the given discriminant
TEST_CASE("DiscriminatedUnionInvalidValues") {
    // Shutdown can't have a SorbetErrorParam.
    REQUIRE_THROWS_AS(RequestMessage("2.0", 1, LSPMethod::Shutdown, make_unique<SorbetErrorParams>(1, "")).toJSON(),
                      InvalidDiscriminatedUnionValueError);
    // Shutdown can't have a string param.
    REQUIRE_THROWS_AS(LSPMessage(makeRequestMessage(LSPMethod::Shutdown, "{\"code\": 1, \"message\": \"\"}")),
                      JSONTypeError);
    // TextDocumentDocumentSymbol must have a parameter.
    REQUIRE_THROWS_AS(LSPMessage(makeRequestMessage(LSPMethod::TextDocumentDocumentSymbol, "null")), JSONTypeError);
}

// Verify that serialization/deserialization code throws an exception when a discriminated union has an invalid
// discriminant
TEST_CASE("DiscriminatedUnionInvalidDiscriminant") {
    // DidOpen is a notification.
    REQUIRE_THROWS_AS(LSPMessage(makeRequestMessage(LSPMethod::TextDocumentDidOpen, "null")),
                      InvalidDiscriminantValueError);
    REQUIRE_THROWS_AS(RequestMessage("2.0", 1, LSPMethod::TextDocumentDidOpen, JSONNullObject()).toJSON(),
                      InvalidDiscriminantValueError);
}

TEST_CASE("RenamedFieldsWorkProperly") {
    parseTest<WatchmanQueryResponse>("{\"version\": \"versionstring\", \"clock\": \"clockvalue\", "
                                     "\"is_fresh_instance\": true, \"files\": [\"foo.rb\"]}",
                                     [](auto &watchmanQueryResponse) -> void {
                                         REQUIRE_EQ(watchmanQueryResponse->version, "versionstring");
                                         REQUIRE_EQ(watchmanQueryResponse->clock, "clockvalue");
                                         REQUIRE(watchmanQueryResponse->isFreshInstance);
                                         REQUIRE_EQ(watchmanQueryResponse->files.size(), 1);
                                         REQUIRE_EQ(watchmanQueryResponse->files.at(0), "foo.rb");
                                     });
}

TEST_CASE("AcceptsNullOnOptionalFields") {
    parseTest<ConfigurationItem>("{\"scopeUri\": null}", [](auto &item) -> void { REQUIRE_FALSE(item->scopeUri); });
}

TEST_CASE("SorbetWorkspaceEditParamsMerge") {
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

    REQUIRE_EQ(13, oldEdit.mergeCount);
    REQUIRE(oldEdit.sorbetCancellationExpected);
    REQUIRE_EQ(5, oldEdit.epoch);
    REQUIRE_EQ(2, oldEdit.updates.size());

    fast_sort(oldEdit.updates, [](auto &a, auto &b) -> bool { return a->path().compare(b->path()) < 0; });

    REQUIRE_EQ("bar.rb", oldEdit.updates.at(0)->path());
    REQUIRE_EQ("foo.rb", oldEdit.updates.at(1)->path());
}

TEST_CASE("SorbetWorkspaceEditParamsMergeTakesNewerEdit") {
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

    REQUIRE_EQ(11, oldEdit.mergeCount);
    REQUIRE(oldEdit.sorbetCancellationExpected);
    REQUIRE_EQ(5, oldEdit.epoch);
    REQUIRE_EQ(1, oldEdit.updates.size());

    REQUIRE_EQ("foo.rb", oldEdit.updates.at(0)->path());
    REQUIRE_EQ("bar", oldEdit.updates.at(0)->source());
}
} // namespace sorbet::realmain::lsp::test
