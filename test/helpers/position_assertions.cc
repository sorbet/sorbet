#include "doctest/doctest.h"
// ^ Include first because it violates linting rules.

#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "common/FileOps.h"
#include "common/concurrency/WorkerPool.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "main/lsp/LSPConfiguration.h"
#include "test/helpers/lsp.h"
#include "test/helpers/position_assertions.h"
#include <iterator>
#include <regex>
#include <string.h>

using namespace std;

namespace sorbet::test {

namespace {
/**
 * prettyPrintComment("foo.bar", {start: {character: 4}, end: {character: 7}}, "error: bar not defined") ->
 * foo.bar
 *     ^^^ error: bar not defined
 */
string prettyPrintRangeComment(string_view sourceLine, const Range &range, string_view comment) {
    int numLeadingSpaces = range.start->character;
    if (numLeadingSpaces < 0) {
        FAIL_CHECK(fmt::format("Invalid range: {} < 0", range.start->character));
        return "";
    }
    string sourceLineNumber = fmt::format("{}", range.start->line + 1);
    {
        INFO("Multi-line ranges are not supported at this time.");
        CHECK_EQ(range.start->line, range.end->line);
    }
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

template <typename T> bool isDuplicateDiagnostic(string_view filename, T *assertion, const Diagnostic &d) {
    return assertion && assertion->matchesDuplicateErrors && assertion->matches(filename, *d.range) == 0 &&
           d.message.find(assertion->message) != string::npos;
}

template <typename T>
void reportMissingError(const string &filename, const T &assertion, string_view sourceLine, string_view errorPrefix,
                        bool missingDuplicate = false) {
    auto coreMessage = missingDuplicate ? "Error was not duplicated" : "Did not find expected error";
    auto messagePostfix = missingDuplicate ? "\nYou can fix this error by changing the assertion to `error:`." : "";
    ADD_FAIL_CHECK_AT(filename.c_str(), assertion.range->start->line + 1,
                      fmt::format("{}{}:\n{}{}", errorPrefix, coreMessage,
                                  prettyPrintRangeComment(sourceLine, *assertion.range, assertion.toString()),
                                  messagePostfix));
}

void reportUnexpectedError(const string &filename, const Diagnostic &diagnostic, string_view sourceLine,
                           string_view errorPrefix) {
    auto diagnosticMessage = (diagnostic.severity == DiagnosticSeverity::Information)
                                 ? fmt::format("untyped: {}", diagnostic.message)
                                 : fmt::format("error: {}", diagnostic.message);
    ADD_FAIL_CHECK_AT(
        filename.c_str(), diagnostic.range->start->line + 1,
        fmt::format(
            "{}Found unexpected error:\n{}\nNote: If there is already an assertion for this error, then this is a "
            "duplicate error. Change the assertion to `# error-with-dupes: <error message>` if the duplicate is "
            "expected.",
            errorPrefix, prettyPrintRangeComment(sourceLine, *diagnostic.range, diagnosticMessage)));
}
string getSourceLine(const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents, const string &filename,
                     int line) {
    if (absl::StartsWith(filename, core::File::URL_PREFIX)) {
        return "";
    }

    auto it = sourceFileContents.find(filename);
    if (it == sourceFileContents.end()) {
        FAIL_CHECK(fmt::format("Unable to find referenced source file `{}`", filename));
        return "";
    }

    auto &file = it->second;
    if (line >= file->lineCount()) {
        ADD_FAIL_CHECK_AT(filename.c_str(), line + 1, "Invalid line number for range.");
        return "";
    } else {
        // Note: line is a 0-indexed line number, but file uses 1-indexed line numbers.
        auto lineView = file->getLine(line + 1);
        return string(lineView);
    }
}

template <typename T>
bool checkAllInner(const sorbet::UnorderedMap<string, shared_ptr<sorbet::core::File>> &files,
                   vector<shared_ptr<T>> errorAssertions,
                   map<string, vector<unique_ptr<Diagnostic>>> &filenamesAndDiagnostics, string errorPrefix) {
    // Sort input error assertions so they are in (filename, line, column) order.
    fast_sort(errorAssertions, sorbet::test::RangeAssertion::compareByRange);

    auto assertionsIt = errorAssertions.begin();

    bool success = true;

    // Due to map's default sort order, this loop iterates over diagnostics in filename order.
    for (auto &filenameAndDiagnostics : filenamesAndDiagnostics) {
        auto &filename = filenameAndDiagnostics.first;
        auto &diagnostics = filenameAndDiagnostics.second;

        // Sort diagnostics within file in range, message order.
        // This explicit sort, combined w/ the map's implicit sort order, ensures that this loop iterates over
        // diagnostics in (filename, range, message) order -- matching the sort order of errorAssertions.
        fast_sort(diagnostics, [](const unique_ptr<Diagnostic> &a, const unique_ptr<Diagnostic> &b) -> bool {
            const int rangeCmp = a->range->cmp(*b->range);
            if (rangeCmp != 0) {
                return rangeCmp < 0;
            }
            return a->message.compare(b->message) < 0;
        });

        auto diagnosticsIt = diagnostics.begin();
        T *lastAssertion = nullptr;
        bool lastAssertionMatchedDuplicate = false;

        while (diagnosticsIt != diagnostics.end() && assertionsIt != errorAssertions.end()) {
            // See if the ranges match.
            auto &diagnostic = *diagnosticsIt;
            auto &assertion = *assertionsIt;

            if (diagnostic->severity.value_or(T::severity) != T::severity) {
                diagnosticsIt++;
                continue;
            }

            if (isDuplicateDiagnostic(filename, lastAssertion, *diagnostic)) {
                diagnosticsIt++;
                lastAssertionMatchedDuplicate = true;
                continue;
            } else {
                if (lastAssertion && lastAssertion->matchesDuplicateErrors && !lastAssertionMatchedDuplicate) {
                    reportMissingError(lastAssertion->filename, *lastAssertion,
                                       getSourceLine(files, lastAssertion->filename, lastAssertion->range->start->line),
                                       errorPrefix, true);
                    success = false;
                }
                lastAssertionMatchedDuplicate = false;
                lastAssertion = nullptr;
            }

            const int cmp = assertion->matches(filename, *diagnostic->range);
            if (cmp > 0) {
                // Diagnostic comes *before* this assertion, so we don't
                // have an assertion that matches the diagnostic.
                reportUnexpectedError(filename, *diagnostic,
                                      getSourceLine(files, filename, diagnostic->range->start->line), errorPrefix);
                // We've 'consumed' the diagnostic -- nothing matches it.
                diagnosticsIt++;
                success = false;
            } else if (cmp < 0) {
                // Diagnostic comes *after* this assertion
                // We don't have a diagnostic that matches the assertion.
                reportMissingError(assertion->filename, *assertion,
                                   getSourceLine(files, assertion->filename, assertion->range->start->line),
                                   errorPrefix);
                // We've 'consumed' this error assertion -- nothing matches it.
                assertionsIt++;
                success = false;
            } else {
                // Ranges match, so check the assertion.
                success = assertion->check(*diagnostic,
                                           getSourceLine(files, assertion->filename, assertion->range->start->line),
                                           errorPrefix) &&
                          success;
                // We've 'consumed' the diagnostic and assertion.
                // Save assertion in case it matches multiple diagnostics.
                lastAssertion = assertion.get();
                diagnosticsIt++;
                assertionsIt++;
            }
        }

        while (diagnosticsIt != diagnostics.end()) {
            // We had more diagnostics than error assertions.
            auto &diagnostic = *diagnosticsIt;

            if (diagnostic->severity.value_or(T::severity) != T::severity) {
                diagnosticsIt++;
                continue;
            }

            if (isDuplicateDiagnostic(filename, lastAssertion, *diagnostic)) {
                lastAssertionMatchedDuplicate = true;
            } else {
                reportUnexpectedError(filename, *diagnostic,
                                      getSourceLine(files, filename, diagnostic->range->start->line), errorPrefix);
                success = false;

                if (lastAssertion && lastAssertion->matchesDuplicateErrors && !lastAssertionMatchedDuplicate) {
                    reportMissingError(lastAssertion->filename, *lastAssertion,
                                       getSourceLine(files, lastAssertion->filename, lastAssertion->range->start->line),
                                       errorPrefix, true);
                }
                lastAssertion = nullptr;
                lastAssertionMatchedDuplicate = false;
            }
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

// Matches '    #    ^^^^^ label: dafhdsjfkhdsljkfh*&#&*%'
// and '    # label: foobar'.
const regex rangeAssertionRegex("(#[ ]*)(\\^*|\\|*)[ ]*([a-zA-Z0-9-]+): (.*)$");

const regex whitespaceRegex("^[ ]*$");

// Maps assertion comment names to their constructors.
const UnorderedMap<
    string, function<shared_ptr<RangeAssertion>(string_view, unique_ptr<Range> &, int, string_view, string_view)>>
    assertionConstructors = {
        {"untyped", UntypedAssertion::make},
        {"error", ErrorAssertion::make},
        {"error-with-dupes", ErrorAssertion::make},
        {"usage", UsageAssertion::make},
        {"import", ImportAssertion::make},
        {"importusage", ImportUsageAssertion::make},
        {"def", DefAssertion::make},
        {"type", TypeAssertion::make},
        {"type-def", TypeDefAssertion::make},
        {"highlight-untyped-values", BooleanPropertyAssertion::make},
        {"disable-fast-path", BooleanPropertyAssertion::make},
        {"disable-stress-incremental", BooleanPropertyAssertion::make},
        {"stripe-mode", BooleanPropertyAssertion::make},
        {"check-out-of-order-constant-references", BooleanPropertyAssertion::make},
        {"enable-packager", BooleanPropertyAssertion::make},
        {"enable-experimental-requires-ancestor", BooleanPropertyAssertion::make},
        {"experimental-ruby3-keyword-args", BooleanPropertyAssertion::make},
        {"typed-super", BooleanPropertyAssertion::make},
        {"enable-suggest-unsafe", BooleanPropertyAssertion::make},
        {"enable-experimental-lsp-extract-to-variable", BooleanPropertyAssertion::make},
        {"selective-apply-code-action", StringPropertyAssertions::make},
        {"use-code-action-resolve", BooleanPropertyAssertion::make},
        {"assert-no-code-action", StringPropertyAssertions::make},
        {"assert-fast-path", FastPathAssertion::make},
        {"assert-slow-path", BooleanPropertyAssertion::make},
        {"hover", HoverAssertion::make},
        {"hover-line", HoverLineAssertion::make},
        {"completion", CompletionAssertion::make},
        {"apply-completion", ApplyCompletionAssertion::make},
        {"apply-code-action", ApplyCodeActionAssertion::make},
        {"no-stdlib", BooleanPropertyAssertion::make},
        {"symbol-search", SymbolSearchAssertion::make},
        {"apply-rename", ApplyRenameAssertion::make},
        {"extra-package-files-directory-prefix-underscore", StringPropertyAssertion::make},
        {"extra-package-files-directory-prefix-slash", StringPropertyAssertion::make},
        {"allow-relaxed-packager-checks-for", StringPropertyAssertion::make},
        {"implementation", ImplementationAssertion::make},
        {"find-implementation", FindImplementationAssertion::make},
        {"show-symbol", ShowSymbolAssertion::make},
        {"enable-typed-false-completion-nudges", BooleanPropertyAssertion::make},
        {"go-to-def-special", GoToDefSpecialAssertion::make},
};

// Ignore any comments that have these labels (e.g. `# typed: true`).
const UnorderedSet<string> ignoredAssertionLabels = {
    "typed",         "TODO",
    "linearization", "commented-out-error",
    "Note",          "See",
    "packaged",      "rubyfmt-force-exit",
    "compiled",      "exclude-from-file-update",
};

constexpr string_view NOTHING_LABEL = "(nothing)"sv;
constexpr string_view NULL_LABEL = "null"sv;

/** Returns true if `b` is a subset of `a`. Only works on single-line ranges. Assumes ranges are well-formed (start <=
 * end) */
bool rangeIsSubset(const Range &a, const Range &b) {
    if (a.start->line != a.end->line || b.start->line != b.end->line || a.start->line != b.start->line) {
        return false;
    }

    // One-liners on same line.
    return b.start->character >= a.start->character && b.end->character <= a.end->character;
}

string_view getLine(const LSPConfiguration &config,
                    const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents, const Location &loc) {
    auto filename = uriToFilePath(config, loc.uri);
    auto foundFile = sourceFileContents.find(filename);
    {
        INFO(fmt::format("Unable to find file `{}`", filename));
        CHECK_NE(sourceFileContents.end(), foundFile);
    }
    auto &file = foundFile->second;
    return file->getLine(loc.range->start->line + 1);
}

struct LocationDiff {
    vector<shared_ptr<Location>> missing;
    vector<unique_ptr<Location>> unexpected;
};

// Given a list of expected locations and a list of actual locations returned by LSP, return the list of missing
// locations in actualLocs and unexpected locations in actualLocs. Uses `cmp` to determine sort order and if locations
// match. Note: Mutates input vectors to sort them by cmp.
LocationDiff diffLocations(vector<unique_ptr<Location>> &expectedLocs, vector<unique_ptr<Location>> &actualLocs,
                           function<int(const Location &, const Location &)> cmp) {
    // Sort input vectors using cmp so we can compare them cheaply.
    auto sortLambda = [&cmp](const auto &a, const auto &b) -> bool { return cmp(*a, *b) < 0; };
    fast_sort(expectedLocs, sortLambda);
    fast_sort(actualLocs, sortLambda);

    LocationDiff output;
    auto expectedIt = expectedLocs.begin();
    auto actualIt = actualLocs.begin();
    while (expectedIt != expectedLocs.end() && actualIt != actualLocs.end()) {
        const auto &expectedLoc = *expectedIt;
        const auto &actualLoc = *actualIt;
        const auto cmpVal = cmp(*actualLoc, *expectedLoc);
        if (cmpVal > 0) {
            // Expected location is *before* actual location.
            output.missing.emplace_back(expectedLoc->copy());
            expectedIt++;
        } else if (cmpVal < 0) {
            // Expected location is *after* actual location
            output.unexpected.emplace_back(actualLoc->copy());
            actualIt++;
        } else {
            // cmpVal == 0; they match
            expectedIt++;
            actualIt++;
        }
    }
    while (expectedIt != expectedLocs.end()) {
        output.missing.emplace_back((*expectedIt)->copy());
        expectedIt++;
    }
    while (actualIt != actualLocs.end()) {
        output.unexpected.emplace_back((*actualIt)->copy());
        actualIt++;
    }
    return output;
}

void assertLocationsMatch(const LSPConfiguration &config,
                          const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents, string_view symbol,
                          const vector<shared_ptr<RangeAssertion>> &assertions, int line, int character,
                          string_view locSourceLine, string_view locFilename, vector<unique_ptr<Location>> &actualLocs,
                          string request) {
    vector<unique_ptr<Location>> expectedLocs;
    for (auto &assertion : assertions) {
        expectedLocs.emplace_back(assertion->getLocation(config));
    }
    auto diff = diffLocations(expectedLocs, actualLocs, [](const auto &a, const auto &b) -> int {
        if (a.uri == b.uri && rangeIsSubset(*a.range, *b.range)) {
            // If true, a is a subset of b; we treat these as equal.
            return 0;
        }
        return a.cmp(b);
    });

    for (auto &expectedLocation : diff.missing) {
        auto expectedFilePath = uriToFilePath(config, expectedLocation->uri);
        ADD_FAIL_CHECK_AT(
            expectedFilePath.c_str(), expectedLocation->range->start->line + 1,
            fmt::format(
                "Sorbet did not report a {} to symbol `{}`.\nGiven symbol at:\n{}\nSorbet "
                "did not report {} at:\n{}",
                request, symbol,
                prettyPrintRangeComment(locSourceLine, *RangeAssertion::makeRange(line, character, character + 1), ""),
                request,
                prettyPrintRangeComment(getLine(config, sourceFileContents, *expectedLocation),
                                        *expectedLocation->range, "")));
    }

    for (auto &unexpected : diff.unexpected) {
        auto unexpectedFilePath = uriToFilePath(config, unexpected->uri);
        ADD_FAIL_CHECK_AT(
            unexpectedFilePath.c_str(), unexpected->range->start->line + 1,
            fmt::format(
                "Sorbet reported unexpected {} to symbol `{}`.\nGiven symbol "
                "at:\n{}\nSorbet reported an unexpected (additional?) {} at:\n{}",
                request, symbol,
                prettyPrintRangeComment(locSourceLine, *RangeAssertion::makeRange(line, character, character + 1), ""),
                request,
                prettyPrintRangeComment(getLine(config, sourceFileContents, *unexpected), *unexpected->range, "")));
    }
}

string updatedFilePath(string filename, string version) {
    auto fileExtension = FileOps::getExtension(filename);

    return fmt::format("{}{}.rbedited", filename.substr(0, filename.size() - fileExtension.size()), version);
}
} // namespace

RangeAssertion::RangeAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine)
    : filename(filename), range(move(range)), assertionLine(assertionLine) {}

int RangeAssertion::matches(string_view otherFilename, const Range &otherRange) {
    const int filenamecmp = filename.compare(otherFilename);
    if (filenamecmp != 0) {
        return filenamecmp;
    }
    if (range->end->character == RangeAssertion::END_OF_LINE_POS) {
        // This assertion matches the whole line.
        // (Will match diagnostics that span multiple lines for parity with existing test logic.)
        const int targetLine = range->start->line;
        const int cmp = targetLine - otherRange.start->line;
        if (cmp >= 0 && targetLine <= otherRange.end->line) {
            return 0;
        } else {
            return cmp;
        }
    }
    if (otherRange.start->cmp(*otherRange.end) == 0) {
        // zero-width error message. Allow single `^` to match this
        // (VS Code will still show a red squiggle for a diagnostic that is zero-width)
        if (range->start->cmp(*otherRange.start) == 0 && range->end->line == otherRange.end->line &&
            range->end->character - 1 == otherRange.end->character) {
            return 0;
        }
    }
    return range->cmp(otherRange);
}

int RangeAssertion::cmp(const RangeAssertion &b) const {
    const int filenameCmp = filename.compare(b.filename);
    if (filenameCmp != 0) {
        return filenameCmp;
    }
    const int rangeCmp = range->cmp(*b.range);
    if (rangeCmp != 0) {
        return rangeCmp;
    }
    return toString().compare(b.toString());
}

ErrorAssertion::ErrorAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine, string_view message,
                               bool matchesDuplicateErrors)
    : RangeAssertion(filename, range, assertionLine), message(message), matchesDuplicateErrors(matchesDuplicateErrors) {
}

shared_ptr<ErrorAssertion> ErrorAssertion::make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                string_view assertionContents, string_view assertionType) {
    return make_shared<ErrorAssertion>(filename, range, assertionLine, assertionContents,
                                       assertionType == "error-with-dupes");
}

string ErrorAssertion::toString() const {
    return fmt::format("{}: {}", (matchesDuplicateErrors ? "error-with-dupes" : "error"), message);
}

bool ErrorAssertion::check(const Diagnostic &diagnostic, string_view sourceLine, string_view errorPrefix) {
    // The error message must contain `message`.
    if (diagnostic.message.find(message) == string::npos) {
        ADD_FAIL_CHECK_AT(filename.c_str(), range->start->line + 1,
                          fmt::format("{}Expected error of form:\n{}\nFound error:\n{}", errorPrefix,
                                      prettyPrintRangeComment(sourceLine, *range, toString()),
                                      prettyPrintRangeComment(sourceLine, *diagnostic.range,
                                                              fmt::format("error: {}", diagnostic.message))));
        return false;
    }
    return true;
}

UntypedAssertion::UntypedAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                   string_view message)
    : RangeAssertion(filename, range, assertionLine), message(message) {}

shared_ptr<UntypedAssertion> UntypedAssertion::make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                    string_view assertionContents, string_view assertionType) {
    return make_shared<UntypedAssertion>(filename, range, assertionLine, assertionContents);
}

string UntypedAssertion::toString() const {
    return fmt::format("{}: {}", "untyped", message);
}

bool UntypedAssertion::checkAll(const UnorderedMap<string, shared_ptr<core::File>> &files,
                                vector<shared_ptr<UntypedAssertion>> errorAssertions,
                                map<string, vector<unique_ptr<Diagnostic>>> &filenamesAndDiagnostics,
                                string errorPrefix) {
    return checkAllInner<UntypedAssertion>(files, errorAssertions, filenamesAndDiagnostics, errorPrefix);
}

bool UntypedAssertion::check(const Diagnostic &diagnostic, string_view sourceLine, string_view errorPrefix) {
    // The error message must contain `message`.
    if (diagnostic.severity != DiagnosticSeverity::Information || diagnostic.message.find(message) == string::npos) {
        auto diagnosticMessage = (diagnostic.severity == DiagnosticSeverity::Information)
                                     ? fmt::format("untyped: {}", diagnostic.message)
                                     : fmt::format("error: {}", diagnostic.message);
        ADD_FAIL_CHECK_AT(filename.c_str(), range->start->line + 1,
                          fmt::format("{}Expected information diagnostic of form:\n{}\nFound diagnostic:\n{}",
                                      errorPrefix, prettyPrintRangeComment(sourceLine, *range, toString()),
                                      prettyPrintRangeComment(sourceLine, *diagnostic.range, diagnosticMessage)));
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

vector<shared_ptr<UntypedAssertion>>
RangeAssertion::getUntypedAssertions(const vector<shared_ptr<RangeAssertion>> &assertions) {
    vector<shared_ptr<UntypedAssertion>> rv;
    for (auto assertion : assertions) {
        if (auto assertionOfType = dynamic_pointer_cast<UntypedAssertion>(assertion)) {
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
    auto filename = string(file->path());
    auto &lineBreaks = file->lineBreaks();

    for (auto lineBreak : lineBreaks) {
        // Ignore first line break entry.
        if (lineBreak == -1) {
            continue;
        }
        string_view lineView = source.substr(nextChar, lineBreak - nextChar);
        auto line = string(lineView);
        nextChar = lineBreak + 1;

        // Groups: Line up until first caret, carets, assertion type, assertion contents.
        smatch matches;
        if (regex_search(line, matches, rangeAssertionRegex)) {
            int numCarets = matches[2].str().size();
            bool zeroLenSelection = matches[2].str()[0] == '|';
            auto textBeforeComment = matches.prefix().str();
            bool lineHasCode = !regex_match(textBeforeComment, whitespaceRegex);
            if (numCarets != 0) {
                // Position assertion assertions.
                if (lineNum == 0) {
                    ADD_FAIL_CHECK_AT(
                        filename.c_str(), lineNum + 1,
                        fmt::format(
                            "Invalid assertion comment found on line 1, before any code:\n{}\nAssertion comments that "
                            "point to "
                            "specific character ranges with carets (^) should come after the code they point to.",
                            line));
                    // Ignore erroneous comment.
                    continue;
                }
                if (zeroLenSelection && numCarets > 1) {
                    // Position assertion with no selection
                    ADD_FAIL_CHECK_AT(filename.c_str(), lineNum + 1,
                                      fmt::format("Invalid assertion comment found on line {}:\n{}\nAssertions with | "
                                                  "should only point to 1 character",
                                                  lineNum, line));
                }
            }

            if (numCarets == 0 && lineHasCode) {
                // Line-based assertion comment is on a line w/ code, meaning
                // the assertion is for that line.
                lastSourceLineNum = lineNum;
            }

            string assertionType = matches[3].str();
            string assertionContents = matches[4].str();

            unique_ptr<Range> range;
            if (numCarets > 0) {
                int caretBeginPos = textBeforeComment.size() + matches[1].str().size();
                int caretEndPos = caretBeginPos + (zeroLenSelection ? 0 : numCarets);
                range = RangeAssertion::makeRange(lastSourceLineNum, caretBeginPos, caretEndPos);
            } else if (assertionContents == "unexpected token tNL") {
                range = RangeAssertion::makeRange(lineNum);
            } else if (assertionContents == "unexpected token \"end of file\"") {
                range = RangeAssertion::makeRange(lineNum);
            } else {
                range = RangeAssertion::makeRange(lastSourceLineNum);
            }

            if (numCarets != 0 && lineHasCode) {
                // Character-based assertion comment is on line w/ code, so
                // next line could point to code on this line.
                lastSourceLineNum = lineNum;
            }

            const auto &findConstructor = assertionConstructors.find(assertionType);
            if (findConstructor != assertionConstructors.end()) {
                assertions.push_back(
                    findConstructor->second(filename, range, lineNum, assertionContents, assertionType));
            } else if (!ignoredAssertionLabels.contains(assertionType)) {
                ADD_FAIL_CHECK_AT(
                    filename.c_str(), lineNum + 1,
                    fmt::format("Found unrecognized assertion of type `{}`. Expected one of {{{}}}.\nIf this is a "
                                "regular comment that just happens to be formatted like an assertion comment, you "
                                "can add the label to `ignoredAssertionLabels`.",
                                assertionType,
                                fmt::map_join(assertionConstructors, ", ",
                                              [](const auto &entry) -> string { return entry.first; })));
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
    fast_sort(assertions, RangeAssertion::compareByRange);

    return assertions;
}

unique_ptr<Location> RangeAssertion::getLocation(const LSPConfiguration &config) const {
    auto uri = filePathToUri(config, filename);
    return make_unique<Location>(uri, range->copy());
}

unique_ptr<DocumentHighlight> RangeAssertion::getDocumentHighlight() {
    return make_unique<DocumentHighlight>(range->copy());
}

tuple<string_view, vector<int>, string_view> getSymbolVersionAndOption(string_view assertionContents) {
    vector<int> versions;
    vector<string_view> split = absl::StrSplit(assertionContents, ' ');
    CHECK_GE(split.size(), 0);
    {
        INFO(fmt::format(
            "Invalid usage and def assertion; multiple words found:\n{}\nUsage and def assertions should be "
            "of the form:\n# [^*] [usage | def | type | type-def]: symbolname [version?] [option?]",
            assertionContents));
        CHECK_LT(split.size(), 4);
    }

    if (split.size() >= 2) {
        string_view versionString = split[1];
        for (auto str : absl::StrSplit(versionString, ',')) {
            versions.emplace_back(stoi(string(str)));
        }
    } else {
        versions.emplace_back(1);
    }

    string_view option;
    if (split.size() == 3) {
        option = split[2];
    }
    return make_tuple(split[0], versions, option);
}

DefAssertion::DefAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine, string_view symbol,
                           int version, bool isDefOfSelf, bool isDefaultArgValue)
    : RangeAssertion(filename, range, assertionLine), symbol(symbol), version(version), isDefOfSelf(isDefOfSelf),
      isDefaultArgValue(isDefaultArgValue) {}

shared_ptr<DefAssertion> DefAssertion::make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                            string_view assertionContents, string_view assertionType) {
    auto [symbol, versions, option] = getSymbolVersionAndOption(assertionContents);
    auto notDefOfSelf = option == "not-def-of-self";
    auto defaultExpr = option == "default-arg-value";
    if (!notDefOfSelf && !defaultExpr && !option.empty()) {
        ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                          fmt::format("Unexpected def assertion option: `{}`", option));
    }
    if (versions.size() > 1) {
        ADD_FAIL_CHECK_AT(
            string(filename).c_str(), assertionLine + 1,
            fmt::format("`def` assertions can only have a single version, but found multiple for symbol `{}` : {}",
                        symbol, absl::StrJoin(versions, ",")));
    }
    return make_shared<DefAssertion>(filename, range, assertionLine, symbol, versions[0], !notDefOfSelf, defaultExpr);
}

vector<unique_ptr<Location>> &extractLocations(ResponseMessage &respMsg) {
    static vector<unique_ptr<Location>> empty;
    auto &result = *(respMsg.result);
    auto &locationsOrNull = get<variant<JSONNullObject, vector<unique_ptr<Location>>>>(result);
    if (auto isNull = get_if<JSONNullObject>(&locationsOrNull)) {
        return empty;
    }
    return get<vector<unique_ptr<Location>>>(locationsOrNull);
}

vector<unique_ptr<DocumentHighlight>> &extractDocumentHighlights(ResponseMessage &respMsg) {
    static vector<unique_ptr<DocumentHighlight>> empty;
    auto &result = *(respMsg.result);
    auto &highlightsOrNull = get<variant<JSONNullObject, vector<unique_ptr<DocumentHighlight>>>>(result);
    if (auto isNull = get_if<JSONNullObject>(&highlightsOrNull)) {
        return empty;
    }
    return get<vector<unique_ptr<DocumentHighlight>>>(highlightsOrNull);
}

void DefAssertion::check(const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents, LSPWrapper &lspWrapper,
                         int &nextId, const Location &queryLoc,
                         const std::vector<std::shared_ptr<DefAssertion>> &definitions) {
    REQUIRE_FALSE(definitions.empty());
    const int line = queryLoc.range->start->line;
    // Can only query with one character, so just use the first one.
    const int character = queryLoc.range->start->character;
    const auto &config = lspWrapper.config();
    auto locSourceLine = getLine(config, sourceFileContents, queryLoc);
    string locFilename = uriToFilePath(config, queryLoc.uri);

    const int id = nextId++;
    auto responses = getLSPResponsesFor(lspWrapper, makeDefinitionRequest(id, queryLoc.uri, line, character));
    {
        INFO("Unexpected number of responses to a `textDocument/definition` request.");
        const auto numResponses = absl::c_count_if(responses, [](const auto &m) { return m->isResponse(); });
        REQUIRE_EQ(1, numResponses);
        const auto numRequests = absl::c_count_if(responses, [](const auto &m) { return m->isRequest(); });
        REQUIRE_EQ(0, numRequests);
        // Ensure the lone response is at the front.
        absl::c_partition(responses, [](const auto &m) { return m->isResponse(); });
        // We would like to verify the number of notifications here, but the number
        // of notifications depends on the typed-ness of the file as well as the
        // particular kind of query we're running, and we don't have access to the
        // latter here.  We can definitely assert that typed files have no notifications,
        // though.
        auto foundFile = sourceFileContents.find(locFilename);
        REQUIRE_NE(sourceFileContents.end(), foundFile);
        auto &file = foundFile->second;
        if (file->strictLevel >= core::StrictLevel::True) {
            const auto numNotifications =
                absl::c_count_if(responses, [](const auto &m) { return m->isNotification(); });
            REQUIRE_EQ(0, numNotifications);
        }
    }
    assertResponseMessage(id, *responses.at(0));

    auto &respMsg = responses.at(0)->asResponse();
    REQUIRE(respMsg.result.has_value());
    REQUIRE_FALSE(respMsg.error.has_value());
    auto &locations = extractLocations(respMsg);

    if (definitions.front()->symbol == NOTHING_LABEL) {
        // Special case: Nothing should be defined here.
        for (auto &location : locations) {
            ADD_FAIL_CHECK_AT(
                locFilename.c_str(), line + 1,
                fmt::format(
                    "Sorbet returned a definition for a location that we expected no definition for. For "
                    "location:\n{}\nFound definition:\n{}",
                    prettyPrintRangeComment(locSourceLine, *makeRange(line, character, character + 1), ""),
                    prettyPrintRangeComment(getLine(config, sourceFileContents, *location), *location->range, "")));
        }
        return;
    }

    vector<unique_ptr<Location>> expectedLocations;
    for (auto &def : definitions) {
        expectedLocations.emplace_back(def->getLocation(config));
    }
    auto diff = diffLocations(expectedLocations, locations, [](const auto &a, const auto &b) -> int {
        // Note: Sorbet will point to the *statement* that defines the symbol, not just the symbol.
        // For example, it'll point to "class Foo" instead of just "Foo", or `5` in `a = 5` instead of `a`.
        // Thus, we just check that it returns the same line.
        if (a.uri == b.uri && a.range->start->line == b.range->start->line) {
            return 0;
        }
        return a.cmp(b);
    });

    for (auto &missing : diff.missing) {
        ADD_FAIL_CHECK_AT(
            locFilename.c_str(), line + 1,
            fmt::format("Sorbet did not return an expected definition for location. Expected "
                        "definition of:\n{}\nTo include:\n{}",
                        prettyPrintRangeComment(locSourceLine, *makeRange(line, character, character + 1), ""),
                        prettyPrintRangeComment(getLine(config, sourceFileContents, *missing), *missing->range, "")));
    }

    for (auto &unexpected : diff.unexpected) {
        ADD_FAIL_CHECK_AT(
            locFilename.c_str(), line + 1,
            fmt::format(
                "Sorbet reported unexpected definition for location. Definition at:\n{}\nReported an "
                "unexpected (additional?) definition at:\n{}",
                prettyPrintRangeComment(locSourceLine, *makeRange(line, character, character + 1), ""),
                prettyPrintRangeComment(getLine(config, sourceFileContents, *unexpected), *unexpected->range, "")));
    }
}

void UsageAssertion::check(const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                           LSPWrapper &lspWrapper, int &nextId, string_view symbol, const Location &queryLoc,
                           const vector<shared_ptr<RangeAssertion>> &allLocs) {
    const int line = queryLoc.range->start->line;
    // Can only query with one character, so just use the first one.
    const int character = queryLoc.range->start->character;
    const auto &config = lspWrapper.config();
    auto locSourceLine = getLine(config, sourceFileContents, queryLoc);
    string locFilename = uriToFilePath(config, queryLoc.uri);

    auto referenceParams =
        make_unique<ReferenceParams>(make_unique<TextDocumentIdentifier>(queryLoc.uri),
                                     // TODO: Try with this false, too.
                                     make_unique<Position>(line, character), make_unique<ReferenceContext>(true));
    int id = nextId++;
    auto responses =
        getLSPResponsesFor(lspWrapper, make_unique<LSPMessage>(make_unique<RequestMessage>(
                                           "2.0", id, LSPMethod::TextDocumentReferences, move(referenceParams))));
    {
        INFO("Unexpected number of responses to a `textDocument/references` request.");
        const auto numResponses = absl::c_count_if(responses, [](const auto &m) { return m->isResponse(); });
        REQUIRE_EQ(1, numResponses);
        const auto numRequests = absl::c_count_if(responses, [](const auto &m) { return m->isRequest(); });
        REQUIRE_EQ(0, numRequests);
        // Ensure the lone response is at the front.
        absl::c_partition(responses, [](const auto &m) { return m->isResponse(); });
        // We would like to verify the number of notifications here, but the number
        // of notifications depends on the typed-ness of the file as well as the
        // particular kind of query we're running, and we don't have access to the
        // latter here.  We can definitely assert that typed files have no notifications,
        // though.
        auto foundFile = sourceFileContents.find(locFilename);
        REQUIRE_NE(sourceFileContents.end(), foundFile);
        auto &file = foundFile->second;
        if (file->strictLevel >= core::StrictLevel::True) {
            const auto numNotifications =
                absl::c_count_if(responses, [](const auto &m) { return m->isNotification(); });
            REQUIRE_EQ(0, numNotifications);
        }
    }

    assertResponseMessage(id, *responses.at(0));
    auto &respMsg = responses.at(0)->asResponse();
    REQUIRE(respMsg.result.has_value());
    REQUIRE_FALSE(respMsg.error.has_value());
    auto &locations = extractLocations(respMsg);
    if (symbol == NOTHING_LABEL) {
        // Special case: This location should not report usages of anything.
        for (auto &foundLocation : locations) {
            auto actualFilePath = uriToFilePath(config, foundLocation->uri);
            ADD_FAIL_CHECK_AT(
                actualFilePath.c_str(), foundLocation->range->start->line + 1,
                fmt::format(
                    "Sorbet returned references for a location that should not report references.\nGiven location "
                    "at:\n{}\nSorbet reported an unexpected reference at:\n{}",
                    prettyPrintRangeComment(locSourceLine, *makeRange(line, character, character + 1), ""),
                    prettyPrintRangeComment(getLine(config, sourceFileContents, *foundLocation), *foundLocation->range,
                                            "")));
        }
        return;
    }

    vector<shared_ptr<RangeAssertion>> newLocs;
    for (const auto &assertion : allLocs) {
        if (dynamic_pointer_cast<GoToDefSpecialAssertion>(assertion) == nullptr) {
            newLocs.emplace_back(assertion);
        }
    }
    assertLocationsMatch(config, sourceFileContents, symbol, newLocs, line, character, locSourceLine, locFilename,
                         locations, "reference");
}

void UsageAssertion::checkHighlights(const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                                     LSPWrapper &lspWrapper, int &nextId, string_view symbol, const Location &queryLoc,
                                     const vector<shared_ptr<RangeAssertion>> &allLocs) {
    const int line = queryLoc.range->start->line;
    // Can only query with one character, so just use the first one.
    const int character = queryLoc.range->start->character;
    const auto &config = lspWrapper.config();
    auto locSourceLine = getLine(config, sourceFileContents, queryLoc);
    string locFilename = uriToFilePath(config, queryLoc.uri);

    int id = nextId++;
    auto request = make_unique<LSPMessage>(make_unique<RequestMessage>(
        "2.0", id, LSPMethod::TextDocumentDocumentHighlight,
        make_unique<TextDocumentPositionParams>(make_unique<TextDocumentIdentifier>(string(queryLoc.uri)),
                                                make_unique<Position>(line, character))));

    auto responses = getLSPResponsesFor(lspWrapper, move(request));
    if (responses.size() != 1) {
        INFO("Unexpected number of responses to a `textDocument/documentHighlight` request.");
        CHECK_EQ(1, responses.size());
        return;
    }

    assertResponseMessage(id, *responses.at(0));
    auto &respMsg = responses.at(0)->asResponse();
    REQUIRE(respMsg.result.has_value());
    REQUIRE_FALSE(respMsg.error.has_value());
    auto &highlights = extractDocumentHighlights(respMsg);

    // Convert highlights to locations, used by common test functions across assertions involving multiple files.
    vector<unique_ptr<Location>> locations;
    for (auto const &highlight : highlights) {
        auto location = make_unique<Location>(queryLoc.uri, move(highlight->range));
        locations.push_back(move(location));
    }

    if (symbol == NOTHING_LABEL) {
        // Special case: This location should not report usages of anything.
        for (auto &foundLocation : locations) {
            auto actualFilePath = uriToFilePath(config, foundLocation->uri);
            ADD_FAIL_CHECK_AT(
                actualFilePath.c_str(), foundLocation->range->start->line + 1,
                fmt::format(
                    "Sorbet returned references for a highlight that should not report references.\nGiven location "
                    "at:\n{}\nSorbet reported an unexpected reference at:\n{}",
                    prettyPrintRangeComment(locSourceLine, *RangeAssertion::makeRange(line, character, character + 1),
                                            ""),
                    prettyPrintRangeComment(getLine(config, sourceFileContents, *foundLocation), *foundLocation->range,
                                            "")));
        }
        return;
    }

    vector<shared_ptr<RangeAssertion>> newLocs;
    for (const auto &assertion : allLocs) {
        if (dynamic_pointer_cast<GoToDefSpecialAssertion>(assertion) == nullptr) {
            newLocs.emplace_back(assertion);
        }
    }
    assertLocationsMatch(config, sourceFileContents, symbol, newLocs, line, character, locSourceLine, locFilename,
                         locations, "highlight");
}

string DefAssertion::toString() const {
    return fmt::format("def: {}", symbol);
}

UsageAssertion::UsageAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine, string_view symbol,
                               vector<int> versions)
    : RangeAssertion(filename, range, assertionLine), symbol(symbol), versions(versions) {}

shared_ptr<UsageAssertion> UsageAssertion::make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                string_view assertionContents, string_view assertionType) {
    auto [symbol, versions, option] = getSymbolVersionAndOption(assertionContents);
    if (!option.empty()) {
        ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                          fmt::format("Unexpected usage assertion option: `{}`", option));
    }
    return make_shared<UsageAssertion>(filename, range, assertionLine, symbol, versions);
}

ImportAssertion::ImportAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine, string_view symbol,
                                 vector<int> versions)
    : UsageAssertion(filename, range, assertionLine, symbol, versions) {}

shared_ptr<ImportAssertion> ImportAssertion::make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                  string_view assertionContents, string_view assertionType) {
    auto [symbol, versions, option] = getSymbolVersionAndOption(assertionContents);
    if (!option.empty()) {
        ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                          fmt::format("Unexpected import assertion option: `{}`", option));
    }
    return make_shared<ImportAssertion>(filename, range, assertionLine, symbol, versions);
}

ImportUsageAssertion::ImportUsageAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                           string_view symbol, vector<int> versions)
    : UsageAssertion(filename, range, assertionLine, symbol, versions) {}

shared_ptr<ImportUsageAssertion> ImportUsageAssertion::make(string_view filename, unique_ptr<Range> &range,
                                                            int assertionLine, string_view assertionContents,
                                                            string_view assertionType) {
    auto [symbol, versions, option] = getSymbolVersionAndOption(assertionContents);
    if (!option.empty()) {
        ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                          fmt::format("Unexpected importusage assertion option: `{}`", option));
    }
    return make_shared<ImportUsageAssertion>(filename, range, assertionLine, symbol, versions);
}

string UsageAssertion::toString() const {
    return fmt::format("usage: {}", symbol);
}

GoToDefSpecialAssertion::GoToDefSpecialAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                 string_view symbol, vector<int> versions)
    : UsageAssertion(filename, range, assertionLine, symbol, versions) {}

shared_ptr<GoToDefSpecialAssertion> GoToDefSpecialAssertion::make(string_view filename, unique_ptr<Range> &range,
                                                                  int assertionLine, string_view assertionContents,
                                                                  string_view assertionType) {
    auto [symbol, versions, option] = getSymbolVersionAndOption(assertionContents);
    if (!option.empty()) {
        ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                          fmt::format("Unexpected import assertion option: `{}`", option));
    }
    return make_shared<GoToDefSpecialAssertion>(filename, range, assertionLine, symbol, versions);
}

string GoToDefSpecialAssertion::toString() const {
    return fmt::format("go-to-def-special: {}", symbol);
}

TypeDefAssertion::TypeDefAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                   string_view symbol)
    : RangeAssertion(filename, range, assertionLine), symbol(symbol) {}

shared_ptr<TypeDefAssertion> TypeDefAssertion::make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                    string_view assertionContents, string_view assertionType) {
    auto [symbol, _versions, option] = getSymbolVersionAndOption(assertionContents);
    if (!option.empty()) {
        ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                          fmt::format("Unexpected type-def assertion option: `{}`", option));
    }
    return make_shared<TypeDefAssertion>(filename, range, assertionLine, symbol);
}

void TypeDefAssertion::check(const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                             LSPWrapper &lspWrapper, int &nextId, string_view symbol, const Location &queryLoc,
                             const std::vector<std::shared_ptr<RangeAssertion>> &typeDefs) {
    const int line = queryLoc.range->start->line;
    // Can only query with one character, so just use the first one.
    const int character = queryLoc.range->start->character;
    const auto &config = lspWrapper.config();
    auto locSourceLine = getLine(config, sourceFileContents, queryLoc);
    string locFilename = uriToFilePath(config, queryLoc.uri);

    const int id = nextId++;
    auto request = make_unique<LSPMessage>(make_unique<RequestMessage>(
        "2.0", id, LSPMethod::TextDocumentTypeDefinition,
        make_unique<TextDocumentPositionParams>(make_unique<TextDocumentIdentifier>(string(queryLoc.uri)),
                                                make_unique<Position>(line, character))));
    auto responses = getLSPResponsesFor(lspWrapper, move(request));
    REQUIRE_EQ(1, responses.size());

    assertResponseMessage(id, *responses.at(0));
    auto &respMsg = responses.at(0)->asResponse();
    REQUIRE(respMsg.result.has_value());
    REQUIRE_FALSE(respMsg.error.has_value());

    auto &locations = extractLocations(respMsg);

    if (symbol == NOTHING_LABEL) {
        // can't add type-def for NOTHING_LABEL
        for (auto &location : locations) {
            auto filePath = uriToFilePath(config, location->uri);
            ADD_FAIL_CHECK_AT(
                filePath.c_str(), location->range->start->line + 1,
                fmt::format(
                    "Sorbet returned references for a location that should not report references.\nGiven go to "
                    "type "
                    "def "
                    "here:\n{}\nSorbet reported an unexpected definition at:\n{}",
                    prettyPrintRangeComment(locSourceLine, *makeRange(line, character, character + 1), ""),
                    prettyPrintRangeComment(getLine(config, sourceFileContents, *location), *location->range, "")));
        }
        return;
    }

    if (typeDefs.empty()) {
        ADD_FAIL_CHECK_AT(
            locFilename.c_str(), line + 1,
            fmt::format("There are no 'type-def: {0}' assertions for this 'type: {0}' assertion:\n{1}\n"
                        "To assert that there are no results, use the {2} label",
                        symbol, prettyPrintRangeComment(locSourceLine, *makeRange(line, character, character + 1), ""),
                        NOTHING_LABEL));
        return;
    }

    assertLocationsMatch(config, sourceFileContents, symbol, typeDefs, line, character, locSourceLine, locFilename,
                         locations, "type definition");
}

string TypeDefAssertion::toString() const {
    return fmt::format("type-def: {}", symbol);
}

TypeAssertion::TypeAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine, string_view symbol)
    : RangeAssertion(filename, range, assertionLine), symbol(symbol) {}

shared_ptr<TypeAssertion> TypeAssertion::make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                              string_view assertionContents, string_view assertionType) {
    auto [symbol, _versions, option] = getSymbolVersionAndOption(assertionContents);
    if (!option.empty()) {
        ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                          fmt::format("Unexpected type assertion option: `{}`", option));
    }
    return make_shared<TypeAssertion>(filename, range, assertionLine, symbol);
}

string TypeAssertion::toString() const {
    return fmt::format("type: {}", symbol);
}

bool ErrorAssertion::checkAll(const UnorderedMap<string, shared_ptr<core::File>> &files,
                              vector<shared_ptr<ErrorAssertion>> errorAssertions,
                              map<string, vector<unique_ptr<Diagnostic>>> &filenamesAndDiagnostics,
                              string errorPrefix) {
    return checkAllInner<ErrorAssertion>(files, errorAssertions, filenamesAndDiagnostics, errorPrefix);
}

shared_ptr<BooleanPropertyAssertion> BooleanPropertyAssertion::make(string_view filename, unique_ptr<Range> &range,
                                                                    int assertionLine, string_view assertionContents,
                                                                    string_view assertionType) {
    {
        INFO("Unrecognized boolean property assertion value: " << assertionContents);
        auto trueOrFalse = assertionContents == "true" || assertionContents == "false";
        CHECK(trueOrFalse);
    }
    return make_shared<BooleanPropertyAssertion>(filename, range, assertionLine, assertionContents == "true",
                                                 assertionType);
}

optional<bool> BooleanPropertyAssertion::getValue(string_view type,
                                                  const vector<shared_ptr<RangeAssertion>> &assertions) {
    {
        INFO("Unrecognized boolean property assertion: " << type);
        CHECK_NE(assertionConstructors.find(string(type)), assertionConstructors.end());
    }
    for (auto &assertion : assertions) {
        if (auto boolAssertion = dynamic_pointer_cast<BooleanPropertyAssertion>(assertion)) {
            if (boolAssertion->assertionType == type) {
                return boolAssertion->value;
            }
        }
    }
    return nullopt;
}

BooleanPropertyAssertion::BooleanPropertyAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                   bool value, string_view assertionType)
    : RangeAssertion(filename, range, assertionLine), assertionType(string(assertionType)), value(value){};

string BooleanPropertyAssertion::toString() const {
    return fmt::format("{}: {}", assertionType, value);
}

shared_ptr<StringPropertyAssertion> StringPropertyAssertion::make(string_view filename, unique_ptr<Range> &range,
                                                                  int assertionLine, string_view assertionContents,
                                                                  string_view assertionType) {
    return make_shared<StringPropertyAssertion>(filename, range, assertionLine, assertionContents.data(),
                                                assertionType);
}

optional<std::string> StringPropertyAssertion::getValue(string_view type,
                                                        const vector<shared_ptr<RangeAssertion>> &assertions) {
    {
        INFO("Unrecognized string property assertion: " << type);
        CHECK_NE(assertionConstructors.find(string(type)), assertionConstructors.end());
    }
    for (auto &assertion : assertions) {
        if (auto stringAssertion = dynamic_pointer_cast<StringPropertyAssertion>(assertion)) {
            if (stringAssertion->assertionType == type) {
                return stringAssertion->value;
            }
        }
    }
    return nullopt;
}

StringPropertyAssertion::StringPropertyAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                 std::string value, string_view assertionType)
    : RangeAssertion(filename, range, assertionLine), assertionType(string(assertionType)), value(value){};

std::string StringPropertyAssertion::toString() const {
    return fmt::format("{}: {}", assertionType, value);
}

shared_ptr<FastPathAssertion> FastPathAssertion::make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                      string_view assertionContents, string_view assertionType) {
    optional<vector<string>> expectedFiles;
    if (!assertionContents.empty()) {
        if (assertionContents == "true") {
            auto filenameStr = string(filename);
            ADD_FAIL_CHECK_AT(filenameStr.c_str(), assertionLine,
                              "Unlike assert-slow-path, assert-fast-path takes a comma-separated list of file "
                              "basenames which should be typechecked on the fast path");
        }
        expectedFiles = absl::StrSplit(assertionContents, ',');
        fast_sort(*expectedFiles);
    }
    return make_shared<FastPathAssertion>(filename, range, assertionLine, std::move(expectedFiles));
}

optional<shared_ptr<FastPathAssertion>> FastPathAssertion::get(const vector<shared_ptr<RangeAssertion>> &assertions) {
    for (auto &assertion : assertions) {
        if (auto fastPathAssertion = dynamic_pointer_cast<FastPathAssertion>(assertion)) {
            return fastPathAssertion;
        }
    }
    return nullopt;
}

FastPathAssertion::FastPathAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                     optional<vector<string>> expectedFiles)
    : RangeAssertion(filename, range, assertionLine), expectedFiles(move(expectedFiles)) {}

void FastPathAssertion::check(SorbetTypecheckRunInfo &info, string_view folder, int updateVersion,
                              string_view errorPrefix) {
    string updateFile = fmt::format("{}.{}.rbupdate", filename.substr(0, -3), updateVersion);
    if (info.typecheckingPath != TypecheckingPath::Fast) {
        ADD_FAIL_CHECK_AT(updateFile.c_str(), assertionLine,
                          errorPrefix << "Expected file update to take fast path, but it took the slow path.");
    }
    if (expectedFiles.has_value()) {
        vector<string> expectedFilePaths;
        for (auto &f : *expectedFiles) {
            if (absl::StartsWith(f, core::File::URL_PREFIX)) {
                expectedFilePaths.push_back(f);
            } else {
                expectedFilePaths.push_back(absl::StrCat(folder, f));
            }
        }
        fast_sort(expectedFilePaths);
        fast_sort(info.filesTypechecked);
        vector<string> unTypecheckedFiles;
        absl::c_set_difference(expectedFilePaths, info.filesTypechecked, back_inserter(unTypecheckedFiles));
        for (auto &f : unTypecheckedFiles) {
            ADD_FAIL_CHECK_AT(updateFile.c_str(), assertionLine,
                              errorPrefix
                                  << fmt::format("Expected file update to cause {} to also be typechecked.", f));
        }
        vector<string> extraFiles;
        absl::c_set_difference(info.filesTypechecked, expectedFilePaths, back_inserter(extraFiles));
        for (auto &f : extraFiles) {
            ADD_FAIL_CHECK_AT(updateFile.c_str(), assertionLine,
                              errorPrefix << fmt::format("File update caused {} to be typechecked unexpectedly.", f));
        }
    }
}

string FastPathAssertion::toString() const {
    return fmt::format("FastPathAssertion: {}", expectedFiles ? fmt::format("{}", fmt::join(*expectedFiles, ",")) : "");
}

shared_ptr<HoverAssertion> HoverAssertion::make(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                string_view assertionContents, string_view assertionType) {
    return make_shared<HoverAssertion>(filename, range, assertionLine, assertionContents);
}
HoverAssertion::HoverAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine, string_view message)
    : RangeAssertion(filename, range, assertionLine), message(string(message)) {}

void HoverAssertion::checkAll(const vector<shared_ptr<RangeAssertion>> &assertions,
                              const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                              LSPWrapper &wrapper, int &nextId, string errorPrefix) {
    for (auto assertion : assertions) {
        if (auto assertionOfType = dynamic_pointer_cast<HoverAssertion>(assertion)) {
            assertionOfType->check(sourceFileContents, wrapper, nextId, errorPrefix);
        }
    }
}

// Retrieve contents of a Hover response as a string.
string_view hoverToString(variant<JSONNullObject, unique_ptr<Hover>> &hoverResult) {
    if (auto nullResp = get_if<JSONNullObject>(&hoverResult)) {
        return NULL_LABEL;
    } else {
        auto &hover = get<unique_ptr<Hover>>(hoverResult);
        string_view value = hover->contents->value;
        if (value.empty()) {
            return NOTHING_LABEL;
        }
        return value;
    }
}

// Returns `true` if `line` matches a full line of text in `text`.
bool containsLine(string_view text, string_view line) {
    for (int pos = text.find(line); pos != string::npos; pos = text.find(line, pos + 1)) {
        const bool startsOnNewLine = pos == 0 || text.at(pos - 1) == '\n';
        const bool endsLine = pos + line.size() == text.size() || text.at(pos + line.size()) == '\n';
        if (startsOnNewLine && endsLine) {
            return true;
        }
    }
    return false;
}

void HoverAssertion::check(const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
                           int &nextId, string errorPrefix) {
    const auto &config = wrapper.config();
    auto uri = filePathToUri(config, filename);
    auto pos = make_unique<TextDocumentPositionParams>(make_unique<TextDocumentIdentifier>(uri), range->start->copy());
    auto id = nextId++;
    auto msg = make_unique<LSPMessage>(make_unique<RequestMessage>("2.0", id, LSPMethod::TextDocumentHover, move(pos)));
    auto responses = getLSPResponsesFor(wrapper, move(msg));
    REQUIRE_EQ(responses.size(), 1);
    auto &responseMsg = responses.at(0);
    REQUIRE(responseMsg->isResponse());
    auto &response = responseMsg->asResponse();
    REQUIRE_MESSAGE(response.result.has_value(), response.error.value()->message);
    auto &hoverResponse = get<variant<JSONNullObject, unique_ptr<Hover>>>(*response.result);
    auto hoverContents = hoverToString(hoverResponse);

    // Match a full line. Makes it possible to disambiguate `String` and `T.nilable(String)`.
    if (!containsLine(hoverContents, this->message)) {
        auto sourceLine = getSourceLine(sourceFileContents, filename, range->start->line);
        ADD_FAIL_CHECK_AT(
            filename.c_str(), range->start->line + 1,
            fmt::format("{}Expected hover contents:\n{}\nFound hover contents:\n{}", errorPrefix,
                        prettyPrintRangeComment(sourceLine, *range, toString()),
                        prettyPrintRangeComment(sourceLine, *range, fmt::format("hover: {}", hoverContents))));
    }
}

string HoverAssertion::toString() const {
    return fmt::format("hover: {}", message);
}

shared_ptr<HoverLineAssertion> HoverLineAssertion::make(string_view filename, unique_ptr<Range> &range,
                                                        int assertionLine, string_view assertionContents,
                                                        string_view assertionType) {
    static const regex multilineRegex(R"(^([0-9]+) (.+)$)");

    smatch matches;
    string assertionContentsString = string(assertionContents);

    if (regex_search(assertionContentsString, matches, multilineRegex)) {
        auto lineno = stoi(matches[1].str());
        auto contents = matches[2].str();
        return make_shared<HoverLineAssertion>(filename, range, assertionLine, lineno, contents);
    }

    ADD_FAIL_CHECK_AT(
        string(filename).c_str(), assertionLine + 1,
        fmt::format("Improperly formatted hover-line assertion. Expected '<lineno> <contents>'. Found '{}'",
                    assertionContents, filename));

    return nullptr;
}
HoverLineAssertion::HoverLineAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine, int lineno,
                                       string_view message)
    : RangeAssertion(filename, range, assertionLine), lineno(lineno), message(string(message)) {}

void HoverLineAssertion::checkAll(const vector<shared_ptr<RangeAssertion>> &assertions,
                                  const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                                  LSPWrapper &wrapper, int &nextId, string errorPrefix) {
    for (auto assertion : assertions) {
        if (auto assertionOfType = dynamic_pointer_cast<HoverLineAssertion>(assertion)) {
            assertionOfType->check(sourceFileContents, wrapper, nextId, errorPrefix);
        }
    }
}

void HoverLineAssertion::check(const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                               LSPWrapper &wrapper, int &nextId, string errorPrefix) {
    const auto &config = wrapper.config();
    auto uri = filePathToUri(config, filename);
    auto pos = make_unique<TextDocumentPositionParams>(make_unique<TextDocumentIdentifier>(uri), range->start->copy());
    auto id = nextId++;
    auto msg = make_unique<LSPMessage>(make_unique<RequestMessage>("2.0", id, LSPMethod::TextDocumentHover, move(pos)));
    auto responses = getLSPResponsesFor(wrapper, move(msg));
    REQUIRE_EQ(responses.size(), 1);
    auto &responseMsg = responses.at(0);
    REQUIRE(responseMsg->isResponse());
    auto &response = responseMsg->asResponse();
    REQUIRE_MESSAGE(response.result.has_value(), response.error.value()->message);
    auto &hoverResponse = get<variant<JSONNullObject, unique_ptr<Hover>>>(*response.result);
    auto hoverContents = hoverToString(hoverResponse);
    vector<string> hoverLines = absl::StrSplit(hoverContents, "\n");

    REQUIRE_LE(1, this->lineno);
    REQUIRE_LE(this->lineno, hoverLines.size());

    // Match a full line. Makes it possible to disambiguate `String` and `T.nilable(String)`.
    if (hoverLines[this->lineno - 1] != this->message) {
        auto sourceLine = getSourceLine(sourceFileContents, filename, range->start->line);
        ADD_FAIL_CHECK_AT(filename.c_str(), range->start->line + 1,
                          fmt::format("{}Expected line {} of hover contents:\n{}\nComplete hover contents:\n{}",
                                      errorPrefix, this->lineno,
                                      prettyPrintRangeComment(sourceLine, *range, toString()),
                                      prettyPrintRangeComment(sourceLine, *range, hoverContents)));
    }
}

string HoverLineAssertion::toString() const {
    return fmt::format("hover-line: {} {}", lineno, message);
}

shared_ptr<CompletionAssertion> CompletionAssertion::make(string_view filename, unique_ptr<Range> &range,
                                                          int assertionLine, string_view assertionContents,
                                                          string_view assertionType) {
    return make_shared<CompletionAssertion>(filename, range, assertionLine, assertionContents);
}
CompletionAssertion::CompletionAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                         string_view message)
    : RangeAssertion(filename, range, assertionLine), message(string(message)) {}

void CompletionAssertion::checkAll(const vector<shared_ptr<RangeAssertion>> &assertions,
                                   const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                                   LSPWrapper &wrapper, int &nextId, string errorPrefix) {
    for (auto assertion : assertions) {
        if (auto assertionOfType = dynamic_pointer_cast<CompletionAssertion>(assertion)) {
            assertionOfType->check(sourceFileContents, wrapper, nextId, errorPrefix);
        }
    }
}

void CompletionAssertion::check(const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                                LSPWrapper &wrapper, int &nextId, string errorPrefix) {
    auto completionList = doTextDocumentCompletion(wrapper, *this->range, nextId, this->filename);
    {
        INFO("doTextDocumentCompletion failed; see error above.");
        REQUIRE_NE(completionList, nullptr);
    }

    string actualMessage =
        completionList->items.empty()
            ? "(nothing)"
            : fmt::format("{}", fmt::map_join(completionList->items, ", ",
                                              [](const auto &item) -> string { return item->label; }));

    auto partial = absl::EndsWith(this->message, ", ...");
    if (partial) {
        auto prefix = this->message.substr(0, this->message.size() - 5);
        if (!absl::StartsWith(actualMessage, prefix)) {
            auto sourceLine = getSourceLine(sourceFileContents, filename, range->start->line);
            ADD_FAIL_CHECK_AT(
                filename.c_str(), range->start->line + 1,
                fmt::format("{}Expected partial completion contents:\n{}\nFound incompatible completion contents:\n{}",
                            errorPrefix, prettyPrintRangeComment(sourceLine, *range, toString()),
                            prettyPrintRangeComment(sourceLine, *range, fmt::format("completion: {}", actualMessage))));
        }
    } else {
        if (this->message != actualMessage) {
            auto sourceLine = getSourceLine(sourceFileContents, filename, range->start->line);
            ADD_FAIL_CHECK_AT(
                filename.c_str(), range->start->line + 1,
                fmt::format("{}Expected completion contents:\n{}\nFound completion contents:\n{}", errorPrefix,
                            prettyPrintRangeComment(sourceLine, *range, toString()),
                            prettyPrintRangeComment(sourceLine, *range, fmt::format("completion: {}", actualMessage))));
        }
    }
}

string CompletionAssertion::toString() const {
    return fmt::format("completion: {}", message);
}

shared_ptr<ApplyCompletionAssertion> ApplyCompletionAssertion::make(string_view filename, unique_ptr<Range> &range,
                                                                    int assertionLine, string_view assertionContents,
                                                                    string_view assertionType) {
    static const regex versionIndexRegex(R"(^\[(\w+)\]\s+item:\s+(\d+)$)");

    smatch matches;
    string assertionContentsString = string(assertionContents);
    if (regex_search(assertionContentsString, matches, versionIndexRegex)) {
        auto version = matches[1].str();
        auto index = stoi(matches[2].str());
        return make_shared<ApplyCompletionAssertion>(filename, range, assertionLine, version, index);
    }

    ADD_FAIL_CHECK_AT(
        string(filename).c_str(), assertionLine + 1,
        fmt::format("Improperly formatted apply-completion assertion. Expected '[<version>] item: <index>'. Found '{}'",
                    assertionContents));

    return nullptr;
}

ApplyCompletionAssertion::ApplyCompletionAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                   string_view version, int index)
    : RangeAssertion(filename, range, assertionLine), version(string(version)), index(index) {}

void ApplyCompletionAssertion::checkAll(const vector<shared_ptr<RangeAssertion>> &assertions,
                                        const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                                        LSPWrapper &wrapper, int &nextId, string errorPrefix) {
    for (auto assertion : assertions) {
        if (auto assertionOfType = dynamic_pointer_cast<ApplyCompletionAssertion>(assertion)) {
            assertionOfType->check(sourceFileContents, wrapper, nextId, errorPrefix);
        }
    }
}

void ApplyCompletionAssertion::check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                                     LSPWrapper &wrapper, int &nextId, std::string errorPrefix) {
    auto completionList = doTextDocumentCompletion(wrapper, *this->range, nextId, this->filename);
    {
        INFO("doTextDocumentCompletion failed; see error above.");
        REQUIRE_NE(completionList, nullptr);
    }

    auto &items = completionList->items;
    REQUIRE_LE(0, this->index);
    REQUIRE_LT(this->index, items.size());

    auto &completionItem = items[this->index];

    auto it = sourceFileContents.find(this->filename);
    {
        INFO(fmt::format("Unable to find source file `{}`", this->filename));
        REQUIRE_NE(it, sourceFileContents.end());
    }
    auto &file = it->second;

    auto expectedUpdatedFilePath = updatedFilePath(this->filename, this->version);

    string expectedEditedFileContents;
    try {
        expectedEditedFileContents = FileOps::read(expectedUpdatedFilePath);
    } catch (FileNotFoundException e) {
        ADD_FAIL_CHECK_AT(filename.c_str(), this->assertionLine + 1,
                          fmt::format("Missing {} which should contain test file after applying code actions.",
                                      expectedUpdatedFilePath));
        return;
    }

    REQUIRE_NE(completionItem->textEdit, nullopt);
    auto &textEdit = completionItem->textEdit.value();
    auto reindent = true;
    auto actualEditedFileContents = applyEdit(file->source(), *file, *textEdit->range, textEdit->newText, reindent);

    {
        CHECK_EQ_DIFF(expectedEditedFileContents, actualEditedFileContents,
                      "The expected (rbedited) file contents for this completion did not match the actual file "
                      "contents post-edit");
    }
}

string ApplyCompletionAssertion::toString() const {
    return fmt::format("apply-completion: [{}] item: {}", version, index);
}

shared_ptr<ApplyRenameAssertion> ApplyRenameAssertion::make(string_view filename, unique_ptr<Range> &range,
                                                            int assertionLine, string_view assertionContents,
                                                            string_view assertionType) {
    static const regex newNameRegex(
        R"(^\[(\w+)\]\s+(?:(?:newName:\s+([a-zA-Z0-9_@]+))?\s?(?:placeholderText:\s+([^ ]+))?\s?(?:invalid:\s+(true))?\s?(?:expectedErrorMessage:\s+(.+))?)$)");

    smatch matches;
    string assertionContentsString = string(assertionContents);
    if (regex_search(assertionContentsString, matches, newNameRegex)) {
        auto version = matches[1].str();
        auto newName = matches[2].str();
        auto placeholderText = matches[3].str();
        if (!newName.empty() && placeholderText.empty()) {
            ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                              "placeholderText must be provided if newName is provided");
            return nullptr;
        }

        auto invalid = !matches[4].str().empty();
        auto expectedErrorMessage = matches[5].str();
        if ((!newName.empty() && !placeholderText.empty()) || invalid) {
            return make_shared<ApplyRenameAssertion>(filename, range, assertionLine, version, newName, placeholderText,
                                                     invalid, expectedErrorMessage);
        }
    }

    ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                      fmt::format("Improperly formatted apply-rename assertion. Expected '[<version>] newName: <name> "
                                  "placeholderText: <name> "
                                  "(invalid: true) (expectedErrorMessage: <message>)'. Found '{}' in file {}",
                                  assertionContents, filename));

    return nullptr;
}

ApplyRenameAssertion::ApplyRenameAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                           string_view version, string newName, string placeholderText, bool invalid,
                                           string expectedErrorMessage)
    : RangeAssertion(filename, range, assertionLine), version(string(version)), newName(newName),
      placeholderText(placeholderText), invalid(invalid), expectedErrorMessage(expectedErrorMessage) {}

void ApplyRenameAssertion::checkAll(const vector<shared_ptr<RangeAssertion>> &assertions,
                                    const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                                    LSPWrapper &wrapper, int &nextId, string errorPrefix) {
    for (auto assertion : assertions) {
        if (auto assertionOfType = dynamic_pointer_cast<ApplyRenameAssertion>(assertion)) {
            assertionOfType->check(sourceFileContents, wrapper, nextId, errorPrefix);
        }
    }
}

void ApplyRenameAssertion::check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                                 LSPWrapper &wrapper, int &nextId, std::string errorPrefix) {
    auto prepareRenameResponse = doTextDocumentPrepareRename(wrapper, *this->range, nextId, this->filename);

    // A rename at an invalid position
    if (newName.empty() && invalid) {
        if (prepareRenameResponse != nullptr) {
            ADD_FAIL_CHECK_AT(this->filename.c_str(), this->assertionLine + 1,
                              "Received a prepareRenameResponse for an invalid rename position");
        }
        REQUIRE_EQ(prepareRenameResponse, nullptr);
    } else {
        REQUIRE_NE(prepareRenameResponse, nullptr);
        auto &optPlaceholder = prepareRenameResponse->placeholder;
        REQUIRE(optPlaceholder.has_value());
        auto &placeholder = *optPlaceholder;
        REQUIRE_EQ(this->placeholderText, placeholder);
    }

    auto workspaceEdits =
        doTextDocumentRename(wrapper, *this->range, nextId, this->filename, newName, expectedErrorMessage);
    // A rename at a valid position but with an invalid new name
    if (invalid) {
        if (workspaceEdits != nullptr) {
            ADD_FAIL_CHECK_AT(this->filename.c_str(), this->assertionLine + 1,
                              "Received workspaceEdits for an invalid rename position");
        }
        REQUIRE_EQ(workspaceEdits, nullptr);
        return;
    }

    {
        INFO("doTextDocumentRename failed; see error above.");
        REQUIRE_NE(workspaceEdits, nullptr);
    }

    auto &renameItems = *workspaceEdits->documentChanges;
    auto it = sourceFileContents.find(this->filename);
    {
        INFO(fmt::format("Unable to find source file `{}`", this->filename));
        REQUIRE_NE(it, sourceFileContents.end());
    }

    size_t index = this->filename.rfind("/", this->filename.length());
    auto testDataPath = this->filename.substr(0, index);
    // map of all .rb and .rbi files before rename
    UnorderedMap<string, string> sourceFiles;
    // map of all .rb and .rbi files after rename
    UnorderedMap<string, string> actualEditedFiles;
    // map of all .rbedited files whose version equals `this->version`
    UnorderedMap<string, string> expectedEditedFiles;
    unique_ptr<WorkerPool> workerPool = WorkerPool::create(0, *spdlog::default_logger());
    for (auto filePath :
         FileOps::listFilesInDir(testDataPath, {".rb", ".rbi", ".rbedited"}, *workerPool, false, {}, {})) {
        auto extension = FileOps::getExtension(filePath);
        if (extension == "rbedited") {
            auto extensionIndex = filePath.rfind(".rbedited", filePath.length());
            auto fileVersion = string(FileOps::getExtension(filePath.substr(0, extensionIndex)));
            if (fileVersion == this->version) {
                expectedEditedFiles[filePath] = FileOps::read(filePath);
            }
        } else {
            sourceFiles[filePath] = FileOps::read(filePath);
        }
    }

    for (auto &renameItem : renameItems) {
        // Get the file path from the edited document's uri so we can determine the path to the .rbedited file
        index = renameItem->textDocument->uri.find(testDataPath, 0);
        string sourceFilePath = renameItem->textDocument->uri.substr(index, renameItem->textDocument->uri.length() - 1);
        string expectedEditedFilePath = updatedFilePath(sourceFilePath, this->version);

        auto &edits = renameItem->edits;
        string expectedEditedFileContents = expectedEditedFiles[expectedEditedFilePath];
        if (expectedEditedFileContents.empty()) {
            ADD_FAIL_CHECK_AT(filename.c_str(), this->assertionLine + 1,
                              fmt::format("Missing {} which should contain test file after applying code actions.",
                                          expectedEditedFilePath));
            return;
        }

        REQUIRE_FALSE(edits.empty());

        // First, sort the edits by increasing starting location
        fast_sort(edits, [](const auto &l, const auto &r) -> bool { return l->range->cmp(*r->range) < 0; });
        // Apply the edits in the reverse order so that the indices don't change.
        absl::c_reverse(edits);

        string actualEditedFileContents = string(sourceFiles[sourceFilePath]);
        for (auto &edit : edits) {
            auto file = core::File(string(sourceFilePath), string(actualEditedFileContents), core::File::Type::Normal);

            auto reindent = false;
            actualEditedFileContents = applyEdit(actualEditedFileContents, file, *edit->range, edit->newText, reindent);
        }
        actualEditedFiles[sourceFilePath] = actualEditedFileContents;
    }

    // Compare every source file to its .rbedited of the same version if one exists
    // There are 4 cases we're handling
    // 1. present in both source and edited maps and contents match => success
    // 2. present in original content, but not edited => fails
    // 3. present in both source and edited maps and contents do not match => fails
    // 4. unexpected edit => fails
    for (auto &[filePath, _] : sourceFiles) {
        string actualEditedFileContents = actualEditedFiles[filePath];
        string expectedEditedFilePath = updatedFilePath(filePath, this->version);
        string expectedEditedFileContents = expectedEditedFiles[expectedEditedFilePath];

        {
            CHECK_EQ_DIFF(
                expectedEditedFileContents, actualEditedFileContents,
                fmt::format("The expected (rbedited) file contents in {} did not match the actual post-edit contents.",
                            expectedEditedFilePath));
        }
    }
}

string ApplyRenameAssertion::toString() const {
    return fmt::format("apply-rename: [{}] newName: {}", version, newName);
}

shared_ptr<ApplyCodeActionAssertion> ApplyCodeActionAssertion::make(string_view filename, unique_ptr<Range> &range,
                                                                    int assertionLine, string_view assertionContents,
                                                                    string_view assertionType) {
    static const regex titleVersionRegex(R"(^\[(\w+)\]\s+(.*?)$)");

    smatch matches;
    string assertionContentsString = string(assertionContents);
    if (regex_search(assertionContentsString, matches, titleVersionRegex)) {
        string version = matches[1].str();
        string title = matches[2].str();
        return make_shared<ApplyCodeActionAssertion>(filename, range, assertionLine, title, version);
    }

    ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                      fmt::format("Found improperly formatted apply-code-action assertion. Expected apply-code-action "
                                  "[version] code-action-title."));
    return nullptr;
}
ApplyCodeActionAssertion::ApplyCodeActionAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                   string_view title, string_view version)
    : RangeAssertion(filename, range, assertionLine), title(string(title)), version(string(version)), kind(nullopt) {}

string ApplyCodeActionAssertion::toString() const {
    return fmt::format("apply-code-action: [{}] {}", version, title);
}
optional<pair<string, string>> ApplyCodeActionAssertion::expectedFile(string filename, string version) {
    auto expectedUpdatedFilePath = updatedFilePath(filename, version);
    string expectedEditedFileContents;
    try {
        expectedEditedFileContents = FileOps::read(expectedUpdatedFilePath);
    } catch (FileNotFoundException e) {
        ADD_FAIL_CHECK_AT(filename.c_str(), assertionLine + 1,
                          fmt::format("Missing {} which should contain test file after applying code actions.",
                                      expectedUpdatedFilePath));
        return nullopt;
    }
    return make_pair(expectedUpdatedFilePath, expectedEditedFileContents);
}

void ApplyCodeActionAssertion::assertResults(std::string expectedPath, std::string expectedContents,
                                             std::string actualContents) {
    CHECK_EQ_DIFF(
        expectedContents, actualContents,
        fmt::format(
            "Invalid quick fix result. Expected edited result ({}) to be:\n{}\n...but actually resulted in:\n{}",
            expectedPath, expectedContents, actualContents));
}

std::unique_ptr<TextDocumentEdit> ApplyCodeActionAssertion::sortEdits(std::unique_ptr<TextDocumentEdit> changes) {
    // First, sort the edits by increasing starting location and verify that none overlap.
    fast_sort(changes->edits, [](const auto &l, const auto &r) -> bool { return l->range->cmp(*r->range) < 0; });
    for (uint32_t i = 1; i < changes->edits.size(); i++) {
        INFO(fmt::format("Received quick fix edit\n{}\nthat overlaps edit\n{}\nThe test runner does not support "
                         "overlapping autocomplete edits, and it's likely that this is a bug.",
                         changes->edits[i - 1]->toJSON(), changes->edits[i]->toJSON()));
        REQUIRE_LT(changes->edits[i - 1]->range->end->cmp(*changes->edits[i]->range->start), 0);
    }

    // Now, apply the edits in the reverse order so that the indices don't change.
    absl::c_reverse(changes->edits);
    return changes;
}

namespace {
shared_ptr<sorbet::core::File>
getFileByUri(const LSPConfiguration &config,
             const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, string uri) {
    auto filename = uriToFilePath(config, uri);
    auto it = sourceFileContents.find(filename);
    {
        INFO(fmt::format("Unable to find referenced source file `{}`", filename));
        REQUIRE_NE(it, sourceFileContents.end());
    }
    return it->second;
}
} // namespace

void ApplyCodeActionAssertion::check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                                     LSPWrapper &wrapper, const CodeAction &codeAction) {
    const auto &config = wrapper.config();
    for (auto &c : *codeAction.edit.value()->documentChanges) {
        auto file = getFileByUri(config, sourceFileContents, c->textDocument->uri);

        string actualEditedFileContents = string(file->source());
        c = sortEdits(move(c));

        auto maybeFile = expectedFile(uriToFilePath(config, c->textDocument->uri), this->version);
        if (!maybeFile.has_value()) {
            return;
        }

        auto [expectedUpdatedFilePath, expectedEditedFileContents] = maybeFile.value();
        for (auto &e : c->edits) {
            auto reindent = false;
            actualEditedFileContents = applyEdit(actualEditedFileContents, *file, *e->range, e->newText, reindent);
        }
        assertResults(expectedUpdatedFilePath, expectedEditedFileContents, actualEditedFileContents);
    }
};

void ApplyCodeActionAssertion::checkAll(
    const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
    const CodeAction &codeAction) {
    const auto &config = wrapper.config();
    UnorderedMap<string, string> accumulatedOriginalEditedContents{};

    // actualEditedFileContents -> (expectedUpdatedFilePath, expectedEditedFileContents)
    // Maps original file contents to a edited filename and contents
    UnorderedMap<string, std::pair<std::string, std::string>> fileToUpdatedFile;
    string actualEditedFileContents;
    for (auto &c : *codeAction.edit.value()->documentChanges) {
        auto file = getFileByUri(config, sourceFileContents, c->textDocument->uri);

        c = sortEdits(move(c));
        actualEditedFileContents = string(file->source());

        for (auto &e : c->edits) {
            auto maybeFile = expectedFile(uriToFilePath(config, c->textDocument->uri), this->version);
            if (!maybeFile.has_value()) {
                continue;
            }
            fileToUpdatedFile.insert_or_assign(actualEditedFileContents, maybeFile.value());

            string oldSource = accumulatedOriginalEditedContents.contains(actualEditedFileContents)
                                   ? accumulatedOriginalEditedContents[actualEditedFileContents]
                                   : actualEditedFileContents;

            auto reindent = false;
            auto newSource = applyEdit(oldSource, *file, *e->range, e->newText, reindent);
            accumulatedOriginalEditedContents.insert_or_assign(actualEditedFileContents, newSource);
        }
    }

    for (auto pair : accumulatedOriginalEditedContents) {
        auto [expectedUpdatedFilePath, expectedEditedFileContents] = fileToUpdatedFile[pair.first];
        assertResults(expectedUpdatedFilePath, expectedEditedFileContents, pair.second);
    }
}
SymbolSearchAssertion::SymbolSearchAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                             string_view query, optional<std::string> name, optional<string> container,
                                             std::optional<int> rank, optional<string> uri)
    : RangeAssertion(filename, range, assertionLine), query(query), name(move(name)), container(move(container)),
      rank(rank), uri(move(uri)) {}

string SymbolSearchAssertion::toString() const {
    auto namePart = name.has_value() ? fmt::format(", name=\"{}\"", name.value()) : "";
    auto containerPart = container.has_value() ? fmt::format(", container=\"{}\"", container.value()) : "";
    auto rankPart = rank.has_value() ? fmt::format(", rank={}", rank.value()) : "";
    auto uriPart = uri.has_value() ? fmt::format(", uri=\"{}\"", uri.value()) : "";
    return fmt::format("symbol-search: \"{}\"{}{}{}{}", query, namePart, containerPart, rankPart, uriPart);
}

shared_ptr<SymbolSearchAssertion> SymbolSearchAssertion::make(string_view filename, unique_ptr<Range> &range,
                                                              int assertionLine, string_view assertionContents,
                                                              string_view assertionType) {
    static const regex contentsRegex(R"(^\s*\"([^\"]+)\"\s*(,.*\S)?\s*$)");
    smatch topMatches;
    string assertionContentsString = string(assertionContents);
    if (!regex_match(assertionContentsString, topMatches, contentsRegex)) {
        ADD_FAIL_CHECK_AT(
            string(filename).c_str(), assertionLine + 1,
            fmt::format(
                "Improperly formatted assertion. Expected 'symbol-search: \"<query>\"[, ...options]*'. Found '{}'",
                assertionContents));
        return nullptr;
    }
    auto query = topMatches[1].str();
    optional<string> remainingOptions = nullopt;
    if (topMatches[2].matched) {
        remainingOptions = topMatches[2].str();
    }

    // Parse options
    optional<string> name = nullopt;
    optional<string> container = nullopt;
    optional<int> rank = nullopt;
    optional<string> uri = nullopt;
    // regex groups:
    //   1 => name
    //   2 => value
    //   3 => string contents of value
    //   4 => digits value
    //   5 =>  remaining options
    static const regex optionsRegex(R"(^,\s*([a-z]+)\s*=\s*(\"([^\"]*)\"|(\d+))\s*(,.*)?\s*$)");
    while (remainingOptions.has_value()) {
        smatch matches;
        if (!regex_match(*remainingOptions, matches, optionsRegex)) {
            ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                              fmt::format("Improperly formatted assertion. Could not parse '{}' for symbol-search.",
                                          *remainingOptions));
            return nullptr;
        }
        auto optionName = matches[1].str();
        auto optionValue = matches[2].str();
        optional<string> valueStringContents = nullopt;
        optional<int> valueAsInt = nullopt;
        if (matches[3].matched) {
            valueStringContents = matches[3].str();
        } else if (matches[4].matched) {
            valueAsInt = atoi(matches[4].str().c_str());
        }
        if (matches[5].matched) {
            remainingOptions = matches[5].str();
        } else {
            remainingOptions = nullopt;
        }

        if (optionName == "name") {
            if (!valueStringContents.has_value()) {
                ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                                  fmt::format("Improperly formatted `symbol-search` assertion.\n"
                                              "Expected `name = \"str\"`, got `{} = {}` in:\n{}\n",
                                              optionName, optionValue, assertionContents));
            }
            name = valueStringContents;
        } else if (optionName == "container") {
            if (!valueStringContents.has_value()) {
                ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                                  fmt::format("Improperly formatted `symbol-search` assertion.\n"
                                              "Expected `container = \"str\"`, got `{} = {}` in:\n{}\n",
                                              optionName, optionValue, assertionContents));
            }
            container = valueStringContents;
        } else if (optionName == "rank") {
            if (!valueAsInt.has_value()) {
                ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                                  fmt::format("Improperly formatted `symbol-search` assertion.\n"
                                              "Expected `rank = integer`, got `{} = {}` in:\n{}\n",
                                              optionName, optionValue, assertionContents));
            }
            rank = valueAsInt;
        } else if (optionName == "uri") {
            if (!valueStringContents.has_value()) {
                ADD_FAIL_CHECK_AT(string(filename).c_str(), assertionLine + 1,
                                  fmt::format("Improperly formatted `symbol-search` assertion.\n"
                                              "Expected `uri = \"substr\"`, got `{} = {}` in:\n{}\n",
                                              optionName, optionValue, assertionContents));
            }
            uri = valueStringContents;
        } else {
            ADD_FAIL_CHECK_AT(
                string(filename).c_str(), assertionLine + 1,
                fmt::format("Improperly formatted `symbol-search` assertion for query {}.\n"
                            "Valid args are name, container, rank, and uri, could not parse `{} = {}` in:\n{}\n",
                            query, optionName, optionValue, assertionContents));
        }
    }
    return make_shared<SymbolSearchAssertion>(filename, range, assertionLine, query, name, container, rank, uri);
}

bool SymbolSearchAssertion::matches(const LSPConfiguration &config, const SymbolInformation &symbol) const {
    auto assertionLocation = getLocation(config);
    auto &symbolLocation = symbol.location;
    if (uri.has_value()) {
        if (symbolLocation->uri.find(*uri) == string::npos) {
            return false;
        }
    } else {
        if (assertionLocation->uri != symbolLocation->uri) {
            return false;
        }
        if (assertionLocation->range->start->line != symbolLocation->range->start->line) {
            return false;
        }
    }
    if (name.has_value() && *name != symbol.name) {
        return false;
    }
    if (container.has_value()) {
        if (*container == NOTHING_LABEL) {
            if (symbol.containerName.has_value()) {
                return false;
            }
        } else if (*container != symbol.containerName) {
            return false;
        }
    }
    return true;
}
namespace {
string pluralized_count(string_view word, int count) {
    return fmt::format("{} {}{}", count, word, (count == 1) ? "" : "s");
}

void addFailureAtLocationWithSource(const Location &location,
                                    const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                                    const LSPConfiguration &config, std::string_view message) {
    if (!config.isUriInWorkspace(location.uri)) {
        ADD_FAIL_CHECK_AT(location.uri.c_str(), location.range->start->line + 1,
                          fmt::format("{}\n(source unavailable for {})\n", message, location.uri));
        return;
    }
    auto sourceLine = getLine(config, sourceFileContents, location);
    ADD_FAIL_CHECK_AT(location.uri.c_str(), location.range->start->line + 1,
                      fmt::format("{}\n{}\n", message, prettyPrintRangeComment(sourceLine, *location.range, "")));
}

void matchSymbolsToAssertions(string_view query, const vector<shared_ptr<SymbolSearchAssertion>> &assertions,
                              vector<pair<SymbolInformation &, SymbolSearchAssertion *>> &symbolMatchStates,
                              const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                              const LSPConfiguration &config, string_view errorPrefix) {
    vector<shared_ptr<SymbolSearchAssertion>> unmatchedAssertions;
    vector<reference_wrapper<SymbolInformation>> unmatchedSymbols;
    // Find assertions with no matching symbol
    for (auto &assertion : assertions) {
        SymbolInformation *matchingSymbol = nullptr;
        for (auto &symbolMatchState : symbolMatchStates) {
            auto &symbol = symbolMatchState.first;
            if (!assertion->matches(config, symbolMatchState.first)) {
                continue;
            }
            auto previousMatchingAssertion = symbolMatchState.second;
            if (previousMatchingAssertion != nullptr) {
                addFailureAtLocationWithSource(*assertion->getLocation(config), sourceFileContents, config,
                                               fmt::format("More than one assertion matches symbol:\n {}\n"
                                                           "At least these two assertions match this symbol:\n"
                                                           "  {}:{}: {}\n"
                                                           "  {}:{}: {}\n"
                                                           "Please refine your test to avoid ambiguity.",
                                                           symbol.toJSON(true), previousMatchingAssertion->filename,
                                                           previousMatchingAssertion->assertionLine + 1,
                                                           previousMatchingAssertion->toString(), assertion->filename,
                                                           assertion->assertionLine + 1, assertion->toString()));
            } else if (matchingSymbol == nullptr) {
                symbolMatchState.second = &*assertion; // record the match
                matchingSymbol = &symbolMatchState.first;
            } else {
                addFailureAtLocationWithSource(*assertion->getLocation(config), sourceFileContents, config,
                                               fmt::format("More than one symbol matches assertion:\n {}\n"
                                                           "Found matching symbols:\n  {}\n  {}\n"
                                                           "Please refine your test to avoid ambiguity.",
                                                           assertion->toString(), matchingSymbol->toJSON(true),
                                                           symbol.toJSON(true)));
            }
        }
        if (matchingSymbol == nullptr) {
            unmatchedAssertions.push_back(assertion);
        }
    }
    // Collect symbols without a matching assertion
    for (auto [symbol, assertion] : symbolMatchStates) {
        if (assertion != nullptr) {
            continue;
        }
        Location &location = *symbol.location.get();
        if (!config.isUriInWorkspace(location.uri)) {
            continue; // ignore extra symbols returned from standard library (not test files)
        }
        unmatchedSymbols.emplace_back(symbol);
    }

    for (auto &assertion : unmatchedAssertions) {
        addFailureAtLocationWithSource(
            *assertion->getLocation(config), sourceFileContents, config,
            fmt::format("{}No results matched `workspace/symbol` expectation: {}", errorPrefix, assertion->toString()));
    }
    for (auto &symbol : unmatchedSymbols) {
        addFailureAtLocationWithSource(*symbol.get().location, sourceFileContents, config,
                                       fmt::format("{}Unexpected result from `workspace/symbol` query \"{}\": {}\n",
                                                   errorPrefix, query, symbol.get().toJSON(true)));
    }

    if (!unmatchedAssertions.empty() || !unmatchedSymbols.empty()) {
        auto numFailures = unmatchedAssertions.size() + unmatchedSymbols.size();
        FAIL_CHECK(fmt::format("{}`workspace/symbol` query \"{}\" {}: {}, {}\n", errorPrefix, query,
                               pluralized_count("failure", numFailures),
                               pluralized_count("unsatisfied expectation", unmatchedAssertions.size()),
                               pluralized_count("unexpected result", unmatchedSymbols.size())));
    }
}

void checkSymbolsReturnedInRankOrder(string_view query,
                                     vector<pair<SymbolInformation &, SymbolSearchAssertion *>> &symbolMatchStates) {
    pair<SymbolInformation &, SymbolSearchAssertion *> *left = nullptr;
    for (auto &right : symbolMatchStates) {
        if (right.second == nullptr || !right.second->rank.has_value()) {
            continue;
        }
        if (left != nullptr) {
            auto leftAssertion = left->second;
            if (leftAssertion != nullptr || leftAssertion->rank.has_value()) {
                auto rightAssertion = right.second;
                auto &leftSymbol = left->first;
                auto &rightSymbol = right.first;
                if (*leftAssertion->rank > *rightAssertion->rank) {
                    FAIL_CHECK(fmt::format(
                        "Symbols not returned in ascending `rank` order for query `{}`:\n"
                        "Symbol for assertion #1 ({} {}) should appear *after* symbol for assertion #2 ({} {}).\n"
                        "Assertion #1: {}\nAssertion #2: {}\nSymbol #1:\n{}\nSymbol #2:\n{}\n",
                        query, leftSymbol.containerName.value_or(""), leftSymbol.name,
                        rightSymbol.containerName.value_or(""), rightSymbol.name, leftAssertion->toString(),
                        rightAssertion->toString(), leftSymbol.toJSON(true), rightSymbol.toJSON(true)));
                }
            }
        }
        left = &right;
    }
}

void checkAllForQuery(std::string query, const vector<shared_ptr<SymbolSearchAssertion>> &assertions,
                      const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents, LSPWrapper &lspWrapper,
                      int &nextId, string_view errorPrefix) {
    const int id = nextId++;

    // Request results from LSP
    auto responses = getLSPResponsesFor(lspWrapper, makeWorkspaceSymbolRequest(id, query));
    {
        INFO("Unexpected number of responses to a `workspace/symbol` request.");
        REQUIRE_EQ(1, responses.size());
    }
    assertResponseMessage(id, *responses.at(0));
    auto &respMsg = responses.at(0)->asResponse();
    REQUIRE(respMsg.result.has_value());
    REQUIRE_FALSE(respMsg.error.has_value());
    auto &result = *(respMsg.result);
    // symbolAssertionMatches maintains the ordering from the LSP results and
    // stores the pairing of symbols <=> assertions.
    vector<pair<SymbolInformation &, SymbolSearchAssertion *>> symbolAssertionMatches;
    if (auto symbolInfos = get_if<vector<unique_ptr<SymbolInformation>>>(
            &get<variant<JSONNullObject, vector<unique_ptr<SymbolInformation>>>>(result))) {
        for (auto &symbolInfo : *symbolInfos) {
            symbolAssertionMatches.emplace_back(*symbolInfo, nullptr);
        }
        constexpr int MAX_RESULTS = 50; // see MAX_RESULTS in workspace_symbols.cc
        if (symbolInfos->size() > MAX_RESULTS) {
            FAIL_CHECK(fmt::format(
                "Too many results for `workspace/symbol` request for `{}`. [Expected no more than {}, got {}].", query,
                MAX_RESULTS, symbolInfos->size()));
        }
    }
    matchSymbolsToAssertions(query, assertions, symbolAssertionMatches, sourceFileContents, lspWrapper.config(),
                             errorPrefix);
    checkSymbolsReturnedInRankOrder(query, symbolAssertionMatches);
}
} // namespace

void SymbolSearchAssertion::checkAll(const vector<shared_ptr<RangeAssertion>> &assertions,
                                     const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                                     LSPWrapper &lspWrapper, int &nextId, string errorPrefix) {
    UnorderedMap<std::string, vector<shared_ptr<SymbolSearchAssertion>>> queryToAssertionsMap;
    for (auto &assertion : assertions) {
        if (auto searchAssertion = dynamic_pointer_cast<SymbolSearchAssertion>(assertion)) {
            queryToAssertionsMap[searchAssertion->query].push_back(searchAssertion);
        }
    }
    for (auto entry : queryToAssertionsMap) {
        checkAllForQuery(entry.first, entry.second, sourceFileContents, lspWrapper, nextId, errorPrefix);
    }
}

ImplementationAssertion::ImplementationAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                                 string_view symbol)
    : RangeAssertion(filename, range, assertionLine), symbol(symbol) {}

shared_ptr<ImplementationAssertion> ImplementationAssertion::make(string_view filename, unique_ptr<Range> &range,
                                                                  int assertionLine, string_view assertionContents,
                                                                  string_view assertionType) {
    auto [symbol, _, __] = getSymbolVersionAndOption(assertionContents);
    return make_shared<ImplementationAssertion>(filename, range, assertionLine, symbol);
}

string ImplementationAssertion::toString() const {
    return fmt::format("implementation: {}", symbol);
}

FindImplementationAssertion::FindImplementationAssertion(string_view filename, unique_ptr<Range> &range,
                                                         int assertionLine, string_view symbol)
    : RangeAssertion(filename, range, assertionLine), symbol(symbol) {}

shared_ptr<FindImplementationAssertion> FindImplementationAssertion::make(string_view filename,
                                                                          unique_ptr<Range> &range, int assertionLine,
                                                                          string_view assertionContents,
                                                                          string_view assertionType) {
    auto [symbol, _, __] = getSymbolVersionAndOption(assertionContents);
    return make_shared<FindImplementationAssertion>(filename, range, assertionLine, symbol);
}

string FindImplementationAssertion::toString() const {
    return fmt::format("find-implementation: {}", symbol);
}

void FindImplementationAssertion::check(
    const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper, int &nextId,
    std::string_view symbol, const Location &queryLoc,
    const std::vector<std::shared_ptr<ImplementationAssertion>> &allImpls) {
    const int line = queryLoc.range->start->line;
    // Can only query with one character, so just use the first one.
    const int character = queryLoc.range->start->character;
    const auto &config = wrapper.config();
    auto locSourceLine = getLine(config, sourceFileContents, queryLoc);
    string locFilename = uriToFilePath(config, queryLoc.uri);

    const int id = nextId++;
    auto request = make_unique<LSPMessage>(make_unique<RequestMessage>(
        "2.0", id, LSPMethod::TextDocumentImplementation,
        make_unique<ImplementationParams>(make_unique<TextDocumentIdentifier>(string(queryLoc.uri)),
                                          make_unique<Position>(line, character))));
    auto responses = getLSPResponsesFor(wrapper, move(request));
    REQUIRE_EQ(1, responses.size());
    assertResponseMessage(id, *responses.at(0));

    auto &respMsg = responses.at(0)->asResponse();
    REQUIRE(respMsg.result.has_value());
    REQUIRE_FALSE(respMsg.error.has_value());

    auto &locations = extractLocations(respMsg);

    // casting from ImplementationAssertion to RangeAssertion
    std::vector<std::shared_ptr<RangeAssertion>> allLocs(allImpls.begin(), allImpls.end());
    assertLocationsMatch(config, sourceFileContents, symbol, allLocs, line, character, locSourceLine, locFilename,
                         locations, "find implementation");
};

shared_ptr<ShowSymbolAssertion> ShowSymbolAssertion::make(string_view filename, unique_ptr<Range> &range,
                                                          int assertionLine, string_view assertionContents,
                                                          string_view assertionType) {
    return make_shared<ShowSymbolAssertion>(filename, range, assertionLine, assertionContents);
}
ShowSymbolAssertion::ShowSymbolAssertion(string_view filename, unique_ptr<Range> &range, int assertionLine,
                                         string_view message)
    : RangeAssertion(filename, range, assertionLine), message(string(message)) {}

void ShowSymbolAssertion::checkAll(const vector<shared_ptr<RangeAssertion>> &assertions,
                                   const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                                   LSPWrapper &wrapper, int &nextId, string errorPrefix) {
    for (auto assertion : assertions) {
        if (auto assertionOfType = dynamic_pointer_cast<ShowSymbolAssertion>(assertion)) {
            assertionOfType->check(sourceFileContents, wrapper, nextId, errorPrefix);
        }
    }
}

string_view symbolInformationToString(variant<JSONNullObject, unique_ptr<SymbolInformation>> &showSymbolResult) {
    if (auto nullResp = get_if<JSONNullObject>(&showSymbolResult)) {
        return NULL_LABEL;
    } else {
        auto &symbolInformation = get<unique_ptr<SymbolInformation>>(showSymbolResult);
        string_view value = symbolInformation->name;
        if (value.empty()) {
            return NOTHING_LABEL;
        }
        return value;
    }
}

void ShowSymbolAssertion::check(const UnorderedMap<string, shared_ptr<core::File>> &sourceFileContents,
                                LSPWrapper &wrapper, int &nextId, string errorPrefix) {
    const auto &config = wrapper.config();
    auto uri = filePathToUri(config, filename);
    auto pos = make_unique<TextDocumentPositionParams>(make_unique<TextDocumentIdentifier>(uri), range->start->copy());
    auto id = nextId++;
    auto msg = make_unique<LSPMessage>(make_unique<RequestMessage>("2.0", id, LSPMethod::SorbetShowSymbol, move(pos)));
    auto responses = getLSPResponsesFor(wrapper, move(msg));
    REQUIRE_EQ(responses.size(), 1);
    auto &responseMsg = responses.at(0);
    REQUIRE(responseMsg->isResponse());
    auto &response = responseMsg->asResponse();
    REQUIRE(response.result.has_value());
    auto &showSymbolResponse = get<variant<JSONNullObject, unique_ptr<SymbolInformation>>>(*response.result);
    auto showSymbolContents = symbolInformationToString(showSymbolResponse);

    if (!containsLine(showSymbolContents, this->message)) {
        auto sourceLine = getSourceLine(sourceFileContents, filename, range->start->line);
        ADD_FAIL_CHECK_AT(filename.c_str(), range->start->line + 1,
                          fmt::format("{}Expected show-symbol contents:\n{}\nFound show-symbol contents:\n{}",
                                      errorPrefix, prettyPrintRangeComment(sourceLine, *range, toString()),
                                      prettyPrintRangeComment(sourceLine, *range,
                                                              fmt::format("show-symbol: {}", showSymbolContents))));
    }
}

string ShowSymbolAssertion::toString() const {
    return fmt::format("show-symbol: {}", message);
}

namespace {
string_view trimString(std::string_view s) {
    const char *whitespace = " \t";
    size_t begin = s.find_first_not_of(whitespace);
    if (begin == std::string::npos) {
        return "";
    }
    size_t end = s.find_last_not_of(whitespace);
    return s.substr(begin, end - begin + 1);
}
} // namespace

std::shared_ptr<StringPropertyAssertions>
StringPropertyAssertions::make(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                               std::string_view assertionContents, std::string_view assertionType) {
    std::vector<std::string> values = absl::StrSplit(assertionContents, ',');
    transform(values.begin(), values.end(), values.begin(), [](auto val) { return trimString(val); });

    return make_shared<StringPropertyAssertions>(filename, range, assertionLine, values, assertionType);
}

StringPropertyAssertions::StringPropertyAssertions(std::string_view filename, std::unique_ptr<Range> &range,
                                                   int assertionLine, std::vector<std::string> values,
                                                   std::string_view assertionType)
    : RangeAssertion(filename, range, assertionLine), assertionType(string(assertionType)), values(values){};

std::optional<std::vector<std::string>>
StringPropertyAssertions::getValues(std::string_view type,
                                    const std::vector<std::shared_ptr<RangeAssertion>> &assertions) {
    for (auto &assertion : assertions) {
        if (auto codeActionAssertion = dynamic_pointer_cast<StringPropertyAssertions>(assertion)) {
            if (codeActionAssertion->assertionType == type) {
                return codeActionAssertion->values;
            }
        }
    }
    return nullopt;
}

string StringPropertyAssertions::toString() const {
    return fmt::format("selective-apply-code-action: {}", fmt::join(values, ", "));
}
} // namespace sorbet::test
