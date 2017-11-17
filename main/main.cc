#include "common/common.h"
int realmain(int argc, char **argv);
int main(int argc, char **argv) {
    try {
        return realmain(argc, argv);
    } catch (ruby_typer::SRubyException e) {
        return 1;
    }
};
