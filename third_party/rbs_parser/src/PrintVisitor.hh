#ifndef RBS_PARSER_PRINT_VISITOR_HH
#define RBS_PARSER_PRINT_VISITOR_HH

#include "ast.hh"
#include <ostream>

namespace rbs_parser {

class PrintVisitor : public Visitor {
public:
    int currentIndent;
    int indentSize;
    std::ostream &output;

    PrintVisitor(std::ostream &output) : output(output) {
        currentIndent = 0;
        indentSize = 2;
    };

    // Print a raw string
    void print(std::string str) { output << str; }

    // Print a tab
    void printt() {
        for (int i = 0; i < currentIndent; i++) {
            for (int j = 0; j < indentSize; j++) {
                print(" ");
            }
        }
    }

    // Print a new line
    void printn() { print("\n"); }

    void printn(std::string str) {
        print(str);
        print("\n");
    }

    // Print a line preceded by a tab and terminated by a new line
    void printl(std::string str) {
        printt();
        print(str);
        printn();
    }

    void indent() { currentIndent += 1; }

    void dedent() { currentIndent -= 1; }
};
} // namespace rbs_parser

#endif
