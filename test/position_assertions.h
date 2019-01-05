#ifndef TEST_POSITION_ASSERTIONS_H
#define TEST_POSITION_ASSERTIONS_H

#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"
#include "test/expectations.h"

namespace sorbet::test {
using namespace sorbet::realmain::lsp;

// Compares the two ranges. Returns -1 if `a` comes before `b`, 1 if `b` comes before `a`, and 0 if they are equivalent.
// If range `a` starts at the same character as `b` but ends earlier, it comes before `b`.
int rangeComparison(const unique_ptr<Range> &a, const unique_ptr<Range> &b);

// Compares the two errors. Returns -1 if `a` comes before `b`, 1 if `b` comes before `a`, and 0 if they are equivalent.
// Compares filenames, then ranges, and then compares messages in the event of a tie.
int errorComparison(string_view aFilename, const unique_ptr<Range> &a, string_view aMessage, string_view bFilename,
                    const unique_ptr<Range> &b, string_view bMessage);

/**
 * prettyPrintComment("foo.bar", {start: {character: 4}, end: {character: 7}}, "error: bar not defined") ->
 * foo.bar
 *     ^^^ error: bar not defined
 */
std::string prettyPrintRangeComment(string_view sourceLine, const unique_ptr<Range> &range, string_view comment);

// Converts a relative file path into an absolute file:// URI.
std::string filePathToUri(string_view prefixUrl, string_view filePath);

// Converts a URI into a relative file path.
std::string uriToFilePath(string_view prefixUrl, string_view uri);

class ErrorAssertion;
class LSPRequestResponseAssertion;
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
    parseAssertions(const UnorderedMap<string, std::shared_ptr<core::File>> filesAndContents);

    // Creates a Range object for a source line and character range. Matches arbitrary subset of line if only sourceLine
    // is provided.
    static std::unique_ptr<Range> makeRange(int sourceLine, int startChar = 0,
                                            int endChar = RangeAssertion::END_OF_LINE_POS);

    /**
     * Filters a vector of assertions and returns only ErrorAssertions.
     */
    static vector<shared_ptr<ErrorAssertion>> getErrorAssertions(const vector<shared_ptr<RangeAssertion>> &assertions);

    /**
     * Filters a vector of assertions and returns only LSPRequestResponseAssertions.
     */
    static vector<shared_ptr<LSPRequestResponseAssertion>>
    getRequestResponseAssertions(const vector<shared_ptr<RangeAssertion>> &assertions);

    const string filename;
    const unique_ptr<Range> range;
    // Used to produce intelligent error messages when assertion comments are malformed/invalid
    const int assertionLine;

    RangeAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine);
    virtual ~RangeAssertion() = default;

    /** Compares this assertion's filename and range with the given filename and range. Returns 0 if it matches, -1 if
     * range comes before otherRange, and 1 if otherRange comes before range. Unlike rangeComparison, this function
     * supports line-only ranges. */
    int compare(string_view otherFilename, const std::unique_ptr<Range> &otherRange);

    // Returns a Location object for this assertion's filename and range.
    unique_ptr<Location> getLocation(string_view uriPrefix);

    virtual string toString() = 0;
};

// # ^^^ error: message
class ErrorAssertion final : public RangeAssertion {
public:
    static shared_ptr<ErrorAssertion> make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                           string_view assertionContents);

    const string message;

    ErrorAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine, string_view message);

    string toString() override;

    void check(const unique_ptr<Diagnostic> &diagnostic, string_view sourceLine);
};

/**
 * These assertions are only valid for LSP tests. They send one or more requests to LSP, and assert things on
 * the responses.
 */
class LSPRequestResponseAssertion : public RangeAssertion {
public:
    LSPRequestResponseAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine);
    ~LSPRequestResponseAssertion() = default;

    virtual void check(const Expectations &expectations, LSPTest &test, unique_ptr<JSONDocument<int>> &doc,
                       string_view uriPrefix, int &nextId) = 0;
};

class UsageAssertion;
// # ^^^ def: symbol
class DefAssertion final : public LSPRequestResponseAssertion {
private:
    void checkDefinition(const Expectations &expectations, LSPTest &test, const string_view uriPrefix,
                         unique_ptr<JSONDocument<int>> &doc, const unique_ptr<Location> &loc, int character, int id,
                         string_view defSourceLine);

    void checkReferences(const Expectations &expectations, LSPTest &test, const string_view uriPrefix,
                         unique_ptr<JSONDocument<int>> &doc, const vector<unique_ptr<Location>> &allLocs,
                         const unique_ptr<Location> &loc, int character, int id, string_view defSourceLine);

public:
    static shared_ptr<DefAssertion> make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                         string_view assertionContents);

    const string symbol;
    vector<shared_ptr<UsageAssertion>> usages;

    DefAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine, string_view symbol);

    void check(const Expectations &expectations, LSPTest &test, unique_ptr<JSONDocument<int>> &doc,
               string_view uriPrefix, int &nextId) override;

    string toString() override;
};

// # ^^^ usage: symbol
// Is checked as a part of DefAssertion.
class UsageAssertion final : public RangeAssertion {
public:
    static shared_ptr<UsageAssertion> make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                           string_view assertionContents);

    const string symbol;
    shared_ptr<DefAssertion> def;

    UsageAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine, string_view symbol);

    string toString() override;
};

} // namespace sorbet::test
#endif // TEST_POSITION_ASSERTIONS_H