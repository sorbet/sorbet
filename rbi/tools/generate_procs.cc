#include <fstream>

constexpr int MAX_PROC_ARITY = 10;

using namespace std;

void emit_proc(std::ofstream &out, int arity) {
    out << "class Proc" << arity << " < Proc" << endl;
    out << "  Return = T.type" << endl;
    for (int i = 0; i < arity; ++i) {
        out << "  Arg" << i << " = T.type" << endl;
    }

    out << endl;

    out << "  sig(" << endl;
    for (int i = 0; i < arity; ++i) {
        out << "    arg" << i << ": "
            << "Arg" << i << "," << endl;
    }
    out << "  )" << endl;
    out << "  .returns(Return)" << endl;
    out << "  def call(";
    for (int i = 0; i < arity; ++i) {
        if (i != 0) {
            out << ", ";
        }
        out << "arg" << i;
    }
    out << ")" << endl;
    out << "  end" << endl;
    out << endl;
    out << "  alias_method :[], :call" << endl;
    out << endl;
    out << "end" << endl << endl;
}

int main(int argc, char **argv) {
    ofstream rb(argv[1], ios::trunc);
    for (int arity = 0; arity <= MAX_PROC_ARITY; ++arity) {
        emit_proc(rb, arity);
    }
}
