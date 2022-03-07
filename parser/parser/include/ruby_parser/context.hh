#ifndef RUBY_PARSER_CONTEXT_HH
#define RUBY_PARSER_CONTEXT_HH

#include <optional>
#include <set>
#include <vector>

namespace ruby_parser {

class Context {
public:
    enum class State {
        CLASS,
        MODULE,
        SCLASS,
        DEF,
        DEFS,
        BLOCK,
        LAMBDA,
        DEF_OPEN_ARGS,
    };

    void push(State state);
    void pop();
    void reset();
    bool inBlock();
    bool inClass();
    bool inDynamicBlock();
    bool inLambda();
    bool inDefOpenArgs();
    bool indirectlyInDef();
    bool classDefintinionAllowed();
    bool moduleDefintinionAllowed();
    bool dynamicConstDefintinionAllowed();
    std::vector<State> stackCopy();

private:
    std::vector<State> stack;

    std::optional<int> firstIndexOfState(State state);
    std::optional<int> lastIndexOfState(State state);
    bool contains(State state);
};

} // namespace ruby_parser

#endif