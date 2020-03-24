#ifndef RBS_PARSER_LEXER_HH
#define RBS_PARSER_LEXER_HH

#include "Parser.hh"

namespace rbs_parser {
// See https://www.colm.net/files/ragel/ragel-guide-6.3.pdf
class Lexer {
public:
    Lexer(const std::string rbs_string);
    void emit(Parser::semantic_type *val, Parser::location_type *loc);
    Parser::token_type lex(Parser::semantic_type *, Parser::location_type *);

private:
    // String to process
    std::string buffer;

    // Data pointer: the next character to process in the buffer
    const char *p;

    // Data end pointer: p + length of the current run of the machine
    const char *pe;

    // End of file pointer
    const char *eof;

    // Machine states stack
    int stack[1];

    // Position in the state stack
    int top;

    // Current state
    int cs;

    // Last matching state
    int act;

    // Pointer to the first and last character of the current token
    const char *ts, *te;

    // Current line
    int cl;

    // Current column
    int cc;
};
}; // namespace rbs_parser

#endif
