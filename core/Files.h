#ifndef SORBET_AST_FILES_H
#define SORBET_AST_FILES_H

#include "core/Names.h"
#include "core/StrictLevel.h"
#include <atomic>
#include <optional>
#include <string>

namespace sorbet::core {
class GlobalState;
class File;
struct GlobalStateHash;
namespace serialize {
class SerializerImpl;
}

class FileRef final {
public:
    FileRef() : _id(0){};
    FileRef(unsigned int id);

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

    inline u2 id() const {
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
    u2 _id;
};
CheckSize(FileRef, 2, 2);

class File final {
public:
    enum Type {
        NotYetRead,
        PayloadGeneration, // Files marked during --store-state
        Payload,           // Files loaded from the binary payload
        Normal,
        TombStone,
    };

    bool cachedParseTree = false;
    bool hasParseErrors = false; // some reasonable invariants don't hold for invalid files
    bool pluginGenerated = false;

    friend class GlobalState;
    friend class ::sorbet::core::serialize::SerializerImpl;

    static StrictLevel fileSigil(std::string_view source);

    std::string_view path() const;
    std::string_view source() const;
    Type sourceType;

    bool isPayload() const;
    bool isRBI() const;

    File(std::string &&path_, std::string &&source_, Type sourceType);
    File(File &&other) = delete;
    File(const File &other) = delete;
    File() = delete;
    std::unique_ptr<File> deepCopy(GlobalState &) const;
    std::vector<int> &lineBreaks() const;
    int lineCount() const;
    StrictLevel minErrorLevel() const;

    std::shared_ptr<GlobalStateHash> getDefinitionHash() const;
    void setDefinitionHash(GlobalStateHash &hash) const;
    /** Given a 1-based line number, returns a string view of the line. */
    std::string_view getLine(int i);

private:
    const std::string path_;
    const std::string source_;
    mutable std::shared_ptr<std::vector<int>> lineBreaks_;
    mutable StrictLevel minErrorLevel_ = StrictLevel::Max;
    mutable std::shared_ptr<GlobalStateHash> globalStateHash;

public:
    const StrictLevel originalSigil;
    StrictLevel strictLevel;
};

template <typename H> H AbslHashValue(H h, const FileRef &m) {
    return H::combine(std::move(h), m.id());
}
} // namespace sorbet::core
#endif // SORBET_AST_FILES_H
