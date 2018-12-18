#include "gtest/gtest.h"
// ^ Include first because it violates linting rules.

#include "test/position_assertions.h"
#include <regex>

// Matches '    #    ^^^^^ label: dafhdsjfkhdsljkfh*&#&*%'
// and '    # label: foobar'.
regex rangeAssertionRegex("(#[ ]*)(\\^*)[ ]*([a-zA-Z]+):[ ]+(.*)$");

regex whitespaceRegex("^[ ]*$");

// Maps assertion comment names to their constructors.
sorbet::UnorderedMap<string, function<shared_ptr<RangeAssertion>(string, unique_ptr<Range> &, int, string)>>
    assertionConstructors = {{"error", ErrorAssertion::make}};

// Ignore any comments that have these labels (e.g. `# typed: true`).
sorbet::UnorderedSet<string> ignoredAssertionLabels = {"typed", "TODO", "linearization"};

int positionComparison(const unique_ptr<Position> &a, const unique_ptr<Position> &b) {
    if (a->line < b->line) {
        return -1; // Less than
    } else if (a->line > b->line) {
        return 1;
    } else {
        // Same line
        if (a->character < b->character) {
            return -1;
        } else if (a->character > b->character) {
            return 1;
        } else {
            // Same position.
            return 0;
        }
    }
}

int rangeComparison(const unique_ptr<Range> &a, const unique_ptr<Range> &b) {
    int startPosCmp = positionComparison(a->start, b->start);
    if (startPosCmp != 0) {
        return startPosCmp;
    }
    // Whichever range ends earlier comes first.
    return positionComparison(a->end, b->end);
}

int errorComparison(const std::string &aFilename, const unique_ptr<Range> &a, const string &aMessage,
                    const std::string &bFilename, const unique_ptr<Range> &b, const string &bMessage) {
    int filecmp = aFilename.compare(bFilename);
    if (filecmp != 0) {
        return filecmp;
    }
    int rangecmp = rangeComparison(a, b);
    if (rangecmp != 0) {
        return rangecmp;
    }
    return aMessage.compare(bMessage);
}

string prettyPrintRangeComment(const string &sourceLine, const unique_ptr<Range> &range, const string &comment) {
    int numLeadingSpaces = range->start->character;
    if (numLeadingSpaces < 0) {
        ADD_FAILURE() << fmt::format("Invalid range: {} < 0", range->start->character);
        return "";
    }
    string sourceLineNumber = fmt::format("{}", range->start->line + 1);
    EXPECT_EQ(range->start->line, range->end->line) << "Multi-line ranges are not supported at this time.";
    if (range->start->line != range->end->line) {
        return comment;
    }

    int numCarets = range->end->character - range->start->character;
    if (numCarets == RangeAssertion::END_OF_LINE_POS) {
        // Caret the entire line.
        numCarets = sourceLine.length();
    }

    return fmt::format("{} {}\n {}{} {}", sourceLineNumber, sourceLine,
                       string(numLeadingSpaces + sourceLineNumber.length(), ' '), string(numCarets, '^'), comment);
}

RangeAssertion::RangeAssertion(string filename, unique_ptr<Range> &range, int assertionLine)
    : filename(filename), range(move(range)), assertionLine(assertionLine) {}

int RangeAssertion::compare(std::string otherFilename, const std::unique_ptr<Range> &otherRange) {
    int filenamecmp = filename.compare(otherFilename);
    if (filenamecmp != 0) {
        return filenamecmp;
    }
    if (range->end->character == RangeAssertion::END_OF_LINE_POS) {
        // This assertion matches the whole line.
        // (Will match diagnostics that span multiple lines for parity with existing test logic.)
        int targetLine = range->start->line;
        if (targetLine >= otherRange->start->line && targetLine <= otherRange->end->line) {
            return 0;
        } else if (targetLine > otherRange->start->line) {
            return 1;
        } else {
            return -1;
        }
    }
    return rangeComparison(range, otherRange);
}

ErrorAssertion::ErrorAssertion(string filename, unique_ptr<Range> &range, int assertionLine, string message)
    : RangeAssertion(filename, range, assertionLine), message(message) {}

shared_ptr<ErrorAssertion> ErrorAssertion::make(string filename, unique_ptr<Range> &range, int assertionLine,
                                                string assertionContents) {
    return make_shared<ErrorAssertion>(filename, range, assertionLine, assertionContents);
}

string ErrorAssertion::toString() {
    return fmt::format("error: {}", message);
}

void ErrorAssertion::check(const unique_ptr<Diagnostic> &diagnostic, const string &sourceLine) {
    // The error message must contain `message`. If message is MULTI, ignore.
    // TODO(jvilk): Migrate MULTI to actual multiple assertions.
    if (diagnostic->message.find(message) == std::string::npos && message != "MULTI") {
        ADD_FAILURE_AT(filename.c_str(), range->start->line + 1) << fmt::format(
            "Expected error of form:\n{}\nFound error:\n{}", prettyPrintRangeComment(sourceLine, range, toString()),
            prettyPrintRangeComment(sourceLine, diagnostic->range, fmt::format("error: {}", diagnostic->message)));
    }
}

unique_ptr<Range> RangeAssertion::makeRange(int sourceLine, int startChar, int endChar) {
    auto rv = make_unique<Range>();
    rv->start = make_unique<Position>();
    rv->end = make_unique<Position>();
    rv->start->line = sourceLine;
    rv->start->character = startChar;
    rv->end->line = sourceLine;
    rv->end->character = endChar;
    return rv;
}

vector<shared_ptr<RangeAssertion>> RangeAssertion::parseAssertions(const std::string filename,
                                                                   const std::string &fileContents) {
    vector<shared_ptr<RangeAssertion>> assertions;

    // Scan file for info.
    stringstream ss(fileContents);
    string line;
    // 'line' is from line linenum
    int lineNum = 0;
    // The last non-comment-assertion line that we've encountered.
    // When we encounter a comment assertion, it will refer to this
    // line.
    int lastSourceLineNum = 0;

    while (getline(ss, line, '\n')) {
        // Groups: Line up until first caret, carets, assertion type, assertion contents.
        smatch matches;
        if (regex_search(line, matches, rangeAssertionRegex)) {
            int numCarets = matches[2].str().size();
            auto textBeforeComment = matches.prefix().str();
            bool lineHasCode = !regex_match(textBeforeComment, whitespaceRegex);
            if (numCarets != 0) {
                // Position assertion assertions.
                if (lineNum == 0) {
                    ADD_FAILURE_AT(filename.c_str(), lineNum + 1) << fmt::format(
                        "Invalid assertion comment found on line 1, before any code:\n{}\nAssertion comments that "
                        "point to "
                        "specific character ranges with carets (^) should come after the code they point to.",
                        line);
                    // Ignore erroneous comment.
                    continue;
                }
            }

            if (numCarets == 0 && lineHasCode) {
                // Line-based assertion comment is on a line w/ code, meaning
                // the assertion is for that line.
                lastSourceLineNum = lineNum;
            }

            unique_ptr<Range> range;
            if (numCarets > 0) {
                int caretBeginPos = textBeforeComment.size() + matches[1].str().size();
                int caretEndPos = caretBeginPos + numCarets;
                range = RangeAssertion::makeRange(lastSourceLineNum, caretBeginPos, caretEndPos);
            } else {
                range = RangeAssertion::makeRange(lastSourceLineNum);
            }

            if (numCarets != 0 && lineHasCode) {
                // Character-based assertion comment is on line w/ code, so
                // next line could point to code on this line.
                lastSourceLineNum = lineNum;
            }

            string assertionType = matches[3].str();
            string assertionContents = matches[4].str();

            const auto &findConstructor = assertionConstructors.find(assertionType);
            if (findConstructor != assertionConstructors.end()) {
                assertions.push_back(findConstructor->second(filename, range, lineNum, assertionContents));
            } else if (ignoredAssertionLabels.find(assertionType) == ignoredAssertionLabels.end()) {
                ADD_FAILURE_AT(filename.c_str(), lineNum + 1)
                    << fmt::format("Found unrecognized assertion of type `{}`. Expected one of {{{}}}.\nIf this is a "
                                   "regular comment that just happens to be formatted like an assertion comment, you "
                                   "can add the label to `ignoredAssertionLabels`.",
                                   assertionType,
                                   fmt::map_join(assertionConstructors.begin(), assertionConstructors.end(), ", ",
                                                 [](const auto &entry) -> std::string { return entry.first; }));
            }
        } else {
            lastSourceLineNum = lineNum;
        }
        lineNum += 1;
    }

    return assertions;
}
