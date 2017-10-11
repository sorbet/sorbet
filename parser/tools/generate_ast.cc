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
    //  ??? not used in gerald.rb
    {"Backref", vector<FieldDef>({{"name", Name}})},
    //  top level, bodies of classes, modules and methods
    {"Begin", vector<FieldDef>({{"stmts", NodeVec}})},
    //  Node is always a send, which is previous call, args is arguments of body
    {"Block", vector<FieldDef>({{"send", Node}, {"args", Node}, {"body", Node}})},
    //  ??? not used in gerald.rb
    {"Blockarg", vector<FieldDef>({{"name", Name}})},
    //  e.g. map(&:token)
    {"BlockPass", vector<FieldDef>({{"block", Node}})},
    {"Break", vector<FieldDef>({{"exprs", NodeVec}})},
    // not used in gerald.rb
    {"Case", vector<FieldDef>({{"condition", Node}, {"whens", NodeVec}, {"else_", Node}})},
    // not used in gerald.rb
    {"Cbase", vector<FieldDef>()},
    // superclass is Null if emtpy superclass, body is Begin
    {"Class", vector<FieldDef>({{"name", Node}, {"superclass", Node}, {"body", Node}})},
    {"Complex", vector<FieldDef>({{"value", String}})},
    // Used as path to Select, scope is Null for end of specified list
    {"Const", vector<FieldDef>({{"scope", Node}, {"name", Name}})},
    // ???, not used in gerald.rb
    {"ConstLhs", vector<FieldDef>({{"scope", Node}, {"name", Name}})},
    // ???, not used in gerald.rb
    {"ConstAsgn", vector<FieldDef>({{"scope", Node}, {"name", Name}, {"expr", Node}})},
    // ???, not used in gerald.rb
    {"CSend", vector<FieldDef>({{"receiver", Node}, {"method", Name}, {"args", NodeVec}})},
    // ???, not used in gerald.rb
    {"CVar", vector<FieldDef>({{"name", Name}})},
    // ???, not used in gerald.rb
    {"CVarLhs", vector<FieldDef>({{"name", Name}})},
    // ???, not used in gerald.rb
    {"CVarAsgn", vector<FieldDef>({{"name", Name}, {"expr", Node}})},
    // args may be NULL, body does not have to be a block.
    {"DefMethod", vector<FieldDef>({{"name", Name}, {"args", Node}, {"body", Node}})},
    // ???, not used in gerald.rb
    {"Defined", vector<FieldDef>({{"value", Node}})},
    // ???, not used in gerald.rb
    {"DefS", vector<FieldDef>({{"name", Name}, {"singleton", Node}, {"args", Node}, {"body", Node}})},
    // string interpolation, all nodes are concatenated in a single string
    {"DString", vector<FieldDef>({{"nodes", NodeVec}})},
    // ???, not used in gerald.rb
    {"DSymbol", vector<FieldDef>({{"nodes", NodeVec}})},
    // ???, not used in gerald.rb
    {"EFlipflop", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    {"EncodingLiteral", vector<FieldDef>()},
    {"Ensure", vector<FieldDef>({{"body", Node}, {"ensure", Node}})},
    {"ERange", vector<FieldDef>({{"from", Node}, {"to", Node}})},
    {"False", vector<FieldDef>()},
    // __FILE__
    {"FileLiteral", vector<FieldDef>()},
    // ???, not used in gerald.rb
    {"For", vector<FieldDef>({{"vars", Node}, {"expr", Node}, {"body", Node}})},
    {"Float", vector<FieldDef>({{"val", String}})},
    // ???, not used in gerald.rb
    {"GVar", vector<FieldDef>({{"name", Name}})},
    // ???, not used in gerald.rb
    {"GVarAsgn", vector<FieldDef>({{"name", Name}, {"expr", Node}})},
    // ???, not used in gerald.rb
    {"GVarLhs", vector<FieldDef>({{"name", Name}})},
    // entries are `Pair`s,
    {"Hash", vector<FieldDef>({{"pairs", NodeVec}})},
    {"Ident", vector<FieldDef>({{"name", Name}})},
    {"If", vector<FieldDef>({{"condition", Node}, {"then_", Node}, {"else_", Node}})},
    // ???, not used in gerald.rb
    {"IFlipflop", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    // inclusive range. Could the subnodes be non-integers?
    {"IRange", vector<FieldDef>({{"from", Node}, {"to", Node}})},
    {"Integer", vector<FieldDef>({{"val", String}})},
    // instance variable reference
    {"IVar", vector<FieldDef>({{"name", Name}})},
    // ivar assign
    {"IVarAsgn", vector<FieldDef>({{"name", Name}, {"expr", Node}})},
    // @rules in `@rules, invalid_rules = ...`
    {"IVarLhs", vector<FieldDef>({{"name", Name}})},
    // ???, not used in gerald.rb
    {"Kwarg", vector<FieldDef>({{"name", Name}})},
    // ???, not used in gerald.rb
    {"Kwbegin", vector<FieldDef>({{"vars", NodeVec}})},
    // optional arg with default value provided
    {"Kwoptarg", vector<FieldDef>({{"name", Name}, {"default_", Node}})},
    {"Kwrestarg", vector<FieldDef>({{"name", Name}})},
    {"Kwsplat", vector<FieldDef>({{"expr", Node}})},
    {"Lambda", vector<FieldDef>()},
    {"LineLiteral", vector<FieldDef>()},
    // local variable referense
    {"LVar", vector<FieldDef>({{"name", Name}})},
    {"LVarAsgn", vector<FieldDef>({{"name", Name}, {"expr", Node}})},
    // invalid_rules in `@rules, invalid_rules = ...`
    {"LVarLhs", vector<FieldDef>({{"name", Name}})},
    // not used in gerald.rb
    {"MatchAsgn", vector<FieldDef>({{"regex", Node}, {"expr", Node}})},
    {"MatchCurLine", vector<FieldDef>({{"cond", Node}})},
    // multiple left hand sides: `@rules, invalid_rules = ...`
    {"Masgn", vector<FieldDef>({{"lhs", Node}, {"rhs", Node}})},
    // multiple left hand sides: `@rules, invalid_rules = ...`
    {"Mlhs", vector<FieldDef>({{"exprs", NodeVec}})},
    {"Module", vector<FieldDef>({{"name", Node}, {"body", Node}})},
    // next. ??? arguments ???
    {"Next", vector<FieldDef>({{"exprs", NodeVec}})},
    {"Nil", vector<FieldDef>()},
    // not used in gerald.rb
    {"NthRef", vector<FieldDef>({{"ref", Uint}})},
    // not used in gerald.rb, seems to be  ${op}=
    {"OpAsgn", vector<FieldDef>({{"left", Node}, {"op", Name}, {"right", Node}})},
    // not used in gerald.rb, seems to be ||
    {"Or", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    // not used in gerald.rb, seems to be ||=
    {"OrAsgn", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    // not used in gerald.rb
    {"Optarg", vector<FieldDef>({{"name", Name}, {"default_", Node}})},
    // entries of Hash
    {"Pair", vector<FieldDef>({{"key", Node}, {"value", Node}})},
    // not used in gerald.rb
    {"Postexe", vector<FieldDef>({{"body", Node}})},
    // not used in gerald.rb
    {"Preexe", vector<FieldDef>({{"body", Node}})},
    // not used in gerald.rb
    {"Procarg0", vector<FieldDef>({{"arg", Node}})},
    // not used in gerald.rb
    {"Rational", vector<FieldDef>({{"val", String}})},
    // not used in gerald.rb
    {"Redo", vector<FieldDef>()},
    // regular expression. ??? why multiple ???
    {"Regexp", vector<FieldDef>({{"regex", NodeVec}, {"opts", Node}})},
    // opts of regexp
    {"Regopt", vector<FieldDef>({{"opts", String}})},
    // not used in gerald.rb
    {"Resbody", vector<FieldDef>({{"exception", Node}, {"var", Node}, {"body", Node}})},
    // not used in gerald.rb
    {"Rescue", vector<FieldDef>({{"body", Node}, {"rescue", NodeVec}, {"else_", Node}})},
    // not used in gerald.rb, likely repeated arg
    {"Restarg", vector<FieldDef>({{"name", Name}})},
    {"Retry", vector<FieldDef>()},
    {"Return", vector<FieldDef>({{"exprs", NodeVec}})},
    // not used in gerald.rb, likely self class
    {"SClass", vector<FieldDef>({{"expr", Node}, {"body", Node}})},
    {"Self", vector<FieldDef>()},
    // invocation
    {"Send", vector<FieldDef>({{"receiver", Node}, {"method", Name}, {"args", NodeVec}})},
    // not used in gerald.rb ???
    {"ShadowArg", vector<FieldDef>({{"name", Name}})},
    {"Splat", vector<FieldDef>({{"var", Node}})},
    {"String", vector<FieldDef>({{"val", String}})},
    {"Super", vector<FieldDef>({{"args", NodeVec}})},
    {"Symbol", vector<FieldDef>({{"val", String}})},
    {"True", vector<FieldDef>()},
    {"Undef", vector<FieldDef>({{"exprs", NodeVec}})},
    {"Until", vector<FieldDef>({{"cond", Node}, {"body", Node}})},
    {"UntilPost", vector<FieldDef>({{"cond", Node}, {"body", Node}})},
    {"When", vector<FieldDef>({{"cond", Node}, {"body", Node}})},
    {"While", vector<FieldDef>({{"cond", Node}, {"body", Node}})},
    // is there a non-syntactic difference in behaviour between post and non-post while?
    {"WhilePost", vector<FieldDef>({{"cond", Node}, {"body", Node}})},
    {"Yield", vector<FieldDef>({{"exprs", NodeVec}})},
    {"ZSuper", vector<FieldDef>()},
};

std::string arg_type(FieldType arg) {
    switch (arg) {
        case Name:
            return "NameRef ";
        case Node:
            return "unique_ptr<Node> ";
        case NodeVec:
            return "vector<unique_ptr<Node>> ";
        case String:
            return "const std::string &";
        case Uint:
            return "u4 ";
    }
}

std::string field_type(FieldType arg) {
    switch (arg) {
        case Name:
            return "NameRef";
        case Node:
            return "unique_ptr<Node>";
        case NodeVec:
            return "vector<unique_ptr<Node>>";
        case String:
            return "const std::string";
        case Uint:
            return "u4";
    }
}

void emit_node_header(NodeDef &node) {
    cout << "class " << node.name << " : public Node {" << endl;
    cout << "public:" << endl;

    // generate constructor
    cout << "    " << node.name << "(Loc loc";
    for (auto &arg : node.fields) {
        cout << ", " << arg_type(arg.type) << arg.name;
    }
    cout << ")" << endl;
    cout << "        : Node(loc)";
    for (auto &arg : node.fields) {
        cout << ", " << arg.name << "(";
        if (arg.type == Node || arg.type == NodeVec)
            cout << "move(";
        cout << arg.name;
        if (arg.type == Node || arg.type == NodeVec)
            cout << ")";
        cout << ")";
    }
    cout << " {}" << endl;
    cout << endl;

    // Generate fields
    for (auto &arg : node.fields) {
        cout << "    " << field_type(arg.type) << " " << arg.name << ";" << endl;
    }
    cout << endl;
    cout << "  virtual std::string toString(ast::ContextBase &ctx, int tabs = 0);" << endl;
    cout << "  virtual std::string nodeName();" << endl;

    cout << "};" << endl;
    cout << endl;
}

void emit_node_classfile(NodeDef &node) {

    cout << "  std::string " << node.name << "::nodeName() {" << endl;
    cout << "    return \"" << node.name << "\";" << endl;
    cout << "  };" << endl << endl;

    cout << "  std::string " << node.name << "::toString(ast::ContextBase &ctx, int tabs) { " << endl
         << "    std::stringstream buf;" << endl;
    cout << "    buf << \"" << node.name << " {\" << std::endl;" << endl;
    // Generate fields
    for (auto &arg : node.fields) {
        cout << "    printTabs(buf, tabs + 1);" << endl;
        switch (arg.type) {
            case Name:
                cout << "    buf << \"" << arg.name << " = \" << " << arg.name
                     << ".name(ctx).toString(ctx) << std::endl;" << endl;
                break;
            case Node:
                cout << "    buf << \"" << arg.name << " = \";" << endl;
                cout << "    printNode(buf, " << arg.name << ", ctx, tabs + 1);" << endl;
                break;
            case NodeVec:
                cout << "    buf << \"" << arg.name << " = [\" << std::endl;" << endl;
                cout << "    for (auto &&a: " << arg.name << ") {" << endl;
                cout << "      printTabs(buf, tabs + 2);" << endl;
                cout << "      printNode(buf, a, ctx, tabs + 2); " << endl;
                cout << "    }" << endl;
                cout << "    printTabs(buf, tabs + 1);";
                cout << "    buf << \"]\" << std::endl;" << endl;
                break;
            case String:
                cout << "    buf << \"" << arg.name << " = \\\"\" << " << arg.name << "<< \"\\\"\" << std::endl;"
                     << endl;
                break;
            case Uint:
                cout << "    buf << \"" << arg.name << " = \" << " << arg.name << " << std::endl;" << endl;
                break;
        }
    }
    cout << "    printTabs(buf, tabs);" << endl;
    cout << "    buf << \"}\";" << endl;
    cout << "    return buf.str();" << endl;
    cout << "  }" << endl;
    cout << endl;
}

int main(int argc, char **argv) {
    // emmit headef file
    auto headerfile = argv[1];
    freopen(headerfile, "w", stdout);
    for (auto &node : nodes) {
        emit_node_header(node);
    }

    auto classfile = argv[2];
    freopen(classfile, "w", stdout);
    cout << "#include \"parser/Node.h\"" << endl << endl;
    cout << "namespace ruby_typer {" << endl;
    cout << "namespace parser {" << endl;
    for (auto &node : nodes) {

        emit_node_classfile(node);
    }
    cout << "}" << endl;
    cout << "}" << endl;
    return 0;
}
