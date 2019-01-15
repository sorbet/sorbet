#ifndef TEST_POSITION_ASSERTIONS_H
#define TEST_POSITION_ASSERTIONS_H

#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"
#include "test/expectations.h"

namespace sorbet::test {
using namespace sorbet::realmain::lsp;

// Compares the two ranges. Returns -1 if `a` comes before `b`, 1 if `b` comes before `a`, and 0 if they are equivalent.
// If range `a` starts at the same character as `b` but ends earlier, it comes before `b`.
int rangeComparison(const std::unique_ptr<Range> &a, const std::unique_ptr<Range> &b);

// Compares the two errors. Returns -1 if `a` comes before `b`, 1 if `b` comes before `a`, and 0 if they are equivalent.
// Compares filenames, then ranges, and then compares messages in the event of a tie.
int errorComparison(std::string_view aFilename, const std::unique_ptr<Range> &a, std::string_view aMessage,
                    std::string_view bFilename, const std::unique_ptr<Range> &b, std::string_view bMessage);

/**
 * prettyPrintComment("foo.bar", {start: {character: 4}, end: {character: 7}}, "error: bar not defined") ->
 * foo.bar
 *     ^^^ error: bar not defined
 */
std::string prettyPrintRangeComment(std::string_view sourceLine, const std::unique_ptr<Range> &range,
                                    std::string_view comment);

// Converts a relative file path into an absolute file:// URI.
std::string filePathToUri(std::string_view prefixUrl, std::string_view filePath);

// Converts a URI into a relative file path.
std::string uriToFilePath(std::string_view prefixUrl, std::string_view uri);

class ErrorAssertion;
class LSPTest;

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

    /** Compares this assertion's filename and range with the given filename and range. Returns 0 if it matches, -1 if
     * range comes before otherRange, and 1 if otherRange comes before range. Unlike rangeComparison, this function
     * supports line-only ranges. */
    int compare(std::string_view otherFilename, const std::unique_ptr<Range> &otherRange);

    // Returns a Location object for this assertion's filename and range.
    std::unique_ptr<Location> getLocation(std::string_view uriPrefix);

    virtual std::string toString() = 0;
};

// # ^^^ error: message
class ErrorAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<ErrorAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                int assertionLine, std::string_view assertionContents);

    const std::string message;

    ErrorAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                   std::string_view message);

    std::string toString() override;

    void check(const std::unique_ptr<Diagnostic> &diagnostic, std::string_view sourceLine);
};

// # ^^^ def: symbol
class DefAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<DefAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                              int assertionLine, std::string_view assertionContents);

    const std::string symbol;
    const int version;

    DefAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine, std::string_view symbol,
                 int version);

    void check(LSPTest &test, std::string_view uriPrefix, const Location &queryLoc);

    std::string toString() override;
};

// # ^^^ usage: symbol
class UsageAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<UsageAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                int assertionLine, std::string_view assertionContents);

    static void check(LSPTest &test, std::string_view uriPrefix, std::string_view symbol, const Location &queryLoc,
                      const std::vector<std::shared_ptr<RangeAssertion>> &allLocs);

    const std::string symbol;
    const int version;
    std::shared_ptr<DefAssertion> def;

    UsageAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine, std::string_view symbol,
                   int version);

    std::string toString() override;
};

} // namespace sorbet::test
#endif // TEST_POSITION_ASSERTIONS_H