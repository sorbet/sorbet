#ifndef SRUBY_AST_FILES_H
#define SRUBY_AST_FILES_H

#include "Names.h"
#include <string>

namespace ruby_typer {
namespace core {
class GlobalState;
class File;

class FileRef final {
public:
    FileRef() : _id(-1){};
    FileRef(unsigned int id) : _id(id) {}

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

    inline int id() const {
        return _id;
    }

    inline bool exists() const {
        return _id != 0;
    }

    File &file(GlobalState &gs) const;

private:
    int _id;
};

class File final {
public:
    friend class GlobalState;

    UTF8Desc path();
    UTF8Desc source();
    bool isPayload = false;

    File(std::string &&path_, std::string &&source_);

    File(File &&other) = default;

    File(const File &other) = delete;

private:
    std::string path_;
    std::string source_;

public:
    const std::vector<int> line_breaks;
};
} // namespace core
} // namespace ruby_typer

#endif // SRUBY_AST_FILES_H
