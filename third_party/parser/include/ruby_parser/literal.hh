#ifndef RUBY_PARSER_LITERAL_HH
#define RUBY_PARSER_LITERAL_HH

#include <string>
#include <utility>
#include <optional>

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
    lexer& _lexer;
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
    const char* buffer_s;
    const char* buffer_e;

  public:
    // lexer needs access to these:
    const char* str_s;
    const char* saved_herebody_s;
    const char* heredoc_e;

    literal(lexer& lexer, literal_type type, std::string delimiter, const char* str_s, const char* heredoc_e = nullptr, bool indent = false, bool dedent_body = false, bool label_allowed = false);

    // delete copy constructor to prevent accidental copies. we never
    // legitimately need to copy literal.
    literal(const literal&) = delete;

    bool words() const;
    bool backslash_delimited() const;
    bool interpolate() const;
    bool regexp() const;
    bool heredoc() const;

    token_type start_token_type() const;

    optional_size dedentLevel() const;

    bool munge_escape(char c) const;

    void infer_indent_level(std::string& line);

    void start_interp_brace();
    bool end_interp_brace_and_try_closing();

    bool nest_and_try_closing(std::string& delimiter, const char* ts, const char* te, std::string lookahead = "");

    void extend_space(const char* ts, const char* te);
    void extend_string(std::string& str, const char* ts, const char* te);
    void extend_content();

    void flush_string();

  private:
    bool is_delimiter(std::string& delimiter) const;
    void clear_buffer();
    void emit_start_token();
    void emit(token_type tok, std::string& value, const char* s, const char* e);
  };
}

// there is a circular dependency between lexer and literal.
// lexer was forward-declared above, but now we need to include it
// properly.
#include "lexer.hh"

#endif
