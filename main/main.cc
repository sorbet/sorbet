#include "common/common.h"
namespace ruby_typer {
int realmain(int argc, char **argv);
}
int main(int argc, char **argv) {
    try {
        return ruby_typer::realmain(argc, argv);
    } catch (ruby_typer::SRubyException e) {
        return 1;
    }
};
