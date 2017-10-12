#include <algorithm>

#include "../ast.h"
#include "Desugar.h"
#include "ast/ast.h"
#include "common/common.h"

namespace ruby_typer {
namespace ast {
namespace desugar {
using namespace parser;

std::unique_ptr<Expr> stat2Expr(std::unique_ptr<Stat> &expr) {
    Error::check(dynamic_cast<Expr *>(expr.get()));
    return std::unique_ptr<Expr>(dynamic_cast<Expr *>(expr.release()));
}

std::unique_ptr<Expr> stat2Expr(std::unique_ptr<Stat> &&expr) {
    return stat2Expr(expr);
}

std::unique_ptr<Stat> mkSend(std::unique_ptr<Stat> &recv, NameRef fun, std::vector<std::unique_ptr<Stat>> &args) {
    Error::check(dynamic_cast<Expr *>(recv.get()));
    auto recvChecked = stat2Expr(recv);
    auto nargs = std::vector<std::unique_ptr<Expr>>();
    for (auto &a : args) {
        nargs.emplace_back(stat2Expr(a));
    }
    return std::make_unique<Send>(std::move(recvChecked), Names::andAnd(), std::move(nargs));
}

std::unique_ptr<Stat> mkSend1(std::unique_ptr<Stat> &recv, NameRef fun, std::unique_ptr<Stat> &arg1) {
    auto recvChecked = stat2Expr(recv);
    auto argChecked = stat2Expr(arg1);
    auto nargs = std::vector<std::unique_ptr<Expr>>();
    nargs.emplace_back(std::move(argChecked));
    return std::make_unique<Send>(std::move(recvChecked), Names::andAnd(), std::move(nargs));
}

std::unique_ptr<Stat> mkSend1(std::unique_ptr<Stat> &&recv, NameRef fun, std::unique_ptr<Stat> &&arg1) {
    return mkSend1(recv, fun, arg1);
}

std::unique_ptr<Stat> mkSend1(std::unique_ptr<Stat> &&recv, NameRef fun, std::unique_ptr<Stat> &arg1) {
    return mkSend1(recv, fun, arg1);
}

std::unique_ptr<Stat> mkSend1(std::unique_ptr<Stat> &recv, NameRef fun, std::unique_ptr<Stat> &&arg1) {
    return mkSend1(recv, fun, arg1);
}

std::unique_ptr<Stat> mkSend0(std::unique_ptr<Stat> &recv, NameRef fun) {
    auto recvChecked = stat2Expr(recv);
    auto nargs = std::vector<std::unique_ptr<Expr>>();
    return std::make_unique<Send>(std::move(recvChecked), Names::andAnd(), std::move(nargs));
}

std::unique_ptr<Stat> mkSend0(std::unique_ptr<Stat> &&recv, NameRef fun) {
    return mkSend0(recv, fun);
}

std::unique_ptr<Stat> mkIdent(SymbolRef symbol) {
    return std::unique_ptr<Stat>(new Ident(symbol));
}

std::unique_ptr<Stat> mkAssign(SymbolRef symbol, std::unique_ptr<Stat> &rhs) {
    auto lhsChecked = stat2Expr(mkIdent(symbol));
    auto rhsChecked = stat2Expr(rhs);
    return std::make_unique<Assign>(std::move(lhsChecked), std::move(rhsChecked));
}

std::unique_ptr<Stat> mkAssign(SymbolRef symbol, std::unique_ptr<Stat> &&rhs) {
    return mkAssign(symbol, rhs);
}

std::unique_ptr<Stat> mkIf(std::unique_ptr<Stat> &cond, std::unique_ptr<Stat> &thenp, std::unique_ptr<Stat> &elsep) {
    return std::make_unique<If>(stat2Expr(cond), stat2Expr(thenp), stat2Expr(elsep));
}

std::unique_ptr<Stat> mkEmptyTree() {
    return std::make_unique<EmptyTree>();
}

    std::unique_ptr<Stat> mkInsSeq(std::vector<std::unique_ptr<Stat>> &&stats, std::unique_ptr<Expr> &&expr) {
        return std::make_unique<InsSeq>(std::move(stats), std::move(expr));
    }

std::unique_ptr<Stat> Desugar::yesPlease(Context ctx, std::unique_ptr<parser::Node> &what) {
    if (what.get() == nullptr)
        return std::unique_ptr<EmptyTree>();
    std::unique_ptr<Stat> result;
    typecase(what.get(),
             [&](parser::And *a) {
                 auto send =
                     mkSend1(Desugar::yesPlease(ctx, a->left), Names::andAnd(), Desugar::yesPlease(ctx, a->right));
                 result.swap(send);
             },
             [&](parser::AndAsgn *a) {
                 auto recv = Desugar::yesPlease(ctx, a->left);
                 auto arg = Desugar::yesPlease(ctx, a->right);
                 auto argChecked = stat2Expr(arg);
                 if (auto s = dynamic_cast<Send *>(recv.get())) {
                     Error::check(s->args.empty());
                     auto tempSym = ctx.state.newTemporary(UniqueNameKind::Desugar, s->fun, ctx.owner);
                     auto temp = mkAssign(tempSym, std::move(s->recv));
                     recv.release();
                     auto cond = mkSend0(mkIdent(tempSym), s->fun);
                     auto body = mkSend1(mkIdent(tempSym), s->fun.addEq(), arg);
                     auto elsep = mkEmptyTree();
                     auto iff = mkIf(cond, body, elsep);
                     result.swap(iff);
                 } else if (auto i = dynamic_cast<Ident *>(recv.get())) {
                     auto cond = mkIdent(i->symbol);
                     auto body = mkAssign(i->symbol, arg);
                     auto elsep = mkEmptyTree();
                     auto iff = mkIf(cond, body, elsep);
                     result.swap(iff);

                 } else {
                     Error::notImplemented();
                 }
             },
             [&](parser::Send *a) {
                 auto rec = Desugar::yesPlease(ctx, a->receiver);
                 std::vector<std::unique_ptr<Stat>> args;
                 for (auto &stat : a->args) {
                     args.emplace_back(stat2Expr(Desugar::yesPlease(ctx, stat)));
                 };

                 auto send = mkSend(rec, a->method, args);
                 result.swap(send);
             },
             [&](parser::Begin *a) {
                 std::vector<std::unique_ptr<Stat>> stats;
                 auto end = --a->stmts.end();
                 for (auto it = a->stmts.begin(); it != end; ++it) {
                     auto &stat = *it;
                     stats.emplace_back(Desugar::yesPlease(ctx, stat));
                 };
                 auto &last = a->stmts.back();
                 auto expr = Desugar::yesPlease(ctx, last);
                 if (auto *epx = dynamic_cast<Expr *>(expr.get())) {
                     auto exp = stat2Expr(expr);
                     auto block = mkInsSeq(std::move(stats), std::move(exp));
                     result.swap(block);
                 } else {
                     stats.emplace_back(std::move(expr));
                     auto block = mkInsSeq(std::move(stats), stat2Expr(mkEmptyTree()));
                     result.swap(block);
                 }
             },
             [&](parser::Node *) {
                 result.reset(new NotSupported());
             });
    return result;
    /*
     *  // alias bar foo
    {"Alias", vector<FieldDef>({{"from", Node}, {"to", Node}})},
    // Wraps every single definition of argument for methods and blocks
    {"Arg", vector<FieldDef>({{"name", Name}})},
    // Wraps block arg, method arg, and send arg
    {"Args", vector<FieldDef>({{"args", NodeVec}})},
    // inline array with elements
    {"Array", vector<FieldDef>({{"elts", NodeVec}})},
    // Used for $`, $& etc magic regex globals
    {"Backref", vector<FieldDef>({{"name", Name}})},
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
    // Constant assignment
    {"ConstAsgn", vector<FieldDef>({{"scope", Node}, {"name", Name}, {"expr", Node}})},
    // &. "conditional-send"/safe-navigation operator
    {"CSend", vector<FieldDef>({{"receiver", Node}, {"method", Name}, {"args", NodeVec}})},
    // @@foo class variable
    {"CVar", vector<FieldDef>({{"name", Name}})},
    // @@foo class variable assignment
    {"CVarAsgn", vector<FieldDef>({{"name", Name}, {"expr", Node}})},
    // @@foo class variable in the lhs of an Mlhs
    {"CVarLhs", vector<FieldDef>({{"name", Name}})},
    // args may be NULL, body does not have to be a block.
    {"DefMethod", vector<FieldDef>({{"name", Name}, {"args", Node}, {"body", Node}})},
    // defined?() built-in pseudo-function
    {"Defined", vector<FieldDef>({{"value", Node}})},
    // def <expr>.name singleton-class method def
    {"DefS", vector<FieldDef>({{"name", Name}, {"singleton", Node}, {"args", Node}, {"body", Node}})},
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
    // Global variable assignment
    {"GVarAsgn", vector<FieldDef>({{"name", Name}, {"expr", Node}})},
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
    // ivar assign
    {"IVarAsgn", vector<FieldDef>({{"name", Name}, {"expr", Node}})},
    // @rules in `@rules, invalid_rules = ...`
    {"IVarLhs", vector<FieldDef>({{"name", Name}})},
    // Required keyword argument inside an (args)
    {"Kwarg", vector<FieldDef>({{"name", Name}})},
    // explicit `begin` keyword
    {"Kwbegin", vector<FieldDef>({{"vars", NodeVec}})},
    // optional arg with default value provided
    {"Kwoptarg", vector<FieldDef>({{"name", Name}, {"default_", Node}})},
    // **kwargs arg
    {"Kwrestarg", vector<FieldDef>({{"name", Name}})},
    // **foo splat
    {"Kwsplat", vector<FieldDef>({{"expr", Node}})},
    {"Lambda", vector<FieldDef>()},
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
    {"ShadowArg", vector<FieldDef>({{"name", Name}})},
    // *foo splat operator
    {"Splat", vector<FieldDef>({{"var", Node}})},
    // string literal
    {"String", vector<FieldDef>({{"val", String}})},
    {"Super", vector<FieldDef>({{"args", NodeVec}})},
    // symbol literal
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
     */
}
} // namespace desugar
} // namespace ast
} // namespace ruby_typer