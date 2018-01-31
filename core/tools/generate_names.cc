#include "common/common.h"
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

enum Phase {
    Core = (1 << 0),
    Parser = (1 << 1),
    Desugar = (1 << 2),
    Namer = (1 << 3),
    Resolver = (1 << 4),
    CFG = (1 << 5),
    Infer = (1 << 6),
};

const map<string, int> phaseNames = {
    {"core", Core},         {"parser", Parser}, {"desugar", Desugar}, {"namer", Namer},
    {"resolver", Resolver}, {"cfg", CFG},       {"infer", Infer},
};

struct NameDef {
    int id;
    string srcName;
    string val;
    int phases;

    NameDef(const char *srcName, const char *val, int phases) : srcName(srcName), val(val), phases(phases) {
        if (strcmp(srcName, val) == 0) {
            ruby_typer::Error::raise("Only pass one arg for '", val, "'");
        }
    }
    NameDef(const char *srcName, int phases) : srcName(srcName), val(srcName), phases(phases) {}
};

NameDef names[] = {
    {"initialize", Infer},
    {"andAnd", "&&", Desugar},
    {"orOr", "||", Desugar},
    {"to_s", Desugar},
    {"to_a", Desugar},
    {"to_hash", Desugar},
    {"to_proc", Desugar},
    {"concat", Desugar},
    {"intern", Desugar},
    {"call", Desugar | Namer | Infer},
    {"bang", "!", Desugar | Infer | Parser},
    {"squareBrackets", "[]", Desugar | Parser | Infer | Resolver},
    {"squareBracketsEq", "[]=", Parser},
    {"unaryPlus", "@+", Parser | Namer},
    {"unaryMinus", "@-", Parser | Namer},
    {"star", "*", Parser},
    {"starStar", "**", Parser},
    {"ampersand", "&", Parser},
    {"tripleEq", "===", Desugar | Infer},
    {"orOp", "|", Desugar},
    {"backtick", "`", Desugar},
    {"slice", Desugar},
    {"defined_p", "defined?", Desugar},
    {"each", Desugar},

    // used in CFG for temporaries
    {"whileTemp", "<whileTemp>", CFG | Core},
    {"ifTemp", "<ifTemp>", CFG | Core},
    {"returnTemp", "<returnTemp>", CFG | Core},
    {"statTemp", "<statTemp>", CFG | Core},
    {"assignTemp", "<assignTemp", Desugar | Infer | Core},
    {"returnMethodTemp", "<returnMethodTemp>", CFG | Core},
    {"debugEnvironmentTemp", "<debugEnvironmentTemp>", CFG | Infer | Core},
    {"blockReturnTemp", "<blockReturnTemp>", CFG | Core},
    {"selfMethodTemp", "<selfMethodTemp>", CFG | Core},
    {"hashTemp", "<hashTemp>", CFG | Core},
    {"arrayTemp", "<arrayTemp>", CFG | Core},
    {"rescueTemp", "<rescueTemp", Desugar | CFG | Core},
    {"castTemp", "<castTemp", Resolver | CFG | Core},
    {"finalReturn", "<finalReturn>", CFG | Infer | Core},
    {"cfgAlias", "<cfgAlias>", CFG | Infer | Core},
    // end CFG temporaries

    {"include", Namer | Resolver},
    {"currentFile", "__FILE__", Desugar},
    {"merge", Desugar},

    // T keywords
    {"sig", Resolver},
    {"abstract", Resolver},
    {"implementation", Resolver},
    {"override_", "override", Resolver},
    {"overridable", Resolver},

    {"returns", Resolver},
    {"checked", Resolver},
    {"all", Resolver | Infer | Core},
    {"any", Resolver | Infer | Core},
    {"enum_", "enum", Resolver},
    {"nilable", Resolver | Desugar | Infer | Core},
    {"untyped", Resolver | Infer | Core},
    {"Array", Infer},
    {"Hash", Infer},
    {"arrayOf", "array_of", Resolver},
    {"Elem", Resolver},  // Array Type parameter name
    {"Key", Resolver},   // Hash Type parameter name
    {"Value", Resolver}, // Hash Type parameter name
    {"hashOf", "hash_of", Resolver},
    {"noreturn", Resolver},
    {"declareVariables", "declare_variables", Resolver},

    {"assertType", "assert_type!", Resolver},
    {"cast", Resolver},
    // end T keywords

    // Ruby DSL methods which we understand
    {"attr", Resolver},
    {"attrAccessor", "attr_accessor", Resolver},
    {"attrWriter", "attr_writer", Resolver},
    {"attrReader", "attr_reader", Resolver},
    {"private_", "private", Namer | Resolver},
    {"protected_", "protected", Namer | Resolver},
    {"public_", "public", Namer | Resolver},
    {"privateClassMethod", "private_class_method", Namer | Resolver},
    {"moduleFunction", "module_function", Namer | Resolver},
    {"aliasMethod", "alias_method", Desugar | Namer | Resolver},
    {"typeDecl", "type", Namer | Resolver},
    {"T", Namer},
    {"covariant", "out", Namer},
    {"contravariant", "in", Namer},
    {"invariant", "<invariant>", Namer},
    {"fixed", Namer | Resolver},
    // end DSL methods

    // Our own special methods which have special meaning
    {"hardAssert", "hard_assert", Infer}, // Kernel.hard_assert
    // end special methods

    // The next two names are used as keys in SymbolInfo::members to store
    // pointers up and down the singleton-class hierarchy. If A's singleton
    // class is B, then A will have a `singletonClass` entry in its membe|CFG|Infer|Corers
    // table which references B, and B will have an `attachedClass` ent|Corery
    // pointing at A.
    //
    // The "attached class" terminology is borrowed from MRI, which refers
    // to the unique instance attached to a singleton class as the "attached
    // object"
    {"singletonClass", "<singleton class>", Core | Desugar | Namer},
    {"attachedClass", "<attached class>", Core},

    {"blockTemp", "<block>", Core | Namer | Resolver},
    {"blockPassTemp", "<block-pass>", Desugar},
    {"forTemp", Desugar},
    {"new_", "new", Desugar | Infer},
    {"blockCall", "<block-call>", CFG | Infer | Core},
    {"blkArg", "<blk>", Namer},

    // Used to generate temporary names for destructuring arguments ala proc do
    //  |(x,y)|; end
    {"destructureArg", "<destructure>", Desugar},

    {"lambda", Parser},
    {"nil_p", "nil?", Desugar | Infer},
    {"nil", Infer},
    {"super", Desugar | Infer},
    {"empty", "", Desugar},

    {"buildHash", "<build-hash>", CFG | Core},
    {"buildArray", "<build-array>", CFG | Core},
    {"splat", "<splat>", Desugar | Core | Resolver},
    {"arg0", Core | Resolver},

    {"is_a_p", "is_a?", Infer | CFG},
    {"kind_of", "kind_of?", Infer},
    {"eqeq", "==", Infer},

    {"staticInit", "<static-init>", Resolver},
};

void emit_name_header(ostream &out, NameDef &name) {
    out << "#ifndef NAME_" << name.srcName << endl;
    out << "#define NAME_" << name.srcName << endl;
    out << "    // \"" << name.val << "\"" << endl;
    out << "    static inline constexpr NameRef " << name.srcName << "() {" << endl;
    out << "        return NameRef(NameRef::WellKnown{}, " << name.id << ");" << endl;
    out << "    }" << endl;
    out << "#endif" << endl;
    out << endl;
}

void emit_name_string(ostream &out, NameDef &name) {
    out << "const char *" << name.srcName << " = \"";
    out << ruby_typer::Strings::escapeCString(name.val) << "\";" << endl;

    out << "absl::string_view " << name.srcName << "_DESC{(char*)";
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
        out << "    ENFORCE(" << name.srcName << "_id._id == " << name.id << "); /* Names::" << name.srcName << "() */"
            << endl;
    }
    out << endl;
    out << "}" << endl;
}

int main(int argc, char **argv) {
    int i = 1;
    for (auto &name : names) {
        name.id = i++;
    }
    int lastId = i - 1;

    // emit header file
    {
        ofstream header(argv[1], ios::trunc);
        if (!header.good()) {
            cerr << "unable to open " << argv[1] << endl;
            return 1;
        }

        header << "namespace Names {" << endl;
        header << "    void registerNames(GlobalState &gs);" << endl;
        header << "};" << endl;
    }

    // emit initialization .cc file
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

    // Emit per-phase name definitions
    for (int i = 3; i < argc; i++) {
        string arg = argv[i];
        auto eq = arg.find('=');
        if (arg.substr(0, 2) != "--" || eq == string::npos) {
            cerr << "bad arg: " << arg << endl;
            return 1;
        }
        auto phase = arg.substr(2, eq - 2);
        auto path = arg.substr(eq + 1);
        auto it = phaseNames.find(phase);
        if (it == phaseNames.end()) {
            cerr << "unknown phase: " << phase << endl;
            return 1;
        }

        ofstream header(path, ios::trunc);
        if (!header.good()) {
            cerr << "unable to open " << path << endl;
            return 1;
        }

        header << "#include \"core/Names.h\"" << endl << endl;
        header << "namespace ruby_typer {" << endl;
        header << "namespace core {" << endl;
        header << "namespace Names {" << endl;

        for (auto &name : names) {
            if ((name.phases & it->second) != 0) {
                emit_name_header(header, name);
            }
        }

        if (phase == "core") {
            header << "#ifndef NAME_LAST_WELL_KNOWN_NAME" << endl;
            header << "#define NAME_LAST_WELL_KNOWN_NAME" << endl;
            header << "constexpr int LAST_WELL_KNOWN_NAME = " << lastId << ";" << endl;
            header << "#endif" << endl;
        }

        header << "};" << endl;
        header << "};" << endl;
        header << "};" << endl;
    }
    return 0;
}
