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
    {"Block", vector<FieldDef>({{"send", Node}, {"args", NodeVec}, {"body", NodeVec}})},
    {"Blockarg", vector<FieldDef>({{"name", Name}})},
    {"BlockPass", vector<FieldDef>({{"block", Node}})},
    {"Break", vector<FieldDef>({{"exprs", NodeVec}})},
    {"Case", vector<FieldDef>({{"condition", Node}, {"whens", NodeVec}, {"else_", Node}})},
    {"Cbase", vector<FieldDef>()},
    {"Class", vector<FieldDef>({{"name", Node}, {"superclass", Node}, {"body", Node}})},
    {"Complex", vector<FieldDef>({{"value", String}})},
    {"Const", vector<FieldDef>({{"scope", Node}, {"name", Name}})},
    {"LhsConst", vector<FieldDef>({{"scope", Node}, {"name", Name}})},
    {"ConstAsgn", vector<FieldDef>({{"scope", Node}, {"name", Name}, {"expr", Node}})},
    {"CSend", vector<FieldDef>({{"target", Node}, {"method", Name}, {"args", NodeVec}})},
    {"Cvar", vector<FieldDef>({{"name", Name}})},
    {"CvarLhs", vector<FieldDef>({{"name", Name}})},
    {"CvarAsgn", vector<FieldDef>({{"name", Name}, {"expr", Node}})},
    {"DefMethod", vector<FieldDef>({{"name", Name}, {"args", Node}, {"body", Node}})},
    {"Defined", vector<FieldDef>({{"value", Node}})},
    {"DefS", vector<FieldDef>({{"name", Name}, {"singleton", Node}, {"args", Node}, {"body", Node}})},
    {"DString", vector<FieldDef>({{"nodes", NodeVec}})},
    {"DSymbol", vector<FieldDef>({{"nodes", NodeVec}})},
    {"EFlipFlop", vector<FieldDef>({{"left", Node}, {"right", Node}})},
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
    {"Rescue", vector<FieldDef>({{"body", Node}, {"rescue", Node}, {"else_", Node}})},
    {"Restarg", vector<FieldDef>({{"name", Name}})},
    {"Retry", vector<FieldDef>()},
    {"Return", vector<FieldDef>({{"exprs", NodeVec}})},
    {"SClass", vector<FieldDef>({{"expr", Node}, {"body", Node}})},
    {"Self", vector<FieldDef>()},
    {"Send", vector<FieldDef>({{"target", Node}, {"method", Name}, {"args", NodeVec}})},
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

void emit_node_class(NodeDef &node) {
    cout << "class " << node.name << " : public Node {" << endl;
    cout << "public:" << endl;

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

    for (auto &arg : node.fields) {
        cout << "    " << field_type(arg.type) << " " << arg.name << ";" << endl;
    }

    cout << "};" << endl;
    cout << endl;
}

int main(int argc, char **argv) {
    for (auto &node : nodes) {
        emit_node_class(node);
    }
    return 0;
}
