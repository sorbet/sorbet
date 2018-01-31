#ifndef SRUBY_AST_FILES_H
#define SRUBY_AST_FILES_H

#include "Names.h"
#include "absl/strings/string_view.h"
#include <string>

namespace ruby_typer {
namespace core {
class GlobalState;
class File;

class FileRef final {
public:
    FileRef() : _id(-1){};
    FileRef(const GlobalState &gs, unsigned int id);

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

    inline int id() const {
        return _id;
    }

    inline bool exists() const {
        return _id > 0;
    }

    const File &data(const GlobalState &gs) const;
    File &data(GlobalState &gs) const;

private:
    int _id;
};

class File final {
public:
    enum Type {
        Payload,
        Untyped,
        Typed,
        TombStone,
    };

    friend class GlobalState;

    absl::string_view path() const;
    absl::string_view source() const;
    Type source_type;

    bool isStdLib() const;

    File(std::string &&path_, std::string &&source_, Type sourcetype);
    File(File &&other) = default;
    File(const File &other) = delete;
    File() = delete;
    File deepCopy();

private:
    std::string path_;
    std::string source_;

public:
    const std::vector<int> line_breaks;
};
} // namespace core
} // namespace ruby_typer

#endif // SRUBY_AST_FILES_H
