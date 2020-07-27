#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

enum class FieldType {
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
        vector<FieldDef>({{"from", FieldType::Node}, {"to", FieldType::Node}}),
    },
    // logical and
    {
        "And",
        "and",
        vector<FieldDef>({{"left", FieldType::Node}, {"right", FieldType::Node}}),
    },
    // &&=
    {
        "AndAsgn",
        "and_asgn",
        vector<FieldDef>({{"left", FieldType::Node}, {"right", FieldType::Node}}),
    },
    // Required positional argument
    {
        "Arg",
        "arg",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // Wraps block arg, method arg, and send arg
    {
        "Args",
        "args",
        vector<FieldDef>({{"args", FieldType::NodeVec}}),
    },
    // inline array with elements
    {
        "Array",
        "array",
        vector<FieldDef>({{"elts", FieldType::NodeVec}}),
    },
    // Used for $`, $& etc magic regex globals
    {
        "Backref",
        "back_ref",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    {
        "Assign",
        "assign",
        vector<FieldDef>({{"lhs", FieldType::Node}, {"rhs", FieldType::Node}}),
    },
    // wraps any set of statements implicitly grouped by syntax (e.g. def, class bodies)
    {
        "Begin",
        "begin",
        vector<FieldDef>({{"stmts", FieldType::NodeVec}}),
    },
    // Node is always a send, which is previous call, args is arguments of body
    {
        "Block",
        "block",
        vector<FieldDef>({{"send", FieldType::Node}, {"args", FieldType::Node}, {"body", FieldType::Node}}),
    },
    // Wraps a `&foo` argument in an argument list
    {
        "Blockarg",
        "blockarg",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    //  e.g. map(&:token)
    {
        "BlockPass",
        "block_pass",
        vector<FieldDef>({{"block", FieldType::Node}}),
    },
    // `break` keyword
    {
        "Break",
        "break",
        vector<FieldDef>({{"exprs", FieldType::NodeVec}}),
    },
    // case statement; whens is a list of (when cond expr) nodes
    {
        "Case",
        "case",
        vector<FieldDef>({{"condition", FieldType::Node}, {"whens", FieldType::NodeVec}, {"else_", FieldType::Node}}),
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
        vector<FieldDef>({{"declLoc", FieldType::Loc},
                          {"name", FieldType::Node},
                          {"superclass", FieldType::Node},
                          {"body", FieldType::Node}}),
    },
    // complex number literal like "42i"
    {
        "Complex",
        "complex",
        vector<FieldDef>({{"value", FieldType::String}}),
    },
    // Used as path to Select, scope is Null for end of specified list
    {
        "Const",
        "const",
        vector<FieldDef>({{"scope", FieldType::Node}, {"name", FieldType::Name}}),
    },
    // Used inside a `Mlhs` if a constant is part of multiple assignment
    {
        "ConstLhs",
        "casgn",
        vector<FieldDef>({{"scope", FieldType::Node}, {"name", FieldType::Name}}),
    },
    // &. "conditional-send"/safe-navigation operator
    {
        "CSend",
        "csend",
        vector<FieldDef>({{"receiver", FieldType::Node}, {"method", FieldType::Name}, {"args", FieldType::NodeVec}}),
    },
    // @@foo class variable
    {
        "CVar",
        "cvar",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // @@foo class variable in the lhs of an Mlhs
    {
        "CVarLhs",
        "cvasgn",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // args may be NULL, body does not have to be a block.
    {
        "DefMethod",
        "def",
        vector<FieldDef>({{"declLoc", FieldType::Loc},
                          {"name", FieldType::Name},
                          {"args", FieldType::Node},
                          {"body", FieldType::Node}}),
    },
    // defined?() built-in pseudo-function
    {
        "Defined",
        "defined?",
        vector<FieldDef>({{"value", FieldType::Node}}),
    },
    // def <expr>.name singleton-class method def
    {
        "DefS",
        "defs",
        vector<FieldDef>({{"declLoc", FieldType::Loc},
                          {"singleton", FieldType::Node},
                          {"name", FieldType::Name},
                          {"args", FieldType::Node},
                          {"body", FieldType::Node}}),
    },
    // string with an interpolation, all nodes are concatenated in a single string
    {
        "DString",
        "dstr",
        vector<FieldDef>({{"nodes", FieldType::NodeVec}}),
    },
    // symbol with an interpolation, :"foo#{bar}"
    {
        "DSymbol",
        "dsym",
        vector<FieldDef>({{"nodes", FieldType::NodeVec}}),
    },
    // ... flip-flop operator inside a conditional
    {
        "EFlipflop",
        "eflipflop",
        vector<FieldDef>({{"left", FieldType::Node}, {"right", FieldType::Node}}),
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
        vector<FieldDef>({{"body", FieldType::Node}, {"ensure", FieldType::Node}}),
    },
    // Exclusive range 1...3
    {
        "ERange",
        "erange",
        vector<FieldDef>({{"from", FieldType::Node}, {"to", FieldType::Node}}),
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
        vector<FieldDef>({{"vars", FieldType::Node}, {"expr", FieldType::Node}, {"body", FieldType::Node}}),
    },
    // float literal like "1.2"
    {
        "Float",
        "float",
        vector<FieldDef>({{"val", FieldType::String}}),
    },
    // Global variable ($foo)
    {
        "GVar",
        "gvar",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // Global variable in the lhs of an mlhs
    {
        "GVarLhs",
        "gvasgn",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // Hash literal, entries are `Pair`s,
    {
        "Hash",
        "hash",
        vector<FieldDef>({{"pairs", FieldType::NodeVec}}),
    },
    // Bareword identifier (foo); should only exist transiently while parsing
    {
        "Ident",
        "UNUSED_ident",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    {
        "If",
        "if",
        vector<FieldDef>({{"condition", FieldType::Node}, {"then_", FieldType::Node}, {"else_", FieldType::Node}}),
    },
    // .. flip-flop operator inside a conditional
    {
        "IFlipflop",
        "iflipflop",
        vector<FieldDef>({{"left", FieldType::Node}, {"right", FieldType::Node}}),
    },
    // inclusive range. Subnodes need not be integers nor literals
    {
        "IRange",
        "irange",
        vector<FieldDef>({{"from", FieldType::Node}, {"to", FieldType::Node}}),
    },
    {
        "Integer",
        "int",
        vector<FieldDef>({{"val", FieldType::String}}),
    },
    // instance variable reference
    {
        "IVar",
        "ivar",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // @rules in `@rules, invalid_rules = ...`
    {
        "IVarLhs",
        "ivasgn",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // Required keyword argument inside an (args)
    {
        "Kwarg",
        "kwarg",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // explicit `begin` keyword.
    // `kwbegin` is emitted _only_ for post-while and post-until loops
    // because they act differently
    {
        "Kwbegin",
        "kwbegin",
        vector<FieldDef>({{"stmts", FieldType::NodeVec}}),
    },
    // optional keyword arg with default value provided
    {
        "Kwoptarg",
        "kwoptarg",
        vector<FieldDef>({{"name", FieldType::Name}, {"nameLoc", FieldType::Loc}, {"default_", FieldType::Node}}),
    },
    // **kwargs arg
    {
        "Kwrestarg",
        "kwrestarg",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // **foo splat
    {
        "Kwsplat",
        "kwsplat",
        vector<FieldDef>({{"expr", FieldType::Node}}),
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
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // invalid_rules in `@rules, invalid_rules = ...`
    {
        "LVarLhs",
        "lvasgn",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // [regex literal] =~ value; autovivifies local vars from match grops
    {
        "MatchAsgn",
        "match_with_lvasgn",
        vector<FieldDef>({{"regex", FieldType::Node}, {"expr", FieldType::Node}}),
    },
    // /foo/ regex literal inside an `if`; implicitly matches against $_
    {
        "MatchCurLine",
        "match_current_line",
        vector<FieldDef>({{"cond", FieldType::Node}}),
    },
    // multiple left hand sides: `@rules, invalid_rules = ...`
    {
        "Masgn",
        "masgn",
        vector<FieldDef>({{"lhs", FieldType::Node}, {"rhs", FieldType::Node}}),
    },
    // multiple left hand sides: `@rules, invalid_rules = ...`
    {
        "Mlhs",
        "mlhs",
        vector<FieldDef>({{"exprs", FieldType::NodeVec}}),
    },
    {
        "Module",
        "module",
        vector<FieldDef>({{"declLoc", FieldType::Loc}, {"name", FieldType::Node}, {"body", FieldType::Node}}),
    },
    // next(args); `next` is like `return` but for blocks
    {
        "Next",
        "next",
        vector<FieldDef>({{"exprs", FieldType::NodeVec}}),
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
        vector<FieldDef>({{"ref", FieldType::Uint}}),
    },
    // foo += 6 for += and other ops
    {
        "OpAsgn",
        "op_asgn",
        vector<FieldDef>({{"left", FieldType::Node}, {"op", FieldType::Name}, {"right", FieldType::Node}}),
    },
    // logical or
    {
        "Or",
        "or",
        vector<FieldDef>({{"left", FieldType::Node}, {"right", FieldType::Node}}),
    },
    // foo ||= bar
    {
        "OrAsgn",
        "or_asgn",
        vector<FieldDef>({{"left", FieldType::Node}, {"right", FieldType::Node}}),
    },
    // optional positional argument inside an (args) list
    {
        "Optarg",
        "optarg",
        vector<FieldDef>({{"name", FieldType::Name}, {"nameLoc", FieldType::Loc}, {"default_", FieldType::Node}}),
    },
    // entries of Hash
    {
        "Pair",
        "pair",
        vector<FieldDef>({{"key", FieldType::Node}, {"value", FieldType::Node}}),
    },
    // END {...}
    {
        "Postexe",
        "postexe",
        vector<FieldDef>({{"body", FieldType::Node}}),
    },
    // BEGIN{...}
    {
        "Preexe",
        "preexe",
        vector<FieldDef>({{"body", FieldType::Node}}),
    },
    // wraps the sole argument of a 1-arg block
    // because there's a diffence between m {|a|} and m{|a,|}
    {
        "Procarg0",
        "procarg0",
        vector<FieldDef>({{"arg", FieldType::Node}}),
    },
    // rational number literal like "42r"
    {
        "Rational",
        "rational",
        vector<FieldDef>({{"val", FieldType::String}}),
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
        vector<FieldDef>({{"regex", FieldType::NodeVec}, {"opts", FieldType::Node}}),
    },
    // opts of regexp
    {
        "Regopt",
        "regopt",
        vector<FieldDef>({{"opts", FieldType::String}}),
    },
    // body of a rescue
    {
        "Resbody",
        "resbody",
        vector<FieldDef>({{"exception", FieldType::Node}, {"var", FieldType::Node}, {"body", FieldType::Node}}),
    },
    // begin; ..; rescue; end; rescue is an array of Resbody
    {
        "Rescue",
        "rescue",
        vector<FieldDef>({{"body", FieldType::Node}, {"rescue", FieldType::NodeVec}, {"else_", FieldType::Node}}),
    },
    // *arg argument inside an (args)
    {
        "Restarg",
        "restarg",
        vector<FieldDef>({{"name", FieldType::Name}, {"nameLoc", FieldType::Loc}}),
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
        vector<FieldDef>({{"exprs", FieldType::NodeVec}}),
    },
    // class << expr; body; end;
    {
        "SClass",
        "sclass",
        vector<FieldDef>({{"declLoc", FieldType::Loc}, {"expr", FieldType::Node}, {"body", FieldType::Node}}),
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
        vector<FieldDef>({{"receiver", FieldType::Node}, {"method", FieldType::Name}, {"args", FieldType::NodeVec}}),
    },
    // m { |;shadowarg| }
    {
        "Shadowarg",
        "shadowarg",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // *foo splat operator
    {
        "Splat",
        "splat",
        vector<FieldDef>({{"var", FieldType::Node}}),
    },
    {
        "SplatLhs",
        "splat",
        vector<FieldDef>({{"var", FieldType::Node}}),
    },
    // string literal
    {
        "String",
        "str",
        vector<FieldDef>({{"val", FieldType::Name}}),
    },
    {
        "Super",
        "super",
        vector<FieldDef>({{"args", FieldType::NodeVec}}),
    },
    // symbol literal
    {
        "Symbol",
        "sym",
        vector<FieldDef>({{"val", FieldType::Name}}),
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
        vector<FieldDef>({{"exprs", FieldType::NodeVec}}),
    },
    {
        "Until",
        "until",
        vector<FieldDef>({{"cond", FieldType::Node}, {"body", FieldType::Node}}),
    },
    {
        "UntilPost",
        "until_post",
        vector<FieldDef>({{"cond", FieldType::Node}, {"body", FieldType::Node}}),
    },
    {
        "When",
        "when",
        vector<FieldDef>({{"patterns", FieldType::NodeVec}, {"body", FieldType::Node}}),
    },
    {
        "While",
        "while",
        vector<FieldDef>({{"cond", FieldType::Node}, {"body", FieldType::Node}}),
    },
    // There's a difference between while_post and while loops:
    // while_post runs the body of the loop at least once.
    {
        "WhilePost",
        "while_post",
        vector<FieldDef>({{"cond", FieldType::Node}, {"body", FieldType::Node}}),
    },
    {
        "XString",
        "xstr",
        vector<FieldDef>({{"nodes", FieldType::NodeVec}}),
    },
    {
        "Yield",
        "yield",
        vector<FieldDef>({{"exprs", FieldType::NodeVec}}),
    },
    {
        "ZSuper",
        "zsuper",
        vector<FieldDef>(),
    },
};

string fieldType(FieldType arg) {
    switch (arg) {
        case FieldType::Name:
            return "core::NameRef";
        case FieldType::Node:
            return "std::unique_ptr<Node>";
        case FieldType::NodeVec:
            return "NodeVec";
        case FieldType::String:
            return "const std::string";
        case FieldType::Uint:
            return "u4";
        case FieldType::Loc:
            return "core::LocOffsets";
    }
}

void emitNodeHeader(ostream &out, NodeDef &node) {
    out << "class " << node.name << " final : public Node {" << '\n';
    out << "public:" << '\n';

    // generate constructor
    out << "    " << node.name << "(core::LocOffsets loc";
    for (auto &arg : node.fields) {
        out << ", " << fieldType(arg.type) << " " << arg.name;
    }
    out << ")" << '\n';
    out << "        : Node(loc)";
    for (auto &arg : node.fields) {
        out << ", " << arg.name << "(";
        if (arg.type == FieldType::Node || arg.type == FieldType::NodeVec) {
            out << "std::move(";
        }
        out << arg.name;
        if (arg.type == FieldType::Node || arg.type == FieldType::NodeVec) {
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
        if (arg.type == FieldType::Loc) {
            continue;
        }
        out << "    printTabs(buf, tabs + 1);" << '\n';
        switch (arg.type) {
            case FieldType::Name:
                out << "    fmt::format_to(buf, \"" << arg.name << " = {}\\n\", " << arg.name
                    << ".data(gs)->showRaw(gs));" << '\n';
                break;
            case FieldType::Node:
                out << "    fmt::format_to(buf, \"" << arg.name << " = \");\n";
                out << "    printNode(buf, " << arg.name << ", gs, tabs + 1);\n";
                break;
            case FieldType::NodeVec:
                out << "    fmt::format_to(buf, \"" << arg.name << " = [\\n\");\n";
                out << "    for (auto &&a: " << arg.name << ") {\n";
                out << "      printTabs(buf, tabs + 2);\n";
                out << "      printNode(buf, a, gs, tabs + 2);\n";
                out << "    }" << '\n';
                out << "    printTabs(buf, tabs + 1);\n";
                out << "    fmt::format_to(buf, \"]\\n\");\n";
                break;
            case FieldType::String:
                out << "    fmt::format_to(buf, \"" << arg.name << " = \\\"{}\\\"\\n\", " << arg.name << ");\n";
                break;
            case FieldType::Uint:
                out << "    fmt::format_to(buf, \"" << arg.name << " = {}\\n\", " << arg.name << ");\n";
                break;
            case FieldType::Loc:
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
        if (arg.type == FieldType::Loc) {
            continue;
        }
        maybeComma = "";
        for (int j = i + 1; j < node.fields.size(); j++) {
            if (node.fields[j].type != FieldType::Loc) {
                maybeComma = ",";
                break;
            }
        }
        out << "    printTabs(buf, tabs + 1);" << '\n';
        switch (arg.type) {
            case FieldType::Name:
                out << "    fmt::format_to(buf,  \"\\\"" << arg.name << "\\\" : \\\"{}\\\"" << maybeComma
                    << "\\n\", JSON::escape(" << arg.name << ".data(gs)->show(gs)));\n";
                break;
            case FieldType::Node:
                out << "    fmt::format_to(buf,  \"\\\"" << arg.name << "\\\" : \");\n";
                out << "    printNodeJSON(buf, " << arg.name << ", gs, tabs + 1);\n";
                out << "    fmt::format_to(buf,  \"" << maybeComma << "\\n\");\n";
                break;
            case FieldType::NodeVec:
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
            case FieldType::String:
                out << "    fmt::format_to(buf,  \"\\\"" << arg.name << "\\\" : \\\"{}\\\"" << maybeComma << "\\n\", "
                    << arg.name << ");\n";
                break;
            case FieldType::Uint:
                out << R"(    fmt::format_to(buf,  "\")" << arg.name << R"(\" : \"{}\")" << maybeComma << "\\n\", "
                    << arg.name << ");\n";
                break;
            case FieldType::Loc:
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
            case FieldType::Name:
                if (node.whitequarkName == "str") {
                    out << "    fmt::format_to(buf, \", \\\"{}\\\"\", " << arg.name << ".toString(gs));\n";
                } else {
                    out << "    fmt::format_to(buf, \", :\" + JSON::escape(" << arg.name << ".data(gs)->show(gs)));\n";
                }
                break;
            case FieldType::Node:
                out << "    fmt::format_to(buf, \",\");\n";
                out << "    if (" << arg.name << ") {\n";
                out << "     fmt::format_to(buf, \"\\n\");\n";
                out << "     printTabs(buf, tabs + 1);" << '\n';
                out << "     printNodeWhitequark(buf, " << arg.name << ", gs, tabs + 1);\n";
                out << "    } else {\n";
                out << "      fmt::format_to(buf, \" nil\");\n";
                out << "    }\n";
                break;
            case FieldType::NodeVec:
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
            case FieldType::String:
                out << "    fmt::format_to(buf, \", \\\"{}\\\"\", " << arg.name << ");\n";
                break;
            case FieldType::Uint:
                out << "    fmt::format_to(buf, \", {}\", " << arg.name << ");\n";
                break;
            case FieldType::Loc:
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
