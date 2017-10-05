#ifndef RUBY_PARSER_STATE_STACK_HH
#define RUBY_PARSER_STATE_STACK_HH

#include <stack>
#include <memory>

namespace ruby_parser {
  class state_stack {
    std::stack<bool> stack;

  public:
    void push(bool state);
    bool pop();
    void lexpop();
    void clear();
    bool active() const;
  };
}

#endif
