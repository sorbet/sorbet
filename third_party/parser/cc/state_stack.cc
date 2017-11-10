#include <ruby_parser/state_stack.hh>

using namespace ruby_parser;

void state_stack::push(bool state) {
  stack.emplace_back(state);
}

bool state_stack::pop() {
  if (stack.empty()) {
    return false;
  } else {
    bool state = stack.back();
    stack.pop_back();
    return state;
  }
}

void state_stack::lexpop() {
  push(pop() || pop());
}

void state_stack::clear() {
  stack.clear();
}

bool state_stack::active() const {
  if (stack.empty()) {
    return false;
  } else {
    return stack.back();
  }
}
