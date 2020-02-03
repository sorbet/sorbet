#include "json_types.h"

#include <string>

#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace std;

namespace sorbet::realmain::lsp {

string stringify(const rapidjson::Value &value) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
}

DeserializationError::DeserializationError(string_view message)
    : runtime_error(fmt::format("Error deserializing JSON message: {}", message)) {}

InvalidStringEnumError::InvalidStringEnumError(string_view enumName, string_view value)
    : DeserializationError(fmt::format("Invalid value for enum type `{}`: {}", enumName, value)),
      enumName(string(enumName)), value(string(value)) {}

MissingFieldError::MissingFieldError(string_view fieldName)
    : DeserializationError(fmt::format("Missing field `{}`.", fieldName)) {}

JSONTypeError::JSONTypeError(string_view fieldName, string_view expectedType)
    : DeserializationError(fmt::format("Expected field `{}` to have value of type `{}`.", fieldName, expectedType)) {}

JSONTypeError::JSONTypeError(string_view fieldName, string_view expectedType, const rapidjson::Value &found)
    : DeserializationError(fmt::format("Expected field `{}` to have value of type `{}`, but had value `{}`.", fieldName,
                                       expectedType, stringify(found))) {}

JSONConstantError::JSONConstantError(string_view fieldName, string_view expectedValue,
                                     const rapidjson::Value &actualValue)
    : DeserializationError(fmt::format("Expected field `{}` to have value \"{}\", but had value `{}`.", fieldName,
                                       expectedValue, stringify(actualValue))) {}

SerializationError::SerializationError(string_view message)
    : runtime_error(fmt::format("Error serializing object to JSON message: {}", message)) {}

MissingVariantValueError::MissingVariantValueError(string_view fieldName)
    : SerializationError(fmt::format("Variant field `{}` does not have a value", fieldName)) {}

InvalidDiscriminantValueError::InvalidDiscriminantValueError(string_view fieldName, string_view discriminantFieldName,
                                                             string_view discriminantValue)
    : SerializationError(
          fmt::format("Invalid discriminant value for variant field `{}`: `{}` (from `{}`) is not a valid discriminant",
                      fieldName, discriminantValue, discriminantFieldName)),
      fieldName(string(fieldName)), discriminantFieldName(string(discriminantFieldName)),
      discriminantValue(string(discriminantValue)) {}

InvalidDiscriminatedUnionValueError::InvalidDiscriminatedUnionValueError(string_view fieldName,
                                                                         string_view discriminantFieldName,
                                                                         string_view discriminantValue,
                                                                         string_view expectedType)
    : SerializationError(fmt::format("Invalid value in discriminated union field `{}`: given discriminant `{}` "
                                     "(from `{}`), expected value of type `{}`",
                                     fieldName, discriminantValue, discriminantFieldName, expectedType)),
      fieldName(string(fieldName)) {}

NullPtrError::NullPtrError(string_view fieldName)
    : SerializationError(fmt::format("Field `{}` does not have a value, and contains a null pointer", fieldName)) {}

InvalidEnumValueError::InvalidEnumValueError(string_view typeName, int value)
    : SerializationError(fmt::format("Found invalid value `{}` for enum of type {}", value, typeName)) {}

InvalidConstantValueError::InvalidConstantValueError(string_view fieldName, string_view expectedValue,
                                                     string_view actualValue)
    : SerializationError(fmt::format("Expected `{}` to have value \"{}\", but found value \"{}\".", fieldName,
                                     expectedValue, actualValue)) {}

InvalidTypeError::InvalidTypeError(string_view fieldName, string_view expectedType,
                                   const unique_ptr<rapidjson::Value> &found)
    : SerializationError(fmt::format("Expected field `{}` to have value of type `{}`, but had value `{}`.", fieldName,
                                     expectedType, stringify(*found))) {}

const std::string JSONBaseType::defaultFieldName = "root";

string JSONBaseType::toJSON(bool prettyPrint) const {
    rapidjson::MemoryPoolAllocator<> alloc;
    auto v = toJSONValue(alloc);
    rapidjson::StringBuffer buffer;
    if (!prettyPrint) {
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        v->Accept(writer);
    } else {
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        v->Accept(writer);
    }
    return buffer.GetString();
}

// Object-specific helpers

unique_ptr<Range> Range::fromLoc(const core::GlobalState &gs, core::Loc loc) {
    if (!loc.exists()) {
        // this will happen if e.g. we disable the stdlib (e.g. to speed up testing in fuzzers).
        return nullptr;
    }
    auto pair = loc.position(gs);
    // All LSP numbers are zero-based, ours are 1-based.
    return make_unique<Range>(make_unique<Position>(pair.first.line - 1, pair.first.column - 1),
                              make_unique<Position>(pair.second.line - 1, pair.second.column - 1));
}

core::Loc Range::toLoc(const core::GlobalState &gs, core::FileRef file) const {
    ENFORCE(start->line >= 0);
    ENFORCE(start->character >= 0);
    ENFORCE(end->line >= 0);
    ENFORCE(end->character >= 0);

    auto sPos = core::Loc::pos2Offset(file.data(gs), core::Loc::Detail{(u4)start->line + 1, (u4)start->character + 1});
    auto ePos = core::Loc::pos2Offset(file.data(gs), core::Loc::Detail{(u4)end->line + 1, (u4)end->character + 1});

    // These offsets are non-nullopt assuming the input Range is valid
    return core::Loc(file, sPos.value(), ePos.value());
}

int Range::cmp(const Range &b) const {
    const int startCmp = start->cmp(*b.start);
    if (startCmp != 0) {
        return startCmp;
    }
    return end->cmp(*b.end);
}

unique_ptr<Range> Range::copy() const {
    return make_unique<Range>(start->copy(), end->copy());
}

int Location::cmp(const Location &b) const {
    const int uriCmp = uri.compare(b.uri);
    if (uriCmp != 0) {
        return uriCmp;
    }
    return range->cmp(*b.range);
}

unique_ptr<Location> Location::copy() const {
    return make_unique<Location>(uri, range->copy());
}

int Position::cmp(const Position &b) const {
    const int lineCmp = line - b.line;
    if (lineCmp != 0) {
        return lineCmp;
    }
    return character - b.character;
}

unique_ptr<Position> Position::copy() const {
    return make_unique<Position>(line, character);
}

unique_ptr<DiagnosticRelatedInformation> DiagnosticRelatedInformation::copy() const {
    return make_unique<DiagnosticRelatedInformation>(location->copy(), message);
}

unique_ptr<Diagnostic> Diagnostic::copy() const {
    auto d = make_unique<Diagnostic>(range->copy(), message);
    d->code = code;
    d->severity = severity;
    d->source = source;
    if (relatedInformation.has_value()) {
        vector<unique_ptr<DiagnosticRelatedInformation>> cloneRelatedInformation;
        for (const auto &ri : *relatedInformation) {
            cloneRelatedInformation.push_back(ri->copy());
        }
        d->relatedInformation = move(cloneRelatedInformation);
    }
    return d;
}

string TextDocumentContentChangeEvent::apply(string_view oldSource) const {
    if (range) {
        auto &r = *range;
        // incremental update
        core::Loc::Detail start, end;
        start.line = r->start->line + 1;
        start.column = r->start->character + 1;
        end.line = r->end->line + 1;
        end.column = r->end->character + 1;
        core::File old("./fake/path.rb", string(oldSource), core::File::Type::Normal);
        // These offsets are non-nullopt assuming the input range is a valid range.
        auto startOffset = core::Loc::pos2Offset(old, start).value();
        auto endOffset = core::Loc::pos2Offset(old, end).value();
        return string(oldSource).replace(startOffset, endOffset - startOffset, text);
    } else {
        return text;
    }
}

string DidChangeTextDocumentParams::getSource(string_view oldFileContents) const {
    string rv = string(oldFileContents);
    for (auto &change : contentChanges) {
        rv = change->apply(rv);
    }
    return rv;
}

void LSPFileUpdates::mergeOlder(const LSPFileUpdates &older) {
    editCount += older.editCount;
    hasNewFiles = hasNewFiles || older.hasNewFiles;
    cancellationExpected = cancellationExpected || older.cancellationExpected;
    preemptionsExpected += older.preemptionsExpected;

    ENFORCE(updatedFiles.size() == updatedFileHashes.size());
    ENFORCE(updatedFiles.size() == updatedFileIndexes.size());
    ENFORCE(older.updatedFiles.size() == older.updatedFileHashes.size());
    ENFORCE(older.updatedFiles.size() == older.updatedFileIndexes.size());

    // For updates, we prioritize _newer_ updates.
    UnorderedSet<string> encountered;
    for (auto &f : updatedFiles) {
        encountered.emplace(f->path());
    }

    int i = -1;
    for (auto &f : older.updatedFiles) {
        i++;
        if (encountered.contains(f->path())) {
            continue;
        }
        encountered.emplace(f->path());
        updatedFiles.push_back(f);
        updatedFileHashes.push_back(older.updatedFileHashes[i]);
        auto &ast = older.updatedFileIndexes[i];
        updatedFileIndexes.push_back(ast::ParsedFile{ast.tree->deepCopy(), ast.file});
    }
}

LSPFileUpdates LSPFileUpdates::copy() const {
    LSPFileUpdates copy;
    copy.epoch = epoch;
    copy.editCount = editCount;
    copy.canTakeFastPath = canTakeFastPath;
    copy.hasNewFiles = hasNewFiles;
    copy.updatedFiles = updatedFiles;
    copy.updatedFileHashes = updatedFileHashes;
    copy.cancellationExpected = cancellationExpected;
    copy.preemptionsExpected = preemptionsExpected;
    for (auto &ast : updatedFileIndexes) {
        copy.updatedFileIndexes.push_back(ast::ParsedFile{ast.tree->deepCopy(), ast.file});
    }
    return copy;
}

void SorbetWorkspaceEditParams::merge(SorbetWorkspaceEditParams &newerParams) {
    // 'newerParams' has newer updates, so merge its contents into this object.
    epoch = newerParams.epoch;

    UnorderedSet<std::string_view> encounteredFiles;
    auto newUpdates = move(newerParams.updates);

    for (auto &f : newUpdates) {
        encounteredFiles.insert(f->path());
    }

    for (auto &f : updates) {
        if (!encounteredFiles.contains(f->path())) {
            encounteredFiles.insert(f->path());
            newUpdates.push_back(move(f));
        }
    }
    updates = move(newUpdates);
    mergeCount += newerParams.mergeCount + 1;
    sorbetCancellationExpected = sorbetCancellationExpected || newerParams.sorbetCancellationExpected;
    sorbetPreemptionsExpected += newerParams.sorbetPreemptionsExpected;
}

} // namespace sorbet::realmain::lsp
