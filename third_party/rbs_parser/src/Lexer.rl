#include "Lexer.hh"

%%{
  machine Lexer;
  alphtype char;
  write data;

  main := |*

    # Keywords

    "alias"         => { emit(val, loc); ret = Parser::token::kALIAS; fbreak; };
    "any"           => { emit(val, loc); ret = Parser::token::kANY; fbreak; };
    "attr_accessor" => { emit(val, loc); ret = Parser::token::kATTRACCESSOR; fbreak; };
    "attr_reader"   => { emit(val, loc); ret = Parser::token::kATTRREADER; fbreak; };
    "attr_writer"   => { emit(val, loc); ret = Parser::token::kATTRWRITER; fbreak; };
    "bool"          => { emit(val, loc); ret = Parser::token::kBOOL; fbreak; };
    "bot"           => { emit(val, loc); ret = Parser::token::kBOT; fbreak; };
    "class"         => { emit(val, loc); ret = Parser::token::kCLASS; fbreak; };
    "def"           => { emit(val, loc); ret = Parser::token::kDEF; fbreak; };
    "end"           => { emit(val, loc); ret = Parser::token::kEND; fbreak; };
    "extend"        => { emit(val, loc); ret = Parser::token::kEXTEND; fbreak; };
    "extension"     => { emit(val, loc); ret = Parser::token::kEXTENSION; fbreak; };
    "false"         => { emit(val, loc); ret = Parser::token::kFALSE; fbreak; };
    "in"            => { emit(val, loc); ret = Parser::token::kIN; fbreak; };
    "include"       => { emit(val, loc); ret = Parser::token::kINCLUDE; fbreak; };
    "incompatible"  => { emit(val, loc); ret = Parser::token::kINCOMPATIBLE; fbreak; };
    "instance"      => { emit(val, loc); ret = Parser::token::kINSTANCE; fbreak; };
    "interface"     => { emit(val, loc); ret = Parser::token::kINTERFACE; fbreak; };
    "module"        => { emit(val, loc); ret = Parser::token::kMODULE; fbreak; };
    "nil"           => { emit(val, loc); ret = Parser::token::kNIL; fbreak; };
    "out"           => { emit(val, loc); ret = Parser::token::kOUT; fbreak; };
    "prepend"       => { emit(val, loc); ret = Parser::token::kPREPEND; fbreak; };
    "private"       => { emit(val, loc); ret = Parser::token::kPRIVATE; fbreak; };
    "public"        => { emit(val, loc); ret = Parser::token::kPUBLIC; fbreak; };
    "self"          => { emit(val, loc); ret = Parser::token::kSELF; fbreak; };
    "self?"         => { emit(val, loc); ret = Parser::token::kSELFQ; fbreak; };
    "singleton"     => { emit(val, loc); ret = Parser::token::kSINGLETON; fbreak; };
    "super"         => { emit(val, loc); ret = Parser::token::kSUPER; fbreak; };
    "top"           => { emit(val, loc); ret = Parser::token::kTOP; fbreak; };
    "true"          => { emit(val, loc); ret = Parser::token::kTRUE; fbreak; };
    "type"          => { emit(val, loc); ret = Parser::token::kTYPE; fbreak; };
    "unchecked"     => { emit(val, loc); ret = Parser::token::kUNCHECKED; fbreak; };
    "untyped"       => { emit(val, loc); ret = Parser::token::kUNTYPED; fbreak; };
    "void"          => { emit(val, loc); ret = Parser::token::kVOID; fbreak; };

    # Punctuation

    "!"          => { emit(val, loc); ret = Parser::token::kEXCLAMATION; fbreak; };
    "!="         => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "!~"         => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "%"          => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "&"          => { emit(val, loc); ret = Parser::token::kAMP; fbreak; };
    "("          => { emit(val, loc); ret = Parser::token::kLPAREN; fbreak; };
    ")"          => { emit(val, loc); ret = Parser::token::kRPAREN; fbreak; };
    "*"          => { emit(val, loc); ret = Parser::token::kSTAR; fbreak; };
    "**"         => { emit(val, loc); ret = Parser::token::kSTAR2; fbreak; };
    "+"          => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "+@"         => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    ","          => { emit(val, loc); ret = Parser::token::kCOMMA; fbreak; };
    "-"          => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "->"         => { emit(val, loc); ret = Parser::token::kARROW; fbreak; };
    "-@"         => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "."          => { emit(val, loc); ret = Parser::token::kDOT; fbreak; };
    "/"          => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    ":"          => { emit(val, loc); ret = Parser::token::kCOLON; fbreak; };
    "::"         => { emit(val, loc); ret = Parser::token::kCOLON2; fbreak; };
    "<"          => { emit(val, loc); ret = Parser::token::kLT; fbreak; };
    "<<"         => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "<="         => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "<=>"        => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "="          => { emit(val, loc); ret = Parser::token::kEQ; fbreak; };
    "=="         => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "==="        => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "=>"         => { emit(val, loc); ret = Parser::token::kFATARROW; fbreak; };
    "=~"         => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    ">"          => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    ">="         => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    ">>"         => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "?"          => { emit(val, loc); ret = Parser::token::kQUESTION; fbreak; };
    "["          => { emit(val, loc); ret = Parser::token::kLBRACKET; fbreak; };
    "[]"         => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "[]="        => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "]"          => { emit(val, loc); ret = Parser::token::kRBRACKET; fbreak; };
    "^"          => { emit(val, loc); ret = Parser::token::kHAT; fbreak; };
    "`"          => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };
    "{"          => { emit(val, loc); ret = Parser::token::kLBRACE; fbreak; };
    "|"          => { emit(val, loc); ret = Parser::token::kBAR; fbreak; };
    "}"          => { emit(val, loc); ret = Parser::token::kRBRACE; fbreak; };
    "~"          => { emit(val, loc); ret = Parser::token::kOPERATOR; fbreak; };

    # Literals

    "\""("\""|[^"\n])*"\""
        =>  { emit(val, loc); ret = Parser::token::tSTRING; fbreak; };

    "'"("'"|[^'])*"'"
        =>  { emit(val, loc); ret = Parser::token::tSTRING; fbreak; };

    ("+"|"-")?[0-9][0-9_]*
        =>  { emit(val, loc); ret = Parser::token::tINTEGER; fbreak; };

    # Tokens

    "`"("\`"|[^` :])+"`"
        =>  { emit(val, loc); ret = Parser::token::tQUOTEDMETHOD; fbreak; };

    "`"[a-zA-Z_][A-Za-z0-9_]*"`"
        =>  { emit(val, loc); ret = Parser::token::tQUOTEDIDENT; fbreak; };

    "_"[a-zA-Z][A-Za-z0-9_]*
        =>  { emit(val, loc); ret = Parser::token::tINTERFACEIDENT; fbreak; };

    "@"[A-Za-z_][A-Za-z0-9_]*
        =>  { emit(val, loc); ret = Parser::token::tIVAR; fbreak; };

    "$"[A-Za-z_][A-Za-z0-9_]*
        =>  { emit(val, loc); ret = Parser::token::tGLOBALIDENT; fbreak; };

    ":"[A-Za-z0-9_]+
        =>  { emit(val, loc); ret = Parser::token::tSYMBOL; fbreak; };

    [a-z][A-Za-z0-9_]*":"
        =>  { emit(val, loc); ret = Parser::token::tLKEYWORD; fbreak; };

    [A-Z][A-Za-z0-9_]*":"
        =>  { emit(val, loc); ret = Parser::token::tUKEYWORD; fbreak; };

    [a-z][A-Za-z0-9_]*"!"
        =>  { emit(val, loc); ret = Parser::token::tEXCLMETHOD; fbreak; };

    [A-Z][A-Za-z0-9_]*"!"
        =>  { emit(val, loc); ret = Parser::token::tEXCLMETHOD; fbreak; };

    "`"[A-Z][A-Za-z0-9_]*"!`"
        =>  { emit(val, loc); ret = Parser::token::tEXCLMETHOD; fbreak; };

    [a-z_][A-Za-z0-9_]*
        =>  { emit(val, loc); ret = Parser::token::tLIDENT; fbreak; };

    [A-Z_][A-Za-z0-9_]*
        =>  { emit(val, loc); ret = Parser::token::tUIDENT; fbreak; };

    "::"?([A-Z][A-Za-z0-9_]*"::")+
        =>  { emit(val, loc); ret = Parser::token::tNAMESPACE; fbreak; };

    # TODO annotations

    # Ignored

    [\r\n]  => { cl += 1; cc = 1; };
    [ \t]   => { cc += te - ts; };
    /#[^\n]*/ =>  { };

  *|;
}%%

rbs_parser::Lexer::Lexer(const std::string input): buffer(input) {
    %%write init;

    p = buffer.c_str();
    pe = p + buffer.size();
    eof = pe;

    cl = 1;
    cc = 1;
};

void rbs_parser::Lexer::emit(Parser::semantic_type* val, Parser::location_type* loc) {
	loc->begin.line = cl;
	loc->begin.column = cc;

	val->string = new std::string(ts, te - ts);
	cc += te - ts;

	loc->end.line = cl;
	loc->end.column = cc - 1;
}

rbs_parser::Parser::token_type rbs_parser::Lexer::lex(Parser::semantic_type* val, Parser::location_type* loc) {
    Parser::token_type ret = Parser::token::tEOF;

    %% write exec;

    if (cs == Lexer_error) {
        te = ts + 1; // We read the next char
        emit(val, loc);
        return Parser::token::tERROR;
    }

    return ret;
}
