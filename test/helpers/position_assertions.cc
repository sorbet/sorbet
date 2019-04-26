#include "gtest/gtest.h"
// ^ Include first because it violates linting rules.

#include "absl/strings/str_split.h"
#include "test/helpers/lsp.h"
#include "test/helpers/position_assertions.h"
#include <regex>

using namespace std;

namespace sorbet::test {
// Matches '    #    ^^^^^ label: dafhdsjfkhdsljkfh*&#&*%'
// and '    # label: foobar'.
const regex rangeAssertionRegex("(#[ ]*)(\\^*)[ ]*([a-zA-Z-]+):[ ]+(.*)$");

const regex whitespaceRegex("^[ ]*$");

// Maps assertion comment names to their constructors.
const UnorderedMap<string, function<shared_ptr<RangeAssertion>(string_view, unique_ptr<Range> &, int, string_view)>>
    assertionConstructors = {
        {"error", ErrorAssertion::make},
        {"usage", UsageAssertion::make},
        {"def", DefAssertion::make},
        {"disable-fast-path", DisableFastPath::make},
};

// Ignore any comments that have these labels (e.g. `# typed: true`).
const UnorderedSet<string> ignoredAssertionLabels = {"typed", "TODO", "linearization", "commented-out-error"};

// Compares the two positions. Returns -1 if `a` comes before `b`, 1 if `b` comes before `a`, and 0 if they are
// equivalent.
int positionComparison(const Position &a, const Position &b) {
    if (a.line < b.line) {
        return -1; // Less than
    } else if (a.line > b.line) {
        return 1;
    } else {
        // Same line
        if (a.character < b.character) {
            return -1;
        } else if (a.character > b.character) {
            return 1;
        } else {
            // Same position.
            return 0;
        }
    }
}

int rangeComparison(const Range &a, const Range &b) {
    int startPosCmp = positionComparison(*a.start, *b.start);
    if (startPosCmp != 0) {
        return startPosCmp;
    }
    // Whichever range ends earlier comes first.
    return positionComparison(*a.end, *b.end);
}

/** Returns true if `b` is a subset of `a`. Only works on single-line ranges. Assumes ranges are well-formed (start <=
 * end) */
bool rangeIsSubset(const Range &a, const Range &b) {
    if (a.start->line != a.end->line || b.start->line != b.end->line || a.start->line != b.start->line) {
        return false;
    }

    // One-liners on same line.
    return b.start->character >= a.start->character && b.end->character <= a.end->character;
}

int errorComparison(string_view aFilename, const Range &a, string_view aMessage, string_view bFilename, const Range &b,
                    string_view bMessage) {
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

string prettyPrintRangeComment(string_view sourceLine, const Range &range, string_view comment) {
    int numLeadingSpaces = range.start->character;
    if (numLeadingSpaces < 0) {
        ADD_FAILURE() << fmt::format("Invalid range: {} < 0", range.start->character);
        return "";
    }
    string sourceLineNumber = fmt::format("{}", range.start->line + 1);
    EXPECT_EQ(range.start->line, range.end->line) << "Multi-line ranges are not supported at this time.";
    if (range.start->line != range.end->line) {
        return string(comment);
    }

    int numCarets = range.end->character - range.start->character;
    if (numCarets == RangeAssertion::END_OF_LINE_POS) {
        // Caret the entire line.
        numCarets = sourceLine.length();
    }

    return fmt::format("{}: {}\n {}{} {}", sourceLineNumber, sourceLine,
                       string(numLeadingSpaces + sourceLineNumber.length() + 1, ' '), string(numCarets, '^'), comment);
}

string_view getLine(const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents, string_view uriPrefix,
                    const Location &loc) {
    auto filename = uriToFilePath(uriPrefix, loc.uri);
    auto foundFile = sourceFileContents.find(filename);
    EXPECT_NE(sourceFileContents.end(), foundFile) << fmt::format("Unable to find file `{}`", filename);
    auto &file = foundFile->second;
    return file->getLine(loc.range->start->line + 1);
}

string filePathToUri(string_view prefixUrl, string_view filePath) {
    return fmt::format("{}/{}", prefixUrl, filePath);
}

string uriToFilePath(string_view prefixUrl, string_view uri) {
    if (uri.substr(0, prefixUrl.length()) != prefixUrl) {
        ADD_FAILURE() << fmt::format(
            "Unrecognized URI: `{}` is not contained in root URI `{}`, and thus does not correspond to a test file.",
            uri, prefixUrl);
        return "";
    }
    return string(uri.substr(prefixUrl.length() + 1));
}

RangeAssertion::RangeAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine)
    : filename(filename), range(move(range)), assertionLine(assertionLine) {}

int RangeAssertion::compare(string_view otherFilename, const Range &otherRange) {
    int filenamecmp = filename.compare(otherFilename);
    if (filenamecmp != 0) {
        return filenamecmp;
    }
    if (range->end->character == RangeAssertion::END_OF_LINE_POS) {
        // This assertion matches the whole line.
        // (Will match diagnostics that span multiple lines for parity with existing test logic.)
        int targetLine = range->start->line;
        if (targetLine >= otherRange.start->line && targetLine <= otherRange.end->line) {
            return 0;
        } else if (targetLine > otherRange.start->line) {
            return 1;
        } else {
            return -1;
        }
    }
    return rangeComparison(*range, otherRange);
}

ErrorAssertion::ErrorAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine, string_view message)
    : RangeAssertion(filename, range, assertionLine), message(message) {}

shared_ptr<ErrorAssertion> ErrorAssertion::make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                string_view assertionContents) {
    return make_shared<ErrorAssertion>(filename, range, assertionLine, assertionContents);
}

string ErrorAssertion::toString() const {
    return fmt::format("error: {}", message);
}

bool ErrorAssertion::check(const Diagnostic &diagnostic, string_view sourceLine, string_view errorPrefix) {
    // The error message must contain `message`.
    if (diagnostic.message.find(message) == string::npos) {
        ADD_FAILURE_AT(filename.c_str(), range->start->line + 1) << fmt::format(
            "{}Expected error of form:\n{}\nFound error:\n{}", errorPrefix,
            prettyPrintRangeComment(sourceLine, *range, toString()),
            prettyPrintRangeComment(sourceLine, *diagnostic.range, fmt::format("error: {}", diagnostic.message)));
        return false;
    }
    return true;
}

unique_ptr<Range> RangeAssertion::makeRange(int sourceLine, int startChar, int endChar) {
    return make_unique<Range>(make_unique<Position>(sourceLine, startChar), make_unique<Position>(sourceLine, endChar));
}

vector<shared_ptr<ErrorAssertion>>
RangeAssertion::getErrorAssertions(const vector<shared_ptr<RangeAssertion>> &assertions) {
    vector<shared_ptr<ErrorAssertion>> rv;
    for (auto assertion : assertions) {
        if (auto assertionOfType = dynamic_pointer_cast<ErrorAssertion>(assertion)) {
            rv.push_back(assertionOfType);
        }
    }
    return rv;
}

vector<shared_ptr<RangeAssertion>> parseAssertionsForFile(const shared_ptr<core::File> &file) {
    vector<shared_ptr<RangeAssertion>> assertions;

    int nextChar = 0;
    // 'line' is from line linenum
    int lineNum = 0;
    // The last non-comment-assertion line that we've encountered.
    // When we encounter a comment assertion, it will refer to this
    // line.
    int lastSourceLineNum = 0;

    auto source = file->source();
    auto filename = string(file->path().begin(), file->path().end());
    auto &lineBreaks = file->lineBreaks();

    for (auto lineBreak : lineBreaks) {
        // Ignore first line break entry.
        if (lineBreak == -1) {
            continue;
        }
        string_view lineView = source.substr(nextChar, lineBreak - nextChar);
        string line = string(lineView.begin(), lineView.end());
        nextChar = lineBreak + 1;

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
                                                 [](const auto &entry) -> string { return entry.first; }));
            }
        } else {
            lastSourceLineNum = lineNum;
        }
        lineNum += 1;
    }
    return assertions;
}

vector<shared_ptr<RangeAssertion>>
RangeAssertion::parseAssertions(const UnorderedMap<string, shared_ptr<core::File>> filesAndContents) {
    vector<shared_ptr<RangeAssertion>> assertions;
    for (auto &fileAndContents : filesAndContents) {
        auto fileAssertions = parseAssertionsForFile(fileAndContents.second);
        assertions.insert(assertions.end(), make_move_iterator(fileAssertions.begin()),
                          make_move_iterator(fileAssertions.end()));
    }

    // Sort assertions in (filename, range, message) order
    fast_sort(assertions, [](const shared_ptr<RangeAssertion> &a, const shared_ptr<RangeAssertion> &b) -> bool {
        return errorComparison(a->filename, *a->range, a->toString(), b->filename, *b->range, b->toString()) == -1;
    });

    return assertions;
}

unique_ptr<Location> RangeAssertion::getLocation(string_view uriPrefix) {
    auto newRange = make_unique<Range>(make_unique<Position>(range->start->line, range->start->character),
                                       make_unique<Position>(range->end->line, range->end->character));
    auto uri = filePathToUri(uriPrefix, filename);
    return make_unique<Location>(uri, move(newRange));
}

pair<string_view, int> getSymbolAndVersion(string_view assertionContents) {
    int version = 1;
    vector<string_view> split = absl::StrSplit(assertionContents, ' ');
    EXPECT_GE(split.size(), 0);
    EXPECT_LT(split.size(), 3) << fmt::format(
        "Invalid usage and def assertion; multiple words found:\n{}\nUsage and def assertions should be "
        "of the form:\n# [^*] [usage | def]: symbolname [version?]",
        assertionContents);
    if (split.size() == 2) {
        string_view versionString = split[1];
        version = stoi(string(versionString));
    }
    return pair<string_view, int>(split[0], version);
}

DefAssertion::DefAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine, string_view symbol,
                           int version)
    : RangeAssertion(filename, range, assertionLine), symbol(symbol), version(version) {}

shared_ptr<DefAssertion> DefAssertion::make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                            string_view assertionContents) {
    auto symbolAndVersion = getSymbolAndVersion(assertionContents);
    return make_shared<DefAssertion>(filename, range, assertionLine, symbolAndVersion.first, symbolAndVersion.second);
}

vector<unique_ptr<Location>> &extractLocations(ResponseMessage &respMsg) {
    auto &result = *(respMsg.result);
    auto &locationsOrNull = get<variant<JSONNullObject, vector<unique_ptr<Location>>>>(result);
    return get<vector<unique_ptr<Location>>>(locationsOrNull);
}

void DefAssertion::check(const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents, LSPWrapper &lspWrapper,
                         int &nextId, string_view uriPrefix, const Location &queryLoc) {
    const int line = queryLoc.range->start->line;
    // Can only query with one character, so just use the first one.
    const int character = queryLoc.range->start->character;
    auto locSourceLine = getLine(sourceFileContents, uriPrefix, queryLoc);
    auto defSourceLine = getLine(sourceFileContents, uriPrefix, *getLocation(uriPrefix));
    string locFilename = uriToFilePath(uriPrefix, queryLoc.uri);
    string defUri = filePathToUri(uriPrefix, filename);

    int id = nextId++;
    auto responses = lspWrapper.getLSPResponsesFor(*makeDefinitionRequest(id, queryLoc.uri, line, character));
    if (responses.size() != 1) {
        EXPECT_EQ(1, responses.size()) << "Unexpected number of responses to a `textDocument/definition` request.";
        return;
    }

    if (assertResponseMessage(id, *responses.at(0))) {
        auto &respMsg = responses.at(0)->asResponse();
        ASSERT_TRUE(respMsg.result.has_value());
        ASSERT_FALSE(respMsg.error.has_value());
        auto &locations = extractLocations(respMsg);

        if (locations.size() == 0) {
            ADD_FAILURE_AT(locFilename.c_str(), line + 1) << fmt::format(
                "Sorbet did not find a definition for location that references symbol `{}`.\nExpected definition "
                "of:\n{}\nTo be:\n{}",
                symbol, prettyPrintRangeComment(locSourceLine, *makeRange(line, character, character + 1), ""),
                prettyPrintRangeComment(defSourceLine, *range, ""));
            return;
        } else if (locations.size() > 1) {
            ADD_FAILURE_AT(locFilename.c_str(), line + 1) << fmt::format(
                "Sorbet unexpectedly returned multiple locations for definition of symbol `{}`.\nFor "
                "location:\n{}\nSorbet returned the following definition locations:\n{}",
                symbol, prettyPrintRangeComment(locSourceLine, *makeRange(line, character, character + 1), ""),
                fmt::map_join(locations, "\n", [&sourceFileContents, &uriPrefix](const auto &arg) -> string {
                    return prettyPrintRangeComment(getLine(sourceFileContents, uriPrefix, *arg), *arg->range, "");
                }));
            return;
        }

        auto &location = locations.at(0);
        // Note: Sorbet will point to the *statement* that defines the symbol, not just the symbol.
        // For example, it'll point to "class Foo" instead of just "Foo", or `5` in `a = 5` instead of `a`.
        // Thus, we just check that it returns the same line.
        if (location->uri != defUri || location->range->start->line != range->start->line) {
            string foundLocationString = "null";
            if (location != nullptr) {
                foundLocationString =
                    prettyPrintRangeComment(getLine(sourceFileContents, uriPrefix, *location), *location->range, "");
            }

            ADD_FAILURE_AT(filename.c_str(), line + 1)
                << fmt::format("Sorbet did not return the expected definition for location. Expected "
                               "definition of:\n{}\nTo be:\n{}\nBut was:\n{}",
                               prettyPrintRangeComment(locSourceLine, *makeRange(line, character, character + 1), ""),
                               prettyPrintRangeComment(defSourceLine, *range, ""), foundLocationString);
        }
    }
}

void UsageAssertion::check(const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                           LSPWrapper &lspWrapper, int &nextId, string_view uriPrefix, string_view symbol,
                           const Location &queryLoc, const vector<shared_ptr<RangeAssertion>> &allLocs) {
    const int line = queryLoc.range->start->line;
    // Can only query with one character, so just use the first one.
    const int character = queryLoc.range->start->character;
    auto locSourceLine = getLine(sourceFileContents, uriPrefix, queryLoc);
    string locFilename = uriToFilePath(uriPrefix, queryLoc.uri);

    auto referenceParams =
        make_unique<ReferenceParams>(make_unique<TextDocumentIdentifier>(queryLoc.uri),
                                     // TODO: Try with this false, too.
                                     make_unique<Position>(line, character), make_unique<ReferenceContext>(true));
    int id = nextId++;
    auto responses = lspWrapper.getLSPResponsesFor(
        LSPMessage(make_unique<RequestMessage>("2.0", id, LSPMethod::TextDocumentReferences, move(referenceParams))));
    if (responses.size() != 1) {
        EXPECT_EQ(1, responses.size()) << "Unexpected number of responses to a `textDocument/references` request.";
        return;
    }

    if (assertResponseMessage(id, *responses.at(0))) {
        auto &respMsg = responses.at(0)->asResponse();
        ASSERT_TRUE(respMsg.result.has_value());
        ASSERT_FALSE(respMsg.error.has_value());
        auto &locations = extractLocations(respMsg);
        fast_sort(locations, [&](const unique_ptr<Location> &a, const unique_ptr<Location> &b) -> bool {
            return errorComparison(a->uri, *a->range, "", b->uri, *b->range, "") == -1;
        });

        auto expectedLocationsIt = allLocs.begin();
        auto actualLocationsIt = locations.begin();
        while (expectedLocationsIt != allLocs.end() && actualLocationsIt != locations.end()) {
            auto expectedLocation = (*expectedLocationsIt)->getLocation(uriPrefix);
            auto &actualLocation = *actualLocationsIt;

            // If true, the expectedLocation is a subset of the actualLocation
            if (actualLocation->uri == expectedLocation->uri &&
                rangeIsSubset(*actualLocation->range, *expectedLocation->range)) {
                // Assertion passes. Consume both.
                actualLocationsIt++;
                expectedLocationsIt++;
            } else {
                switch (errorComparison(expectedLocation->uri, *expectedLocation->range, "", actualLocation->uri,
                                        *actualLocation->range, "")) {
                    case -1: {
                        // Expected location is *before* actual location.
                        auto expectedFilePath = uriToFilePath(uriPrefix, expectedLocation->uri);
                        ADD_FAILURE_AT(expectedFilePath.c_str(), expectedLocation->range->start->line + 1)
                            << fmt::format(
                                   "Sorbet did not report a reference to symbol `{}`.\nGiven symbol at:\n{}\nSorbet "
                                   "did not report reference at:\n{}",
                                   symbol,
                                   prettyPrintRangeComment(locSourceLine, *makeRange(line, character, character + 1),
                                                           ""),
                                   prettyPrintRangeComment(getLine(sourceFileContents, uriPrefix, *expectedLocation),
                                                           *expectedLocation->range, ""));
                        expectedLocationsIt++;
                        break;
                    }
                    case 1: {
                        // Expected location is *after* actual location
                        auto actualFilePath = uriToFilePath(uriPrefix, actualLocation->uri);
                        ADD_FAILURE_AT(actualFilePath.c_str(), actualLocation->range->start->line + 1) << fmt::format(
                            "Sorbet reported unexpected reference to symbol `{}`.\nGiven symbol "
                            "at:\n{}\nSorbet reported an unexpected reference at:\n{}",
                            symbol,
                            prettyPrintRangeComment(locSourceLine, *makeRange(line, character, character + 1), ""),
                            prettyPrintRangeComment(getLine(sourceFileContents, uriPrefix, *actualLocation),
                                                    *actualLocation->range, ""));
                        actualLocationsIt++;
                        break;
                    }
                    default:
                        // Should never happen.
                        ADD_FAILURE()
                            << "Error in test runner: identical locations weren't reported as subsets of one another.";
                        break;
                }
            }
        }

        while (expectedLocationsIt != allLocs.end()) {
            auto expectedLocation = (*expectedLocationsIt)->getLocation(uriPrefix);
            auto expectedFilePath = uriToFilePath(uriPrefix, expectedLocation->uri);
            ADD_FAILURE_AT(expectedFilePath.c_str(), expectedLocation->range->start->line + 1) << fmt::format(
                "Sorbet did not report a reference to symbol `{}`.\nGiven symbol at:\n{}\nSorbet "
                "did not report reference at:\n{}",
                symbol, prettyPrintRangeComment(locSourceLine, *makeRange(line, character, character + 1), ""),
                prettyPrintRangeComment(getLine(sourceFileContents, uriPrefix, *expectedLocation),
                                        *expectedLocation->range, ""));
            expectedLocationsIt++;
        }

        while (actualLocationsIt != locations.end()) {
            auto &actualLocation = *actualLocationsIt;
            auto actualFilePath = uriToFilePath(uriPrefix, actualLocation->uri);
            ADD_FAILURE_AT(actualFilePath.c_str(), actualLocation->range->start->line + 1) << fmt::format(
                "Sorbet reported unexpected reference to symbol `{}`.\nGiven symbol "
                "at:\n{}\nSorbet reported an unexpected reference at:\n{}",
                symbol, prettyPrintRangeComment(locSourceLine, *makeRange(line, character, character + 1), ""),
                prettyPrintRangeComment(getLine(sourceFileContents, uriPrefix, *actualLocation), *actualLocation->range,
                                        ""));
            actualLocationsIt++;
        }
    }
}

string DefAssertion::toString() const {
    return fmt::format("def: {}", symbol);
}

UsageAssertion::UsageAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine, string_view symbol,
                               int version)
    : RangeAssertion(filename, range, assertionLine), symbol(symbol), version(version) {}

shared_ptr<UsageAssertion> UsageAssertion::make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                string_view assertionContents) {
    auto symbolAndVersion = getSymbolAndVersion(assertionContents);
    return make_shared<UsageAssertion>(filename, range, assertionLine, symbolAndVersion.first, symbolAndVersion.second);
}

string UsageAssertion::toString() const {
    return fmt::format("usage: {}", symbol);
}

void reportMissingError(const string &filename, const ErrorAssertion &assertion, string_view sourceLine,
                        string_view errorPrefix) {
    ADD_FAILURE_AT(filename.c_str(), assertion.range->start->line + 1)
        << fmt::format("{}Did not find expected error:\n{}", errorPrefix,
                       prettyPrintRangeComment(sourceLine, *assertion.range, assertion.toString()));
}

void reportUnexpectedError(const string &filename, const Diagnostic &diagnostic, string_view sourceLine,
                           string_view errorPrefix) {
    ADD_FAILURE_AT(filename.c_str(), diagnostic.range->start->line + 1) << fmt::format(
        "{}Found unexpected error:\n{}", errorPrefix,
        prettyPrintRangeComment(sourceLine, *diagnostic.range, fmt::format("error: {}", diagnostic.message)));
}

/** Returns true if a and b are different Diagnostic objects but have the same range and message. */
bool isDuplicateDiagnostic(const Diagnostic *a, const Diagnostic *b) {
    return a != b && rangeComparison(*a->range, *b->range) == 0 && a->message == b->message;
}

string getSourceLine(const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents, const string &filename,
                     int line) {
    auto it = sourceFileContents.find(filename);
    if (it == sourceFileContents.end()) {
        ADD_FAILURE() << fmt::format("Unable to find referenced source file `{}`", filename);
        return "";
    }

    auto &file = it->second;
    if (line >= file->lineCount()) {
        ADD_FAILURE_AT(filename.c_str(), line + 1) << "Invalid line number for range.";
        return "";
    } else {
        // Note: line is a 0-indexed line number, but file uses 1-indexed line numbers.
        auto lineView = file->getLine(line + 1);
        return string(lineView.begin(), lineView.end());
    }
}

bool ErrorAssertion::checkAll(const UnorderedMap<string, shared_ptr<core::File>> &files,
                              vector<shared_ptr<ErrorAssertion>> errorAssertions,
                              map<string, vector<unique_ptr<Diagnostic>>> &filenamesAndDiagnostics,
                              string errorPrefix) {
    // Sort input error assertions so they are in (filename, line, column) order.
    fast_sort(errorAssertions, [](const shared_ptr<ErrorAssertion> &a, const shared_ptr<ErrorAssertion> &b) -> bool {
        return errorComparison(a->filename, *a->range, a->message, b->filename, *b->range, b->message) == -1;
    });

    auto assertionsIt = errorAssertions.begin();

    bool success = true;

    // Due to map's default sort order, this loop iterates over diagnostics in filename order.
    for (auto &filenameAndDiagnostics : filenamesAndDiagnostics) {
        auto &filename = filenameAndDiagnostics.first;
        auto &diagnostics = filenameAndDiagnostics.second;

        // Sort diagnostics within file in range, message order.
        // This explicit sort, combined w/ the map's implicit sort order, ensures that this loop iterates over
        // diagnostics in (filename, range, message) order -- matching the sort order of errorAssertions.
        fast_sort(diagnostics, [&filename](const unique_ptr<Diagnostic> &a, const unique_ptr<Diagnostic> &b) -> bool {
            return errorComparison(filename, *a->range, a->message, filename, *b->range, b->message) == -1;
        });

        auto diagnosticsIt = diagnostics.begin();
        auto *lastDiagnostic = diagnosticsIt == diagnostics.end() ? nullptr : (*diagnosticsIt).get();

        while (diagnosticsIt != diagnostics.end() && assertionsIt != errorAssertions.end()) {
            // See if the ranges match.
            auto &diagnostic = *diagnosticsIt;
            auto &assertion = *assertionsIt;

            // TODO: Remove duplicate diagnostics for parity with old runner.
            // Remove this check when ruby types team fixes duplicate diagnostics.
            if (isDuplicateDiagnostic(lastDiagnostic, diagnostic.get())) {
                diagnosticsIt++;
                continue;
            }
            lastDiagnostic = diagnostic.get();

            switch (assertion->compare(filename, *diagnostic->range)) {
                case 1: {
                    // Diagnostic comes *before* this assertion, so we don't
                    // have an assertion that matches the diagnostic.
                    reportUnexpectedError(filename, *diagnostic,
                                          getSourceLine(files, filename, diagnostic->range->start->line), errorPrefix);
                    // We've 'consumed' the diagnostic -- nothing matches it.
                    diagnosticsIt++;
                    success = false;
                    break;
                }
                case -1: {
                    // Diagnostic comes *after* this assertion
                    // We don't have a diagnostic that matches the assertion.
                    reportMissingError(assertion->filename, *assertion,
                                       getSourceLine(files, assertion->filename, assertion->range->start->line),
                                       errorPrefix);
                    // We've 'consumed' this error assertion -- nothing matches it.
                    assertionsIt++;
                    success = false;
                    break;
                }
                default: {
                    // Ranges match, so check the assertion.
                    success = assertion->check(*diagnostic,
                                               getSourceLine(files, assertion->filename, assertion->range->start->line),
                                               errorPrefix) &&
                              success;
                    // We've 'consumed' the diagnostic and assertion.
                    diagnosticsIt++;
                    assertionsIt++;
                    break;
                }
            }
        }

        while (diagnosticsIt != diagnostics.end()) {
            // We had more diagnostics than error assertions.
            // Drain dupes.
            // TODO: Remove when ruby types team fixes duplicate diagnostics; see note above.
            if (!isDuplicateDiagnostic(lastDiagnostic, (*diagnosticsIt).get())) {
                auto &diagnostic = *diagnosticsIt;
                reportUnexpectedError(filename, *diagnostic,
                                      getSourceLine(files, filename, diagnostic->range->start->line), errorPrefix);
                success = false;
            }
            lastDiagnostic = (*diagnosticsIt).get();
            diagnosticsIt++;
        }
    }

    while (assertionsIt != errorAssertions.end()) {
        // Had more error assertions than diagnostics
        reportMissingError((*assertionsIt)->filename, **assertionsIt,
                           getSourceLine(files, (*assertionsIt)->filename, (*assertionsIt)->range->start->line),
                           errorPrefix);
        success = false;
        assertionsIt++;
    }
    return success;
}

shared_ptr<DisableFastPath> DisableFastPath::make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                  string_view assertionContents) {
    return make_shared<DisableFastPath>(filename, range, assertionLine, assertionContents == "true");
}

bool DisableFastPath::getValue(const std::vector<std::shared_ptr<RangeAssertion>> &assertions) {
    for (auto &assertion : assertions) {
        if (auto disableFastPath = dynamic_pointer_cast<DisableFastPath>(assertion)) {
            return disableFastPath->value;
        }
    }
    return false;
}

DisableFastPath::DisableFastPath(string_view filename, unique_ptr<Range> &range, int assertionLine, bool value)
    : RangeAssertion(filename, range, assertionLine), value(value){};

string DisableFastPath::toString() const {
    return fmt::format("disable-fast-path: {}", value);
}

} // namespace sorbet::test
