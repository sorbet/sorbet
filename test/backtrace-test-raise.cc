#include <signal.h>

int main() {
    raise(SIGSEGV);
}
