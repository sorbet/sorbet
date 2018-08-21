#ifndef SORBET_AST_FILES_H
#define SORBET_AST_FILES_H

#include "Names.h"
#include "StrictLevel.h"
#include "absl/strings/string_view.h"
#include "mio/shared_mmap.hpp"
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
    File(std::string &&path_, mio::shared_mmap_source mmapped, Type sourceType);
    File(File &&other) = default;
    File(const File &other) = delete;
    File() = delete;
    File withNewSource(std::string &&source_);
    File withNewSource(mio::shared_mmap_source mmapped);
    std::vector<int> &line_breaks() const;
    int lineCount() const;
    bool hadErrors() const;

private:
    /*
     * `path_` and `source_` will be references into objects inside this structure.
     * It should not in generally be directly accessed.
     */
    struct {
        std::shared_ptr<std::string> source;
        std::shared_ptr<std::string> path;
        mio::shared_mmap_source mmap;
    } storage;
    const absl::string_view path_;
    const absl::string_view source_;
    mutable std::shared_ptr<std::vector<int>> line_breaks_;
    mutable bool hadErrors_ = false;
    File(std::shared_ptr<std::string> keepingPathAlive, std::shared_ptr<std::string> keepingSourceAlive,
         mio::shared_mmap_source keepingMmapAlive, absl::string_view path_, absl::string_view source_,
         StrictLevel sigil)
        : storage{keepingSourceAlive, keepingPathAlive, keepingMmapAlive}, path_(path_), source_(source_),
          sigil(sigil){};

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
