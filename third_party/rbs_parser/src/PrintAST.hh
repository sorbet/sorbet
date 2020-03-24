#ifndef RBS_PARSER_PRINT_AST_HH
#define RBS_PARSER_PRINT_AST_HH

#include "File.hh"
#include "PrintVisitor.hh"

namespace rbs_parser {

class PrintAST : public PrintVisitor {
public:
    PrintAST(std::ostream &output) : PrintVisitor(output) {}

    virtual void visit(File *file) {
        for (auto *decl : file->decls) {
            enterVisit(decl);
        }
    }

    // Printing utils

    void printLoc(Node *node) {
        printt();
        print(node->loc.toString());
        print(": ");
    }

    void printTypes(std::vector<Type *> types, std::string begin, std::string sep, std::string end) {
        print(begin);
        for (int i = 0; i < types.size(); i++) {
            enterVisit(types[i]);
            if (i < types.size() - 1) {
                print(sep);
            }
        }
        print(end);
    }

    void printTypeParams(std::vector<TypeParam *> params) {
        if (params.empty()) {
            return;
        }
        print("[");
        for (int i = 0; i < params.size(); i++) {
            enterVisit(params[i]);
            if (i < params.size() - 1) {
                print(", ");
            }
        }
        print("]");
    }

    void printAttr(Attr *decl, std::string kind) {
        printLoc(decl);
        print(kind + ": " + *decl->name);
        if (decl->ivar) {
            print(" (" + *decl->ivar + ")");
        }
        print(": ");
        enterVisit(decl->type);
        printn();
    }

    // Types

    virtual void visit(TypeBool *type) { print("bool"); }

    virtual void visit(TypeNil *type) { print("nil"); }

    virtual void visit(TypeSelf *type) { print("self"); }

    virtual void visit(TypeUntyped *type) { print("untyped"); }

    virtual void visit(TypeVoid *type) { print("void"); }

    virtual void visit(TypeSimple *type) { print(*type->name); }

    virtual void visit(TypeSingleton *type) { print("singleton(" + *type->name + ")"); }

    virtual void visit(TypeNilable *type) {
        enterVisit(type->type);
        print("?");
    }

    virtual void visit(TypeUnion *type) { printTypes(type->types, "(", " | ", ")"); }

    virtual void visit(TypeIntersection *type) { printTypes(type->types, "(", " & ", ")"); }

    virtual void visit(TypeTuple *type) { printTypes(type->types, "[", ", ", "]"); }

    virtual void visit(TypeTrue *type) { print("true"); }

    virtual void visit(TypeFalse *type) { print("false"); }

    virtual void visit(TypeBot *type) { print("bot"); }

    virtual void visit(TypeAny *type) { print("any"); }

    virtual void visit(TypeClass *type) { print("class"); }

    virtual void visit(TypeSelfQ *type) { print("self?"); }

    virtual void visit(TypeInstance *type) { print("instance"); }

    virtual void visit(TypeInteger *type) { print(*type->integer); }

    virtual void visit(TypeString *type) { print(*type->string); }

    virtual void visit(TypeSymbol *type) { print(*type->symbol); }

    virtual void visit(TypeTop *type) { print("top"); }

    virtual void visit(TypeGeneric *type) {
        print(*type->name);
        printTypes(type->types, "[", ", ", "]");
    }

    virtual void visit(TypeProc *type) {
        printLoc(type);
        printn("signature");
        indent();
        for (int i = 0; i < type->params.size(); i++) {
            enterVisit(type->params[i]);
        }
        if (type->ret) {
            printLoc(type->ret);
            enterVisit(type->ret);
            printn();
        }
        dedent();
    }

    // Records

    virtual void visit(RecordField *field) {
        printLoc(field);
        print(*field->name);
        print(" => ");
        enterVisit(field->type);
        if (!dynamic_cast<Record *>(field->type)) {
            printn();
        }
    }

    virtual void visit(Record *type) {
        printn("record");
        indent();
        for (int i = 0; i < type->fields.size(); i++) {
            enterVisit(type->fields[i]);
        }
        dedent();
    }

    // Decl

    virtual void visit(TypeParam *param) {
        if (param->variance) {
            print(*param->variance + " ");
        }
        if (param->unchecked) {
            print("unchecked ");
        }
        print(*param->name);
    }

    virtual void visit(Class *decl) {
        printLoc(decl);
        print("class " + *decl->name);
        printTypeParams(decl->typeParams);
        if (decl->parent) {
            print(" < " + *decl->parent);
        }
        printn();
        indent();
        for (int i = 0; i < decl->members.size(); i++) {
            enterVisit(decl->members[i]);
        }
        dedent();
    }

    virtual void visit(Module *decl) {
        printLoc(decl);
        print("module " + *decl->name);
        printTypeParams(decl->typeParams);
        if (decl->selfType != NULL) {
            print(": ");
            enterVisit(decl->selfType);
        }
        printn();
        indent();
        for (int i = 0; i < decl->members.size(); i++) {
            enterVisit(decl->members[i]);
        }
        dedent();
    }

    virtual void visit(Interface *decl) {
        printLoc(decl);
        print("interface " + *decl->name);
        printTypeParams(decl->typeParams);
        printn();
        indent();
        for (int i = 0; i < decl->members.size(); i++) {
            enterVisit(decl->members[i]);
        }
        dedent();
    }

    virtual void visit(Extension *decl) {
        printLoc(decl);
        print("extension (" + *decl->extensionName + ") " + *decl->name);
        printTypeParams(decl->typeParams);
        printn();
        indent();
        for (int i = 0; i < decl->members.size(); i++) {
            enterVisit(decl->members[i]);
        }
        dedent();
    }

    virtual void visit(Const *decl) {
        printLoc(decl);
        print("const: " + *decl->name + " = ");
        enterVisit(decl->type);
        printn();
    }

    virtual void visit(Global *decl) {
        printLoc(decl);
        print("global: " + *decl->name + " = ");
        enterVisit(decl->type);
        printn();
    }

    virtual void visit(TypeDecl *decl) {
        printLoc(decl);
        print("type: " + *decl->name + " = ");
        if (Block *d = dynamic_cast<Block *>(decl->type)) {
            print("block");
            printn();
            indent();
            enterVisit(d);
            dedent();
        } else {
            enterVisit(decl->type);
            printn();
        }
    }

    // Class members

    virtual void visit(AttrReader *decl) { printAttr(decl, "attr_reader"); }

    virtual void visit(AttrWriter *decl) { printAttr(decl, "attr_writer"); }

    virtual void visit(AttrAccessor *decl) { printAttr(decl, "attr_accessor"); }

    virtual void visit(Alias *decl) {
        printLoc(decl);
        if (decl->singleton) {
            printn("alias: self." + *decl->from + " self." + *decl->to);
        } else {
            printn("alias: " + *decl->from + " " + *decl->to);
        }
    }

    virtual void visit(Include *decl) {
        printLoc(decl);
        print("include ");
        enterVisit(decl->type);
        printn();
    }

    virtual void visit(Extend *decl) {
        printLoc(decl);
        print("extend ");
        enterVisit(decl->type);
        printn();
    }

    virtual void visit(Prepend *decl) {
        printLoc(decl);
        print("prepend ");
        enterVisit(decl->type);
        printn();
    }

    virtual void visit(Visibility *decl) {
        printLoc(decl);
        printn(*decl->name);
    }

    virtual void visit(Method *decl) {
        printLoc(decl);
        if (decl->incompatible) {
            print("incompatible ");
        }
        print("def ");
        if (decl->singleton) {
            print("self");
            if (decl->instance) {
                print("?");
            }
            print(".");
        }
        printn(*decl->name);
        indent();
        for (int i = 0; i < decl->types.size(); i++) {
            enterVisit(decl->types[i]);
        }
        dedent();
    }

    virtual void visit(MethodType *type) {
        printLoc(type);
        print("method_type");
        printTypeParams(type->typeParams);
        printn();
        indent();
        enterVisit(type->sig);
        if (type->block) {
            enterVisit(type->block);
        }
        dedent();
    }

    virtual void visit(Block *type) {
        printLoc(type);
        printn(type->optional ? "block optional" : "block");
        indent();
        enterVisit(type->sig);
        dedent();
    }

    virtual void visit(Param *param) {
        printLoc(param);
        print("param: ");
        if (param->optional) {
            print("?");
        }
        if (param->vararg) {
            print("*");
        }
        enterVisit(param->type);
        if (param->name) {
            print(" " + *param->name);
        }
        printn();
    }
};
} // namespace rbs_parser

#endif
