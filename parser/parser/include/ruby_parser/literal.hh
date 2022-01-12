#ifndef RUBY_PARSER_LITERAL_HH
#define RUBY_PARSER_LITERAL_HH

#include <optional>
#include <string>
#include <utility>

#include "token.hh"

namespace ruby_parser {
enum class literal_type {
    SQUOTE_STRING,
    SQUOTE_HEREDOC,
    LOWERQ_STRING,
    DQUOTE_STRING,
    DQUOTE_HEREDOC,
    PERCENT_STRING,
    UPPERQ_STRING,
    LOWERW_WORDS,
    UPPERW_WORDS,
    LOWERI_SYMBOLS,
    UPPERI_SYMBOLS,
    SQUOTE_SYMBOL,
    LOWERS_SYMBOL,
    DQUOTE_SYMBOL,
    SLASH_REGEXP,
    PERCENT_REGEXP,
    LOWERX_XSTRING,
    BACKTICK_XSTRING,
    BACKTICK_HEREDOC,
};

using optional_size = std::optional<size_t>;

class lexer;

class literal {
    lexer &_lexer;
    size_t _nesting;
    literal_type _type;
    std::string start_delim;
    std::string end_delim;
    bool indent;
    bool dedent_body;
    bool label_allowed;
    optional_size _dedentLevel;
    size_t _interp_braces;
    bool space_emitted;
    bool monolithic;
    std::string buffer;
    const char *buffer_s;
    const char *buffer_e;

public:
    // lexer needs access to these:
    const char *str_s;
    const char *saved_herebody_s;
    const char *heredoc_e;

    literal(lexer &lexer, literal_type type, std::string delimiter, const char *str_s, const char *heredoc_e = nullptr,
            bool indent = false, bool dedent_body = false, bool label_allowed = false);

    // delete copy constructor to prevent accidental copies. we never
    // legitimately need to copy literal.
    literal(const literal &) = delete;

    bool words() const;
    bool backslash_delimited() const;
    bool interpolate() const;
    bool regexp() const;
    bool heredoc() const;
    bool squiggly_heredoc() const;
    bool support_line_continuation_via_slash() const;

    token_type start_token_type() const;

    optional_size dedentLevel() const;

    bool munge_escape(char c) const;

    void infer_indent_level(std::string &line);

    void start_interp_brace();
    bool end_interp_brace_and_try_closing();

    bool nest_and_try_closing(std::string_view delimiter, const char *ts, const char *te,
                              std::string_view lookahead = "");

    void extend_space(const char *ts, const char *te);
    void extend_string(std::string_view str, const char *ts, const char *te);
    void extend_content();

    void flush_string();

private:
    bool is_delimiter(std::string_view delimiter) const;
    void clear_buffer();
    void emit_start_token();
    void emit(token_type tok, std::string_view value, const char* s, const char* e);
    void emit(token_type tok, const std::string &value, const char* s, const char* e);
};
} // namespace ruby_parser

// there is a circular dependency between lexer and literal.
// lexer was forward-declared above, but now we need to include it
// properly.
#include "lexer.hh"

#endif
