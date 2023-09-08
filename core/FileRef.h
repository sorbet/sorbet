#ifndef SORBET_CORE_FILEREF_H
#define SORBET_CORE_FILEREF_H

#include "common/common.h"

namespace sorbet::core {
class GlobalState;
class File;

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

    // Normally, using .data requires that the file have been read.
    // But File::Flag data is populated when the File is created, before needing to read the
    // file. This helper exposes that without tripping an ENFORCE.
    bool isPackage(const GlobalState &gs) const;

private:
    uint32_t _id;
};
CheckSize(FileRef, 4, 4);

} // namespace sorbet::core

#endif
