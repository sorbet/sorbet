#ifndef RUBY_PARSER_LEXER_HH
#define RUBY_PARSER_LEXER_HH

#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <string>

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
};

class lexer {
public:
    using environment = std::set<std::string>;
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
    const std::string source_buffer;

    std::stack<environment> static_env;
    std::stack<literal> literal_stack;
    std::queue<token_t> token_queue;

    int cs;
    const char *_p;
    const char *_pe;
    const char *ts;
    const char *te;
    int act;

    const std::string FORWARD_ARGS = "FORWARD_ARGS";

    // State before =begin / =end block comment
    int cs_before_block_comment;

    std::vector<int> stack;
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

    void check_stack_capacity();
    int stack_pop();
    int arg_or_cmdarg(int cmd_state);
    void emit_comment(const char *s, const char *e);
    char unescape(uint32_t cp);
    std::string tok();
    std::string tok(const char *start);
    std::string tok(const char *start, const char *end);
    std::string_view tok_view();
    std::string_view tok_view(const char *start);
    std::string_view tok_view(const char *start, const char *end);
    void emit(token_type type);
    void emit(token_type type, std::string_view str);
    void emit(token_type type, std::string_view str, const char *start, const char *end);
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

    size_t last_token_s;
    size_t last_token_e;

    bool in_kwarg; // true at the end of "def foo a:"
    Context context;

    lexer(diagnostics_t &diag, ruby_version version, const std::string &source_buffer_);

    token_t advance();

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
    void declare(const std::string &name);
    bool is_declared(const std::string &identifier) const;
    void declare_forward_args();
    bool is_declared_forward_args();

    optional_size dedentLevel();
};
} // namespace ruby_parser

#include "driver.hh"

#endif
