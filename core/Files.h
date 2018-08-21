#ifndef SORBET_AST_FILES_H
#define SORBET_AST_FILES_H

#include "Names.h"
#include "StrictLevel.h"
#include "absl/strings/string_view.h"
#include <string>

namespace sorbet {
namespace core {
class GlobalState;
class File;

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

    const File &data(const GlobalState &gs, bool allowTombStones = false) const;
    File &data(GlobalState &gs, bool allowTombStones = false) const;

private:
    u2 _id;
};
CheckSize(FileRef, 2, 2);

class File final {
public:
    enum Type {
        PayloadGeneration, // Files marked during --store-state
        Payload,           // Files loaded from the binary payload
        Normal,
        TombStone,
    };

    bool cachedParseTree = false;

    friend class GlobalState;

    absl::string_view path() const;
    absl::string_view source() const;
    Type sourceType;

    bool isPayload() const;
    bool isRBI() const;

    File(std::string &&path_, std::string &&source_, Type sourceType);
    File(File &&other) = default;
    File(const File &other) = delete;
    File() = delete;
    File deepCopy();
    std::vector<int> &line_breaks() const;
    int lineCount() const;
    bool hadErrors() const;

private:
    const std::string path_;
    const std::string source_;
    mutable std::shared_ptr<std::vector<int>> line_breaks_;
    mutable bool hadErrors_ = false;

public:
    const StrictLevel sigil;
    StrictLevel strict;
};
} // namespace core
} // namespace sorbet
namespace std {
template <> struct hash<sorbet::core::FileRef> {
    std::size_t operator()(const sorbet::core::FileRef k) const {
        return k.id();
    }
};
} // namespace std
#endif // SORBET_AST_FILES_H
