#include <ruby_parser/state_stack.hh>

using namespace ruby_parser;

void state_stack::push(bool state) {
  stack.push(state);
}

bool state_stack::pop() {
  if (stack.empty()) {
    return false;
  } else {
    bool state = stack.top();
    stack.pop();
    return state;
  }
}

void state_stack::lexpop() {
  push(pop() || pop());
}

void state_stack::clear() {
  stack = std::stack<bool>();
}

bool state_stack::active() const {
  if (stack.empty()) {
    return false;
  } else {
    return stack.top();
  }
}
