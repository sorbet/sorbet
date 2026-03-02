#ifndef RUBY_PARSER_DRIVER_HH
#define RUBY_PARSER_DRIVER_HH

#include <memory>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/types/span.h"
#include "common/StableStringStorage.h"

#include "builder.hh"
#include "common/common.h"
#include "core/LocOffsets.h"
#include "diagnostic.hh"
#include "lexer.hh"

namespace ruby_parser {

struct node_list {
    node_list() = default;
    node_list(ForeignPtr node) {
        nodes_.emplace_back(node);
    }

    node_list &operator=(const ForeignPtr &other) = delete;
    node_list &operator=(ForeignPtr &&other) = delete;

    inline size_t size() const {
        return nodes_.size();
    }

    inline bool empty() const {
        return nodes_.empty();
    }

    inline void emplace_back(const ForeignPtr &ptr) {
        nodes_.emplace_back(ptr);
    }

    inline void push_front(const ForeignPtr &ptr) {
        nodes_.insert(nodes_.begin(), ptr);
    }

    inline absl::Span<const ForeignPtr> nodes() const {
        return absl::MakeSpan(nodes_);
    }

    inline ForeignPtr at(size_t n) const {
        return nodes_.at(n);
    }

    inline void concat(node_list *other) {
        nodes_.insert(nodes_.end(), std::make_move_iterator(other->nodes_.begin()),
                      std::make_move_iterator(other->nodes_.end()));
    }

protected:
    std::vector<ForeignPtr> nodes_;
};

struct delimited_node_list {
    delimited_node_list() = default;
    delimited_node_list(const token_t &begin, node_list *inner, const token_t &end)
        : begin(begin), inner(inner), end(end) {}

    token_t begin = nullptr;
    node_list *inner = nullptr;
    token_t end = nullptr;
};

struct delimited_block {
    delimited_block() = default;
    delimited_block(const token_t &begin, ForeignPtr args, ForeignPtr body, const token_t &end)
        : begin(begin), args(args), body(body), end(end) {}

    token_t begin = nullptr;
    ForeignPtr args = nullptr;
    ForeignPtr body = nullptr;
    token_t end = nullptr;
};

struct node_with_token {
    node_with_token() = default;
    node_with_token(const token_t &token_, ForeignPtr node_) : tok(token_), nod(node_) {}

    token_t tok = nullptr;
    ForeignPtr nod = nullptr;
};

struct node_with_context {
    node_with_context() = default;
    node_with_context(ForeignPtr node, Context context) : node(node), context(context) {}

    ForeignPtr node = nullptr;
    Context context;
};

struct token_with_context {
    token_with_context() = default;
    token_with_context(const token_t &token, Context context) : token(token), context(context) {}

    token_t token = nullptr;
    Context context;
};

struct case_body {
    case_body() = default;
    case_body(node_with_token *else_) : els(else_) {}
    node_list whens;
    node_with_token *els = nullptr;
};

class mempool {
    pool<ruby_parser::node_list, 16> _node_list;
    pool<ruby_parser::delimited_node_list, 32> _delimited_node_list;
    pool<ruby_parser::delimited_block, 32> _delimited_block;
    pool<ruby_parser::node_with_token, 32> _node_with_token;
    pool<ruby_parser::node_with_context, 32> _node_with_context;
    pool<ruby_parser::token_with_context, 32> _token_with_context;
    pool<ruby_parser::case_body, 32> _case_body;
    pool<ruby_parser::state_stack, 8> _stacks;
    friend class base_driver;

public:
    mempool() = default;

    template <typename... Args> ruby_parser::node_list *node_list(Args &&...args) {
        return _node_list.alloc(std::forward<Args>(args)...);
    }

    template <typename... Args> ruby_parser::delimited_node_list *delimited_node_list(Args &&...args) {
        return _delimited_node_list.alloc(std::forward<Args>(args)...);
    }

    template <typename... Args> ruby_parser::delimited_block *delimited_block(Args &&...args) {
        return _delimited_block.alloc(std::forward<Args>(args)...);
    }

    template <typename... Args> ruby_parser::node_with_token *node_with_token(Args &&...args) {
        return _node_with_token.alloc(std::forward<Args>(args)...);
    }

    template <typename... Args> ruby_parser::node_with_context *node_with_context(Args &&...args) {
        return _node_with_context.alloc(std::forward<Args>(args)...);
    }

    template <typename... Args> ruby_parser::token_with_context *token_with_context(Args &&...args) {
        return _token_with_context.alloc(std::forward<Args>(args)...);
    }

    template <typename... Args> ruby_parser::case_body *case_body(Args &&...args) {
        return _case_body.alloc(std::forward<Args>(args)...);
    }
};

/*
 * Stack that holds names of current arguments,
 * i.e. while parsing
 *   def m1(a = (def m2(b = def m3(c = 1); end); end)); end
 *                                   ^
 * stack is [:a, :b, :c]
 *
 * Emulates `p->cur_arg` in MRI's parse.y
 */
class current_arg_stack {
    std::vector<std::string> stack;

public:
    void push(std::string_view value) {
        stack.emplace_back(value);
    }

    void set(std::string_view value) {
        if (stack.empty()) {
            push(value);
        } else {
            stack.back() = value;
        }
    }

    void pop() {
        if (!stack.empty()) {
            stack.pop_back();
        }
    }

    void reset() {
        stack.clear();
    }

    const std::string &top() {
        static std::string empty("");
        return stack.empty() ? empty : stack.back();
    }
};

// This stack stores the numparams used in each scope (ie a module, lambda, etc...)
// For each scope we save the highest numparam used (or 0) and all the nodes in the
// scope referencing a numparam.
//
// The stack has 3 states:
//  * `top = 0`: no parameter (ordinary or numbered) in this scope
//  * `top < 0`: ordinary parameter(s) in this scope
//  * `top > 0`: at least one numbered parameter in this scope (top being the highest one found)
class max_numparam_stack {
    struct NumparamScope {
        // Note: max uses special values:
        // - 0: No parameters
        // - -1: Ordinary parameters
        // - 1-9: Numbered parameters (_1 through _9)
        // - INT_MAX: 'it' parameter
        int max;
        ruby_parser::node_list *decls;
        bool staticContext = false;
        token_t opening_token = nullptr;
    };

    std::vector<NumparamScope> stack;

    friend class base_driver;

public:
    max_numparam_stack() = default;

    // We encountered an ordinary param while visiting the scope (top = -1)
    void set_ordinary_params() {
        if (!stack.empty()) {
            top()->max = -1;
        }
    }

    // Have we encountered an ordinary param before? (top < 0)
    bool seen_ordinary_params() {
        return !stack.empty() && top()->max < 0;
    }

    // Special value to represent 'it' parameter in max field
    static constexpr int IT_PARAM_MARKER = std::numeric_limits<int>::max();

    // Have we encountered a num param before? (top > 0, but not IT_PARAM_MARKER)
    bool seen_numparams() {
        return !stack.empty() && top()->max > 0 && top()->max != IT_PARAM_MARKER;
    }

    // Mark that 'it' parameter is used in the current scope
    void set_it_param() {
        if (!stack.empty()) {
            top()->max = IT_PARAM_MARKER;
        }
    }

    // Have we encountered 'it' parameter before?
    bool seen_it_param() {
        return !stack.empty() && top()->max == IT_PARAM_MARKER;
    }

    // Check if 'it' parameter is used in any outer scope (not current scope)
    bool seen_it_param_in_outer_scope() {
        if (stack.size() <= 1) {
            return false;
        }
        // Check all scopes except the current (top) one
        return absl::c_any_of(absl::MakeConstSpan(stack.data(), stack.size() - 1),
            [](auto const& scope) { return scope.max == IT_PARAM_MARKER; });
    }

    // Register a numparam in the current scope
    void regis(int numparam, ruby_parser::node_list *decls) {
        if (stack.empty()) {
            push(decls, false, nullptr);
        } else {
            top()->decls->concat(decls);
        }
        if (numparam > top()->max) {
            top()->max = numparam;
        }
    }

    // Current scope
    NumparamScope *top() {
        return &stack.back();
    }

    // Push a new scope on the stack (top = 0)
    void push(ruby_parser::node_list *decls, bool staticContext, token_t opening_token) {
        stack.push_back(NumparamScope{0, decls, staticContext, opening_token});
    }

    // Pop the current scope
    void pop() {
        if (!stack.empty()) {
            stack.pop_back();
        }
    }

    std::vector<NumparamScope> stackCopy() {
        return stack;
    }
};

class pattern_variables_stack {
    std::vector<std::set<std::string>> stack;
    friend class base_driver;

public:
    pattern_variables_stack() {
        push();
    };

    void push() {
        std::set<std::string> s;
        stack.emplace_back(s);
    }

    void pop() {
        stack.pop_back();
    }

    void declare(std::string name) {
        stack.back().insert(name);
    }

    bool declared(std::string name) {
        if (stack.empty()) {
            return false;
        }
        return stack.back().find(name) != stack.back().end();
    }
};

class base_driver {
public:
    diagnostics_t diagnostics;
    const builder &build;
    lexer lex;
    mempool alloc;
    current_arg_stack current_arg_stack;
    max_numparam_stack numparam_stack;
    pattern_variables_stack pattern_variables;
    pattern_variables_stack pattern_hash_keys;

    bool pending_error;
    size_t def_level;
    ForeignPtr ast;
    // true when in indentation-aware error recovery mode
    bool indentationAware;
    token_t last_token;

    // Stores a reference to the private yytname_ field from the generated bison parser,
    // which lets us look up pretty token names.
    const char *const *yytname;

    // Stores a reference to the private yytranslate_ field from bison,
    // which lets us convert our token IDs to bison's token IDs so that we can look them up in yytname_.
    //
    // yytranslate_ is a static method, so we don't have to worry about binding `this`
    std::function<unsigned char(int)> yytranslate;

    // Stores a lambda that can be called to clear Bison's current lookahead token.
    std::function<void()> clear_lookahead;

    base_driver(ruby_version version, std::string_view source, sorbet::StableStringStorage<> &scratch,
                const struct builder &builder, bool traceLexer, bool indentationAware);
    virtual ~base_driver() {}
    virtual ForeignPtr parse(SelfPtr self, bool traceParser) = 0;

    bool valid_kwarg_name(const token *name) {
        char c = name->view().at(0);
        return !(c >= 'A' && c <= 'Z');
    }

    ruby_parser::state_stack *copy_stack() {
        return alloc._stacks.alloc(lex.cmdarg);
    }

    void replace_stack(ruby_parser::state_stack *stack) {
        lex.cmdarg = *stack;
    }

    void external_diagnostic(dlevel lvl, dclass cls, size_t begin, size_t end, const std::string &msg) {
        diagnostics.emplace_back(lvl, cls, diagnostic::range(begin, end), msg);
        if (lvl == dlevel::ERROR) {
            pending_error = true;
        }
    }

    template <class... Args> void replace_last_diagnostic(Args &&...args) {
        ENFORCE(!diagnostics.empty());
        diagnostics.pop_back();
        diagnostics.emplace_back(std::forward<Args>(args)...);
    }

    const char *const token_name(token_type type);

    // Find the latest diagnostic reported at `location` with `type` and update it to include the
    // specified `extra_location`.
    void enrich_diagnostic_at(dclass type, diagnostic::range location, diagnostic::range extra_location);

    void enrich_rparen_diagnostic(ruby_parser::token_t rparen, ruby_parser::location extra_location);

    // We've patched the lexer to break compatibility with Ruby w.r.t. method calls
    // for methods sharing names with ruby reserved words. See this PR:
    //   https://github.com/sorbet/sorbet/pull/1993
    // That makes some error recovery better, and other parts worse. This fixes the
    // parts it makes worse.
    //
    // The idea is that there's a rule in the parser below like
    //
    //     stmts: ... | stmts terms stmt_or_begin
    //
    // which means that a list of statements grows by adding a 'terms' (\n or ;) and
    // a 'stmt' to an existing list of `stmts`. But when we consider this example:
    //
    // def foo
    //   x = 1
    //   x.
    // end
    //
    // Due to our lexer change, the parser sees `tIDENTIFIER tDOT kEND`, which
    // means there's no 'terms' in between the `stmts` (`x = 1`) and the
    // (recovered) method call "x.", so another error token shows up, and drops a
    // bunch of the already-parsed program.
    //
    // So in rules where we expect something like that to happen, we can call this
    // function to request that the lexer start again after `tDOT` token in the
    // expr_end state.
    void rewind_and_reset(size_t newPos);

    // Similar in spirit to rewind_and_reset, but resets to the expr_beg state.
    // This is less common--usually it's common to recover from an error by placing the lexer
    // position directly on top of the location of a known punctuation_end token, like `tNL` or
    // `tCOMMA` (which, after being lexed in the expr_end state, the lexer will transition itself to
    // expr_beg). Sometimes that is not possible, and we want to forcibly transition to expr_beg.
    void rewind_and_reset_to_beg(size_t newPos);

    // When recovering from errors, sometimes we'd like to force a production rule to become an
    // error if indentation didn't match, so that hopefully a token that would have been eagerly
    // consumed can be delayed until a later production rule.
    //
    // This helper essentially implements a crude form of backtracking.
    //
    // Use `force = true` to ignore checking indentationAware. This probably is only ok to use if
    // you're implementing an "`error` rule in disguise" kind of rule (i.e., reducing tokens
    // manually for the purpose of emitting an error).
    void rewind_if_dedented(token_t token, token_t kEND, bool force = false);

    // A helper similar to rewind_if_dedented above, but rewinds if the tokens are on different
    // lines, regardless of the indentation.
    bool rewind_if_different_line(token_t token1, token_t token2);

    // Like rewind_if_dedented above, but has special logic for detecting when the decrease in
    // indentation happens midway through a list of statements. The properly indented statements are
    // returned, and the lexer is reset to a point where the improperly indented statements will be
    // re-lexed and re-parsed.
    //
    // Most params are self explanatory.
    //
    // `headerEndPos` should be the end of the line immediately before the start of the body. It's
    // what we rewind to if even the first thing in the body is improperly indented.
    ForeignPtr rewind_and_munge_body_if_dedented(SelfPtr self, token_t defToken, size_t headerEndPos, ForeignPtr body,
                                                 token_t bodyStartToken, token_t lastTokBeforeDedent, token_t endToken);

    void local_push();
    void local_pop();

    std::vector<sorbet::core::LocOffsets> get_comment_locations() const {
        std::vector<sorbet::core::LocOffsets> result;
        result.reserve(lex.comment_locations.size());

        for (const auto &loc : lex.comment_locations) {
            result.emplace_back(loc.first, loc.second);
        }

        return result;
    }

private:
    void rewind_to_tok_start(token_t endToken);
    void rewind_to_tok_end(token_t endToken);
};

class typedruby_release : public base_driver {
public:
    typedruby_release(std::string_view source, sorbet::StableStringStorage<> &scratch, const struct builder &builder,
                      bool traceLexer, bool indentationAware);
    virtual ForeignPtr parse(SelfPtr self, bool traceParser);
    ~typedruby_release() {}
};

class typedruby_debug : public base_driver {
public:
    typedruby_debug(std::string_view source, sorbet::StableStringStorage<> &scratch, const struct builder &builder,
                    bool traceLexer, bool indentationAware);
    virtual ForeignPtr parse(SelfPtr self, bool traceParser);
    ~typedruby_debug() {}
};

} // namespace ruby_parser

#endif
