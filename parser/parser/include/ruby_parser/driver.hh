#ifndef RUBY_PARSER_DRIVER_HH
#define RUBY_PARSER_DRIVER_HH

#include <memory>
#include <vector>

#include "common/StableStringStorage.h"

#include "builder.hh"
#include "common/common.h"
#include "diagnostic.hh"
#include "lexer.hh"

namespace ruby_parser {

struct node_list {
    node_list() = default;
    node_list(ForeignPtr node) {
        nodes.emplace_back(node);
    }

    node_list &operator=(const ForeignPtr &other) = delete;
    node_list &operator=(ForeignPtr &&other) = delete;

    inline size_t size() const {
        return nodes.size();
    }

    inline void emplace_back(const ForeignPtr &ptr) {
        nodes.emplace_back(ptr);
    }

    inline void push_front(const ForeignPtr &ptr) {
        nodes.insert(nodes.begin(), ptr);
    }

    inline ForeignPtr &at(size_t n) {
        return nodes.at(n);
    }

    inline void concat(node_list *other) {
        nodes.insert(nodes.end(), std::make_move_iterator(other->nodes.begin()),
                     std::make_move_iterator(other->nodes.end()));
    }

protected:
    std::vector<ForeignPtr> nodes;
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
//  * `top > 0`: at leat one numbered parameter in this scope (top being the highest one found)
class max_numparam_stack {
    struct NumparamScope {
        int max;
        ruby_parser::node_list *decls;
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
        return stack.empty() ? false : top()->max < 0;
    }

    // Have we encountered a num param before? (top > 0)
    bool seen_numparams() {
        return stack.empty() ? false : top()->max > 0;
    }

    // Register a numparam in the current scope
    void regis(int numparam, ruby_parser::node_list *decls) {
        if (stack.empty()) {
            push(decls);
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
    void push(ruby_parser::node_list *decls) {
        stack.push_back(NumparamScope{0, decls});
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
    token_t last_token;

    base_driver(ruby_version version, std::string_view source, sorbet::StableStringStorage<> &scratch,
                const struct builder &builder, bool traceLexer);
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
};

class typedruby_release27 : public base_driver {
public:
    typedruby_release27(std::string_view source, sorbet::StableStringStorage<> &scratch, const struct builder &builder,
                        bool traceLexer);
    virtual ForeignPtr parse(SelfPtr self, bool traceParser);
    ~typedruby_release27() {}
};

class typedruby_debug27 : public base_driver {
public:
    typedruby_debug27(std::string_view source, sorbet::StableStringStorage<> &scratch, const struct builder &builder,
                      bool traceLexer);
    virtual ForeignPtr parse(SelfPtr self, bool traceParser);
    ~typedruby_debug27() {}
};

} // namespace ruby_parser

#endif
