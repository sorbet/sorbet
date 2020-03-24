#ifndef RBS_PARSER_PRINT_RBI_HH
#define RBS_PARSER_PRINT_RBI_HH

#include "File.hh"
#include "PrintVisitor.hh"
#include <regex>
#include <set>

namespace rbs_parser {

class PrintRBI : public PrintVisitor {
public:
    std::string typedLevel;
    std::set<std::string> typeNames;
    bool inInclude;

    PrintRBI(std::ostream &output) : PrintVisitor(output), typedLevel("true"), inInclude(false){};
    PrintRBI(std::ostream &output, std::string level) : PrintVisitor(output), typedLevel(level), inInclude(false){};

    virtual void visit(File *file) {
        printl("# typed: " + typedLevel);
        bool lastWasBlock = true;
        for (int i = 0; i < file->decls.size(); i++) {
            auto *decl = file->decls[i];
            if (lastWasBlock) {
                printn();
            }
            if (dynamic_cast<Scope *>(decl)) {
                lastWasBlock = true;
            } else {
                lastWasBlock = false;
            }
            enterVisit(decl);
        }
    }

    void warnUnsupported(Node *node, std::string message) {
        std::cerr << node->loc.toString();
        std::cerr << ": Warning: ";
        std::cerr << message << std::endl;
    }

    // Names sanitizing

    std::vector<std::string> splitNamespace(std::string name) {
        std::vector<std::string> res;
        size_t pos = 0;
        std::string token;
        std::string delimiter = "::";

        while ((pos = name.find(delimiter)) != std::string::npos) {
            token = name.substr(0, pos);
            res.emplace_back(token);
            name.erase(0, pos + delimiter.length());
        }
        res.emplace_back(name);

        return res;
    }

    std::string sanitizeTypeName(std::string name) {
        if (name == "string") {
            return "String";
        } else if (name == "int") {
            return "Integer";
        } else if (name == "Array" || name == "::Array") {
            return "T::Array";
        } else if (name == "Hash" || name == "::Hash") {
            return "T::Hash";
        } else if (name == "Set" || name == "::Set") {
            return "T::Set";
        } else if (name == "Range" || name == "::Range") {
            return "T::Range";
        } else if (name == "Enumerator" || name == "::Enumerator") {
            return "T::Enumerator";
        } else if (name == "Enumerable" || name == "::Enumerable") {
            return "T::Enumerable";
        } else {
            name = sanitizeInterfaceName(name);
            name = sanitizeAliasName(name);
            return name;
        }
    }

    std::string sanitizeInterfaceName(std::string name) {
        std::string res;
        std::vector<std::string> names = splitNamespace(name);
        for (int i = 0; i < names.size(); i++) {
            if (i != 0) {
                res += "::";
            }
            if (i == names.size() - 1 && names[i].find("_") == 0) {
                res += names[i].substr(1, names[i].length() - 1);
            } else {
                res += names[i];
            }
        }
        return res;
    }

    std::string sanitizeAliasName(std::string name) {
        std::string res;
        std::vector<std::string> names = splitNamespace(name);
        for (int i = 0; i < names.size(); i++) {
            if (i != 0) {
                res += "::";
            }
            if (i == names.size() - 1 && std::islower(names[i][0])) {
                res += std::toupper(names[i][0]);
                res += names[i].substr(1, names[i].length() - 1);
            } else {
                res += names[i];
            }
        }
        return res;
    }

    std::string sanitizeDefName(std::string name) {
        if (name.length() > 1 && name.find("`", 0) == 0) {
            name = name.substr(1, name.length() - 2);
        }
        if (name == "undef") {
            name = "_undef";
        }
        return name;
    }

    // Types

    virtual void visit(TypeBool *type) { print("T::Boolean"); }

    virtual void visit(TypeNil *type) { print("NilClass"); }

    virtual void visit(TypeSelf *type) { print("T.self_type"); }

    virtual void visit(TypeUntyped *type) { print("T.untyped"); }

    virtual void visit(TypeVoid *type) { print("void"); }

    virtual void visit(TypeAny *type) { print("T.untyped"); }

    virtual void visit(TypeBot *type) {
        warnUnsupported(static_cast<Node *>(type), "Unsupported `bot`");
        print("T.untyped");
    }

    virtual void visit(TypeClass *type) { print("T.class_of(T.self_type)"); }

    virtual void visit(TypeFalse *type) { print("T::Boolean"); }

    virtual void visit(TypeTrue *type) { print("T::Boolean"); }

    virtual void visit(TypeSelfQ *type) {
        warnUnsupported(static_cast<Node *>(type), "Unsupported `self?`");
        print("T.untyped");
    }

    virtual void visit(TypeString *type) { print("String"); }

    virtual void visit(TypeInteger *type) { print("Integer"); }

    virtual void visit(TypeInstance *type) { print("T.attached_class"); }

    virtual void visit(TypeSymbol *type) { print("Symbol"); }

    virtual void visit(TypeTop *type) {
        warnUnsupported(static_cast<Node *>(type), "Unsupported `top`");
        print("T.untyped");
    }

    virtual void visit(TypeNilable *type) {
        print("T.nilable(");
        enterVisit(type->type);
        print(")");
    }

    virtual void visit(TypeSimple *type) {
        bool isTypeParam = typeNames.find(*type->name) != typeNames.end();
        if (isTypeParam) {
            print("T.type_parameter(:");
        }
        print(sanitizeTypeName(*type->name));
        if (isTypeParam) {
            print(")");
        }
    }

    virtual void visit(TypeSingleton *type) {
        print("T.class_of(");
        print(*type->name);
        print(")");
    }

    void printTypes(std::vector<Type *> types) {
        for (int i = 0; i < types.size(); i++) {
            enterVisit(types[i]);
            if (i < types.size() - 1) {
                print(", ");
            }
        }
    }

    virtual void visit(TypeUnion *type) {
        print("T.any(");
        printTypes(type->types);
        print(")");
    }

    virtual void visit(TypeIntersection *type) {
        print("T.all(");
        printTypes(type->types);
        print(")");
    }

    virtual void visit(TypeTuple *type) {
        print("[");
        printTypes(type->types);
        print("]");
    }

    virtual void visit(TypeGeneric *type) {
        print(sanitizeTypeName(*type->name));
        if (!inInclude) {
            print("[");
            printTypes(type->types);
            print("]");
        }
    }

    virtual void visit(TypeProc *type) {
        print("T.proc");
        if (!type->params.empty()) {
            print(".params(");
            for (int i = 0; i < type->params.size(); i++) {
                printParam(type->params[i], i);
                if (i < type->params.size() - 1) {
                    print(", ");
                }
            }
            print(")");
        }
        print(".");
        if (TypeVoid *type_void = dynamic_cast<TypeVoid *>(type->ret)) {
            enterVisit(type_void);
        } else {
            print("returns(");
            enterVisit(type->ret);
            print(")");
        }
    }

    // Records
    virtual void visit(RecordField *field) {
        // TODO sanitize name
        if (field->name->find(":", 0) == 0) {
            print(field->name->substr(1, field->name->length() - 1));
        } else if (std::regex_match(*field->name, std::regex("[0-9]+"))) {
            print("\"" + *field->name + "\"");
        } else {
            print(*field->name);
        }
        print(": ");
        enterVisit(field->type);
    }

    virtual void visit(Record *type) {
        print("{ ");
        for (int i = 0; i < type->fields.size(); i++) {
            enterVisit(type->fields[i]);
            if (i < type->fields.size() - 1) {
                print(", ");
            }
        }
        print(" }");
    }

    // Declarations

    virtual void visit(TypeParam *param) {
        printt();
        print(*param->name);
        print(" = type_member(");
        // TODO bound (fixed|upper|lower): type
        if (param->variance) {
            print(":" + *param->variance);
        }
        printn(")");
        if (param->unchecked) {
            warnUnsupported(static_cast<Node *>(param), "Unsupported `unchecked`");
        }
    }

    void printScope(std::string kind, std::string name, Scope *decl) {
        printl(kind + " " + name);
        indent();
        if (!decl->typeParams.empty()) {
            printl("extend T::Generic");
            for (int i = 0; i < decl->typeParams.size(); i++) {
                printn();
                enterVisit(decl->typeParams[i]);
            }
        }
        for (int i = 0; i < decl->members.size(); i++) {
            if (i > 0 || !decl->typeParams.empty()) {
                printn();
            }
            enterVisit(decl->members[i]);
        }
        dedent();
        printl("end");
    }

    virtual void visit(Class *decl) {
        printScope("class", *decl->name + (decl->parent ? " < " + *decl->parent : ""), decl);
    }

    virtual void visit(Module *decl) {
        if (decl->selfType != NULL) {
            warnUnsupported(static_cast<Node *>(decl->selfType), "Unsupported `module self type`");
        }
        printScope("module", *decl->name, decl);
    }

    virtual void visit(Interface *decl) {
        std::string name = sanitizeInterfaceName(*decl->name);
        printScope("module", name, decl);
    }

    virtual void visit(Extension *decl) { warnUnsupported(static_cast<Node *>(decl), "Unsupported `extension`"); }

    virtual void visit(Const *decl) {
        printt();
        print(*decl->name + " = ");
        enterVisit(decl->type);
        printn();
    }

    virtual void visit(Global *decl) { warnUnsupported(static_cast<Node *>(decl), "Unsupported `global`"); }

    virtual void visit(TypeDecl *decl) {
        printt();
        print(sanitizeAliasName(*decl->name));
        print(" = T.type_alias { ");
        enterVisit(decl->type);
        printn(" }");
    }

    // Class members

    virtual void visit(Alias *alias) {
        printt();
        print("alias ");
        print(sanitizeDefName(*alias->from));
        print(" ");
        print(sanitizeDefName(*alias->to));
        printn();
    }

    virtual void visit(Attr *decl) {}

    virtual void visit(AttrReader *decl) {
        printt();
        print("sig { returns(");
        enterVisit(decl->type);
        printn(") }");
        printl("attr_reader :" + *decl->name);
    }

    virtual void visit(AttrWriter *decl) {
        printt();
        print("sig { params(" + *decl->name + ": ");
        enterVisit(decl->type);
        printn(").void }");
        printl("attr_writer :" + *decl->name);
    }

    virtual void visit(AttrAccessor *decl) {
        printt();
        print("sig { returns(");
        enterVisit(decl->type);
        printn(") }");
        printl("attr_accessor :" + *decl->name);
    }

    void printInclude(std::string kind, Type *type) {
        inInclude = true;
        printt();
        print(kind);
        print(" ");
        enterVisit(type);
        printn();
        inInclude = false;
    }

    virtual void visit(Include *include) { printInclude("include", include->type); }

    virtual void visit(Extend *extend) { printInclude("extend", extend->type); }

    virtual void visit(Prepend *prepend) { printInclude("prepend", prepend->type); }

    virtual void visit(Visibility *decl) {
        warnUnsupported(static_cast<Node *>(decl), "Unsupported `" + *decl->name + "`");
    }

    virtual void visit(Method *decl) {
        if (decl->incompatible) {
            warnUnsupported(static_cast<Node *>(decl), "Unsupported `incompatible`");
        }
        for (int i = 0; i < decl->types.size(); i++) {
            enterVisit(decl->types[i]);
            // break; // TODO handle multiple signatures
        }
        printt();
        print("def ");
        if (decl->singleton) {
            print("self."); // TODO isBoth
        }
        print(sanitizeDefName(*decl->name));
        if (!decl->types.empty()) {
            auto type = decl->types[0];
            if (!type->sig->params.empty() || type->block != NULL) {
                print("(");
                for (int i = 0; i < type->sig->params.size(); i++) {
                    if (type->sig->params[i]->name) {
                        print(sanitizeDefName(*type->sig->params[i]->name));
                    } else {
                        print("arg" + std::to_string(i));
                    }
                    if (type->sig->params[i]->optional) {
                        print(" = nil");
                    }
                    if (i < type->sig->params.size() - 1) {
                        print(", ");
                    }
                }
                if (type->block) {
                    if (!type->sig->params.empty()) {
                        print(", ");
                    }
                    print("_blk");
                    if (type->block->optional) {
                        print(" = nil");
                    }
                }
                print(")");
            }
        }
        printn("; end");
        // TODO singleton
    }

    void printParam(Param *param, int count) {
        if (param->name) {
            print(sanitizeDefName(*param->name));
        } else {
            print("arg" + std::to_string(count));
        }
        print(": ");
        if (param->optional && !dynamic_cast<TypeNilable *>(param->type)) {
            print("T.nilable(");
            enterVisit(param->type);
            print(")");
        } else {
            enterVisit(param->type);
        }
    }

    virtual void visit(MethodType *type) {
        printt();
        print("sig { ");
        if (!type->typeParams.empty()) {
            print("type_parameters(");
            for (int i = 0; i < type->typeParams.size(); i++) {
                typeNames.insert(*type->typeParams[i]->name);
                print(":" + *type->typeParams[i]->name);
                if (i < type->typeParams.size() - 1) {
                    print(", ");
                }
            }
            print(")");
        }
        if (!type->sig->params.empty() || type->block != NULL) {
            if (!type->typeParams.empty()) {
                print(".");
            }
            print("params(");
            for (int i = 0; i < type->sig->params.size(); i++) {
                printParam(type->sig->params[i], i);
                if (i < type->sig->params.size() - 1) {
                    print(", ");
                }
            }
            if (type->block) {
                if (!type->sig->params.empty()) {
                    print(", ");
                }
                print("_blk: ");
                enterVisit(type->block);
            }
            print(").");
        }
        if (TypeVoid *type_void = dynamic_cast<TypeVoid *>(type->sig->ret)) {
            enterVisit(type_void);
        } else {
            print("returns(");
            enterVisit(type->sig->ret);
            print(")");
        }
        printn(" }");
        typeNames.clear();
    }

    virtual void visit(Block *block) {
        if (block->optional) {
            print("T.nilable(");
        }
        enterVisit(block->sig);
        if (block->optional) {
            print(")");
        }
    }

    virtual void visit(Param *param) {}
};
} // namespace rbs_parser

#endif
