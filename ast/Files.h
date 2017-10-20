#include "Names.h"
#include <string>

namespace ruby_typer {
namespace ast {
class ContextBase;
class File;

class FileRef {
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

    File &file(ContextBase &ctx) const;

private:
    int _id;
};

class File {
public:
    friend class ContextBase;

    UTF8Desc path();
    UTF8Desc source();

    File() noexcept {};

    File(File &&other) noexcept = default;

    File(const File &other) = delete;

private:
    std::string path_;
    std::string source_;
};
} // namespace ast
} // namespace ruby_typer
