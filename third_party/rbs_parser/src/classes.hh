#ifndef RBS_PARSER_AST_CLASSES_HH
#define RBS_PARSER_AST_CLASSES_HH

namespace rbs_parser {
class File;
class Loc;
class Node;
class NodeList;
class Visitor;

class Type;
class TypeAny;
class TypeBool;
class TypeBot;
class TypeClass;
class TypeFalse;
class TypeGeneric;
class TypeInstance;
class TypeInteger;
class TypeIntersection;
class TypeNil;
class TypeNilable;
class TypeProc;
class TypeSelf;
class TypeSelfQ;
class TypeSimple;
class TypeSingleton;
class TypeString;
class TypeSymbol;
class TypeTop;
class TypeTrue;
class TypeTuple;
class TypeUnion;
class TypeUntyped;
class TypeVoid;

class Record;
class RecordField;

class Decl;
class Scope;
class Class;
class Const;
class Extension;
class Global;
class Interface;
class Module;
class TypeDecl;

class Member;
class Alias;
class Attr;
class AttrAccessor;
class AttrReader;
class AttrWriter;
class Extend;
class Include;
class Method;
class Prepend;
class Visibility;

class MethodType;
class Block;
class Param;

class TypeParam;

} // namespace rbs_parser

#endif
