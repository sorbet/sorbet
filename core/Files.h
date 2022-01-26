#ifndef SORBET_AST_FILES_H
#define SORBET_AST_FILES_H

#include "core/CompiledLevel.h"
#include "core/Names.h"
#include "core/StrictLevel.h"
#include <string>
#include <string_view>

namespace sorbet::core {
class GlobalState;
class File;
struct GlobalStateHash;
struct FileHash;
namespace serialize {
class SerializerImpl;
}

class FileRef final {
public:
    FileRef() : _id(0){};
    FileRef(unsigned int id);

    FileRef(FileRef &f) = default;
    FileRef(const FileRef &f) = default;
    FileRef(FileRef &&f) = default;
    FileRef &operator=(const FileRef &f) = default;
    FileRef &operator=(FileRef &&f) = default;

    bool operator==(const FileRef &rhs) const {
        return _id == rhs._id;
    }

    bool operator!=(const FileRef &rhs) const {
        return !(rhs == *this);
    }

    bool operator<(const FileRef &rhs) const {
        return _id < rhs._id;
    }

    bool operator>(const FileRef &rhs) const {
        return _id > rhs._id;
    }

    inline unsigned int id() const {
        return _id;
    }

    inline bool exists() const {
        return _id > 0;
    }

    const File &data(const GlobalState &gs) const;
    File &data(GlobalState &gs) const;
    const File &dataAllowingUnsafe(const GlobalState &gs) const;
    File &dataAllowingUnsafe(GlobalState &gs) const;

private:
    uint32_t _id;
};
CheckSize(FileRef, 4, 4);

class File final {
public:
    enum class Type {
        NotYetRead,
        PayloadGeneration, // Files marked during --store-state
        Payload,           // Files loaded from the binary payload
        Package,           // Stripe Ruby package
        Normal,
        TombStone,
    };

    // Epoch is _only_ used in LSP mode. Do not depend on it elsewhere.
    // TODO(jvilk): Delurk epoch usage and use something like pointer equality to check if a file has changed.
    const uint32_t epoch;

    friend class GlobalState;
    friend class ::sorbet::core::serialize::SerializerImpl;

    static StrictLevel fileStrictSigil(std::string_view source);
    static CompiledLevel fileCompiledSigil(std::string_view source);

    std::string_view path() const;
    std::string_view source() const;
    Type sourceType;

    bool isPayload() const;
    bool isRBI() const;
    bool isStdlib() const;
    bool isPackage() const;

    // flag accessors
    bool isPackagedTest() const;

    bool hasParseErrors() const;
    void setHasParseErrors(bool value);

    bool cached() const;
    void setCached(bool value);

    File(std::string &&path_, std::string &&source_, Type sourceType, uint32_t epoch = 0);
    File(File &&other) = delete;
    File(const File &other) = delete;
    File() = delete;
    std::unique_ptr<File> deepCopy(GlobalState &) const;
    std::vector<int> &lineBreaks() const;
    int lineCount() const;
    StrictLevel minErrorLevel() const;

    /** Given a 1-based line number, returns a string view of the line. */
    std::string_view getLine(int i);

    void setFileHash(std::unique_ptr<const FileHash> hash);
    const std::shared_ptr<const FileHash> &getFileHash() const;

    static constexpr std::string_view URL_PREFIX = "https://github.com/sorbet/sorbet/tree/master/";
    static std::string censorFilePathForSnapshotTests(std::string_view orig);

private:
    struct Flags {
        // if 'true' file is completely cached in kvstore
        bool cached : 1;
        // some reasonable invariants don't hold for invalid files
        bool hasParseErrors : 1;
        // only relevant in --stripe-packages mode: is the file a `.test.rb` file?
        bool isPackagedTest : 1;

        Flags() : cached(false), hasParseErrors(false), isPackagedTest(false) {};
    };

    Flags flags;

    const std::string path_;
    const std::string source_;
    mutable std::shared_ptr<std::vector<int>> lineBreaks_;
    mutable StrictLevel minErrorLevel_ = StrictLevel::Max;
    std::shared_ptr<const FileHash> hash_;

public:
    const StrictLevel originalSigil;
    StrictLevel strictLevel;

    const CompiledLevel compiledLevel;
};

template <typename H> H AbslHashValue(H h, const FileRef &m) {
    return H::combine(std::move(h), m.id());
}
} // namespace sorbet::core
#endif // SORBET_AST_FILES_H
