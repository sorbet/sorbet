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

    {"Alias", vector<FieldDef>({{"from", Node}, {"to", Node}})},
    {"And", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    {"AndAsgn", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    {"Arg", vector<FieldDef>({{"name", Name}})},
    {"Args", vector<FieldDef>({{"args", NodeVec}})},
    {"Array", vector<FieldDef>({{"elts", NodeVec}})},
    {"Backref", vector<FieldDef>({{"name", Name}})},
    {"Begin", vector<FieldDef>({{"stmts", NodeVec}})},
    {"Block", vector<FieldDef>({{"send", Node}, {"args", Node}, {"body", Node}})},
    {"Blockarg", vector<FieldDef>({{"name", Name}})},
    {"BlockPass", vector<FieldDef>({{"block", Node}})},
    {"Break", vector<FieldDef>({{"exprs", NodeVec}})},
    {"Case", vector<FieldDef>({{"condition", Node}, {"whens", NodeVec}, {"else_", Node}})},
    {"Cbase", vector<FieldDef>()},
    {"Class", vector<FieldDef>({{"name", Node}, {"superclass", Node}, {"body", Node}})},
    {"Complex", vector<FieldDef>({{"value", String}})},
    {"Const", vector<FieldDef>({{"scope", Node}, {"name", Name}})},
    {"ConstLhs", vector<FieldDef>({{"scope", Node}, {"name", Name}})},
    {"ConstAsgn", vector<FieldDef>({{"scope", Node}, {"name", Name}, {"expr", Node}})},
    {"CSend", vector<FieldDef>({{"receiver", Node}, {"method", Name}, {"args", NodeVec}})},
    {"CVar", vector<FieldDef>({{"name", Name}})},
    {"CVarLhs", vector<FieldDef>({{"name", Name}})},
    {"CVarAsgn", vector<FieldDef>({{"name", Name}, {"expr", Node}})},
    {"DefMethod", vector<FieldDef>({{"name", Name}, {"args", Node}, {"body", Node}})},
    {"Defined", vector<FieldDef>({{"value", Node}})},
    {"DefS", vector<FieldDef>({{"name", Name}, {"singleton", Node}, {"args", Node}, {"body", Node}})},
    {"DString", vector<FieldDef>({{"nodes", NodeVec}})},
    {"DSymbol", vector<FieldDef>({{"nodes", NodeVec}})},
    {"EFlipflop", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    {"EncodingLiteral", vector<FieldDef>()},
    {"Ensure", vector<FieldDef>({{"body", Node}, {"ensure", Node}})},
    {"ERange", vector<FieldDef>({{"from", Node}, {"to", Node}})},
    {"False", vector<FieldDef>()},
    {"FileLiteral", vector<FieldDef>()},
    {"For", vector<FieldDef>({{"vars", Node}, {"expr", Node}, {"body", Node}})},
    {"Float", vector<FieldDef>({{"val", String}})},
    {"GVar", vector<FieldDef>({{"name", Name}})},
    {"GVarAsgn", vector<FieldDef>({{"name", Name}, {"expr", Node}})},
    {"GVarLhs", vector<FieldDef>({{"name", Name}})},
    {"Hash", vector<FieldDef>({{"pairs", NodeVec}})},
    {"Ident", vector<FieldDef>({{"name", Name}})},
    {"If", vector<FieldDef>({{"condition", Node}, {"then_", Node}, {"else_", Node}})},
    {"IFlipflop", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    {"IRange", vector<FieldDef>({{"from", Node}, {"to", Node}})},
    {"Integer", vector<FieldDef>({{"val", String}})},
    {"IVar", vector<FieldDef>({{"name", Name}})},
    {"IVarAsgn", vector<FieldDef>({{"name", Name}, {"expr", Node}})},
    {"IVarLhs", vector<FieldDef>({{"name", Name}})},
    {"Kwarg", vector<FieldDef>({{"name", Name}})},
    {"Kwbegin", vector<FieldDef>({{"vars", NodeVec}})},
    {"Kwoptarg", vector<FieldDef>({{"name", Name}, {"default_", Node}})},
    {"Kwrestarg", vector<FieldDef>({{"name", Name}})},
    {"Kwsplat", vector<FieldDef>({{"expr", Node}})},
    {"Lambda", vector<FieldDef>()},
    {"LineLiteral", vector<FieldDef>()},
    {"LVar", vector<FieldDef>({{"name", Name}})},
    {"LVarAsgn", vector<FieldDef>({{"name", Name}, {"expr", Node}})},
    {"LVarLhs", vector<FieldDef>({{"name", Name}})},
    {"MatchAsgn", vector<FieldDef>({{"regex", Node}, {"expr", Node}})},
    {"MatchCurLine", vector<FieldDef>({{"cond", Node}})},
    {"Masgn", vector<FieldDef>({{"lhs", Node}, {"rhs", Node}})},
    {"Mlhs", vector<FieldDef>({{"exprs", NodeVec}})},
    {"Module", vector<FieldDef>({{"name", Node}, {"body", Node}})},
    {"Next", vector<FieldDef>({{"exprs", NodeVec}})},
    {"Nil", vector<FieldDef>()},
    {"NthRef", vector<FieldDef>({{"ref", Uint}})},
    {"OpAsgn", vector<FieldDef>({{"left", Node}, {"op", Name}, {"right", Node}})},
    {"Or", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    {"OrAsgn", vector<FieldDef>({{"left", Node}, {"right", Node}})},
    {"Optarg", vector<FieldDef>({{"name", Name}, {"default_", Node}})},
    {"Pair", vector<FieldDef>({{"key", Node}, {"value", Node}})},
    {"Postexe", vector<FieldDef>({{"body", Node}})},
    {"Preexe", vector<FieldDef>({{"body", Node}})},
    {"Procarg0", vector<FieldDef>({{"arg", Node}})},
    {"Rational", vector<FieldDef>({{"val", String}})},
    {"Redo", vector<FieldDef>()},
    {"Regexp", vector<FieldDef>({{"regex", NodeVec}, {"opts", Node}})},
    {"Regopt", vector<FieldDef>({{"opts", String}})},
    {"Resbody", vector<FieldDef>({{"exception", Node}, {"var", Node}, {"body", Node}})},
    {"Rescue", vector<FieldDef>({{"body", Node}, {"rescue", NodeVec}, {"else_", Node}})},
    {"Restarg", vector<FieldDef>({{"name", Name}})},
    {"Retry", vector<FieldDef>()},
    {"Return", vector<FieldDef>({{"exprs", NodeVec}})},
    {"SClass", vector<FieldDef>({{"expr", Node}, {"body", Node}})},
    {"Self", vector<FieldDef>()},
    {"Send", vector<FieldDef>({{"receiver", Node}, {"method", Name}, {"args", NodeVec}})},
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

    cout << "  std::string "<< node.name <<"::nodeName() {" << endl;
    cout << "    return \"" << node.name <<"\";" << endl;
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
