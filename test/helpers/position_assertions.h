#ifndef TEST_HELPERS_POSITION_ASSERTIONS_H
#define TEST_HELPERS_POSITION_ASSERTIONS_H

#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"
#include "main/lsp/wrapper.h"
#include "test/helpers/expectations.h"

namespace sorbet::test {
using namespace sorbet::realmain::lsp;

// Compares the two errors. Returns -1 if `a` comes before `b`, 1 if `b` comes before `a`, and 0 if they are equivalent.
// Compares filenames, then ranges, and then compares messages in the event of a tie.
int errorComparison(std::string_view aFilename, const Range &a, std::string_view aMessage, std::string_view bFilename,
                    const Range &b, std::string_view bMessage);

/**
 * prettyPrintComment("foo.bar", {start: {character: 4}, end: {character: 7}}, "error: bar not defined") ->
 * foo.bar
 *     ^^^ error: bar not defined
 */
std::string prettyPrintRangeComment(std::string_view sourceLine, const Range &range, std::string_view comment);

// Converts a relative file path into an absolute file:// URI.
std::string filePathToUri(std::string_view prefixUrl, std::string_view filePath);

// Converts a URI into a relative file path.
std::string uriToFilePath(std::string_view prefixUrl, std::string_view uri);

class ErrorAssertion;

/**
 * An assertion that is relevant to a specific set of characters on a line.
 * If Range is set such that the start character is 0 and end character is END_OF_LINE_POS, then the assertion
 * only checks that the line matches.
 */
class RangeAssertion {
public:
    static constexpr int END_OF_LINE_POS = 0xffffff;

    /**
     * Returns all of the RangeAssertions contained in all of the files in order by filename, range, and string
     * contents. Also sanity checks the assertion comment format. Input is a map from filename => file contents.
     */
    static std::vector<std::shared_ptr<RangeAssertion>>
    parseAssertions(const UnorderedMap<std::string, std::shared_ptr<core::File>> filesAndContents);

    // Creates a Range object for a source line and character range. Matches arbitrary subset of line if only sourceLine
    // is provided.
    static std::unique_ptr<Range> makeRange(int sourceLine, int startChar = 0,
                                            int endChar = RangeAssertion::END_OF_LINE_POS);

    /**
     * Filters a vector of assertions and returns only ErrorAssertions.
     */
    static std::vector<std::shared_ptr<ErrorAssertion>>
    getErrorAssertions(const std::vector<std::shared_ptr<RangeAssertion>> &assertions);

    const std::string filename;
    const std::unique_ptr<Range> range;
    // Used to produce intelligent error messages when assertion comments are malformed/invalid
    const int assertionLine;

    RangeAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine);
    virtual ~RangeAssertion() = default;

    /** Checks if the provided filename and range matches the assertion. Returns 0 if they match, a
     * negative int if they come after, and a positive int if they come before. Unlike
     * `cmp`, this function has special logic for line-only ranges. */
    int matches(std::string_view otherFilename, const Range &otherRange);

    int cmp(const RangeAssertion &b) const;

    // Returns a Location object for this assertion's filename and range.
    std::unique_ptr<Location> getLocation(std::string_view uriPrefix);

    virtual std::string toString() const = 0;
};

// # ^^^ error: message
class ErrorAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<ErrorAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                int assertionLine, std::string_view assertionContents,
                                                std::string_view assertionType);

    /**
     * Given a set of position-based assertions and Sorbet-generated diagnostics, check that the assertions pass.
     */
    static bool checkAll(const UnorderedMap<std::string, std::shared_ptr<core::File>> &files,
                         std::vector<std::shared_ptr<ErrorAssertion>> errorAssertions,
                         std::map<std::string, std::vector<std::unique_ptr<Diagnostic>>> &filenamesAndDiagnostics,
                         std::string errorPrefix = "");

    const std::string message;
    const bool matchesDuplicateErrors;

    ErrorAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                   std::string_view message, bool matchesDuplicateErrors);

    std::string toString() const override;

    bool check(const Diagnostic &diagnostic, std::string_view sourceLine, std::string_view errorPrefix);
};

// # ^^^ def: symbol
class DefAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<DefAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                              int assertionLine, std::string_view assertionContents,
                                              std::string_view assertionType);

    const std::string symbol;
    const int version;

    DefAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine, std::string_view symbol,
                 int version);

    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
               int &nextId, std::string_view uriPrefix, const Location &queryLoc);

    std::string toString() const override;
};

// # ^^^ usage: symbol
class UsageAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<UsageAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                int assertionLine, std::string_view assertionContents,
                                                std::string_view assertionType);

    static void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                      LSPWrapper &wrapper, int &nextId, std::string_view uriPrefix, std::string_view symbol,
                      const Location &queryLoc, const std::vector<std::shared_ptr<RangeAssertion>> &allLocs);

    const std::string symbol;
    const int version;
    std::shared_ptr<DefAssertion> def;

    UsageAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine, std::string_view symbol,
                   int version);

    std::string toString() const override;
};

// # ^^^ type-def: symbol
class TypeDefAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<TypeDefAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                  int assertionLine, std::string_view assertionContents,
                                                  std::string_view assertionType);

    const std::string symbol;

    TypeDefAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                     std::string_view symbol);

    static void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                      LSPWrapper &wrapper, int &nextId, std::string_view uriPrefix, std::string_view symbol,
                      const Location &queryLoc, const std::vector<std::shared_ptr<RangeAssertion>> &allLocs);

    std::string toString() const override;
};

// # ^^^ type: symbol
class TypeAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<TypeAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                               int assertionLine, std::string_view assertionContents,
                                               std::string_view assertionType);

    const std::string symbol;
    std::shared_ptr<DefAssertion> def;

    TypeAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine, std::string_view symbol);

    std::string toString() const override;
};

// # disable-fast-path: true
// # assert-slow-path: true
class BooleanPropertyAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<BooleanPropertyAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                          int assertionLine, std::string_view assertionContents,
                                                          std::string_view assertionType);

    static std::optional<bool> getValue(std::string_view type,
                                        const std::vector<std::shared_ptr<RangeAssertion>> &assertions);

    const std::string assertionType;
    const bool value;

    BooleanPropertyAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine, bool value,
                             std::string_view assertionType);

    std::string toString() const override;
};

// # assert-fast-path: file1.rb,file2.rb,...
class FastPathAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<FastPathAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                   int assertionLine, std::string_view assertionContents,
                                                   std::string_view assertionType);
    static std::optional<std::shared_ptr<FastPathAssertion>>
    get(const std::vector<std::shared_ptr<RangeAssertion>> &assertions);

    FastPathAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                      std::optional<std::vector<std::string>> expectedFiles);

    const std::optional<std::vector<std::string>> expectedFiles;

    void check(SorbetTypecheckRunInfo &info, std::string_view folder, int updateVersion, std::string_view errorPrefix);

    std::string toString() const override;
};

// # ^ hover: foo
class HoverAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<HoverAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                int assertionLine, std::string_view assertionContents,
                                                std::string_view assertionType);
    /** Checks all HoverAssertions within the assertion vector. Skips over non-hover assertions.*/
    static void checkAll(const std::vector<std::shared_ptr<RangeAssertion>> &assertions,
                         const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                         LSPWrapper &wrapper, int &nextId, std::string_view uriPrefix, std::string errorPrefix = "");

    HoverAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                   std::string_view message);

    const std::string message;

    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
               int &nextId, std::string_view uriPrefix, std::string errorPrefix = "");

    std::string toString() const override;
};

// # ^ completion: foo
class CompletionAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<CompletionAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                     int assertionLine, std::string_view assertionContents,
                                                     std::string_view assertionType);
    /** Checks all CompletionAssertions within the assertion vector. Skips over non-CompletionAssertions. */
    static void checkAll(const std::vector<std::shared_ptr<RangeAssertion>> &assertions,
                         const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                         LSPWrapper &wrapper, int &nextId, std::string_view uriPrefix, std::string errorPrefix = "");

    CompletionAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                        std::string_view message);

    const std::string message;

    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
               int &nextId, std::string_view uriPrefix, std::string errorPrefix = "");

    std::string toString() const override;
};

// ^ apply-completion: [version] index
class ApplyCompletionAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<ApplyCompletionAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                          int assertionLine, std::string_view assertionContents,
                                                          std::string_view assertionType);

    /** Checks all ApplyCompletionAssertions within the assertion vector. Skips over non-ApplyCompletionAssertions. */
    static void checkAll(const std::vector<std::shared_ptr<RangeAssertion>> &assertions,
                         const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                         LSPWrapper &wrapper, int &nextId, std::string_view uriPrefix, std::string errorPrefix = "");

    ApplyCompletionAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                             std::string_view version, int index);

    // The part between [..] in the assertion which specifies which `.[..].rbedited` file to compare against
    const std::string version;
    // Index into CompletionItem list of which item to apply (zero-indexed)
    const int index;

    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
               int &nextId, std::string_view uriPrefix, std::string errorPrefix = "");

    std::string toString() const override;
};

// # ^^^ apply-code-action: [version] title
class ApplyCodeActionAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<ApplyCodeActionAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                          int assertionLine, std::string_view assertionContents,
                                                          std::string_view assertionType);

    ApplyCodeActionAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                             std::string_view version, std::string_view title);

    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
               const CodeAction &codeAction, std::string_view rootUri);

    const std::string title;
    const std::string version;

    std::string toString() const override;
};

// # symbol-search: "query"
class SymbolSearchAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<SymbolSearchAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                       int assertionLine, std::string_view assertionContents,
                                                       std::string_view assertionType);

    const std::string query;
    const int rank = 0;

    /** Checks all SymbolSearchAssertions within the assertion vector. Skips over non-CompletionAssertions. */
    static void checkAll(const std::vector<std::shared_ptr<RangeAssertion>> &assertions,
                         const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                         LSPWrapper &wrapper, int &nextId, std::string_view uriPrefix, std::string errorPrefix = "");

    SymbolSearchAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                          std::string_view query);

    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
               int &nextId, std::string_view uriPrefix, const Location &queryLoc);

    /** Returns true if the given symbol matches this assertion. */
    bool matches(std::string_view uriPrefix, std::shared_ptr<SymbolInformation> symbol);

    std::string toString() const override;
};

} // namespace sorbet::test
#endif // TEST_HELPERS_POSITION_ASSERTIONS_H
