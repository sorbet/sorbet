#ifndef SORBET_AST_FILES_H
#define SORBET_AST_FILES_H

#include "core/FileRef.h"
#include "core/LocOffsets.h"
#include "core/Names.h"
#include "core/StrictLevel.h"
#include "xxhash.h"
#include <array>
#include <string>
#include <string_view>

namespace sorbet::core {
class GlobalState;
struct LocalSymbolTableHashes;
struct FileHash;
namespace serialize {
class SerializerImpl;
}

class File final {
public:
    enum class Type : uint8_t {
        NotYetRead,
        PayloadGeneration, // Files marked during --store-state
        Payload,           // Files loaded from the binary payload
        Normal,
    };

    // Epoch is _only_ used in LSP mode. Do not depend on it elsewhere.
    // TODO(jvilk): Delurk epoch usage and use something like pointer equality to check if a file has changed.
    const uint32_t epoch;

    friend class GlobalState;
    friend class ::sorbet::core::serialize::SerializerImpl;

    static StrictLevel fileStrictSigil(std::string_view source);

    std::string_view path() const;
    std::string_view source() const;
    // The length of `source()`, without reading the text back if it has been released.
    uint32_t sourceSize() const {
        return this->sourceSize_;
    }
    // Whether the text at `range` is exactly `expected`. Unlike `source()`, this does not read a released text back in
    // full: it compares against the resident text when there is one and otherwise reads back just those bytes.
    bool sourceEquals(LocOffsets range, std::string_view expected) const;
    Type sourceType;

    bool isPayload() const;
    bool isRBI() const;
    bool isStdlib() const;
    bool isPackageRBI() const;

    bool permitOverloadDefinitions() const;

    static bool isRBIPath(std::string_view path);
    static bool isPackagePath(std::string_view path);

    bool hasPackageRbPath() const;
    bool isPackage(const GlobalState &gs) const;
    bool isTestPackage(const GlobalState &gs) const;

    // Whether the file is open in the LSP client. (Always false if not running under LSP.)
    bool isOpenInClient() const;
    // Set whether the file is open in the LSP client. Should only be used by LSPPreprocessor when
    // creating files using the set of openFiles.
    void setIsOpenInClient(bool isOpenInClient);

    // flag accessors
    bool isTestPath() const;
    bool isPackagedTest() const;
    bool isPackagedTestHelper() const;

    bool hasIndexErrors() const;
    void setHasIndexErrors(bool value);

    // `sourceIsPathContents` says that the text is exactly what `path_` holds on disk, so that the text may be
    // dropped once the file has been indexed and read back if an error in it has to be rendered (see
    // `releaseSource`). The indexer passes true for the files it reads; anything synthesised -- the payload, `-e`
    // input, a buffer an editor sent us -- leaves it false, because there is nothing to read back.
    File(std::string &&path_, std::string &&source_, Type sourceType, uint32_t epoch = 0,
         bool sourceIsPathContents = false);
    File(File &&other) = delete;
    File(const File &other) = delete;
    File() = delete;
    std::unique_ptr<File> deepCopy(GlobalState &) const;

    // Maps a 0-indexed line number to the offset of the end of the line
    //
    // If the line ends with the end of the file, then the offset points to the end of the file
    // If the line ends with a newline character, then the offset points to the newline character.
    //
    // This means that for a file contents like "foo\n", this is considered two lines:
    // - line 0: "foo"
    // - line 1: ""
    absl::Span<const uint32_t> lineBreaks() const;
    int lineCount() const;
    StrictLevel minErrorLevel() const;

    /** Given a 1-based line number, returns a string view of the line. */
    std::string_view getLine(int i) const;

    void setFileHash(std::unique_ptr<const FileHash> hash);
    const std::shared_ptr<const FileHash> &getFileHash() const;

    static constexpr std::string_view URL_PREFIX = "https://github.com/sorbet/sorbet/tree/master/";
    static std::string censorFilePathForSnapshotTests(std::string_view orig);

    // Returns the hash of the file content. Requires that the file has been read.
    absl::Span<const uint8_t> sourceHash() const;

    // Drops the file's text, which `source()` then reads back from `path()` on demand. Sorbet holds every file's text
    // from the moment it is read until the process exits, which is gigabytes on a large codebase, and the only thing
    // that wants it after indexing is rendering an error in the file -- true of a few thousand files, not all of them.
    // A no-op unless the text is known to be exactly what is on disk (see the constructor).
    //
    // Call this from the thread that has just finished with the file, as the indexer does: a `std::string_view` handed
    // out by `source()` on another thread does not keep the text alive.
    void releaseSource() const;

private:
    struct Flags {
        // some reasonable invariants don't hold for invalid files
        bool hasIndexErrors : 1;
        // only relevant in --sorbet-packages mode: is the file contained in a `/test/` directory?
        bool isTestPath : 1;
        // only relevant in --sorbet-packages mode: is the file a `.test.rb` file?
        bool isTestFile : 1;
        // Caches whether the file's path ends with `*.package.rbi`, i.e. whether the file is an RBI
        // generated by --package-gen-output.
        bool hasPackageRBIPath : 1;
        // Caches whether the file's path ends with `__package.rb`. Should not be accessed directly
        // unless we've already also checked whether `--sorbet-packages` is enabled.
        bool hasPackageRbPath : 1;
        // Documented on public accessors
        bool isOpenInClient : 1;

        Flags(std::string_view path);
    };
    CheckSize(Flags, 1, 1);

    Flags flags;

    const std::string path_;
    // The text, once read. `releaseSource` may drop it and `source()` read it back, so this is loaded atomically; it is
    // only ever replaced by an equal-length string (see `source()`).
    mutable std::shared_ptr<const std::string> source_;
    const uint32_t sourceSize_;
    const bool sourceIsPathContents_;

    // This is always a pure function of the `source_` string, so it is computed lazily, thus the
    // `mutable`. We generally don't need `lineBreaks_` for every file, unless we're showing errors
    // for that file, and computing lazily allows saving space.
    mutable std::shared_ptr<std::vector<uint32_t>> lineBreaks_;

    // This is the XXH128 hash of the source, lazily computed.
    mutable XXH128_hash_t sourceHash_;

    mutable bool sourceHashComputed_ = false;
    mutable StrictLevel minErrorLevel_ = StrictLevel::Max;

public:
    const StrictLevel originalSigil;
    StrictLevel strictLevel;

private:
    std::shared_ptr<const FileHash> hash_;
};

CheckSize(File, 112, 8);

template <typename H> H AbslHashValue(H h, const FileRef &m) {
    return H::combine(std::move(h), m.id());
}
} // namespace sorbet::core
#endif // SORBET_AST_FILES_H
