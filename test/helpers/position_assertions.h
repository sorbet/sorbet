#ifndef TEST_HELPERS_POSITION_ASSERTIONS_H
#define TEST_HELPERS_POSITION_ASSERTIONS_H

#include "main/lsp/json_types.h"
#include "main/lsp/wrapper.h"
#include "test/helpers/expectations.h"
#include <regex>

namespace sorbet::test {
using namespace sorbet::realmain::lsp;

class ErrorAssertion;
class UntypedAssertion;

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

    static bool compareByRange(const std::shared_ptr<RangeAssertion> &a, const std::shared_ptr<RangeAssertion> &b) {
        return a->cmp(*b) < 0;
    }

    /**
     * Filters a vector of assertions and returns only ErrorAssertions.
     */
    static std::vector<std::shared_ptr<ErrorAssertion>>
    getErrorAssertions(const std::vector<std::shared_ptr<RangeAssertion>> &assertions);

    static std::vector<std::shared_ptr<UntypedAssertion>>
    getUntypedAssertions(const std::vector<std::shared_ptr<RangeAssertion>> &assertions);

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
    std::unique_ptr<Location> getLocation(const LSPConfiguration &config) const;

    // Returns a DocumentHighlight object for this assertion's range.
    std::unique_ptr<DocumentHighlight> getDocumentHighlight();

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
    static constexpr DiagnosticSeverity severity = DiagnosticSeverity::Error;

    ErrorAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                   std::string_view message, bool matchesDuplicateErrors);

    std::string toString() const override;

    bool check(const Diagnostic &diagnostic, std::string_view sourceLine, std::string_view errorPrefix);
};

class UntypedAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<UntypedAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                  int assertionLine, std::string_view assertionContents,
                                                  std::string_view assertionType);

    UntypedAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                     std::string_view message);

    std::string toString() const override;
    const std::string message;

    // this exists solely to allow us to reuse ErrorAssertion's checkAll
    // for UntypedAssertion. It should *always* be false.
    static constexpr bool matchesDuplicateErrors = false;
    static constexpr DiagnosticSeverity severity = DiagnosticSeverity::Information;

    static bool checkAll(const UnorderedMap<std::string, std::shared_ptr<core::File>> &files,
                         std::vector<std::shared_ptr<UntypedAssertion>> errorAssertions,
                         std::map<std::string, std::vector<std::unique_ptr<Diagnostic>>> &filenamesAndDiagnostics,
                         std::string errorPrefix = "");

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
    const bool isDefOfSelf;
    const bool isDefaultArgValue;

    DefAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine, std::string_view symbol,
                 int version, bool isDefOfSelf, bool isDefaultArgValue);

    static void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                      LSPWrapper &wrapper, int &nextId, const Location &queryLoc,
                      const std::vector<std::shared_ptr<DefAssertion>> &definitions);

    std::string toString() const override;
};

// # ^^^ usage: symbol
class UsageAssertion : public RangeAssertion {
public:
    static std::shared_ptr<UsageAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                int assertionLine, std::string_view assertionContents,
                                                std::string_view assertionType);

    static void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                      LSPWrapper &wrapper, int &nextId, std::string_view symbol, const Location &queryLoc,
                      const std::vector<std::shared_ptr<RangeAssertion>> &allLocs);

    // Runs assertions for documentHighlight LSP results.
    static void checkHighlights(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                                LSPWrapper &wrapper, int &nextId, std::string_view symbol, const Location &queryLoc,
                                const std::vector<std::shared_ptr<RangeAssertion>> &allLocs);

    const std::string symbol;
    const std::vector<int> versions;
    std::shared_ptr<DefAssertion> def;

    UsageAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine, std::string_view symbol,
                   std::vector<int> versions);

    std::string toString() const override;
};

// # ^^^ import: symbol
class ImportAssertion final : public UsageAssertion {
public:
    static std::shared_ptr<ImportAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                 int assertionLine, std::string_view assertionContents,
                                                 std::string_view assertionType);

    ImportAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                    std::string_view symbol, std::vector<int> versions);
};

// # ^^^ importusage: symbol
class ImportUsageAssertion final : public UsageAssertion {
public:
    static std::shared_ptr<ImportUsageAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                      int assertionLine, std::string_view assertionContents,
                                                      std::string_view assertionType);

    ImportUsageAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                         std::string_view symbol, std::vector<int> versions);
};

// # some-property: foo
class StringPropertyAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<StringPropertyAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                         int assertionLine, std::string_view assertionContents,
                                                         std::string_view assertionType);

    static std::optional<std::string> getValue(std::string_view type,
                                               const std::vector<std::shared_ptr<RangeAssertion>> &assertions);

    const std::string assertionType;
    const std::string value;

    StringPropertyAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                            std::string value, std::string_view assertionType);

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
                      LSPWrapper &wrapper, int &nextId, std::string_view symbol, const Location &queryLoc,
                      const std::vector<std::shared_ptr<RangeAssertion>> &allLocs);

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
                         LSPWrapper &wrapper, int &nextId, std::string errorPrefix = "");

    HoverAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                   std::string_view message);

    const std::string message;

    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
               int &nextId, std::string errorPrefix = "");

    std::string toString() const override;
};

// # ^ hover-line: 1 foo
class HoverLineAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<HoverLineAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                    int assertionLine, std::string_view assertionContents,
                                                    std::string_view assertionType);
    /** Checks all HoverLineAssertions within the assertion vector. Skips over non-hover assertions.*/
    static void checkAll(const std::vector<std::shared_ptr<RangeAssertion>> &assertions,
                         const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                         LSPWrapper &wrapper, int &nextId, std::string errorPrefix = "");

    HoverLineAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine, int lineno,
                       std::string_view message);

    const int lineno;
    const std::string message;

    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
               int &nextId, std::string errorPrefix = "");

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
                         LSPWrapper &wrapper, int &nextId, std::string errorPrefix = "");

    CompletionAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                        std::string_view message);

    const std::string message;

    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
               int &nextId, std::string errorPrefix = "");

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
                         LSPWrapper &wrapper, int &nextId, std::string errorPrefix = "");

    ApplyCompletionAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                             std::string_view version, int index);

    // The part between [..] in the assertion which specifies which `.[..].rbedited` file to compare against
    const std::string version;
    // Index into CompletionItem list of which item to apply (zero-indexed)
    const int index;

    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
               int &nextId, std::string errorPrefix = "");

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

    // method applies an edit and asserts results after each edit
    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
               const CodeAction &codeAction);

    // method applies all edits and asserts results after every edit is applied
    void checkAll(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
                  const CodeAction &codeAction);

    const std::string title;
    const std::string version;
    std::optional<CodeActionKind> kind;

    std::string toString() const override;

private:
    std::optional<std::pair<std::string, std::string>> expectedFile();
    void assertResults(std::string expectedPath, std::string expectedContents, std::string actualContents);
    std::unique_ptr<TextDocumentEdit> sortEdits(std::unique_ptr<TextDocumentEdit> changes);
};

// ^ apply-rename: [version] newName
class ApplyRenameAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<ApplyRenameAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                      int assertionLine, std::string_view assertionContents,
                                                      std::string_view assertionType);

    /** Checks all ApplyRenameAssertions within the assertion vector. Skips over non-ApplyRenameAssertions. */
    static void checkAll(const std::vector<std::shared_ptr<RangeAssertion>> &assertions,
                         const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                         LSPWrapper &wrapper, int &nextId, std::string errorPrefix = "");

    ApplyRenameAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                         std::string_view version, std::string newName, std::string placeholderText, bool invalid,
                         std::string expectedErrorMessage);

    // The part between [..] in the assertion which specifies which `.[..].rbedited` file to compare against
    const std::string version;
    // New name for constant
    const std::string newName;
    // The name of the thing being renamed
    const std::string placeholderText;
    const bool invalid;
    const std::string expectedErrorMessage;

    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
               int &nextId, std::string errorPrefix = "");

    std::string toString() const override;
};

// # ^^^ symbol-search: "query" [, optional_key = value ]*
// Checks that a `workspace/symbol` result for the given "query" returns a result
// that matches the indicated range in the given file.  Options:
// * `name = "str"` => the result's `name` must *exactly* match the given string
//   (useful for synthetic results, like the `foo=` of an `attr_writer`)
// * `container = "str"` => the `containerName` must *exactly* match the given string (also supports "(nothing)" to
//   dictate no entry)
// * `uri = "substr"` => the `location->uri` must *contain* the given string,
//   rather than matching the containing file
//   (container + uri can be useful for matching entries in `rbi` files)
// * `rank = int` => for each query, verifies that any ranked assertions
//   appear in order of *ascending* rank
class SymbolSearchAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<SymbolSearchAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                       int assertionLine, std::string_view assertionContents,
                                                       std::string_view assertionType);

    const std::string query;
    const std::optional<std::string> name;
    const std::optional<std::string> container;
    const std::optional<int> rank;
    const std::optional<std::string> uri; // uses substring match

    /** Checks all SymbolSearchAssertions within the assertion vector. Skips over non-CompletionAssertions. */
    static void checkAll(const std::vector<std::shared_ptr<RangeAssertion>> &assertions,
                         const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                         LSPWrapper &wrapper, int &nextId, std::string errorPrefix = "");

    SymbolSearchAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                          std::string_view query, std::optional<std::string> name, std::optional<std::string> container,
                          std::optional<int> rank, std::optional<std::string> uri);

    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
               int &nextId, const Location &queryLoc);

    /** Returns true if the given symbol matches this assertion. */
    bool matches(const LSPConfiguration &config, const SymbolInformation &symbol) const;

    std::string toString() const override;
};

// # ^^^ implementation: symbol
class ImplementationAssertion final : public RangeAssertion {
public:
    ImplementationAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                            std::string_view symbol);
    static std::shared_ptr<ImplementationAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                         int assertionLine, std::string_view assertionContents,
                                                         std::string_view assertionType);
    const std::string symbol;
    std::string toString() const override;
};

// # ^^^ find-implementation: symbol
class FindImplementationAssertion final : public RangeAssertion {
public:
    FindImplementationAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                                std::string_view symbol);
    static std::shared_ptr<FindImplementationAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                             int assertionLine, std::string_view assertionContents,
                                                             std::string_view assertionType);

    static void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                      LSPWrapper &wrapper, int &nextId, std::string_view symbol, const Location &queryLoc,
                      const std::vector<std::shared_ptr<ImplementationAssertion>> &allLocs);

    const std::string symbol;
    std::string toString() const override;
};

// # ^ show-symbol: Foo
class ShowSymbolAssertion final : public RangeAssertion {
public:
    static std::shared_ptr<ShowSymbolAssertion> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                     int assertionLine, std::string_view assertionContents,
                                                     std::string_view assertionType);
    /** Checks all ShowSymbolAssertions within the assertion vector. Skips over non-hover assertions.*/
    static void checkAll(const std::vector<std::shared_ptr<RangeAssertion>> &assertions,
                         const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents,
                         LSPWrapper &wrapper, int &nextId, std::string errorPrefix = "");

    ShowSymbolAssertion(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                        std::string_view message);

    const std::string message;

    void check(const UnorderedMap<std::string, std::shared_ptr<core::File>> &sourceFileContents, LSPWrapper &wrapper,
               int &nextId, std::string errorPrefix = "");

    std::string toString() const override;
};

// # selective-apply-code-action: quickfix
class StringPropertyAssertions final : public RangeAssertion {
public:
    static std::shared_ptr<StringPropertyAssertions> make(std::string_view filename, std::unique_ptr<Range> &range,
                                                          int assertionLine, std::string_view assertionContents,
                                                          std::string_view assertionType);

    const std::string assertionType;
    const std::vector<std::string> values;

    static std::optional<std::vector<std::string>>
    getValues(std::string_view type, const std::vector<std::shared_ptr<RangeAssertion>> &assertions);
    StringPropertyAssertions(std::string_view filename, std::unique_ptr<Range> &range, int assertionLine,
                             std::vector<std::string> values, std::string_view assertionType);

    std::string toString() const override;
};
} // namespace sorbet::test
#endif // TEST_HELPERS_POSITION_ASSERTIONS_H
