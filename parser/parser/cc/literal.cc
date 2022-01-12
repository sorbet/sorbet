#include <cassert>
#include <ruby_parser/literal.hh>

using namespace ruby_parser;
using namespace std::literals::string_view_literals;

literal::literal(lexer &lexer, literal_type type, std::string delimiter, const char *str_s, const char *heredoc_e,
                 bool indent, bool dedent_body, bool label_allowed)
    : _lexer(lexer), _nesting(1), _type(type), indent(indent), dedent_body(dedent_body), label_allowed(label_allowed),
      _interp_braces(0), space_emitted(true), str_s(str_s), saved_herebody_s(nullptr), heredoc_e(heredoc_e) {
    if (delimiter == "(") {
        start_delim = "(";
        end_delim = ")";
    } else if (delimiter == "[") {
        start_delim = "[";
        end_delim = "]";
    } else if (delimiter == "{") {
        start_delim = "{";
        end_delim = "}";
    } else if (delimiter == "<") {
        start_delim = "<";
        end_delim = ">";
    } else {
        start_delim = "";
        end_delim = delimiter;
    }

    // Monolithic strings are glued into a single token, e.g.
    // tSTRING_BEG tSTRING_CONTENT tSTRING_END -> tSTRING.
    monolithic = (type == literal_type::SQUOTE_STRING || type == literal_type::DQUOTE_STRING);

    clear_buffer();

    if (!monolithic) {
        emit_start_token();
    }
}

bool literal::words() const {
    return _type == literal_type::UPPERW_WORDS || _type == literal_type::LOWERW_WORDS ||
           _type == literal_type::UPPERI_SYMBOLS || _type == literal_type::LOWERI_SYMBOLS;
}

bool literal::backslash_delimited() const {
    return end_delim == "\\";
}

bool literal::interpolate() const {
    return _type == literal_type::DQUOTE_STRING || _type == literal_type::DQUOTE_HEREDOC ||
           _type == literal_type::PERCENT_STRING || _type == literal_type::UPPERQ_STRING ||
           _type == literal_type::UPPERW_WORDS || _type == literal_type::UPPERI_SYMBOLS ||
           _type == literal_type::DQUOTE_SYMBOL || _type == literal_type::SLASH_REGEXP ||
           _type == literal_type::PERCENT_REGEXP || _type == literal_type::LOWERX_XSTRING ||
           _type == literal_type::BACKTICK_XSTRING || _type == literal_type::BACKTICK_HEREDOC;
}

bool literal::regexp() const {
    return _type == literal_type::SLASH_REGEXP || _type == literal_type::PERCENT_REGEXP;
}

bool literal::heredoc() const {
    return heredoc_e != nullptr;
}

bool literal::squiggly_heredoc() const {
    return heredoc() && dedent_body;
}

bool literal::support_line_continuation_via_slash() const {
    return !words() && interpolate();
}

token_type literal::start_token_type() const {
    switch (_type) {
        case literal_type::SQUOTE_STRING:
        case literal_type::SQUOTE_HEREDOC:
        case literal_type::LOWERQ_STRING:
        case literal_type::DQUOTE_STRING:
        case literal_type::DQUOTE_HEREDOC:
        case literal_type::PERCENT_STRING:
        case literal_type::UPPERQ_STRING:
            return token_type::tSTRING_BEG;

        case literal_type::LOWERW_WORDS:
            return token_type::tQWORDS_BEG;

        case literal_type::UPPERW_WORDS:
            return token_type::tWORDS_BEG;

        case literal_type::LOWERI_SYMBOLS:
            return token_type::tQSYMBOLS_BEG;

        case literal_type::UPPERI_SYMBOLS:
            return token_type::tSYMBOLS_BEG;

        case literal_type::SQUOTE_SYMBOL:
        case literal_type::LOWERS_SYMBOL:
        case literal_type::DQUOTE_SYMBOL:
            return token_type::tSYMBEG;

        case literal_type::SLASH_REGEXP:
        case literal_type::PERCENT_REGEXP:
            return token_type::tREGEXP_BEG;

        case literal_type::LOWERX_XSTRING:
        case literal_type::BACKTICK_XSTRING:
        case literal_type::BACKTICK_HEREDOC:
            return token_type::tXSTRING_BEG;
        default:
            assert(false);
    }
}

optional_size literal::dedentLevel() const {
    return _dedentLevel;
}

bool literal::munge_escape(char c) const {
    if (words() && (c == ' ' || c == '\t' || c == '\v' || c == '\r' || c == '\f' || c == '\n')) {
        return true;
    } else if (c == '\\' || (start_delim.size() == 1 && start_delim.at(0) == c) ||
               (end_delim.size() == 1 && end_delim.at(0) == c)) {
        return true;
    } else {
        return false;
    }
}

void literal::infer_indent_level(std::string &line) {
    if (!dedent_body) {
        return;
    }

    size_t indent_level = 0;

    for (auto it = line.cbegin(); it != line.cend(); ++it) {
        if (*it == ' ') {
            indent_level++;
            continue;
        }

        if (*it == '\t') {
            indent_level += (8 - indent_level % 8);
            continue;
        }

        if (!_dedentLevel || *_dedentLevel > indent_level) {
            _dedentLevel = indent_level;
        }
        break;
    }
}

void literal::start_interp_brace() {
    _interp_braces++;
}

bool literal::end_interp_brace_and_try_closing() {
    _interp_braces--;

    return _interp_braces == 0;
}

// copied from MRI's include/ruby/ruby.h:
static bool rb_isspace(char c) {
    return c == ' ' || ('\t' <= c && c <= '\r');
}

static std::string_view lstrip(std::string_view str) {
    size_t index = 0;

    while (index < str.size()) {
        if (rb_isspace(str.at(index))) {
            index++;
        } else {
            break;
        }
    }

    return str.substr(index);
}

bool literal::is_delimiter(std::string_view delimiter) const {
    if (indent) {
        std::string_view stripped_delimiter = lstrip(delimiter);
        return end_delim == stripped_delimiter;
    } else {
        return end_delim == delimiter;
    }
}

static bool lookahead_quoted_label(std::string_view lookahead) {
    switch (lookahead.size()) {
        case 0:
            return false;

        case 1:
            return lookahead.at(0) == ':';

        default:
            return lookahead.at(0) == ':' && lookahead.at(1) != ':';
    }
}

bool literal::nest_and_try_closing(std::string_view delimiter, const char *ts, const char *te,
                                   std::string_view lookahead) {
    if (start_delim.size() > 0 && start_delim == delimiter) {
        _nesting++;
    } else if (is_delimiter(delimiter)) {
        _nesting--;
    }

    if (_nesting == 0) {
        if (words()) {
            extend_space(ts, ts);
        }

        if (label_allowed && lookahead_quoted_label(lookahead) && start_token_type() == token_type::tSTRING_BEG) {
            // This is a quoted label.
            flush_string();
            emit(token_type::tLABEL_END, end_delim, ts, te + 1);
            return true;
        } else if (monolithic) {
            // Emit the string as a single token.
            emit(token_type::tSTRING, buffer, str_s, te);
            return true;
        } else {
            // If this is a heredoc, @buffer contains the sentinel now.
            // Just throw it out. Lexer flushes the heredoc after each
            // non-heredoc-terminating \n anyway, so no data will be lost.
            if (!heredoc()) {
                flush_string();
            }

            emit(token_type::tSTRING_END, end_delim, ts, te);
            return true;
        }
    }

    return false;
}

void literal::extend_space(const char *ts, const char *te) {
    flush_string();

    if (!space_emitted) {
        emit(token_type::tSPACE, ""sv, ts, te);

        space_emitted = true;
    }
}

void literal::extend_string(std::string_view str, const char *ts, const char *te) {
    if (!buffer_s) {
        buffer_s = ts;
    }

    buffer_e = te;

    buffer += str;
}

void literal::extend_content() {
    space_emitted = false;
}

void literal::flush_string() {
    if (monolithic) {
        emit_start_token();
        monolithic = false;
    }

    if (!buffer.empty()) {
        emit(token_type::tSTRING_CONTENT, buffer, buffer_s, buffer_e);

        clear_buffer();
        extend_content();
    }
}

void literal::clear_buffer() {
    buffer = "";
    buffer_s = nullptr;
    buffer_e = nullptr;
}

void literal::emit_start_token() {
    auto str_type_length = 1 /* TODO @str_type.length */;
    auto str_e = heredoc_e ? heredoc_e : str_s + str_type_length;
    emit(start_token_type(), ""sv, str_s, str_e);
}

void literal::emit(token_type tok, std::string_view value, const char *s, const char *e) {
    _lexer.emit(tok, value, s, e);
}

void literal::emit(token_type tok, const std::string &value, const char* s, const char* e) {
  _lexer.emit(tok, value, s, e);
}
