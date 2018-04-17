#include "common/common.h"
#include "main/realmain.h"
int main(int argc, char **argv) {
    try {
        return ruby_typer::realmain::realmain(argc, argv);
    } catch (ruby_typer::SRubyException e) {
        return 1;
    }
};
