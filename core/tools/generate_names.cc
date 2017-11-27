#include "common/common.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct NameDef {
    int id;
    string srcName;
    string val;

    NameDef(const char *srcName, const char *val) : srcName(srcName), val(val) {}
    NameDef(const char *srcName) : srcName(srcName), val(srcName) {}
};

NameDef names[] = {
    {"initialize"},
    {"andAnd", "&&"},
    {"orOr", "||"},
    {"to_s"},
    {"concat"},
    {"intern"},
    {"call"},
    {"bang", "!"},
    {"squareBrackets", "[]"},
    {"squareBracketsEq", "[]="},
    {"unaryPlus", "@+"},
    {"unaryMinus", "@-"},
    {"star", "*"},
    {"starStar", "**"},
    {"ampersand", "&"},
    {"tripleEq", "==="},

    // used in CFG for temporaries
    {"whileTemp"},
    {"ifTemp"},
    {"returnTemp"},
    {"statTemp"},
    {"assignTemp"},
    {"returnMethodTemp"},
    {"blockReturnTemp"},
    {"selfMethodTemp"},
    {"hashTemp"},
    {"rescueTemp"},
    // end CFG temporaries

    {"include"},
    {"currentFile", "__FILE__"},
    {"merge"},

    // standard_method keywords
    {"standardMethod", "standard_method"},
    {"returns"},
    {"all"},
    {"any"},
    {"nilable"},
    {"untyped"},
    {"arrayOf", "array_of"},
    {"hashOf", "hash_of"},
    {"noreturn"},
    {"declareVariables", "declare_variables"},
    // end standard_method keywords

    // The next two names are used as keys in SymbolInfo::members to store
    // pointers up and down the singleton-class hierarchy. If A's singleton
    // class is B, then A will have a `singletonClass` entry in its members
    // table which references B, and B will have an `attachedClass` entry
    // pointing at A.
    //
    // The "attached class" terminology is borrowed from MRI, which refers
    // to the unique instance attached to a singleton class as the "attached
    // object"
    {"singletonClass", "<singleton class>"},
    {"attachedClass", "<attached class>"},

    {"blockTemp", "<block>"},
    {"new_", "new"},
    {"blockCall", "<block-call>"},

    // Used to generate temporary names for destructuring arguments ala proc do
    //  |(x,y)|; end
    {"destructureArg", "<destructure>"},
    {"lambda"},
    {"nil_p", "nil?"},
    {"super"},
    {"empty", ""},

    {"emptyHash", "{}"},
    {"buildHash", "<build-hash>"},
    {"arg0"},
};

void emit_name_header(ostream &out, NameDef &name) {
    out << "    // \"" << name.val << "\"" << endl;
    out << "    static inline constexpr NameRef " << name.srcName << "() {" << endl;
    out << "        return NameRef(" << name.id << ");" << endl;
    out << "    }" << endl;
    out << endl;
}

void emit_name_string(ostream &out, NameDef &name) {
    out << "const char *" << name.srcName << " = \"";
    out << ruby_typer::Strings::escapeCString(name.val) << "\";" << endl;

    out << "UTF8Desc " << name.srcName << "_DESC{(char*)";
    out << name.srcName << "," << name.val.size() << "};" << endl;
    out << endl;
}

void emit_register(ostream &out) {
    out << "void Names::registerNames(GlobalState &gs) {" << endl;
    for (auto &name : names) {
        out << "    NameRef " << name.srcName << "_id = gs.enterNameUTF8(" << name.srcName << "_DESC);" << endl;
    }
    out << endl;
    for (auto &name : names) {
        out << "    DEBUG_ONLY(Error::check(" << name.srcName << "_id == Names::" << name.srcName << "()));" << endl;
    }
    out << endl;
    out << "}" << endl;
}

int main(int argc, char **argv) {
    int i = 1;
    for (auto &name : names) {
        name.id = i++;
    }

    // emit headef file
    {
        ofstream header(argv[1], ios::trunc);
        if (!header.good()) {
            cerr << "unable to open " << argv[1] << endl;
            return 1;
        }

        header << "class Names final {" << endl;
        header << "    friend class GlobalState;" << endl;
        header << "    static void registerNames(GlobalState &gs);" << endl;
        header << "public:" << endl;

        for (auto &name : names) {
            emit_name_header(header, name);
        }

        header << "};" << endl;
    }

    {
        ofstream classfile(argv[2], ios::trunc);
        if (!classfile.good()) {
            cerr << "unable to open " << argv[2] << endl;
            return 1;
        }
        classfile << "#include \"core/GlobalState.h\"" << endl << endl;
        classfile << "namespace ruby_typer {" << endl;
        classfile << "namespace core {" << endl;
        classfile << "namespace {" << endl;
        for (auto &name : names) {
            emit_name_string(classfile, name);
        }
        classfile << "}" << endl;
        classfile << endl;

        emit_register(classfile);

        classfile << "}" << endl;
        classfile << "}" << endl;
    }
    return 0;
}
