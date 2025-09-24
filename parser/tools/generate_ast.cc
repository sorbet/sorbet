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
    Bool,
    Symbol,
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
    // required positional parameter
    {
        "Param",
        "param",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // inline array with elements
    {
        "Array",
        "array",
        vector<FieldDef>({{"elts", FieldType::NodeVec}}),
    },
    // Pattern matching case with an array: `in [x]`
    {
        "ArrayPattern",
        "array_pattern",
        vector<FieldDef>({{"elts", FieldType::NodeVec}}),
    },
    // Pattern matching case with an array containing a trailing comma: `in [x, ]`
    {
        "ArrayPatternWithTail",
        "array_pattern_with_tail",
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
    // A method call with a blockis modelled as a Send node with a Block as a parent.
    // The `send` is always a `parser::Send` node, and the `params` models the parameters of the block.
    {
        "Block",
        "block",
        vector<FieldDef>({{"send", FieldType::Node}, {"params", FieldType::Node}, {"body", FieldType::Node}}),
    },
    // Wraps a `&foo` parameter in a parameter list
    {
        "BlockParam",
        "blockparam",
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
    // Pattern matching case: `case x; in a; end`
    {
        "CaseMatch",
        "case_match",
        vector<FieldDef>({{"expr", FieldType::Node}, {"inBodies", FieldType::NodeVec}, {"elseBody", FieldType::Node}}),
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
    // Pattern matching case with a const: `in A`
    {
        "ConstPattern",
        "const_pattern",
        vector<FieldDef>({{"scope", FieldType::Node}, {"pattern", FieldType::Node}}),
    },
    // &. "conditional-send"/safe-navigation operator
    {
        "CSend",
        "csend",
        vector<FieldDef>({{"receiver", FieldType::Node},
                          {"method", FieldType::Name},
                          {"methodLoc", FieldType::Loc},
                          {"args", FieldType::NodeVec}}),
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
    // params may be NULL, body does not have to be a block.
    {
        "DefMethod",
        "def",
        vector<FieldDef>({{"declLoc", FieldType::Loc},
                          {"name", FieldType::Name},
                          {"params", FieldType::Node},
                          {"body", FieldType::Node}}),
    },
    // defined?() built-in pseudo-function
    {
        "Defined",
        "defined?",
        vector<FieldDef>({{"value", FieldType::Node}}),
    },
    // def name, instance method def
    {"DefnHead", "defnhead", vector<FieldDef>({{"name", FieldType::Name}})},
    // def <expr>.name singleton-class method def
    {
        "DefS",
        "defs",
        vector<FieldDef>({{"declLoc", FieldType::Loc},
                          {"singleton", FieldType::Node},
                          {"name", FieldType::Name},
                          {"params", FieldType::Node},
                          {"body", FieldType::Node}}),
    },
    // def <expr>.name singleton-class name method def
    {
        "DefsHead",
        "defshead",
        vector<FieldDef>({
            {"definee", FieldType::Node},
            {"name", FieldType::Name},
        }),
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
    // Pattern matching implicit empty `else` block
    {
        "EmptyElse",
        "empty_else",
        vector<FieldDef>(),
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
    // Find pattern
    {
        "FindPattern",
        "find_pattern",
        vector<FieldDef>({{"elements", FieldType::NodeVec}}),
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
    // "..." argument forwarding in definition site
    {
        "ForwardArg",
        "forward_arg",
        vector<FieldDef>(),
    },
    // "..." argument forwarding in call site
    {
        "ForwardedArgs",
        "forwarded_args",
        vector<FieldDef>(),
    },
    // "*" argument forwarding in call site
    {
        "ForwardedRestArg",
        "forwarded_restarg",
        vector<FieldDef>(),
    },
    // "**" argument forwarding in call site
    {
        "ForwardedKwrestArg",
        "forwarded_kwrestarg",
        vector<FieldDef>(),
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
        vector<FieldDef>({{"kwargs", FieldType::Bool}, {"pairs", FieldType::NodeVec}}),
    },
    // Pattern matching hash pattern: `in {x: y}`
    {
        "HashPattern",
        "hash_pattern",
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
    // Pattern matching if guard: `in x if foo`
    {"IfGuard", "if_guard", vector<FieldDef>({{"condition", FieldType::Node}})},
    // .. flip-flop operator inside a conditional
    {
        "IFlipflop",
        "iflipflop",
        vector<FieldDef>({{"left", FieldType::Node}, {"right", FieldType::Node}}),
    },
    // Pattern matching pattern: `in x`
    {
        "InPattern",
        "in_pattern",
        vector<FieldDef>({{"pattern", FieldType::Node}, {"guard", FieldType::Node}, {"body", FieldType::Node}}),
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
    // Keyword nil argument
    {
        "Kwnilarg",
        "kwnilarg",
        vector<FieldDef>(),
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
    // local variable reference
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
    // Pattern matching pattern variable with an alternation: `in A | B`
    {
        "MatchAlt",
        "match_alt",
        vector<FieldDef>({{"left", FieldType::Node}, {"right", FieldType::Node}}),
    },
    // Pattern matching pattern assigning a variable: `in A => x`
    {
        "MatchAs",
        "match_as",
        vector<FieldDef>({{"value", FieldType::Node}, {"as", FieldType::Node}}),
    },
    // [regex literal] =~ value; autovivifies local vars from match groups
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
    // Pattern matching pattern variable with nil
    {
        "MatchNilPattern",
        "match_nil_pattern",
        vector<FieldDef>(),
    },
    // Pattern matching pattern with `=`
    {
        "MatchPattern",
        "match_pattern",
        vector<FieldDef>({{"lhs", FieldType::Node}, {"rhs", FieldType::Node}}),
    },
    // Pattern matching pattern with `in`
    {
        "MatchPatternP",
        "match_pattern_p",
        vector<FieldDef>({{"lhs", FieldType::Node}, {"rhs", FieldType::Node}}),
    },
    // Pattern matching pattern variable with rest
    {
        "MatchRest",
        "match_rest",
        vector<FieldDef>({{"var", FieldType::Node}}),
    },
    // Pattern matching pattern variable
    {
        "MatchVar",
        "match_var",
        vector<FieldDef>({{"name", FieldType::Name}}),
    },
    // Pattern matching pattern with a trailing comma: `in x,`
    {
        "MatchWithTrailingComma",
        "match_with_trailing_comma",
        vector<FieldDef>({{"match", FieldType::Node}}),
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
    // numbered parameters
    {
        "NumParams",
        "numparams",
        vector<FieldDef>({{"decls", FieldType::NodeVec}}),
    },
    // foo += 6 for += and other ops
    {
        "OpAsgn",
        "op_asgn",
        vector<FieldDef>({
            {"left", FieldType::Node},
            {"op", FieldType::Name},
            {"opLoc", FieldType::Loc},
            {"right", FieldType::Node},
        }),
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
    // optional positional parameter
    {
        "OptParam",
        "optparam",
        vector<FieldDef>({{"name", FieldType::Name}, {"nameLoc", FieldType::Loc}, {"default_", FieldType::Node}}),
    },
    // entries of Hash
    {
        "Pair",
        "pair",
        vector<FieldDef>({{"key", FieldType::Node}, {"value", FieldType::Node}}),
    },
    // Pattern matching pined variable: `in [a, b, ^x]`
    {
        "Pin",
        "pin",
        vector<FieldDef>({{"var", FieldType::Node}}),
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
    // because there's a difference between m {|a|} and m{|a,|}
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
    // RBS placeholder for a bind comment
    {
        "RBSPlaceholder",
        "rbs_placeholder",
        vector<FieldDef>({{"kind", FieldType::Name}}),
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
    // A synthetic constant node pre-resolved to a symbol
    {
        "ResolvedConst",
        "resolved_const",
        vector<FieldDef>({{"symbol", FieldType::Symbol}}),
    },
    // A *rest argument inside a Params node.
    {
        "RestParam",
        "restparam",
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
    // Parameter of a method definition or literal block.
    {
        "Params",
        "params",
        vector<FieldDef>({{"params", FieldType::NodeVec}}),
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
        vector<FieldDef>({{"receiver", FieldType::Node},
                          {"method", FieldType::Name},
                          {"methodLoc", FieldType::Loc},
                          {"args", FieldType::NodeVec}}),
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
    // Pattern matching unless guard: `in x unless foo`
    {
        "UnlessGuard",
        "unless_guard",
        vector<FieldDef>({{"condition", FieldType::Node}}),
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

string constructorArgType(FieldType arg) {
    switch (arg) {
        case FieldType::Name:
            return "core::NameRef";
        case FieldType::Symbol:
            return "core::SymbolRef";
        case FieldType::Node:
            return "std::unique_ptr<Node>";
        case FieldType::NodeVec:
            return "NodeVec";
        case FieldType::String:
            return "std::string_view";
        case FieldType::Uint:
            return "uint32_t";
        case FieldType::Loc:
            return "core::LocOffsets";
        case FieldType::Bool:
            return "bool";
    }
}

string fieldType(FieldType arg) {
    switch (arg) {
        case FieldType::Name:
            return "core::NameRef";
        case FieldType::Symbol:
            return "core::SymbolRef";
        case FieldType::Node:
            return "std::unique_ptr<Node>";
        case FieldType::NodeVec:
            return "NodeVec";
        case FieldType::String:
            return "const std::string";
        case FieldType::Uint:
            return "uint32_t";
        case FieldType::Loc:
            return "core::LocOffsets";
        case FieldType::Bool:
            return "bool";
    }
}

void emitNodeHeader(ostream &out, NodeDef &node) {
    out << "class " << node.name << " final : public Node {" << '\n';
    out << "public:" << '\n';

    // generate constructor
    out << "    " << node.name << "(core::LocOffsets loc";
    for (auto &arg : node.fields) {
        out << ", " << constructorArgType(arg.type) << " " << arg.name;
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
    out << "  virtual std::string toJSONWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0);"
        << '\n';
    out << "  virtual std::string toWhitequark(const core::GlobalState &gs, int tabs = 0);" << '\n';
    out << "  virtual std::string nodeName() const;" << '\n';

    out << "};" << '\n';
    out << '\n';
}

void emitNodeClassfile(ostream &out, NodeDef &node) {
    out << "  std::string " << node.name << "::nodeName() const {" << '\n';
    out << "    return \"" << node.name << "\";" << '\n';
    out << "  };" << '\n' << '\n';

    out << "  std::string " << node.name << "::toStringWithTabs(const core::GlobalState &gs, int tabs) const {" << '\n'
        << "    fmt::memory_buffer buf;" << '\n';
    out << "    fmt::format_to(std::back_inserter(buf), \"" << node.name << " {{\\n\");" << '\n';
    // Generate fields
    for (auto &arg : node.fields) {
        if (arg.type == FieldType::Loc) {
            continue;
        }
        out << "    printTabs(buf, tabs + 1);" << '\n';
        switch (arg.type) {
            case FieldType::Name:
                out << "    fmt::format_to(std::back_inserter(buf), \"" << arg.name << " = {}\\n\", " << arg.name
                    << ".showRaw(gs));" << '\n';
                break;
            case FieldType::Symbol:
                out << "    fmt::format_to(std::back_inserter(buf), \"" << arg.name << " = {}\\n\", " << arg.name
                    << ".showRaw(gs));" << '\n';
                break;
            case FieldType::Node:
                out << "    fmt::format_to(std::back_inserter(buf), \"" << arg.name << " = \");\n";
                out << "    printNode(buf, " << arg.name << ", gs, tabs + 1);\n";
                break;
            case FieldType::NodeVec:
                out << "    fmt::format_to(std::back_inserter(buf), \"" << arg.name << " = [\\n\");\n";
                out << "    for (auto &&a: " << arg.name << ") {\n";
                out << "      printTabs(buf, tabs + 2);\n";
                out << "      printNode(buf, a, gs, tabs + 2);\n";
                out << "    }" << '\n';
                out << "    printTabs(buf, tabs + 1);\n";
                out << "    fmt::format_to(std::back_inserter(buf), \"]\\n\");\n";
                break;
            case FieldType::String:
                out << "    fmt::format_to(std::back_inserter(buf), \"" << arg.name << " = \\\"{}\\\"\\n\", "
                    << arg.name << ");\n";
                break;
            case FieldType::Uint:
                out << "    fmt::format_to(std::back_inserter(buf), \"" << arg.name << " = {}\\n\", " << arg.name
                    << ");\n";
                break;
            case FieldType::Loc:
                // Placate the compiler; we skip these
                abort();
                break;
            case FieldType::Bool:
                out << "    fmt::format_to(std::back_inserter(buf), \"" << arg.name << " = {}\\n\", " << arg.name
                    << ");\n";
                break;
        }
    }
    out << "    printTabs(buf, tabs);\n";
    out << "    fmt::format_to(std::back_inserter(buf), \"}}\");\n";
    out << "    return to_string(buf);\n";
    out << "  }" << '\n';
    out << '\n';

    // toJSON
    // TODO: This function would be faster and safer if we used rapidjson or proto to build the JSON.
    // (See tracing.cc and/or any of the proto stuff)
    out << "  std::string " << node.name << "::toJSON(const core::GlobalState &gs, int tabs) {" << '\n'
        << "    fmt::memory_buffer buf;" << '\n';
    out << "    fmt::format_to(std::back_inserter(buf),  \"{{\\n\");\n";
    out << "    printTabs(buf, tabs + 1);" << '\n';
    auto maybeComma = "";
    if (!node.fields.empty()) {
        maybeComma = ",";
    }
    out << R"(    fmt::format_to(std::back_inserter(buf),  "\"type\" : \")" << node.name << "\\\"" << maybeComma
        << "\\n\");\n";
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
                out << "    fmt::format_to(std::back_inserter(buf),  \"\\\"" << arg.name << "\\\" : \\\"{}\\\""
                    << maybeComma << "\\n\", JSON::escape(" << arg.name << ".show(gs)));\n";
                break;
            case FieldType::Symbol:
                out << "    fmt::format_to(std::back_inserter(buf),  \"\\\"" << arg.name << "\\\" : \\\"{}\\\""
                    << maybeComma << "\\n\", JSON::escape(" << arg.name << ".show(gs)));\n";
                break;
            case FieldType::Node:
                out << "    fmt::format_to(std::back_inserter(buf),  \"\\\"" << arg.name << "\\\" : \");\n";
                out << "    printNodeJSON(buf, " << arg.name << ", gs, tabs + 1);\n";
                out << "    fmt::format_to(std::back_inserter(buf),  \"" << maybeComma << "\\n\");\n";
                break;
            case FieldType::NodeVec:
                out << "    fmt::format_to(std::back_inserter(buf),  \"\\\"" << arg.name << "\\\" : [\\n\");\n";
                out << "    int i = -1;" << '\n';
                out << "    for (auto &&a: " << arg.name << ") { \n";
                out << "      i++;\n";
                out << "      printTabs(buf, tabs + 2);\n";
                out << "      printNodeJSON(buf, a, gs, tabs + 2);\n";
                out << "      if (i + 1 < " << arg.name << ".size()) {\n";
                out << "        fmt::format_to(std::back_inserter(buf),  \",\");" << '\n';
                out << "      }" << '\n';
                out << "      fmt::format_to(std::back_inserter(buf),  \"\\n\");\n";
                out << "    }" << '\n';
                out << "    printTabs(buf, tabs + 1);\n";
                out << "    fmt::format_to(std::back_inserter(buf),  \"]" << maybeComma << "\\n\")\n;";
                break;
            case FieldType::String:
                out << "    fmt::format_to(std::back_inserter(buf),  \"\\\"" << arg.name << "\\\" : \\\"{}\\\""
                    << maybeComma << "\\n\", " << arg.name << ");\n";
                break;
            case FieldType::Uint:
                out << R"(    fmt::format_to(std::back_inserter(buf),  "\")" << arg.name << R"(\" : \"{}\")"
                    << maybeComma << "\\n\", " << arg.name << ");\n";
                break;
            case FieldType::Loc:
                // quiet the compiler; we skip Loc fields above
                abort();
            case FieldType::Bool:
                out << "    fmt::format_to(std::back_inserter(buf), \"" << arg.name << " = {}\\n\", " << arg.name
                    << ");\n";
                break;
        }
    }
    out << "    printTabs(buf, tabs);" << '\n';
    out << "    fmt::format_to(std::back_inserter(buf),  \"}}\");\n";
    out << "    return to_string(buf);\n";
    out << "  }" << '\n';
    out << '\n';

    // toJSONWithLocs
    out << "  std::string " << node.name
        << "::toJSONWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs) {" << '\n'
        << "    fmt::memory_buffer buf;" << '\n';
    out << "    fmt::format_to(std::back_inserter(buf),  \"{{\\n\");\n";
    out << "    printTabs(buf, tabs + 1);" << '\n';
    maybeComma = "";
    if (!node.fields.empty()) {
        maybeComma = ",";
    }
    out << R"(    fmt::format_to(std::back_inserter(buf),  "\"type\" : \")" << node.name << "\\\"" << maybeComma
        << "\\n\");\n";
    i = -1;
    // Generate fields
    for (auto &arg : node.fields) {
        i++;
        maybeComma = "";
        if (i < node.fields.size() - 1) {
            maybeComma = ",";
        }
        out << "    printTabs(buf, tabs + 1);" << '\n';
        switch (arg.type) {
            case FieldType::Name:
                out << "    fmt::format_to(std::back_inserter(buf),  \"\\\"" << arg.name << "\\\" : \\\"{}\\\""
                    << maybeComma << "\\n\", JSON::escape(" << arg.name << ".show(gs)));\n";
                break;
            case FieldType::Symbol:
                out << "    fmt::format_to(std::back_inserter(buf),  \"\\\"" << arg.name << "\\\" : \\\"{}\\\""
                    << maybeComma << "\\n\", JSON::escape(" << arg.name << ".show(gs)));\n";
                break;
            case FieldType::Node:
                out << "    fmt::format_to(std::back_inserter(buf),  \"\\\"" << arg.name << "\\\" : \");\n";
                out << "    printNodeJSONWithLocs(buf, " << arg.name << ", gs, file, tabs + 1);\n";
                out << "    fmt::format_to(std::back_inserter(buf),  \"" << maybeComma << "\\n\");\n";
                break;
            case FieldType::NodeVec:
                out << "    fmt::format_to(std::back_inserter(buf),  \"\\\"" << arg.name << "\\\" : [\\n\");\n";
                out << "    int i = -1;" << '\n';
                out << "    for (auto &&a: " << arg.name << ") { \n";
                out << "      i++;\n";
                out << "      printTabs(buf, tabs + 2);\n";
                out << "      printNodeJSONWithLocs(buf, a, gs, file, tabs + 2);\n";
                out << "      if (i + 1 < " << arg.name << ".size()) {\n";
                out << "        fmt::format_to(std::back_inserter(buf),  \",\");" << '\n';
                out << "      }" << '\n';
                out << "      fmt::format_to(std::back_inserter(buf),  \"\\n\");\n";
                out << "    }" << '\n';
                out << "    printTabs(buf, tabs + 1);\n";
                out << "    fmt::format_to(std::back_inserter(buf),  \"]" << maybeComma << "\\n\")\n;";
                break;
            case FieldType::String:
                out << "    fmt::format_to(std::back_inserter(buf),  \"\\\"" << arg.name << "\\\" : \\\"{}\\\""
                    << maybeComma << "\\n\", " << arg.name << ");\n";
                break;
            case FieldType::Uint:
                out << R"(    fmt::format_to(std::back_inserter(buf),  "\")" << arg.name << R"(\" : \"{}\")"
                    << maybeComma << "\\n\", " << arg.name << ");\n";
                break;
            case FieldType::Loc:
                out << "      bool showFull = true;";
                out << R"(    fmt::format_to(std::back_inserter(buf),  "\")" << arg.name << R"(\" : \"{}\")"
                    << maybeComma << "\\n\", "
                    << "core::Loc(file, " << arg.name << ").filePosToString(gs, showFull));\n";
                break;
            case FieldType::Bool:
                out << R"(    fmt::format_to(std::back_inserter(buf),  "\")" << arg.name << R"(\" : \"{}\")"
                    << maybeComma << "\\n\", " << arg.name << ");\n";
                break;
        }
    }
    out << "    printTabs(buf, tabs);" << '\n';
    out << "    fmt::format_to(std::back_inserter(buf),  \"}}\");\n";
    out << "    return to_string(buf);\n";
    out << "  }" << '\n';
    out << '\n';

    // toWhitequark
    out << "  std::string " << node.name << "::toWhitequark(const core::GlobalState &gs, int tabs) {" << '\n'
        << "    fmt::memory_buffer buf;" << '\n';
    out << "    fmt::format_to(std::back_inserter(buf), \"s(:" << node.whitequarkName << "\");" << '\n';
    // Generate fields
    for (auto &arg : node.fields) {
        switch (arg.type) {
            case FieldType::Name:
                if (node.whitequarkName == "str") {
                    out << "    fmt::format_to(std::back_inserter(buf), \", \\\"{}\\\"\", " << arg.name
                        << ".toString(gs));\n";
                } else {
                    out << "    fmt::format_to(std::back_inserter(buf), \", :{}\", JSON::escape(" << arg.name
                        << ".show(gs)));\n";
                }
                break;
            case FieldType::Symbol:
                out << "    fmt::format_to(std::back_inserter(buf), \", :{}\", JSON::escape(" << arg.name
                    << ".show(gs)));\n";
                break;
            case FieldType::Node:
                out << "    fmt::format_to(std::back_inserter(buf), \",\");\n";
                out << "    if (" << arg.name << ") {\n";
                out << "     fmt::format_to(std::back_inserter(buf), \"\\n\");\n";
                out << "     printTabs(buf, tabs + 1);" << '\n';
                out << "     printNodeWhitequark(buf, " << arg.name << ", gs, tabs + 1);\n";
                out << "    } else {\n";
                out << "      fmt::format_to(std::back_inserter(buf), \" nil\");\n";
                out << "    }\n";
                break;
            case FieldType::NodeVec:
                out << "    for (auto &&a: " << arg.name << ") {\n";
                out << "      fmt::format_to(std::back_inserter(buf), \",\");\n";
                out << "      if (a) {\n";
                out << "        fmt::format_to(std::back_inserter(buf), \"\\n\");\n";
                out << "        printTabs(buf, tabs + 1);" << '\n';
                out << "        printNodeWhitequark(buf, a, gs, tabs + 1);\n";
                out << "      } else {\n";
                out << "        fmt::format_to(std::back_inserter(buf), \" nil\");\n";
                out << "      }\n";
                out << "    }\n";
                break;
            case FieldType::String:
                out << "    fmt::format_to(std::back_inserter(buf), \", \\\"{}\\\"\", " << arg.name << ");\n";
                break;
            case FieldType::Uint:
                out << "    fmt::format_to(std::back_inserter(buf), \", {}\", " << arg.name << ");\n";
                break;
            case FieldType::Loc:
                continue;
            case FieldType::Bool:
                continue;
        }
    }

    out << "    fmt::format_to(std::back_inserter(buf), \")\");";
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
