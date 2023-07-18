#ifndef RUBY_PARSER_LEXER_HH
#define RUBY_PARSER_LEXER_HH

#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include "common/StableStringStorage.h"

#include "context.hh"
#include "diagnostic.hh"
#include "literal.hh"
#include "pool.hh"
#include "state_stack.hh"
#include "token.hh"

namespace ruby_parser {
enum class ruby_version {
    RUBY_18,
    RUBY_19,
    RUBY_20,
    RUBY_21,
    RUBY_22,
    RUBY_23,
    RUBY_24,
    RUBY_25,
    RUBY_26,
    RUBY_27,
    RUBY_30,
    RUBY_31,
};

class lexer {
public:
    using environment = std::set<std::string, std::less<void>>;
    struct token_table_entry {
        std::string_view token;
        token_type type;
    };

    enum class num_xfrm_type {
        NONE,
        RATIONAL,
        IMAGINARY,
        RATIONAL_IMAGINARY,
        FLOAT,
        FLOAT_IMAGINARY,
    };

private:
    diagnostics_t &diagnostics;
    pool<token, 64> mempool;

    ruby_version version;
    std::string_view source_buffer;
    sorbet::StableStringStorage<> &scratch;
    const std::vector<int> lineBreaks;

    std::stack<environment> static_env;
    std::stack<literal> literal_stack;
    std::deque<token_t> token_queue;

    // Required by Ragel to implement its finite state machine.
    // It uses an int to keep track of the state machine's internal state.
    //
    // Mnemonic: Current State
    int cs;
    // Comments for p and pe are in the lexer implementation.
    // These fields mirror the values that Ragel uses directly.
    const char *_p;
    const char *_pe;

    // We're using an "advanced" feature of Ragel: the abiility to make a scanner, which means we
    // give a list of pattern alternatives and ask Ragel to select the one that best matches.
    //
    // When using Ragel as a scanner, it needs ts, te, and act so that it can backtrack.

    // Start of the token matched by the scanner.
    //
    // Mnemonic: Token Start
    const char *ts;

    // One past the end of the toke matched by the scanner.
    //
    // Mnemonic: Token End
    const char *te;

    // Used for recording the identity of the last pattern matched for backtracking in the scanner.
    //
    // Mnemonic: Action
    int act;

    const std::string FORWARD_ARGS = "FORWARD_ARGS";
    const std::string ANONYMOUS_BLOCKARG = "ANONYMOUS_BLOCKARG";
    const std::string ANONYMOUS_RESTARG = "ANONYMOUS_RESTARG";
    const std::string ANONYMOUS_KWRESTARG = "ANONYMOUS_KWRESTARG";

    // State before =begin / =end block comment
    int cs_before_block_comment;

    // Used by ragel to store integer values representing states (see `cs`).
    std::vector<int> stack;
    // Used by ragel. Represents an index into `stack` representing the next available spot on the
    // top of the stack.
    int top;

    const char *eq_begin_s; // location of last encountered =begin
    const char *sharp_s;    // location of last encountered #
    const char *newline_s;  // location of last encountered newline

    // Ruby 1.9 ->() lambdas emit a distinct token if do/{ is
    // encountered after a matching closing parenthesis.
    size_t paren_nest;
    std::stack<size_t> lambda_stack;

    // If the lexer is in `command state' (aka expr_value)
    // at the entry to #advance, it will transition to expr_cmdarg
    // instead of expr_arg at certain points.
    bool command_start;

    int num_base;             // last numeric base
    const char *num_digits_s; // starting position of numeric digits
    const char *num_suffix_s; // starting position of numeric suffix
    num_xfrm_type num_xfrm;   // numeric suffix-induced transformation

    const char *escape_s;                // starting position of current sequence
    std::unique_ptr<std::string> escape; // last escaped sequence, as string

    const char *herebody_s; // starting position of current heredoc line

    // After encountering the closing line of <<~SQUIGGLY_HEREDOC,
    // we store the indentation level and give it out to the parser
    // on request. It is not possible to infer indentation level just
    // from the AST because escape sequences such as `\ ` or `\t` are
    // expanded inside the lexer, but count as non-whitespace for
    // indentation purposes.
    optional_size dedentLevel_;

    bool traceLexer;

    size_t line_start(token_type type, size_t beginPos);
    void check_stack_capacity();
    int stack_pop();
    int arg_or_cmdarg(int cmd_state);
    void emit_comment(const char *s, const char *e);
    char unescape(uint32_t cp);
    std::string tok() const;
    std::string tok(const char *start) const;
    std::string tok(const char *start, const char *end) const;
    std::string_view tok_view() const;
    std::string_view tok_view(const char *start) const;
    std::string_view tok_view(const char *start, const char *end) const;
    void emit(token_type type);
    void emit(token_type type, std::string_view str);
    void emit(token_type type, std::string_view str, const char *start, const char *end);
    // Without these overloads, emit(..., "do") would be ambiguous.
    // It's OK to store points to constants, as they live in static storage and will
    // therefore not be going away.
    template <size_t N> void emit(token_type type, const char (&str)[N]) {
        emit(type, std::string_view{&str[0], N - 1});
    }
    template <size_t N> void emit(token_type type, const char (&str)[N], const char *start, const char *end) {
        emit(type, std::string_view{&str[0], N - 1}, start, end);
    }
    void emit(token_type type, const std::string &str);
    void emit(token_type type, const std::string &str, const char *start, const char *end);
    void emit_do(bool do_block = false);
    void emit_table(const token_table_entry *table);
    void emit_num(const std::string &num);
    std::string convert_base(const std::string &num, int num_base);
    diagnostic::range range(const char *start, const char *end);
    void diagnostic_(dlevel level, dclass type, const std::string &data = "");
    void diagnostic_(dlevel level, dclass type, diagnostic::range &&range, const std::string &data = "");
    template <typename... Args> int push_literal(Args &&...args);
    int next_state_for_literal(literal &lit);
    literal &literal_();
    int pop_literal();

    token_t advance_();

    // literal needs to call emit:
    friend class literal;

public:
    state_stack cond;
    state_stack cmdarg;

    Context context;

    lexer(diagnostics_t &diag, ruby_version version, std::string_view source_buffer,
          sorbet::StableStringStorage<> &scratch, bool traceLexer);

    // Main interface consumed by yylex function in parser
    token_t advance();

    // Opposite of `advance` method. Used to put an already-lexed token into the token_queue again.
    // Useful for certain kinds of syntax error recovery (grep for examples).
    void unadvance(token_t token);

    std::string_view tok_view_from_offsets(size_t start, size_t end) const;

    // Useful for error recovery. Manually sets `_p` so that the next call to `advance` starts
    // from the desired location, and resets some state to trick the lexer into thinking it's in the
    // expr_end state.
    //
    // Note: this method has only been tested for a limited number of error recovery use cases.
    // Using it for new use cases might require expanding the pieces of state it has to reset.
    void rewind_and_reset_to_expr_end(size_t newPos);

    // Similar to rewind_and_reset_to_expr_end, but less common. Comes with the same caveats.
    void rewind_and_reset_to_expr_beg(size_t newPos);

    int compare_indent_level(token_t left, token_t right);

    void set_state_expr_beg();
    void set_state_expr_end();
    void set_state_expr_endarg();
    void set_state_expr_fname();
    void set_state_expr_value();

    void unset_command_start() {
        command_start = false;
    }

    void extend_static();
    void extend_dynamic();
    void unextend();
    void declare(std::string_view name);
    bool is_declared(std::string_view identifier) const;
    void declare_forward_args();
    bool is_declared_forward_args();
    void declare_anonymous_args();
    bool is_declared_anonymous_args();
    void declare_anonymous_restarg();
    bool is_declared_anonymous_restarg();
    void declare_anonymous_kwrestarg();
    bool is_declared_anonymous_kwrestarg();

    optional_size dedentLevel();
};
} // namespace ruby_parser

#include "driver.hh"

#endif
