#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

enum FieldType {
    Name,
    Node,
    NodeVec,
    String,
    Uint,
};

struct FieldDef {
    string name;
    FieldType type;
};

struct NodeDef {
    string name;
    vector<FieldDef> fields;
};

NodeDef nodes[] = {
    // alias bar foo
    {"Alias", vector<FieldDef>({{"from", Node}, {"to", Node}})},
    // logical and
    {"And", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    // &&=
    {"AndAsgn", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    // Wraps every single definition of argument for methods and blocks
    {"Arg", vector<FieldDef>({{"name", Name}})},
    // Wraps block arg, method arg, and send arg
    {"Args", vector<FieldDef>({{"args", NodeVec}})},
    // inline array with elements
    {"Array", vector<FieldDef>({{"elts", NodeVec}})},
    // Used for $`, $& etc magic regex globals
    {"Backref", vector<FieldDef>({{"name", Name}})},
    {"Assign", vector<FieldDef>({{"lhs", Node}, {"rhs", Node}})},
    // wraps any set of statements implicitly grouped by syntax (e.g. def, class bodies)
    {"Begin", vector<FieldDef>({{"stmts", NodeVec}})},
    // Node is always a send, which is previous call, args is arguments of body
    {"Block", vector<FieldDef>({{"send", Node}, {"args", Node}, {"body", Node}})},
    // Wraps a `&foo` argument in an argument list
    {"Blockarg", vector<FieldDef>({{"name", Name}})},
    //  e.g. map(&:token)
    {"BlockPass", vector<FieldDef>({{"block", Node}})},
    // `break` keyword
    {"Break", vector<FieldDef>({{"exprs", NodeVec}})},
    // case statement; whens is a list of (when cond expr) nodes
    {"Case", vector<FieldDef>({{"condition", Node}, {"whens", NodeVec}, {"else_", Node}})},
    // appears in the `scope` of a `::Constant` `Const` node
    {"Cbase", vector<FieldDef>()},
    // superclass is Null if empty superclass, body is Begin if multiple statements
    {"Class", vector<FieldDef>({{"name", Node}, {"superclass", Node}, {"body", Node}})},
    {"Complex", vector<FieldDef>({{"value", String}})},
    // Used as path to Select, scope is Null for end of specified list
    {"Const", vector<FieldDef>({{"scope", Node}, {"name", Name}})},
    // Used inside a `Mlhs` if a constant is part of multiple assignment
    {"ConstLhs", vector<FieldDef>({{"scope", Node}, {"name", Name}})},
    // &. "conditional-send"/safe-navigation operator
    {"CSend", vector<FieldDef>({{"receiver", Node}, {"method", Name}, {"args", NodeVec}})},
    // @@foo class variable
    {"CVar", vector<FieldDef>({{"name", Name}})},
    // @@foo class variable in the lhs of an Mlhs
    {"CVarLhs", vector<FieldDef>({{"name", Name}})},
    // args may be NULL, body does not have to be a block.
    {"DefMethod", vector<FieldDef>({{"name", Name}, {"args", Node}, {"body", Node}})},
    // defined?() built-in pseudo-function
    {"Defined", vector<FieldDef>({{"value", Node}})},
    // def <expr>.name singleton-class method def
    {"DefS", vector<FieldDef>({{"singleton", Node}, {"name", Name}, {"args", Node}, {"body", Node}})},
    // string interpolation, all nodes are concatenated in a single string
    {"DString", vector<FieldDef>({{"nodes", NodeVec}})},
    // symbol interoplation, :"foo#{bar}"
    {"DSymbol", vector<FieldDef>({{"nodes", NodeVec}})},
    // ... flip-flop operator inside a conditional
    {"EFlipflop", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    // __ENCODING__
    {"EncodingLiteral", vector<FieldDef>()},
    {"Ensure", vector<FieldDef>({{"body", Node}, {"ensure", Node}})},
    {"ERange", vector<FieldDef>({{"from", Node}, {"to", Node}})},
    {"False", vector<FieldDef>()},
    // __FILE__
    {"FileLiteral", vector<FieldDef>()},
    // For loop
    {"For", vector<FieldDef>({{"vars", Node}, {"expr", Node}, {"body", Node}})},
    {"Float", vector<FieldDef>({{"val", String}})},
    // Global variable ($foo)
    {"GVar", vector<FieldDef>({{"name", Name}})},
    // Global variable in the lhs of an mlhs
    {"GVarLhs", vector<FieldDef>({{"name", Name}})},
    // entries are `Pair`s,
    {"Hash", vector<FieldDef>({{"pairs", NodeVec}})},
    // Bareword identifier (foo); I *think* should only exist transiently while parsing
    {"Ident", vector<FieldDef>({{"name", Name}})},
    {"If", vector<FieldDef>({{"condition", Node}, {"then_", Node}, {"else_", Node}})},
    // .. flip-flop operator inside a conditional
    {"IFlipflop", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    // inclusive range. Subnodes need not be integers nor literals
    {"IRange", vector<FieldDef>({{"from", Node}, {"to", Node}})},
    {"Integer", vector<FieldDef>({{"val", String}})},
    // instance variable reference
    {"IVar", vector<FieldDef>({{"name", Name}})},
    // @rules in `@rules, invalid_rules = ...`
    {"IVarLhs", vector<FieldDef>({{"name", Name}})},
    // Required keyword argument inside an (args)
    {"Kwarg", vector<FieldDef>({{"name", Name}})},
    // explicit `begin` keyword. Is there a difference between explicit and implicit one?
    {"Kwbegin", vector<FieldDef>({{"stmts", NodeVec}})},
    // optional arg with default value provided
    {"Kwoptarg", vector<FieldDef>({{"name", Name}, {"default_", Node}})},
    // **kwargs arg
    {"Kwrestarg", vector<FieldDef>({{"name", Name}})},
    // **foo splat
    {"Kwsplat", vector<FieldDef>({{"expr", Node}})},
    {"LineLiteral", vector<FieldDef>()},
    // local variable referense
    {"LVar", vector<FieldDef>({{"name", Name}})},
    {"LVarAsgn", vector<FieldDef>({{"name", Name}, {"expr", Node}})},
    // invalid_rules in `@rules, invalid_rules = ...`
    {"LVarLhs", vector<FieldDef>({{"name", Name}})},
    // [regex literal] =~ value; autovivifies local vars from match grops
    {"MatchAsgn", vector<FieldDef>({{"regex", Node}, {"expr", Node}})},
    // /foo/ regex literal inside an `if`; implicitly matches against $_
    {"MatchCurLine", vector<FieldDef>({{"cond", Node}})},
    // multiple left hand sides: `@rules, invalid_rules = ...`
    {"Masgn", vector<FieldDef>({{"lhs", Node}, {"rhs", Node}})},
    // multiple left hand sides: `@rules, invalid_rules = ...`
    {"Mlhs", vector<FieldDef>({{"exprs", NodeVec}})},
    {"Module", vector<FieldDef>({{"name", Node}, {"body", Node}})},
    // next(args); `next` is like `return` but for blocks
    {"Next", vector<FieldDef>({{"exprs", NodeVec}})},
    {"Nil", vector<FieldDef>()},
    // $1, $2, etc
    {"NthRef", vector<FieldDef>({{"ref", Uint}})},
    // foo += 6 for += and other ops
    {"OpAsgn", vector<FieldDef>({{"left", Node}, {"op", Name}, {"right", Node}})},
    // logical or
    {"Or", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    // foo ||= bar
    {"OrAsgn", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    // optional argument inside an (args) list
    {"Optarg", vector<FieldDef>({{"name", Name}, {"default_", Node}})},
    // entries of Hash
    {"Pair", vector<FieldDef>({{"key", Node}, {"value", Node}})},
    // END {...}
    {"Postexe", vector<FieldDef>({{"body", Node}})},
    // BEGIN{...}
    {"Preexe", vector<FieldDef>({{"body", Node}})},
    // wraps the sole argument of a 1-arg block for some reason
    {"Procarg0", vector<FieldDef>({{"arg", Node}})},
    //
    {"Rational", vector<FieldDef>({{"val", String}})},
    // `redo` keyword
    {"Redo", vector<FieldDef>()},
    // regular expression; string interpolation in body is flattened into the array
    {"Regexp", vector<FieldDef>({{"regex", NodeVec}, {"opts", Node}})},
    // opts of regexp
    {"Regopt", vector<FieldDef>({{"opts", String}})},
    // body of a rescue
    {"Resbody", vector<FieldDef>({{"exception", Node}, {"var", Node}, {"body", Node}})},
    // begin; ..; rescue; end; rescue is an array of Resbody
    {"Rescue", vector<FieldDef>({{"body", Node}, {"rescue", NodeVec}, {"else_", Node}})},
    // *arg argument inside an (args)
    {"Restarg", vector<FieldDef>({{"name", Name}})},
    // `retry` keyword
    {"Retry", vector<FieldDef>()},
    // `return` keyword
    {"Return", vector<FieldDef>({{"exprs", NodeVec}})},
    // class << expr; body; end;
    {"SClass", vector<FieldDef>({{"expr", Node}, {"body", Node}})},
    {"Self", vector<FieldDef>()},
    // invocation
    {"Send", vector<FieldDef>({{"receiver", Node}, {"method", Name}, {"args", NodeVec}})},
    // not used in gerald.rb ???
    {"Shadowarg", vector<FieldDef>({{"name", Name}})},
    // *foo splat operator
    {"Splat", vector<FieldDef>({{"var", Node}})},
    {"SplatLhs", vector<FieldDef>({{"var", Node}})},
    // string literal
    {"String", vector<FieldDef>({{"val", Name}})},
    {"Super", vector<FieldDef>({{"args", NodeVec}})},
    // symbol literal
    {"Symbol", vector<FieldDef>({{"val", Name}})},
    {"True", vector<FieldDef>()},
    {"Undef", vector<FieldDef>({{"exprs", NodeVec}})},
    {"Until", vector<FieldDef>({{"cond", Node}, {"body", Node}})},
    {"UntilPost", vector<FieldDef>({{"cond", Node}, {"body", Node}})},
    {"When", vector<FieldDef>({{"patterns", NodeVec}, {"body", Node}})},
    {"While", vector<FieldDef>({{"cond", Node}, {"body", Node}})},
    // is there a non-syntactic difference in behaviour between post and non-post while?
    {"WhilePost", vector<FieldDef>({{"cond", Node}, {"body", Node}})},
    {"XString", vector<FieldDef>({{"nodes", NodeVec}})},
    {"Yield", vector<FieldDef>({{"exprs", NodeVec}})},
    {"ZSuper", vector<FieldDef>()},
};

string arg_type(FieldType arg) {
    switch (arg) {
        case Name:
            return "NameRef ";
        case Node:
            return "unique_ptr<Node> ";
        case NodeVec:
            return "NodeVec ";
        case String:
            return "const std::string &";
        case Uint:
            return "u4 ";
    }
}

string field_type(FieldType arg) {
    switch (arg) {
        case Name:
            return "NameRef";
        case Node:
            return "unique_ptr<Node>";
        case NodeVec:
            return "NodeVec";
        case String:
            return "const std::string";
        case Uint:
            return "u4";
    }
}

void emit_node_header(ostream &out, NodeDef &node) {
    out << "class " << node.name << " final : public Node {" << endl;
    out << "public:" << endl;

    // generate constructor
    out << "    " << node.name << "(Loc loc";
    for (auto &arg : node.fields) {
        out << ", " << arg_type(arg.type) << arg.name;
    }
    out << ")" << endl;
    out << "        : Node(loc)";
    for (auto &arg : node.fields) {
        out << ", " << arg.name << "(";
        if (arg.type == Node || arg.type == NodeVec) {
            out << "move(";
        }
        out << arg.name;
        if (arg.type == Node || arg.type == NodeVec) {
            out << ")";
        }
        out << ")";
    }
    out << " {}" << endl;
    out << endl;

    // Generate fields
    for (auto &arg : node.fields) {
        out << "    " << field_type(arg.type) << " " << arg.name << ";" << endl;
    }
    out << endl;
    out << "  virtual std::string toString(core::GlobalState &gs, int tabs = 0);" << endl;
    out << "  virtual std::string nodeName();" << endl;

    out << "};" << endl;
    out << endl;
}

void emit_node_classfile(ostream &out, NodeDef &node) {
    out << "  std::string " << node.name << "::nodeName() {" << endl;
    out << "    return \"" << node.name << "\";" << endl;
    out << "  };" << endl << endl;

    out << "  std::string " << node.name << "::toString(core::GlobalState &gs, int tabs) {" << endl
        << "    std::stringstream buf;" << endl;
    out << "    buf << \"" << node.name << " {\" << std::endl;" << endl;
    // Generate fields
    for (auto &arg : node.fields) {
        out << "    printTabs(buf, tabs + 1);" << endl;
        switch (arg.type) {
            case Name:
                out << "    buf << \"" << arg.name << " = \" << " << arg.name << ".name(gs).toString(gs) << std::endl;"
                    << endl;
                break;
            case Node:
                out << "    buf << \"" << arg.name << " = \";" << endl;
                out << "    printNode(buf, " << arg.name << ", gs, tabs + 1);" << endl;
                break;
            case NodeVec:
                out << "    buf << \"" << arg.name << " = [\" << std::endl;" << endl;
                out << "    for (auto &&a: " << arg.name << ") {" << endl;
                out << "      printTabs(buf, tabs + 2);" << endl;
                out << "      printNode(buf, a, gs, tabs + 2); " << endl;
                out << "    }" << endl;
                out << "    printTabs(buf, tabs + 1);";
                out << "    buf << \"]\" << std::endl;" << endl;
                break;
            case String:
                out << "    buf << \"" << arg.name << " = \\\"\" << " << arg.name << "<< \"\\\"\" << std::endl;"
                    << endl;
                break;
            case Uint:
                out << "    buf << \"" << arg.name << " = \" << " << arg.name << " << std::endl;" << endl;
                break;
        }
    }
    out << "    printTabs(buf, tabs);" << endl;
    out << "    buf << \"}\";" << endl;
    out << "    return buf.str();" << endl;
    out << "  }" << endl;
    out << endl;
}

int main(int argc, char **argv) {
    // emit headef file
    {
        ofstream header(argv[1], ios::trunc);
        if (!header.good()) {
            cerr << "unable to open " << argv[1] << endl;
            return 1;
        }
        for (auto &node : nodes) {
            emit_node_header(header, node);
        }
    }

    {
        ofstream classfile(argv[2], ios::trunc);
        if (!classfile.good()) {
            cerr << "unable to open " << argv[2] << endl;
            return 1;
        }
        classfile << "#include \"parser/Node.h\"" << endl << endl;
        classfile << "namespace ruby_typer {" << endl;
        classfile << "namespace parser {" << endl;
        for (auto &node : nodes) {
            emit_node_classfile(classfile, node);
        }
        classfile << "}" << endl;
        classfile << "}" << endl;
    }
    return 0;
}
