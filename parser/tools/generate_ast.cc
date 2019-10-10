#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

enum FieldType {
    Name,
    Node,
    NodeVec,
    String,
    Uint,
    Loc,
};

struct FieldDef {
    string name;
    FieldType type;
};

struct NodeDef {
    string name;
    string whitequarkName;
    vector<FieldDef> fields;
};

NodeDef nodes[] = {
    // alias bar foo
    {
        "Alias",
        "alias",
        vector<FieldDef>({{"from", Node}, {"to", Node}}),
    },
    // logical and
    {
        "And",
        "and",
        vector<FieldDef>({{"left", Node}, {"right", Node}}),
    },
    // &&=
    {
        "AndAsgn",
        "and_asgn",
        vector<FieldDef>({{"left", Node}, {"right", Node}}),
    },
    // Required positional argument
    {
        "Arg",
        "arg",
        vector<FieldDef>({{"name", Name}}),
    },
    // Wraps block arg, method arg, and send arg
    {
        "Args",
        "args",
        vector<FieldDef>({{"args", NodeVec}}),
    },
    // inline array with elements
    {
        "Array",
        "array",
        vector<FieldDef>({{"elts", NodeVec}}),
    },
    // Used for $`, $& etc magic regex globals
    {
        "Backref",
        "back_ref",
        vector<FieldDef>({{"name", Name}}),
    },
    {
        "Assign",
        "assign",
        vector<FieldDef>({{"lhs", Node}, {"rhs", Node}}),
    },
    // wraps any set of statements implicitly grouped by syntax (e.g. def, class bodies)
    {
        "Begin",
        "begin",
        vector<FieldDef>({{"stmts", NodeVec}}),
    },
    // Node is always a send, which is previous call, args is arguments of body
    {
        "Block",
        "block",
        vector<FieldDef>({{"send", Node}, {"args", Node}, {"body", Node}}),
    },
    // Wraps a `&foo` argument in an argument list
    {
        "Blockarg",
        "blockarg",
        vector<FieldDef>({{"name", Name}}),
    },
    //  e.g. map(&:token)
    {
        "BlockPass",
        "block_pass",
        vector<FieldDef>({{"block", Node}}),
    },
    // `break` keyword
    {
        "Break",
        "break",
        vector<FieldDef>({{"exprs", NodeVec}}),
    },
    // case statement; whens is a list of (when cond expr) nodes
    {
        "Case",
        "case",
        vector<FieldDef>({{"condition", Node}, {"whens", NodeVec}, {"else_", Node}}),
    },
    // appears in the `scope` of a `::Constant` `Const` node
    {
        "Cbase",
        "cbase",
        vector<FieldDef>(),
    },
    // superclass is Null if empty superclass, body is Begin if multiple statements
    {
        "Class",
        "class",
        vector<FieldDef>({{"declLoc", Loc}, {"name", Node}, {"superclass", Node}, {"body", Node}}),
    },
    // complex number literal like "42i"
    {
        "Complex",
        "complex",
        vector<FieldDef>({{"value", String}}),
    },
    // Used as path to Select, scope is Null for end of specified list
    {
        "Const",
        "const",
        vector<FieldDef>({{"scope", Node}, {"name", Name}}),
    },
    // Used inside a `Mlhs` if a constant is part of multiple assignment
    {
        "ConstLhs",
        "casgn",
        vector<FieldDef>({{"scope", Node}, {"name", Name}}),
    },
    // &. "conditional-send"/safe-navigation operator
    {
        "CSend",
        "csend",
        vector<FieldDef>({{"receiver", Node}, {"method", Name}, {"args", NodeVec}}),
    },
    // @@foo class variable
    {
        "CVar",
        "cvar",
        vector<FieldDef>({{"name", Name}}),
    },
    // @@foo class variable in the lhs of an Mlhs
    {
        "CVarLhs",
        "cvasgn",
        vector<FieldDef>({{"name", Name}}),
    },
    // args may be NULL, body does not have to be a block.
    {
        "DefMethod",
        "def",
        vector<FieldDef>({{"declLoc", Loc}, {"name", Name}, {"args", Node}, {"body", Node}}),
    },
    // defined?() built-in pseudo-function
    {
        "Defined",
        "defined?",
        vector<FieldDef>({{"value", Node}}),
    },
    // def <expr>.name singleton-class method def
    {
        "DefS",
        "defs",
        vector<FieldDef>({{"declLoc", Loc}, {"singleton", Node}, {"name", Name}, {"args", Node}, {"body", Node}}),
    },
    // string with an interpolation, all nodes are concatenated in a single string
    {
        "DString",
        "dstr",
        vector<FieldDef>({{"nodes", NodeVec}}),
    },
    // symbol with an interpolation, :"foo#{bar}"
    {
        "DSymbol",
        "dsym",
        vector<FieldDef>({{"nodes", NodeVec}}),
    },
    // ... flip-flop operator inside a conditional
    {
        "EFlipflop",
        "eflipflop",
        vector<FieldDef>({{"left", Node}, {"right", Node}}),
    },
    // __ENCODING__
    {
        "EncodingLiteral",
        "__ENCODING__",
        vector<FieldDef>(),
    },
    // "ensure" keyword
    {
        "Ensure",
        "ensure",
        vector<FieldDef>({{"body", Node}, {"ensure", Node}}),
    },
    // Exclusive range 1...3
    {
        "ERange",
        "erange",
        vector<FieldDef>({{"from", Node}, {"to", Node}}),
    },
    // "false" keyword
    {
        "False",
        "false",
        vector<FieldDef>(),
    },
    // __FILE__
    {
        "FileLiteral",
        "__FILE__",
        vector<FieldDef>(),
    },
    // For loop
    {
        "For",
        "for",
        vector<FieldDef>({{"vars", Node}, {"expr", Node}, {"body", Node}}),
    },
    // float literal like "1.2"
    {
        "Float",
        "float",
        vector<FieldDef>({{"val", String}}),
    },
    // Global variable ($foo)
    {
        "GVar",
        "gvar",
        vector<FieldDef>({{"name", Name}}),
    },
    // Global variable in the lhs of an mlhs
    {
        "GVarLhs",
        "gvasgn",
        vector<FieldDef>({{"name", Name}}),
    },
    // Hash literal, entries are `Pair`s,
    {
        "Hash",
        "hash",
        vector<FieldDef>({{"pairs", NodeVec}}),
    },
    // Bareword identifier (foo); should only exist transiently while parsing
    {
        "Ident",
        "UNUSED_ident",
        vector<FieldDef>({{"name", Name}}),
    },
    {
        "If",
        "if",
        vector<FieldDef>({{"condition", Node}, {"then_", Node}, {"else_", Node}}),
    },
    // .. flip-flop operator inside a conditional
    {
        "IFlipflop",
        "iflipflop",
        vector<FieldDef>({{"left", Node}, {"right", Node}}),
    },
    // inclusive range. Subnodes need not be integers nor literals
    {
        "IRange",
        "irange",
        vector<FieldDef>({{"from", Node}, {"to", Node}}),
    },
    {
        "Integer",
        "int",
        vector<FieldDef>({{"val", String}}),
    },
    // instance variable reference
    {
        "IVar",
        "ivar",
        vector<FieldDef>({{"name", Name}}),
    },
    // @rules in `@rules, invalid_rules = ...`
    {
        "IVarLhs",
        "ivasgn",
        vector<FieldDef>({{"name", Name}}),
    },
    // Required keyword argument inside an (args)
    {
        "Kwarg",
        "kwarg",
        vector<FieldDef>({{"name", Name}}),
    },
    // explicit `begin` keyword.
    // `kwbegin` is emitted _only_ for post-while and post-until loops
    // because they act differently
    {
        "Kwbegin",
        "kwbegin",
        vector<FieldDef>({{"stmts", NodeVec}}),
    },
    // optional keyword arg with default value provided
    {
        "Kwoptarg",
        "kwoptarg",
        vector<FieldDef>({{"name", Name}, {"nameLoc", Loc}, {"default_", Node}}),
    },
    // **kwargs arg
    {
        "Kwrestarg",
        "kwrestarg",
        vector<FieldDef>({{"name", Name}}),
    },
    // **foo splat
    {
        "Kwsplat",
        "kwsplat",
        vector<FieldDef>({{"expr", Node}}),
    },
    {
        "LineLiteral",
        "__LINE__",
        vector<FieldDef>(),
    },
    // local variable referense
    {
        "LVar",
        "lvar",
        vector<FieldDef>({{"name", Name}}),
    },
    // invalid_rules in `@rules, invalid_rules = ...`
    {
        "LVarLhs",
        "lvasgn",
        vector<FieldDef>({{"name", Name}}),
    },
    // [regex literal] =~ value; autovivifies local vars from match grops
    {
        "MatchAsgn",
        "match_with_lvasgn",
        vector<FieldDef>({{"regex", Node}, {"expr", Node}}),
    },
    // /foo/ regex literal inside an `if`; implicitly matches against $_
    {
        "MatchCurLine",
        "match_current_line",
        vector<FieldDef>({{"cond", Node}}),
    },
    // multiple left hand sides: `@rules, invalid_rules = ...`
    {
        "Masgn",
        "masgn",
        vector<FieldDef>({{"lhs", Node}, {"rhs", Node}}),
    },
    // multiple left hand sides: `@rules, invalid_rules = ...`
    {
        "Mlhs",
        "mlhs",
        vector<FieldDef>({{"exprs", NodeVec}}),
    },
    {
        "Module",
        "module",
        vector<FieldDef>({{"declLoc", Loc}, {"name", Node}, {"body", Node}}),
    },
    // next(args); `next` is like `return` but for blocks
    {
        "Next",
        "next",
        vector<FieldDef>({{"exprs", NodeVec}}),
    },
    {
        "Nil",
        "nil",
        vector<FieldDef>(),
    },
    // $1, $2, etc
    {
        "NthRef",
        "nth_ref",
        vector<FieldDef>({{"ref", Uint}}),
    },
    // foo += 6 for += and other ops
    {
        "OpAsgn",
        "op_asgn",
        vector<FieldDef>({{"left", Node}, {"op", Name}, {"right", Node}}),
    },
    // logical or
    {
        "Or",
        "or",
        vector<FieldDef>({{"left", Node}, {"right", Node}}),
    },
    // foo ||= bar
    {
        "OrAsgn",
        "or_asgn",
        vector<FieldDef>({{"left", Node}, {"right", Node}}),
    },
    // optional positional argument inside an (args) list
    {
        "Optarg",
        "optarg",
        vector<FieldDef>({{"name", Name}, {"nameLoc", Loc}, {"default_", Node}}),
    },
    // entries of Hash
    {
        "Pair",
        "pair",
        vector<FieldDef>({{"key", Node}, {"value", Node}}),
    },
    // END {...}
    {
        "Postexe",
        "postexe",
        vector<FieldDef>({{"body", Node}}),
    },
    // BEGIN{...}
    {
        "Preexe",
        "preexe",
        vector<FieldDef>({{"body", Node}}),
    },
    // wraps the sole argument of a 1-arg block
    // because there's a diffence between m {|a|} and m{|a,|}
    {
        "Procarg0",
        "procarg0",
        vector<FieldDef>({{"arg", Node}}),
    },
    // rational number literal like "42r"
    {
        "Rational",
        "rational",
        vector<FieldDef>({{"val", String}}),
    },
    // `redo` keyword
    {
        "Redo",
        "redo",
        vector<FieldDef>(),
    },
    // regular expression; string interpolation in body is flattened into the array
    {
        "Regexp",
        "regexp",
        vector<FieldDef>({{"regex", NodeVec}, {"opts", Node}}),
    },
    // opts of regexp
    {
        "Regopt",
        "regopt",
        vector<FieldDef>({{"opts", String}}),
    },
    // body of a rescue
    {
        "Resbody",
        "resbody",
        vector<FieldDef>({{"exception", Node}, {"var", Node}, {"body", Node}}),
    },
    // begin; ..; rescue; end; rescue is an array of Resbody
    {
        "Rescue",
        "rescue",
        vector<FieldDef>({{"body", Node}, {"rescue", NodeVec}, {"else_", Node}}),
    },
    // *arg argument inside an (args)
    {
        "Restarg",
        "restarg",
        vector<FieldDef>({{"name", Name}, {"nameLoc", Loc}}),
    },
    // `retry` keyword
    {
        "Retry",
        "retry",
        vector<FieldDef>(),
    },
    // `return` keyword
    {
        "Return",
        "return",
        vector<FieldDef>({{"exprs", NodeVec}}),
    },
    // class << expr; body; end;
    {
        "SClass",
        "sclass",
        vector<FieldDef>({{"declLoc", Loc}, {"expr", Node}, {"body", Node}}),
    },
    {
        "Self",
        "self",
        vector<FieldDef>(),
    },
    // invocation
    {
        "Send",
        "send",
        vector<FieldDef>({{"receiver", Node}, {"method", Name}, {"args", NodeVec}}),
    },
    // m { |;shadowarg| }
    {
        "Shadowarg",
        "shadowarg",
        vector<FieldDef>({{"name", Name}}),
    },
    // *foo splat operator
    {
        "Splat",
        "splat",
        vector<FieldDef>({{"var", Node}}),
    },
    {
        "SplatLhs",
        "splat",
        vector<FieldDef>({{"var", Node}}),
    },
    // string literal
    {
        "String",
        "str",
        vector<FieldDef>({{"val", Name}}),
    },
    {
        "Super",
        "super",
        vector<FieldDef>({{"args", NodeVec}}),
    },
    // symbol literal
    {
        "Symbol",
        "sym",
        vector<FieldDef>({{"val", Name}}),
    },
    {
        "True",
        "true",
        vector<FieldDef>(),
    },
    // "undef" keyword
    {
        "Undef",
        "undef",
        vector<FieldDef>({{"exprs", NodeVec}}),
    },
    {
        "Until",
        "until",
        vector<FieldDef>({{"cond", Node}, {"body", Node}}),
    },
    {
        "UntilPost",
        "until_post",
        vector<FieldDef>({{"cond", Node}, {"body", Node}}),
    },
    {
        "When",
        "when",
        vector<FieldDef>({{"patterns", NodeVec}, {"body", Node}}),
    },
    {
        "While",
        "while",
        vector<FieldDef>({{"cond", Node}, {"body", Node}}),
    },
    // There's a difference between while_post and while loops:
    // while_post runs the body of the loop at least once.
    {
        "WhilePost",
        "while_post",
        vector<FieldDef>({{"cond", Node}, {"body", Node}}),
    },
    {
        "XString",
        "xstr",
        vector<FieldDef>({{"nodes", NodeVec}}),
    },
    {
        "Yield",
        "yield",
        vector<FieldDef>({{"exprs", NodeVec}}),
    },
    {
        "ZSuper",
        "zsuper",
        vector<FieldDef>(),
    },
};

string fieldType(FieldType arg) {
    switch (arg) {
        case Name:
            return "core::NameRef";
        case Node:
            return "std::unique_ptr<Node>";
        case NodeVec:
            return "NodeVec";
        case String:
            return "const std::string";
        case Uint:
            return "u4";
        case Loc:
            return "core::Loc";
    }
}

void emitNodeHeader(ostream &out, NodeDef &node) {
    out << "class " << node.name << " final : public Node {" << '\n';
    out << "public:" << '\n';

    // generate constructor
    out << "    " << node.name << "(core::Loc loc";
    for (auto &arg : node.fields) {
        out << ", " << fieldType(arg.type) << " " << arg.name;
    }
    out << ")" << '\n';
    out << "        : Node(loc)";
    for (auto &arg : node.fields) {
        out << ", " << arg.name << "(";
        if (arg.type == Node || arg.type == NodeVec) {
            out << "std::move(";
        }
        out << arg.name;
        if (arg.type == Node || arg.type == NodeVec) {
            out << ")";
        }
        out << ")";
    }
    out << '\n';
    out << "{";
    out << R"(    categoryCounterInc("nodes", ")" << node.name << "\");" << '\n';
    out << "}" << '\n';
    out << '\n';

    // Generate fields
    for (auto &arg : node.fields) {
        out << "    " << fieldType(arg.type) << " " << arg.name << ";" << '\n';
    }
    out << '\n';
    out << "  virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;" << '\n';
    out << "  virtual std::string toJSON(const core::GlobalState &gs, int tabs = 0);" << '\n';
    out << "  virtual std::string toWhitequark(const core::GlobalState &gs, int tabs = 0);" << '\n';
    out << "  virtual std::string nodeName();" << '\n';

    out << "};" << '\n';
    out << '\n';
}

void emitNodeClassfile(ostream &out, NodeDef &node) {
    out << "  std::string " << node.name << "::nodeName() {" << '\n';
    out << "    return \"" << node.name << "\";" << '\n';
    out << "  };" << '\n' << '\n';

    out << "  std::string " << node.name << "::toStringWithTabs(const core::GlobalState &gs, int tabs) const {" << '\n'
        << "    fmt::memory_buffer buf;" << '\n';
    out << "    fmt::format_to(buf, \"" << node.name << " {{\\n\");" << '\n';
    // Generate fields
    for (auto &arg : node.fields) {
        if (arg.type == Loc) {
            continue;
        }
        out << "    printTabs(buf, tabs + 1);" << '\n';
        switch (arg.type) {
            case Name:
                out << "    fmt::format_to(buf, \"" << arg.name << " = {}\\n\", " << arg.name
                    << ".data(gs)->showRaw(gs));" << '\n';
                break;
            case Node:
                out << "    fmt::format_to(buf, \"" << arg.name << " = \");\n";
                out << "    printNode(buf, " << arg.name << ", gs, tabs + 1);\n";
                break;
            case NodeVec:
                out << "    fmt::format_to(buf, \"" << arg.name << " = [\\n\");\n";
                out << "    for (auto &&a: " << arg.name << ") {\n";
                out << "      printTabs(buf, tabs + 2);\n";
                out << "      printNode(buf, a, gs, tabs + 2);\n";
                out << "    }" << '\n';
                out << "    printTabs(buf, tabs + 1);\n";
                out << "    fmt::format_to(buf, \"]\\n\");\n";
                break;
            case String:
                out << "    fmt::format_to(buf, \"" << arg.name << " = \\\"{}\\\"\\n\", " << arg.name << ");\n";
                break;
            case Uint:
                out << "    fmt::format_to(buf, \"" << arg.name << " = {}\\n\", " << arg.name << ");\n";
                break;
            case Loc:
                // Placate the compiler; we skip these
                abort();
                break;
        }
    }
    out << "    printTabs(buf, tabs);\n";
    out << "    fmt::format_to(buf, \"}}\");\n";
    out << "    return to_string(buf);\n";
    out << "  }" << '\n';
    out << '\n';

    // toJSON
    out << "  std::string " << node.name << "::toJSON(const core::GlobalState &gs, int tabs) {" << '\n'
        << "    fmt::memory_buffer buf;" << '\n';
    out << "    fmt::format_to(buf,  \"{{\\n\");\n";
    out << "    printTabs(buf, tabs + 1);" << '\n';
    auto maybeComma = "";
    if (!node.fields.empty()) {
        maybeComma = ",";
    }
    out << R"(    fmt::format_to(buf,  "\"type\" : \")" << node.name << "\\\"" << maybeComma << "\\n\");\n";
    int i = -1;
    // Generate fields
    for (auto &arg : node.fields) {
        i++;
        if (arg.type == Loc) {
            continue;
        }
        maybeComma = "";
        for (int j = i + 1; j < node.fields.size(); j++) {
            if (node.fields[j].type != Loc) {
                maybeComma = ",";
                break;
            }
        }
        out << "    printTabs(buf, tabs + 1);" << '\n';
        switch (arg.type) {
            case Name:
                out << "    fmt::format_to(buf,  \"\\\"" << arg.name << "\\\" : \\\"{}\\\"" << maybeComma
                    << "\\n\", JSON::escape(" << arg.name << ".data(gs)->show(gs)));\n";
                break;
            case Node:
                out << "    fmt::format_to(buf,  \"\\\"" << arg.name << "\\\" : \");\n";
                out << "    printNodeJSON(buf, " << arg.name << ", gs, tabs + 1);\n";
                out << "    fmt::format_to(buf,  \"" << maybeComma << "\\n\");\n";
                break;
            case NodeVec:
                out << "    fmt::format_to(buf,  \"\\\"" << arg.name << "\\\" : [\\n\");\n";
                out << "    int i = -1;" << '\n';
                out << "    for (auto &&a: " << arg.name << ") { \n";
                out << "      i++;\n";
                out << "      printTabs(buf, tabs + 2);\n";
                out << "      printNodeJSON(buf, a, gs, tabs + 2);\n";
                out << "      if (i + 1 < " << arg.name << ".size()) {\n";
                out << "        fmt::format_to(buf,  \",\");" << '\n';
                out << "      }" << '\n';
                out << "      fmt::format_to(buf,  \"\\n\");\n";
                out << "    }" << '\n';
                out << "    printTabs(buf, tabs + 1);\n";
                out << "    fmt::format_to(buf,  \"]" << maybeComma << "\\n\")\n;";
                break;
            case String:
                out << "    fmt::format_to(buf,  \"\\\"" << arg.name << "\\\" : \\\"{}\\\"" << maybeComma << "\\n\", "
                    << arg.name << ");\n";
                break;
            case Uint:
                out << R"(    fmt::format_to(buf,  "\")" << arg.name << R"(\" : \"{}\")" << maybeComma << "\\n\", "
                    << arg.name << ");\n";
                break;
            case Loc:
                // quiet the compiler; we skip Loc fields above
                abort();
                break;
        }
    }
    out << "    printTabs(buf, tabs);" << '\n';
    out << "    fmt::format_to(buf,  \"}}\");\n";
    out << "    return to_string(buf);\n";
    out << "  }" << '\n';
    out << '\n';

    // toWhitequark
    out << "  std::string " << node.name << "::toWhitequark(const core::GlobalState &gs, int tabs) {" << '\n'
        << "    fmt::memory_buffer buf;" << '\n';
    out << "    fmt::format_to(buf, \"s(:" << node.whitequarkName << "\");" << '\n';
    // Generate fields
    for (auto &arg : node.fields) {
        switch (arg.type) {
            case Name:
                if (node.whitequarkName == "str") {
                    out << "    fmt::format_to(buf, \", \\\"{}\\\"\", " << arg.name << ".toString(gs));\n";
                } else {
                    out << "    fmt::format_to(buf, \", :\" + JSON::escape(" << arg.name << ".data(gs)->show(gs)));\n";
                }
                break;
            case Node:
                out << "    fmt::format_to(buf, \",\");\n";
                out << "    if (" << arg.name << ") {\n";
                out << "     fmt::format_to(buf, \"\\n\");\n";
                out << "     printTabs(buf, tabs + 1);" << '\n';
                out << "     printNodeWhitequark(buf, " << arg.name << ", gs, tabs + 1);\n";
                out << "    } else {\n";
                out << "      fmt::format_to(buf, \" nil\");\n";
                out << "    }\n";
                break;
            case NodeVec:
                out << "    for (auto &&a: " << arg.name << ") {\n";
                out << "      fmt::format_to(buf, \",\");\n";
                out << "      if (a) {\n";
                out << "        fmt::format_to(buf, \"\\n\");\n";
                out << "        printTabs(buf, tabs + 1);" << '\n';
                out << "        printNodeWhitequark(buf, a, gs, tabs + 1);\n";
                out << "      } else {\n";
                out << "        fmt::format_to(buf, \" nil\");\n";
                out << "      }\n";
                out << "    }\n";
                break;
            case String:
                out << "    fmt::format_to(buf, \", \\\"{}\\\"\", " << arg.name << ");\n";
                break;
            case Uint:
                out << "    fmt::format_to(buf, \", {}\", " << arg.name << ");\n";
                break;
            case Loc:
                continue;
        }
    }

    out << "    fmt::format_to(buf, \")\");";
    out << "    return to_string(buf);\n"
        << "  }\n";
    out << '\n';
}

int main(int argc, char **argv) {
    // emit headef file
    {
        ofstream header(argv[1], ios::trunc);
        if (!header.good()) {
            cerr << "unable to open " << argv[1] << '\n';
            return 1;
        }
        for (auto &node : nodes) {
            emitNodeHeader(header, node);
        }
    }

    {
        ofstream classfile(argv[2], ios::trunc);
        if (!classfile.good()) {
            cerr << "unable to open " << argv[2] << '\n';
            return 1;
        }
        classfile << "#include \"parser/Node.h\"" << '\n' << '\n';
        classfile << "#include \"common/JSON.h\"" << '\n' << '\n';
        classfile << "namespace sorbet {" << '\n';
        classfile << "namespace parser {" << '\n';
        for (auto &node : nodes) {
            emitNodeClassfile(classfile, node);
        }
        classfile << "}" << '\n';
        classfile << "}" << '\n';
    }
    return 0;
}
