#include <csignal>

int main() {
    raise(SIGSEGV);
}
