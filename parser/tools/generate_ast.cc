#include <iostream>
#include <string>
#include <vector>

using namespace std;

enum FieldType {
    Name,
    Node,
    NodeVec,
    String,
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

    {"Ident", vector<FieldDef>({
                  {"name", Name},
              })},

    {"Class", vector<FieldDef>({
                  {
                      "name", Node,
                  },
                  {
                      "superclass", Node,
                  },
                  {
                      "body", Node,
                  },

              })},

    {"Module", vector<FieldDef>({{
                                     "name", Node,
                                 },
                                 {
                                     "body", Node,
                                 }})

    },

    {"DefMethod", vector<FieldDef>({{
                                        "name", Name,
                                    },
                                    {
                                        "args", Node,
                                    },
                                    {
                                        "body", Node,
                                    }})

    },

    {"SClass", vector<FieldDef>({{
                                     "expr", Node,
                                 },
                                 {
                                     "body", Node,
                                 }})}};

std::string arg_type(FieldType arg) {
    switch (arg) {
        case Name:
            return "NameRef ";
        case Node:
            return "unique_ptr<Node> &&";
        case NodeVec:
            return "vector<unique_ptr<Node>> &&";
        case String:
            return "const std::string &";
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
        if (arg.type == Node)
            cout << "move(";
        cout << arg.name;
        if (arg.type == Node)
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
