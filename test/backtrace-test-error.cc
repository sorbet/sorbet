#include "common/common.h"
#include <csignal>

int main() {
    sorbet::Exception::raise("oops");
}
