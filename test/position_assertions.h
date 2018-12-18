#ifndef TEST_POSITION_ASSERTIONS_H
#define TEST_POSITION_ASSERTIONS_H

#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

using namespace std;
using namespace sorbet::realmain::lsp;

// Compares the two positions. Returns -1 if `a` comes before `b`, 1 if `b` comes before `a`, and 0 if they are
// equivalent.
int positionComparison(const unique_ptr<Position> &a, const unique_ptr<Position> &b);

// Compares the two ranges. Returns -1 if `a` comes before `b`, 1 if `b` comes before `a`, and 0 if they are equivalent.
// If range `a` starts at the same character as `b` but ends earlier, it comes before `b`.
int rangeComparison(const unique_ptr<Range> &a, const unique_ptr<Range> &b);

// Compares the two errors. Returns -1 if `a` comes before `b`, 1 if `b` comes before `a`, and 0 if they are equivalent.
// Compares filenames, then ranges, and then compares messages in the event of a tie.
int errorComparison(const std::string &aFilename, const unique_ptr<Range> &a, const string &aMessage,
                    const std::string &bFilename, const unique_ptr<Range> &b, const string &bMessage);

/**
 * prettyPrintComment("foo.bar", {start: {character: 4}, end: {character: 7}}, "error: bar not defined") ->
 * foo.bar
 *     ^^^ error: bar not defined
 */
std::string prettyPrintRangeComment(const string &sourceLine, const unique_ptr<Range> &range, const string &comment);

/**
 * An assertion that is relevant to a specific set of characters on a line.
 * If Range is set such that the start character is 0 and end character is END_OF_LINE_POS, then the assertion
 * only checks that the line matches.
 */
class RangeAssertion {
public:
    static constexpr int END_OF_LINE_POS = 0xffffff;

    /**
     * Returns all of the RangeAssertions contained in the file in order by range, followed by their string contents.
     * Also sanity checks the assertion comment format.
     */
    static std::vector<std::shared_ptr<RangeAssertion>> parseAssertions(const std::string filename,
                                                                        const std::string &fileContents);

    // Creates a Range object for a source line and character range. Matches arbitrary subset of line if only sourceLine
    // is provided.
    static std::unique_ptr<Range> makeRange(int sourceLine, int startChar = 0,
                                            int endChar = RangeAssertion::END_OF_LINE_POS);

    string filename;
    unique_ptr<Range> range;
    // Used to produce intelligent error messages when assertion comments are malformed/invalid
    int assertionLine;

    RangeAssertion(string filename, unique_ptr<Range> &range, int assertionLine);

    /** Compares this assertion's filename and range with the given filename and range. Returns 0 if it matches, -1 if
     * range comes before otherRange, and 1 if otherRange comes before range. Unlike rangeComparison, this function
     * supports line-only ranges. */
    int compare(string otherFilename, const std::unique_ptr<Range> &otherRange);

    virtual string toString() = 0;
};

// # ^^^ error: message
class ErrorAssertion final : public RangeAssertion {
public:
    static shared_ptr<ErrorAssertion> make(string filename, unique_ptr<Range> &range, int assertionLine,
                                           string assertionContents);

    string message;

    ErrorAssertion(string filename, unique_ptr<Range> &range, int assertionLine, string message);

    string toString() override;

    void check(const unique_ptr<Diagnostic> &diagnostic, const string &sourceLine);
};

#endif // TEST_POSITION_ASSERTIONS_H