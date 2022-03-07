#include <ruby_parser/context.hh>

using namespace ruby_parser;
using State = Context::State;

std::optional<int> Context::firstIndexOfState(State state) {
    for (int i = 0; i < stack.size(); i++) {
        if (stack[i] == state) {
            return i;
        }
    }

    return std::nullopt;
}

std::optional<int> Context::lastIndexOfState(State state) {
    for (int i = stack.size() - 1; i >= 0; i--) {
        if (stack[i] == state) {
            return i;
        }
    }

    return std::nullopt;
}

bool Context::contains(State state) {
    return firstIndexOfState(state) != std::nullopt;
}

void Context::push(State state) {
    stack.push_back(state);
}

void Context::pop() {
    stack.pop_back();
}

void Context::reset() {
    stack.clear();
}

bool Context::inBlock() {
    return !stack.empty() && stack[stack.size() - 1] == State::BLOCK;
}

bool Context::inClass() {
    return !stack.empty() && stack[stack.size() - 1] == State::CLASS;
}

bool Context::inDynamicBlock() {
    return inBlock() || inLambda();
}

bool Context::inLambda() {
    return !stack.empty() && stack[stack.size() - 1] == State::LAMBDA;
}

bool Context::indirectlyInDef() {
    return contains(State::DEF) || contains(State::DEFS);
}

bool Context::inDefOpenArgs() {
    return !stack.empty() && stack[stack.size() - 1] == State::DEF_OPEN_ARGS;
}

bool Context::classDefintinionAllowed() {
    auto defIndex = std::max(lastIndexOfState(State::DEF), lastIndexOfState(State::DEFS));
    auto sclassIndex = lastIndexOfState(State::SCLASS);

    if (!defIndex) {
        return true;
    }

    return sclassIndex && defIndex && (*sclassIndex) > (*defIndex);
}

bool Context::moduleDefintinionAllowed() {
    return classDefintinionAllowed();
}

bool Context::dynamicConstDefintinionAllowed() {
    return classDefintinionAllowed();
}

std::vector<State> Context::stackCopy() {
    return stack;
}
