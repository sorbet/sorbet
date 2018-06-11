#include "common/common.h"
#include <csignal>

int main() {
    sorbet::Error::raise("oops");
}
