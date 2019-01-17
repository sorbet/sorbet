#include "parser/Builder.h"
#include "core/Names.h"
#include "parser/Dedenter.h"
#include "parser/parser.h"

#include "ruby_parser/builder.hh"
#include "ruby_parser/diagnostic.hh"

#include <algorithm>
#include <typeinfo>

using ruby_parser::foreign_ptr;
using ruby_parser::node_list;
using ruby_parser::self_ptr;
using ruby_parser::token;

using sorbet::core::GlobalState;
using std::make_unique;
using std::string;
using std::string_view;
using std::type_info;
using std::unique_ptr;
using std::vector;

namespace sorbet::parser {

string Dedenter::dedent(string_view str) {
    string out;
    for (auto ch : str) {
        if (spacesToRemove > 0) {
            switch (ch) {
                case ' ':
                    spacesToRemove--;
                    break;
                case '\n':
                    spacesToRemove = dedentLevel;
                    break;
                case '\t': {
                    int indent = dedentLevel - spacesToRemove;
                    spacesToRemove -= (8 - indent % 8);
                    break;
                }
                default:
                    Exception::raise("unexpected whitespace: '", std::to_string(ch), "'");
            }
        } else {
            out.push_back(ch);
        }
    }
    if (!out.empty() && out.back() == '\n') {
        spacesToRemove = dedentLevel;
    }
    return out;
}

class Builder::Impl {
public:
    Impl(GlobalState &gs, core::FileRef file) : gs_(gs), file_(file) {
        this->max_off_ = file.data(gs).source().size();
        foreign_nodes_.emplace_back();
    }

    GlobalState &gs_;
    u2 uniqueCounter_ = 1;
    core::FileRef file_;
    u4 max_off_;
    ruby_parser::base_driver *driver_;

    vector<unique_ptr<Node>> foreign_nodes_;

    u4 clamp(u4 off) {
        return std::min(off, max_off_);
    }

    Loc tok_loc(const token *tok) {
        return Loc{file_, clamp((u4)tok->start()), clamp((u4)tok->end())};
    }

    Loc maybe_loc(unique_ptr<Node> &node) {
        if (node == nullptr) {
            return Loc::none(file_);
        }
        return node->loc;
    }

    Loc tok_loc(const token *begin, const token *end) {
        return Loc{file_, clamp((u4)begin->start()), clamp((u4)end->end())};
    }

    Loc collection_loc(const token *begin, sorbet::parser::NodeVec &elts, const token *end) {
        if (begin != nullptr) {
            ENFORCE(end != nullptr);
            return tok_loc(begin, end);
        }
        ENFORCE(end == nullptr);
        if (elts.empty()) {
            return Loc::none(file_);
        }
        return elts.front()->loc.join(elts.back()->loc);
    }

    Loc collection_loc(sorbet::parser::NodeVec &elts) {
        return collection_loc(nullptr, elts, nullptr);
    }

    unique_ptr<Node> transform_condition(unique_ptr<Node> cond) {
        if (auto *b = parser::cast_node<Begin>(cond.get())) {
            if (b->stmts.size() == 1) {
                b->stmts[0] = transform_condition(std::move(b->stmts[0]));
            }
        } else if (auto *a = parser::cast_node<And>(cond.get())) {
            a->left = transform_condition(std::move(a->left));
            a->right = transform_condition(std::move(a->right));
        } else if (auto *o = parser::cast_node<Or>(cond.get())) {
            o->left = transform_condition(std::move(o->left));
            o->right = transform_condition(std::move(o->right));
        } else if (auto *ir = parser::cast_node<IRange>(cond.get())) {
            return make_unique<IFlipflop>(ir->loc, transform_condition(std::move(ir->from)),
                                          transform_condition(std::move(ir->to)));
        } else if (auto *er = parser::cast_node<ERange>(cond.get())) {
            return make_unique<EFlipflop>(er->loc, transform_condition(std::move(er->from)),
                                          transform_condition(std::move(er->to)));
        } else if (auto *re = parser::cast_node<Regexp>(cond.get())) {
            return make_unique<MatchCurLine>(re->loc, std::move(cond));
        }
        return cond;
    }

    void error(ruby_parser::dclass err, Loc loc) {
        driver_->external_diagnostic(ruby_parser::dlevel::ERROR, err, loc.beginPos(), loc.endPos(), "");
    }

    /* Begin callback methods */

    unique_ptr<Node> accessible(unique_ptr<Node> node) {
        if (auto *id = parser::cast_node<Ident>(node.get())) {
            auto name = id->name.data(gs_);
            ENFORCE(name->kind == core::UTF8);
            if (driver_->lex.is_declared(name->toString(gs_))) {
                return make_unique<LVar>(node->loc, id->name);
            } else {
                return make_unique<Send>(node->loc, nullptr, id->name, sorbet::parser::NodeVec());
            }
        }
        return node;
    }

    unique_ptr<Node> alias(const token *alias, unique_ptr<Node> to, unique_ptr<Node> from) {
        return make_unique<Alias>(tok_loc(alias).join(from->loc), std::move(to), std::move(from));
    }

    unique_ptr<Node> arg(const token *name) {
        return make_unique<Arg>(tok_loc(name), gs_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> args(const token *begin, sorbet::parser::NodeVec args, const token *end, bool check_args) {
        if (begin == nullptr && args.empty() && end == nullptr) {
            return nullptr;
        }
        return make_unique<Args>(collection_loc(begin, args, end), std::move(args));
    }

    unique_ptr<Node> array(const token *begin, sorbet::parser::NodeVec elements, const token *end) {
        return make_unique<Array>(collection_loc(begin, elements, end), std::move(elements));
    }

    unique_ptr<Node> assign(unique_ptr<Node> lhs, const token *eql, unique_ptr<Node> rhs) {
        Loc loc = lhs->loc.join(rhs->loc);

        if (auto *s = parser::cast_node<Send>(lhs.get())) {
            s->args.emplace_back(std::move(rhs));
            return make_unique<Send>(loc, std::move(s->receiver), s->method, std::move(s->args));
        } else if (auto *s = parser::cast_node<CSend>(lhs.get())) {
            s->args.emplace_back(std::move(rhs));
            return make_unique<CSend>(loc, std::move(s->receiver), s->method, std::move(s->args));
        } else {
            return make_unique<Assign>(loc, std::move(lhs), std::move(rhs));
        }
    }

    unique_ptr<Node> assignable(unique_ptr<Node> node) {
        if (auto *id = parser::cast_node<Ident>(node.get())) {
            auto name = id->name.data(gs_);
            driver_->lex.declare(name->toString(gs_));
            return make_unique<LVarLhs>(id->loc, id->name);
        } else if (auto *iv = parser::cast_node<IVar>(node.get())) {
            return make_unique<IVarLhs>(iv->loc, iv->name);
        } else if (auto *c = parser::cast_node<Const>(node.get())) {
            return make_unique<ConstLhs>(c->loc, std::move(c->scope), c->name);
        } else if (auto *cv = parser::cast_node<CVar>(node.get())) {
            return make_unique<CVarLhs>(cv->loc, cv->name);
        } else if (auto *gv = parser::cast_node<GVar>(node.get())) {
            return make_unique<GVarLhs>(gv->loc, gv->name);
        } else if (parser::isa_node<Backref>(node.get()) || parser::isa_node<NthRef>(node.get())) {
            error(ruby_parser::dclass::BackrefAssignment, node->loc);
            return make_unique<Nil>(node->loc);
        } else {
            error(ruby_parser::dclass::InvalidAssignment, node->loc);
            return make_unique<Nil>(node->loc);
        }
    }

    unique_ptr<Node> associate(const token *begin, sorbet::parser::NodeVec pairs, const token *end) {
        return make_unique<Hash>(collection_loc(begin, pairs, end), std::move(pairs));
    }

    unique_ptr<Node> attr_asgn(unique_ptr<Node> receiver, const token *dot, const token *selector) {
        NameRef method = gs_.enterNameUTF8(selector->string() + "=");
        Loc loc = receiver->loc.join(tok_loc(selector));
        if ((dot != nullptr) && dot->string() == "&.") {
            return make_unique<CSend>(loc, std::move(receiver), method, sorbet::parser::NodeVec());
        }
        return make_unique<Send>(loc, std::move(receiver), method, sorbet::parser::NodeVec());
    }

    unique_ptr<Node> back_ref(const token *tok) {
        return make_unique<Backref>(tok_loc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> begin(const token *begin, unique_ptr<Node> body, const token *end) {
        Loc loc;
        if (begin != nullptr) {
            loc = tok_loc(begin).join(tok_loc(end));
        } else {
            loc = body->loc;
        }

        if (body == nullptr) {
            return make_unique<Begin>(loc, sorbet::parser::NodeVec());
        }
        if (auto *b = parser::cast_node<Begin>(body.get())) {
            return body;
        }
        if (auto *m = parser::cast_node<Mlhs>(body.get())) {
            return body;
        }
        sorbet::parser::NodeVec stmts;
        stmts.emplace_back(std::move(body));
        return make_unique<Begin>(loc, std::move(stmts));
    }

    unique_ptr<Node> begin_body(unique_ptr<Node> body, sorbet::parser::NodeVec rescue_bodies, const token *else_tok,
                                unique_ptr<Node> else_, const token *ensure_tok, unique_ptr<Node> ensure) {
        if (!rescue_bodies.empty()) {
            if (else_ == nullptr) {
                body = make_unique<Rescue>(maybe_loc(body).join(rescue_bodies.back()->loc), std::move(body),
                                           std::move(rescue_bodies), nullptr);
            } else {
                body = make_unique<Rescue>(maybe_loc(body).join(else_->loc), std::move(body), std::move(rescue_bodies),
                                           std::move(else_));
            }
        } else if (else_ != nullptr) {
            // TODO: We're losing the source-level information that there was an
            // `else` here.
            sorbet::parser::NodeVec stmts;
            if (body != nullptr) {
                stmts.emplace_back(std::move(body));
            }
            stmts.emplace_back(std::move(else_));
            body = make_unique<Begin>(collection_loc(stmts), std::move(stmts));
        }

        if (ensure_tok != nullptr) {
            Loc loc;
            if (body != nullptr) {
                loc = body->loc;
            } else {
                loc = tok_loc(ensure_tok);
            }
            loc = loc.join(maybe_loc(ensure));
            body = make_unique<Ensure>(loc, std::move(body), std::move(ensure));
        }

        return body;
    }

    unique_ptr<Node> begin_keyword(const token *begin, unique_ptr<Node> body, const token *end) {
        Loc loc = tok_loc(begin).join(tok_loc(end));
        if (body != nullptr) {
            if (auto *b = parser::cast_node<Begin>(body.get())) {
                return make_unique<Kwbegin>(loc, std::move(b->stmts));
            } else {
                sorbet::parser::NodeVec nodes;
                nodes.emplace_back(std::move(body));
                return make_unique<Kwbegin>(loc, std::move(nodes));
            }
        }
        return make_unique<Kwbegin>(loc, sorbet::parser::NodeVec());
    }

    unique_ptr<Node> binary_op(unique_ptr<Node> receiver, const token *oper, unique_ptr<Node> arg) {
        Loc loc = receiver->loc.join(arg->loc);

        sorbet::parser::NodeVec args;
        args.emplace_back(std::move(arg));

        return make_unique<Send>(loc, std::move(receiver), gs_.enterNameUTF8(oper->string()), std::move(args));
    }

    unique_ptr<Node> block(unique_ptr<Node> method_call, const token *begin, unique_ptr<Node> args,
                           unique_ptr<Node> body, const token *end) {
        if (auto *y = parser::cast_node<Yield>(method_call.get())) {
            error(ruby_parser::dclass::BlockGivenToYield, y->loc);
            return make_unique<Yield>(y->loc, sorbet::parser::NodeVec());
        }

        sorbet::parser::NodeVec *callargs = nullptr;
        if (auto *s = parser::cast_node<Send>(method_call.get())) {
            callargs = &s->args;
        }
        if (auto *s = parser::cast_node<CSend>(method_call.get())) {
            callargs = &s->args;
        }
        if (auto *s = parser::cast_node<Super>(method_call.get())) {
            callargs = &s->args;
        }
        if (callargs != nullptr && !callargs->empty()) {
            if (auto *bp = parser::cast_node<BlockPass>(callargs->back().get())) {
                error(ruby_parser::dclass::BlockAndBlockarg, bp->loc);
            }
        }

        Node &n = *method_call;
        const type_info &ty = typeid(n);
        if (ty == typeid(Send) || ty == typeid(CSend) || ty == typeid(Super) || ty == typeid(ZSuper)) {
            return make_unique<Block>(method_call->loc.join(tok_loc(end)), std::move(method_call), std::move(args),
                                      std::move(body));
        }

        sorbet::parser::NodeVec *exprs;
        typecase(method_call.get(), [&](Break *b) { exprs = &b->exprs; },

                 [&](Return *r) { exprs = &r->exprs; },

                 [&](Next *n) { exprs = &n->exprs; },

                 [&](Node *n) { Exception::raise("Unexpected send node: ", n->nodeName()); });

        auto &send = exprs->front();
        Loc block_loc = send->loc.join(tok_loc(end));
        unique_ptr<Node> block = make_unique<Block>(block_loc, std::move(send), std::move(args), std::move(body));
        exprs->front().swap(block);
        return method_call;
    }

    unique_ptr<Node> block_pass(const token *amper, unique_ptr<Node> arg) {
        return make_unique<BlockPass>(tok_loc(amper).join(arg->loc), std::move(arg));
    }

    unique_ptr<Node> blockarg(const token *amper, const token *name) {
        Loc loc = tok_loc(amper);
        NameRef nm;

        if (name != nullptr) {
            loc = loc.join(tok_loc(name));
            nm = gs_.enterNameUTF8(name->string());
        } else {
            nm = gs_.freshNameUnique(core::UniqueNameKind::Parser, core::Names::ampersand(), ++uniqueCounter_);
        }
        return make_unique<Blockarg>(loc, nm);
    }

    unique_ptr<Node> call_lambda(const token *lambda) {
        return make_unique<Send>(tok_loc(lambda), nullptr, core::Names::lambda(), NodeVec());
    }

    unique_ptr<Node> call_method(unique_ptr<Node> receiver, const token *dot, const token *selector,
                                 const token *lparen, sorbet::parser::NodeVec args, const token *rparen) {
        Loc selector_loc, start_loc;
        if (selector != nullptr) {
            selector_loc = tok_loc(selector);
        } else {
            selector_loc = tok_loc(dot);
        }
        if (receiver == nullptr) {
            start_loc = selector_loc;
        } else {
            start_loc = receiver->loc;
        }

        Loc loc;
        if (rparen != nullptr) {
            loc = start_loc.join(tok_loc(rparen));
        } else if (!args.empty()) {
            loc = start_loc.join(args.back()->loc);
        } else {
            loc = start_loc.join(selector_loc);
        }

        NameRef method;
        if (selector == nullptr) {
            method = core::Names::bang();
        } else {
            method = gs_.enterNameUTF8(selector->string());
        }

        if ((dot != nullptr) && dot->string() == "&.") {
            return make_unique<CSend>(loc, std::move(receiver), method, std::move(args));
        } else {
            return make_unique<Send>(loc, std::move(receiver), method, std::move(args));
        }
    }

    unique_ptr<Node> case_(const token *case_, unique_ptr<Node> expr, sorbet::parser::NodeVec when_bodies,
                           const token *else_tok, unique_ptr<Node> else_body, const token *end) {
        return make_unique<Case>(tok_loc(case_).join(tok_loc(end)), std::move(expr), std::move(when_bodies),
                                 std::move(else_body));
    }

    unique_ptr<Node> character(const token *char_) {
        return make_unique<String>(tok_loc(char_), gs_.enterNameUTF8(char_->string()));
    }

    unique_ptr<Node> complex(const token *tok) {
        return make_unique<Complex>(tok_loc(tok), tok->string());
    }

    unique_ptr<Node> compstmt(sorbet::parser::NodeVec nodes) {
        switch (nodes.size()) {
            case 0:
                return nullptr;
            case 1:
                return std::move(nodes.back());
            default:
                return make_unique<Begin>(collection_loc(nodes), std::move(nodes));
        }
    }

    unique_ptr<Node> condition(const token *cond_tok, unique_ptr<Node> cond, const token *then,
                               unique_ptr<Node> if_true, const token *else_, unique_ptr<Node> if_false,
                               const token *end) {
        Loc loc = tok_loc(cond_tok).join(cond->loc);
        if (then != nullptr) {
            loc = loc.join(tok_loc(then));
        }
        if (if_true != nullptr) {
            loc = loc.join(if_true->loc);
        }
        if (else_ != nullptr) {
            loc = loc.join(tok_loc(else_));
        }
        if (if_false != nullptr) {
            loc = loc.join(if_false->loc);
        }
        if (end != nullptr) {
            loc = loc.join(tok_loc(end));
        }
        return make_unique<If>(loc, transform_condition(std::move(cond)), std::move(if_true), std::move(if_false));
    }

    unique_ptr<Node> condition_mod(unique_ptr<Node> if_true, unique_ptr<Node> if_false, unique_ptr<Node> cond) {
        Loc loc = cond->loc;
        if (if_true != nullptr) {
            loc = loc.join(if_true->loc);
        } else {
            loc = loc.join(if_false->loc);
        }
        return make_unique<If>(loc, transform_condition(std::move(cond)), std::move(if_true), std::move(if_false));
    }

    unique_ptr<Node> const_(const token *name) {
        return make_unique<Const>(tok_loc(name), nullptr, gs_.enterNameConstant(name->string()));
    }

    unique_ptr<Node> const_fetch(unique_ptr<Node> scope, const token *colon, const token *name) {
        return make_unique<Const>(scope->loc.join(tok_loc(name)), std::move(scope),
                                  gs_.enterNameConstant(name->string()));
    }

    unique_ptr<Node> const_global(const token *colon, const token *name) {
        return make_unique<Const>(tok_loc(colon).join(tok_loc(name)), make_unique<Cbase>(tok_loc(colon)),
                                  gs_.enterNameConstant(name->string()));
    }

    unique_ptr<Node> const_op_assignable(unique_ptr<Node> node) {
        return node;
    }

    unique_ptr<Node> cvar(const token *tok) {
        return make_unique<CVar>(tok_loc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> dedent_string(unique_ptr<Node> node, size_t dedentLevel) {
        if (dedentLevel == 0) {
            return node;
        }

        Dedenter dedenter(dedentLevel);
        unique_ptr<Node> result;

        typecase(node.get(),

                 [&](String *s) {
                     std::string dedented = dedenter.dedent(s->val.data(gs_)->shortName(gs_));
                     result = make_unique<String>(s->loc, gs_.enterNameUTF8(dedented));
                 },

                 [&](DString *d) {
                     for (auto &p : d->nodes) {
                         if (auto *s = parser::cast_node<String>(p.get())) {
                             std::string dedented = dedenter.dedent(s->val.data(gs_)->shortName(gs_));
                             unique_ptr<Node> newstr = make_unique<String>(s->loc, gs_.enterNameUTF8(dedented));
                             p.swap(newstr);
                         }
                     }
                     result = std::move(node);
                 },

                 [&](Node *n) { Exception::raise("Unexpected dedent node: ", n->nodeName()); });

        return result;
    }

    unique_ptr<Node> def_class(const token *class_, unique_ptr<Node> name, const token *lt_,
                               unique_ptr<Node> superclass, unique_ptr<Node> body, const token *end_) {
        Loc declLoc = tok_loc(class_).join(maybe_loc(name)).join(maybe_loc(superclass));
        Loc loc = tok_loc(class_, end_);

        return make_unique<Class>(loc, declLoc, std::move(name), std::move(superclass), std::move(body));
    }

    unique_ptr<Node> def_method(const token *def, const token *name, unique_ptr<Node> args, unique_ptr<Node> body,
                                const token *end) {
        Loc declLoc = tok_loc(def, name).join(maybe_loc(args));
        Loc loc = tok_loc(def, end);

        return make_unique<DefMethod>(loc, declLoc, gs_.enterNameUTF8(name->string()), std::move(args),
                                      std::move(body));
    }

    unique_ptr<Node> def_module(const token *module, unique_ptr<Node> name, unique_ptr<Node> body, const token *end_) {
        Loc declLoc = tok_loc(module).join(maybe_loc(name));
        Loc loc = tok_loc(module, end_);
        return make_unique<Module>(loc, declLoc, std::move(name), std::move(body));
    }

    unique_ptr<Node> def_sclass(const token *class_, const token *lshft_, unique_ptr<Node> expr, unique_ptr<Node> body,
                                const token *end_) {
        Loc declLoc = tok_loc(class_);
        Loc loc = tok_loc(class_, end_);
        return make_unique<SClass>(loc, declLoc, std::move(expr), std::move(body));
    }

    unique_ptr<Node> def_singleton(const token *def, unique_ptr<Node> definee, const token *dot, const token *name,
                                   unique_ptr<Node> args, unique_ptr<Node> body, const token *end) {
        Loc declLoc = tok_loc(def, name).join(maybe_loc(args));
        Loc loc = tok_loc(def, end);

        // TODO: Ruby interprets (e.g.) def 1.method as a parser error; Do we
        // need to reject it here, or can we defer that until later analysis?
        return make_unique<DefS>(loc, declLoc, std::move(definee), gs_.enterNameUTF8(name->string()), std::move(args),
                                 std::move(body));
    }

    unique_ptr<Node> encoding_literal(const token *tok) {
        return make_unique<EncodingLiteral>(tok_loc(tok));
    }

    unique_ptr<Node> false_(const token *tok) {
        return make_unique<False>(tok_loc(tok));
    }

    unique_ptr<Node> file_literal(const token *tok) {
        return make_unique<FileLiteral>(tok_loc(tok));
    }

    unique_ptr<Node> float_(const token *tok) {
        return make_unique<Float>(tok_loc(tok), tok->string());
    }

    unique_ptr<Node> float_complex(const token *tok) {
        return make_unique<Complex>(tok_loc(tok), tok->string());
    }

    unique_ptr<Node> for_(const token *for_, unique_ptr<Node> iterator, const token *in_, unique_ptr<Node> iteratee,
                          const token *do_, unique_ptr<Node> body, const token *end) {
        return make_unique<For>(tok_loc(for_).join(tok_loc(end)), std::move(iterator), std::move(iteratee),
                                std::move(body));
    }

    unique_ptr<Node> gvar(const token *tok) {
        return make_unique<GVar>(tok_loc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> ident(const token *tok) {
        return make_unique<Ident>(tok_loc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> index(unique_ptr<Node> receiver, const token *lbrack, sorbet::parser::NodeVec indexes,
                           const token *rbrack) {
        return make_unique<Send>(receiver->loc.join(tok_loc(rbrack)), std::move(receiver),
                                 core::Names::squareBrackets(), std::move(indexes));
    }

    unique_ptr<Node> index_asgn(unique_ptr<Node> receiver, const token *lbrack, sorbet::parser::NodeVec indexes,
                                const token *rbrack) {
        return make_unique<Send>(receiver->loc.join(tok_loc(rbrack)), std::move(receiver),
                                 core::Names::squareBracketsEq(), std::move(indexes));
    }

    unique_ptr<Node> integer(const token *tok) {
        return make_unique<Integer>(tok_loc(tok), tok->string());
    }

    unique_ptr<Node> ivar(const token *tok) {
        return make_unique<IVar>(tok_loc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> keyword_break(const token *keyword, const token *lparen, sorbet::parser::NodeVec args,
                                   const token *rparen) {
        Loc loc = tok_loc(keyword).join(collection_loc(lparen, args, rparen));
        return make_unique<Break>(loc, std::move(args));
    }

    unique_ptr<Node> keyword_defined(const token *keyword, unique_ptr<Node> arg) {
        return make_unique<Defined>(tok_loc(keyword).join(arg->loc), std::move(arg));
    }

    unique_ptr<Node> keyword_next(const token *keyword, const token *lparen, sorbet::parser::NodeVec args,
                                  const token *rparen) {
        return make_unique<Next>(tok_loc(keyword).join(collection_loc(lparen, args, rparen)), std::move(args));
    }

    unique_ptr<Node> keyword_redo(const token *keyword) {
        return make_unique<Redo>(tok_loc(keyword));
    }

    unique_ptr<Node> keyword_retry(const token *keyword) {
        return make_unique<Retry>(tok_loc(keyword));
    }

    unique_ptr<Node> keyword_return(const token *keyword, const token *lparen, sorbet::parser::NodeVec args,
                                    const token *rparen) {
        Loc loc = tok_loc(keyword).join(collection_loc(lparen, args, rparen));
        return make_unique<Return>(loc, std::move(args));
    }

    unique_ptr<Node> keyword_super(const token *keyword, const token *lparen, sorbet::parser::NodeVec args,
                                   const token *rparen) {
        Loc loc = tok_loc(keyword);
        Loc argloc = collection_loc(lparen, args, rparen);
        if (argloc.exists()) {
            loc = loc.join(argloc);
        }
        return make_unique<Super>(loc, std::move(args));
    }

    unique_ptr<Node> keyword_yield(const token *keyword, const token *lparen, sorbet::parser::NodeVec args,
                                   const token *rparen) {
        Loc loc = tok_loc(keyword).join(collection_loc(lparen, args, rparen));
        if (!args.empty() && parser::isa_node<BlockPass>(args.back().get())) {
            error(ruby_parser::dclass::BlockGivenToYield, loc);
        }
        return make_unique<Yield>(loc, std::move(args));
    }

    unique_ptr<Node> keyword_zsuper(const token *keyword) {
        return make_unique<ZSuper>(tok_loc(keyword));
    }

    unique_ptr<Node> kwarg(const token *name) {
        return make_unique<Kwarg>(tok_loc(name), gs_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> kwoptarg(const token *name, unique_ptr<Node> value) {
        return make_unique<Kwoptarg>(tok_loc(name).join(value->loc), gs_.enterNameUTF8(name->string()),
                                     std::move(value));
    }

    unique_ptr<Node> kwrestarg(const token *dstar, const token *name) {
        Loc loc = tok_loc(dstar);
        NameRef nm;

        if (name != nullptr) {
            loc = loc.join(tok_loc(name));
            nm = gs_.enterNameUTF8(name->string());
        } else {
            nm = gs_.freshNameUnique(core::UniqueNameKind::Parser, core::Names::starStar(), ++uniqueCounter_);
        }
        return make_unique<Kwrestarg>(loc, nm);
    }

    unique_ptr<Node> kwsplat(const token *dstar, unique_ptr<Node> arg) {
        return make_unique<Kwsplat>(tok_loc(dstar).join(arg->loc), std::move(arg));
    }

    unique_ptr<Node> line_literal(const token *tok) {
        return make_unique<LineLiteral>(tok_loc(tok));
    }

    unique_ptr<Node> logical_and(unique_ptr<Node> lhs, const token *op, unique_ptr<Node> rhs) {
        return make_unique<And>(lhs->loc.join(rhs->loc), std::move(lhs), std::move(rhs));
    }

    unique_ptr<Node> logical_or(unique_ptr<Node> lhs, const token *op, unique_ptr<Node> rhs) {
        return make_unique<Or>(lhs->loc.join(rhs->loc), std::move(lhs), std::move(rhs));
    }

    unique_ptr<Node> loop_until(const token *keyword, unique_ptr<Node> cond, const token *do_, unique_ptr<Node> body,
                                const token *end) {
        return make_unique<Until>(tok_loc(keyword).join(tok_loc(end)), std::move(cond), std::move(body));
    }

    unique_ptr<Node> loop_until_mod(unique_ptr<Node> body, unique_ptr<Node> cond) {
        return make_unique<UntilPost>(body->loc.join(cond->loc), std::move(cond), std::move(body));
    }

    unique_ptr<Node> loop_while(const token *keyword, unique_ptr<Node> cond, const token *do_, unique_ptr<Node> body,
                                const token *end) {
        return make_unique<While>(tok_loc(keyword).join(tok_loc(end)), std::move(cond), std::move(body));
    }

    unique_ptr<Node> loop_while_mod(unique_ptr<Node> body, unique_ptr<Node> cond) {
        return make_unique<WhilePost>(body->loc.join(cond->loc), std::move(cond), std::move(body));
    }

    unique_ptr<Node> match_op(unique_ptr<Node> receiver, const token *oper, unique_ptr<Node> arg) {
        // TODO(nelhage): If the LHS here is a regex literal with (?<...>..)
        // groups, Ruby will autovivify the match groups as locals. If we were
        // to support that, we'd need to analyze that here and call
        // `driver_->lex.declare`.
        Loc loc = receiver->loc.join(arg->loc);
        sorbet::parser::NodeVec args;
        args.emplace_back(std::move(arg));
        return make_unique<Send>(loc, std::move(receiver), gs_.enterNameUTF8(oper->string()), std::move(args));
    }

    unique_ptr<Node> multi_assign(unique_ptr<Node> mlhs, unique_ptr<Node> rhs) {
        return make_unique<Masgn>(mlhs->loc.join(rhs->loc), std::move(mlhs), std::move(rhs));
    }

    unique_ptr<Node> multi_lhs(const token *begin, sorbet::parser::NodeVec items, const token *end) {
        return make_unique<Mlhs>(collection_loc(begin, items, end), std::move(items));
    }

    unique_ptr<Node> multi_lhs1(const token *begin, unique_ptr<Node> item, const token *end) {
        if (auto *mlhs = parser::cast_node<Mlhs>(item.get())) {
            return item;
        }
        sorbet::parser::NodeVec args;
        args.emplace_back(std::move(item));
        return make_unique<Mlhs>(collection_loc(begin, args, end), std::move(args));
    }

    unique_ptr<Node> nil(const token *tok) {
        return make_unique<Nil>(tok_loc(tok));
    }

    unique_ptr<Node> not_op(const token *not_, const token *begin, unique_ptr<Node> receiver, const token *end) {
        if (receiver != nullptr) {
            Loc loc;
            if (end != nullptr) {
                loc = tok_loc(not_).join(tok_loc(end));
            } else {
                loc = tok_loc(not_).join(receiver->loc);
            }
            return make_unique<Send>(loc, std::move(receiver), core::Names::bang(), sorbet::parser::NodeVec());
        }

        ENFORCE(begin != nullptr && end != nullptr);
        auto body = make_unique<Begin>(tok_loc(begin).join(tok_loc(end)), sorbet::parser::NodeVec());
        return make_unique<Send>(tok_loc(not_).join(body->loc), std::move(body), core::Names::bang(),
                                 sorbet::parser::NodeVec());
    }

    unique_ptr<Node> nth_ref(const token *tok) {
        return make_unique<NthRef>(tok_loc(tok), atoi(tok->string().c_str()));
    }

    unique_ptr<Node> op_assign(unique_ptr<Node> lhs, const token *op, unique_ptr<Node> rhs) {
        if (parser::isa_node<Backref>(lhs.get()) || parser::isa_node<NthRef>(lhs.get())) {
            error(ruby_parser::dclass::BackrefAssignment, lhs->loc);
        }

        if (op->string() == "&&") {
            return make_unique<AndAsgn>(lhs->loc.join(rhs->loc), std::move(lhs), std::move(rhs));
        }
        if (op->string() == "||") {
            return make_unique<OrAsgn>(lhs->loc.join(rhs->loc), std::move(lhs), std::move(rhs));
        }
        return make_unique<OpAsgn>(lhs->loc.join(rhs->loc), std::move(lhs), gs_.enterNameUTF8(op->string()),
                                   std::move(rhs));
    }

    unique_ptr<Node> optarg_(const token *name, const token *eql, unique_ptr<Node> value) {
        return make_unique<Optarg>(tok_loc(name).join(value->loc), gs_.enterNameUTF8(name->string()), std::move(value));
    }

    unique_ptr<Node> pair(unique_ptr<Node> key, const token *assoc, unique_ptr<Node> value) {
        return make_unique<Pair>(key->loc.join(value->loc), std::move(key), std::move(value));
    }

    unique_ptr<Node> pair_keyword(const token *key, unique_ptr<Node> value) {
        return make_unique<Pair>(tok_loc(key).join(value->loc),
                                 make_unique<Symbol>(tok_loc(key), gs_.enterNameUTF8(key->string())), std::move(value));
    }

    unique_ptr<Node> pair_quoted(const token *begin, sorbet::parser::NodeVec parts, const token *end,
                                 unique_ptr<Node> value) {
        auto sym = make_unique<DSymbol>(tok_loc(begin).join(tok_loc(end)), std::move(parts));
        return make_unique<Pair>(tok_loc(begin).join(value->loc), std::move(sym), std::move(value));
    }

    unique_ptr<Node> postexe(const token *begin, unique_ptr<Node> node, const token *rbrace) {
        return make_unique<Postexe>(tok_loc(begin).join(tok_loc(rbrace)), std::move(node));
    }

    unique_ptr<Node> preexe(const token *begin, unique_ptr<Node> node, const token *rbrace) {
        return make_unique<Preexe>(tok_loc(begin).join(tok_loc(rbrace)), std::move(node));
    }

    unique_ptr<Node> procarg0(unique_ptr<Node> arg) {
        return arg;
    }

    unique_ptr<Node> range_exclusive(unique_ptr<Node> lhs, const token *oper, unique_ptr<Node> rhs) {
        return make_unique<ERange>(lhs->loc.join(rhs->loc), std::move(lhs), std::move(rhs));
    }

    unique_ptr<Node> range_inclusive(unique_ptr<Node> lhs, const token *oper, unique_ptr<Node> rhs) {
        return make_unique<IRange>(lhs->loc.join(rhs->loc), std::move(lhs), std::move(rhs));
    }

    unique_ptr<Node> rational(const token *tok) {
        return make_unique<Rational>(tok_loc(tok), tok->string());
    }

    unique_ptr<Node> rational_complex(const token *tok) {
        // TODO(nelhage): We're losing this information that this was marked as
        // a Rational in the source.
        return make_unique<Complex>(tok_loc(tok), tok->string());
    }

    unique_ptr<Node> regexp_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end,
                                    unique_ptr<Node> options) {
        Loc loc = tok_loc(begin).join(tok_loc(end)).join(maybe_loc(options));
        return make_unique<Regexp>(loc, std::move(parts), std::move(options));
    }

    unique_ptr<Node> regexp_options(const token *regopt) {
        return make_unique<Regopt>(tok_loc(regopt), regopt->string());
    }

    unique_ptr<Node> rescue_body(const token *rescue, unique_ptr<Node> exc_list, const token *assoc,
                                 unique_ptr<Node> exc_var, const token *then, unique_ptr<Node> body) {
        Loc loc = tok_loc(rescue);
        if (exc_list != nullptr) {
            loc = loc.join(exc_list->loc);
        }
        if (exc_var != nullptr) {
            loc = loc.join(exc_var->loc);
        }
        if (body != nullptr) {
            loc = loc.join(body->loc);
        }
        return make_unique<Resbody>(loc, std::move(exc_list), std::move(exc_var), std::move(body));
    }

    unique_ptr<Node> restarg(const token *star, const token *name) {
        Loc loc = tok_loc(star);
        NameRef nm;

        if (name != nullptr) {
            loc = loc.join(tok_loc(name));
            nm = gs_.enterNameUTF8(name->string());
        } else {
            nm = gs_.freshNameUnique(core::UniqueNameKind::Parser, core::Names::star(), ++uniqueCounter_);
        }
        return make_unique<Restarg>(loc, nm);
    }

    unique_ptr<Node> self_(const token *tok) {
        return make_unique<Self>(tok_loc(tok));
    }

    unique_ptr<Node> shadowarg(const token *name) {
        return make_unique<Shadowarg>(tok_loc(name), gs_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> splat(const token *star, unique_ptr<Node> arg) {
        return make_unique<Splat>(tok_loc(star).join(arg->loc), std::move(arg));
    }

    unique_ptr<Node> splat_mlhs(const token *star, unique_ptr<Node> arg) {
        Loc loc = tok_loc(star).join(maybe_loc(arg));
        return make_unique<SplatLhs>(loc, std::move(arg));
    }

    unique_ptr<Node> string(const token *string_) {
        return make_unique<String>(tok_loc(string_), gs_.enterNameUTF8(string_->string()));
    }

    unique_ptr<Node> string_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        Loc loc = collection_loc(begin, parts, end);
        return make_unique<DString>(loc, std::move(parts));
    }

    unique_ptr<Node> string_internal(const token *string_) {
        return make_unique<String>(tok_loc(string_), gs_.enterNameUTF8(string_->string()));
    }

    unique_ptr<Node> symbol(const token *symbol) {
        return make_unique<Symbol>(tok_loc(symbol), gs_.enterNameUTF8(symbol->string()));
    }

    unique_ptr<Node> symbol_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        return make_unique<DSymbol>(collection_loc(begin, parts, end), std::move(parts));
    }

    unique_ptr<Node> symbol_internal(const token *symbol) {
        return make_unique<Symbol>(tok_loc(symbol), gs_.enterNameUTF8(symbol->string()));
    }

    unique_ptr<Node> symbols_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        Loc loc = collection_loc(begin, parts, end);
        sorbet::parser::NodeVec out_parts;
        out_parts.reserve(parts.size());
        for (auto &p : parts) {
            if (auto *s = parser::cast_node<String>(p.get())) {
                out_parts.emplace_back(make_unique<Symbol>(s->loc, s->val));
            } else if (auto *d = parser::cast_node<DString>(p.get())) {
                out_parts.emplace_back(make_unique<DSymbol>(d->loc, std::move(d->nodes)));
            } else {
                out_parts.emplace_back(std::move(p));
            }
        }
        return make_unique<Array>(loc, std::move(out_parts));
    }

    unique_ptr<Node> ternary(unique_ptr<Node> cond, const token *question, unique_ptr<Node> if_true, const token *colon,
                             unique_ptr<Node> if_false) {
        Loc loc = cond->loc.join(if_false->loc);
        return make_unique<If>(loc, transform_condition(std::move(cond)), std::move(if_true), std::move(if_false));
    }

    unique_ptr<Node> true_(const token *tok) {
        return make_unique<True>(tok_loc(tok));
    }

    unique_ptr<Node> unary_op(const token *oper, unique_ptr<Node> receiver) {
        Loc loc = tok_loc(oper).join(receiver->loc);

        if (auto *num = parser::cast_node<Integer>(receiver.get())) {
            return make_unique<Integer>(loc, oper->string() + num->val);
        }
        if (auto *num = parser::cast_node<Float>(receiver.get())) {
            return make_unique<Float>(loc, oper->string() + num->val);
        }
        if (auto *num = parser::cast_node<Rational>(receiver.get())) {
            Exception::raise("non-implemented");
        }

        NameRef op;
        if (oper->string() == "+") {
            op = core::Names::unaryPlus();
        } else if (oper->string() == "-") {
            op = core::Names::unaryMinus();
        } else {
            op = gs_.enterNameUTF8(oper->string());
        }

        return make_unique<Send>(loc, std::move(receiver), op, sorbet::parser::NodeVec());
    }

    unique_ptr<Node> undef_method(const token *undef, sorbet::parser::NodeVec name_list) {
        Loc loc = tok_loc(undef);
        if (!name_list.empty()) {
            loc = loc.join(name_list.back()->loc);
        }
        return make_unique<Undef>(loc, std::move(name_list));
    }

    unique_ptr<Node> when(const token *when, sorbet::parser::NodeVec patterns, const token *then,
                          unique_ptr<Node> body) {
        Loc loc = tok_loc(when);
        if (body != nullptr) {
            loc = loc.join(body->loc);
        } else if (then != nullptr) {
            loc = loc.join(tok_loc(then));
        } else {
            loc = loc.join(patterns.back()->loc);
        }
        return make_unique<When>(loc, std::move(patterns), std::move(body));
    }

    unique_ptr<Node> word(sorbet::parser::NodeVec parts) {
        Loc loc = collection_loc(parts);
        return make_unique<DString>(loc, std::move(parts));
    }

    unique_ptr<Node> words_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        return make_unique<Array>(collection_loc(begin, parts, end), std::move(parts));
    }

    unique_ptr<Node> xstring_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        return make_unique<XString>(collection_loc(begin, parts, end), std::move(parts));
    }

    /* End callback methods */

    /* methods for marshalling to and from the parser's foreign pointers */

    unique_ptr<Node> cast_node(foreign_ptr node) {
        auto off = reinterpret_cast<size_t>(node);
        if (off == 0) {
            return nullptr;
        }

        ENFORCE(foreign_nodes_[off] != nullptr);
        return std::move(foreign_nodes_[off]);
    }

    foreign_ptr to_foreign(unique_ptr<Node> node) {
        if (node == nullptr) {
            return reinterpret_cast<foreign_ptr>(0);
        }
        foreign_nodes_.emplace_back(std::move(node));
        return reinterpret_cast<foreign_ptr>(foreign_nodes_.size() - 1);
    }

    sorbet::parser::NodeVec convert_node_list(const node_list *cargs) {
        sorbet::parser::NodeVec out;
        if (cargs == nullptr) {
            return out;
        }

        auto *args = const_cast<node_list *>(cargs);
        out.reserve(args->size());
        for (int i = 0; i < args->size(); i++) {
            out.emplace_back(cast_node(args->at(i)));
        }
        return out;
    }
};

Builder::Builder(GlobalState &gs, core::FileRef file) : impl_(new Builder::Impl(gs, file)) {}
Builder::~Builder() = default;

}; // namespace sorbet::parser

namespace {

using sorbet::parser::Builder;
using sorbet::parser::Node;

Builder::Impl *cast_builder(self_ptr builder) {
    return const_cast<Builder::Impl *>(reinterpret_cast<const Builder::Impl *>(builder));
}

foreign_ptr accessible(self_ptr builder, foreign_ptr node) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->accessible(build->cast_node(node)));
}

foreign_ptr alias(self_ptr builder, const token *alias, foreign_ptr to, foreign_ptr from) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->alias(alias, build->cast_node(to), build->cast_node(from)));
}

foreign_ptr arg(self_ptr builder, const token *name) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->arg(name));
}

foreign_ptr args(self_ptr builder, const token *begin, const node_list *args, const token *end, bool check_args) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->args(begin, build->convert_node_list(args), end, check_args));
}

foreign_ptr array(self_ptr builder, const token *begin, const node_list *elements, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->array(begin, build->convert_node_list(elements), end));
}

foreign_ptr assign(self_ptr builder, foreign_ptr lhs, const token *eql, foreign_ptr rhs) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->assign(build->cast_node(lhs), eql, build->cast_node(rhs)));
}

foreign_ptr assignable(self_ptr builder, foreign_ptr node) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->assignable(build->cast_node(node)));
}

foreign_ptr associate(self_ptr builder, const token *begin, const node_list *pairs, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->associate(begin, build->convert_node_list(pairs), end));
}

foreign_ptr attr_asgn(self_ptr builder, foreign_ptr receiver, const token *dot, const token *selector) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->attr_asgn(build->cast_node(receiver), dot, selector));
}

foreign_ptr back_ref(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->back_ref(tok));
}

foreign_ptr begin(self_ptr builder, const token *begin, foreign_ptr body, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->begin(begin, build->cast_node(body), end));
}

foreign_ptr begin_body(self_ptr builder, foreign_ptr body, const node_list *rescue_bodies, const token *else_tok,
                       foreign_ptr else_, const token *ensure_tok, foreign_ptr ensure) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->begin_body(build->cast_node(body), build->convert_node_list(rescue_bodies),
                                               else_tok, build->cast_node(else_), ensure_tok,
                                               build->cast_node(ensure)));
}

foreign_ptr begin_keyword(self_ptr builder, const token *begin, foreign_ptr body, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->begin_keyword(begin, build->cast_node(body), end));
}

foreign_ptr binary_op(self_ptr builder, foreign_ptr receiver, const token *oper, foreign_ptr arg) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->binary_op(build->cast_node(receiver), oper, build->cast_node(arg)));
}

foreign_ptr block(self_ptr builder, foreign_ptr method_call, const token *begin, foreign_ptr args, foreign_ptr body,
                  const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(
        build->block(build->cast_node(method_call), begin, build->cast_node(args), build->cast_node(body), end));
}

foreign_ptr block_pass(self_ptr builder, const token *amper, foreign_ptr arg) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->block_pass(amper, build->cast_node(arg)));
}

foreign_ptr blockarg(self_ptr builder, const token *amper, const token *name) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->blockarg(amper, name));
}

foreign_ptr call_lambda(self_ptr builder, const token *lambda) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->call_lambda(lambda));
}

foreign_ptr call_method(self_ptr builder, foreign_ptr receiver, const token *dot, const token *selector,
                        const token *lparen, const node_list *args, const token *rparen) {
    auto build = cast_builder(builder);
    return build->to_foreign(
        build->call_method(build->cast_node(receiver), dot, selector, lparen, build->convert_node_list(args), rparen));
}

foreign_ptr case_(self_ptr builder, const token *case_, foreign_ptr expr, const node_list *when_bodies,
                  const token *else_tok, foreign_ptr else_body, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->case_(case_, build->cast_node(expr), build->convert_node_list(when_bodies),
                                          else_tok, build->cast_node(else_body), end));
}

foreign_ptr character(self_ptr builder, const token *char_) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->character(char_));
}

foreign_ptr complex(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->complex(tok));
}

foreign_ptr compstmt(self_ptr builder, const node_list *node) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->compstmt(build->convert_node_list(node)));
}

foreign_ptr condition(self_ptr builder, const token *cond_tok, foreign_ptr cond, const token *then, foreign_ptr if_true,
                      const token *else_, foreign_ptr if_false, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->condition(cond_tok, build->cast_node(cond), then, build->cast_node(if_true), else_,
                                              build->cast_node(if_false), end));
}

foreign_ptr condition_mod(self_ptr builder, foreign_ptr if_true, foreign_ptr if_false, foreign_ptr cond) {
    auto build = cast_builder(builder);
    return build->to_foreign(
        build->condition_mod(build->cast_node(if_true), build->cast_node(if_false), build->cast_node(cond)));
}

foreign_ptr const_(self_ptr builder, const token *name) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->const_(name));
}

foreign_ptr const_fetch(self_ptr builder, foreign_ptr scope, const token *colon, const token *name) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->const_fetch(build->cast_node(scope), colon, name));
}

foreign_ptr const_global(self_ptr builder, const token *colon, const token *name) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->const_global(colon, name));
}

foreign_ptr const_op_assignable(self_ptr builder, foreign_ptr node) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->const_op_assignable(build->cast_node(node)));
}

foreign_ptr cvar(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->cvar(tok));
}

foreign_ptr dedent_string(self_ptr builder, foreign_ptr node, size_t dedentLevel) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->dedent_string(build->cast_node(node), dedentLevel));
}

foreign_ptr def_class(self_ptr builder, const token *class_, foreign_ptr name, const token *lt_, foreign_ptr superclass,
                      foreign_ptr body, const token *end_) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->def_class(class_, build->cast_node(name), lt_, build->cast_node(superclass),
                                              build->cast_node(body), end_));
}

foreign_ptr def_method(self_ptr builder, const token *def, const token *name, foreign_ptr args, foreign_ptr body,
                       const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->def_method(def, name, build->cast_node(args), build->cast_node(body), end));
}

foreign_ptr def_module(self_ptr builder, const token *module, foreign_ptr name, foreign_ptr body, const token *end_) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->def_module(module, build->cast_node(name), build->cast_node(body), end_));
}

foreign_ptr def_sclass(self_ptr builder, const token *class_, const token *lshft_, foreign_ptr expr, foreign_ptr body,
                       const token *end_) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->def_sclass(class_, lshft_, build->cast_node(expr), build->cast_node(body), end_));
}

foreign_ptr def_singleton(self_ptr builder, const token *def, foreign_ptr definee, const token *dot, const token *name,
                          foreign_ptr args, foreign_ptr body, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->def_singleton(def, build->cast_node(definee), dot, name, build->cast_node(args),
                                                  build->cast_node(body), end));
}

foreign_ptr encoding_literal(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->encoding_literal(tok));
}

foreign_ptr false_(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->false_(tok));
}

foreign_ptr file_literal(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->file_literal(tok));
}

foreign_ptr float_(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->float_(tok));
}

foreign_ptr float_complex(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->float_complex(tok));
}

foreign_ptr for_(self_ptr builder, const token *for_, foreign_ptr iterator, const token *in_, foreign_ptr iteratee,
                 const token *do_, foreign_ptr body, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->for_(for_, build->cast_node(iterator), in_, build->cast_node(iteratee), do_,
                                         build->cast_node(body), end));
}

foreign_ptr gvar(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->gvar(tok));
}

foreign_ptr ident(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->ident(tok));
}

foreign_ptr index(self_ptr builder, foreign_ptr receiver, const token *lbrack, const node_list *indexes,
                  const token *rbrack) {
    auto build = cast_builder(builder);
    return build->to_foreign(
        build->index(build->cast_node(receiver), lbrack, build->convert_node_list(indexes), rbrack));
}

foreign_ptr index_asgn(self_ptr builder, foreign_ptr receiver, const token *lbrack, const node_list *indexes,
                       const token *rbrack) {
    auto build = cast_builder(builder);
    return build->to_foreign(
        build->index_asgn(build->cast_node(receiver), lbrack, build->convert_node_list(indexes), rbrack));
}

foreign_ptr integer(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->integer(tok));
}

foreign_ptr ivar(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->ivar(tok));
}

foreign_ptr keyword_break(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                          const token *rparen) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->keyword_break(keyword, lparen, build->convert_node_list(args), rparen));
}

foreign_ptr keyword_defined(self_ptr builder, const token *keyword, foreign_ptr arg) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->keyword_defined(keyword, build->cast_node(arg)));
}

foreign_ptr keyword_next(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                         const token *rparen) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->keyword_next(keyword, lparen, build->convert_node_list(args), rparen));
}

foreign_ptr keyword_redo(self_ptr builder, const token *keyword) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->keyword_redo(keyword));
}

foreign_ptr keyword_retry(self_ptr builder, const token *keyword) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->keyword_retry(keyword));
}

foreign_ptr keyword_return(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                           const token *rparen) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->keyword_return(keyword, lparen, build->convert_node_list(args), rparen));
}

foreign_ptr keyword_super(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                          const token *rparen) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->keyword_super(keyword, lparen, build->convert_node_list(args), rparen));
}

foreign_ptr keyword_yield(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                          const token *rparen) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->keyword_yield(keyword, lparen, build->convert_node_list(args), rparen));
}

foreign_ptr keyword_zsuper(self_ptr builder, const token *keyword) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->keyword_zsuper(keyword));
}

foreign_ptr kwarg(self_ptr builder, const token *name) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->kwarg(name));
}

foreign_ptr kwoptarg(self_ptr builder, const token *name, foreign_ptr value) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->kwoptarg(name, build->cast_node(value)));
}

foreign_ptr kwrestarg(self_ptr builder, const token *dstar, const token *name) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->kwrestarg(dstar, name));
}

foreign_ptr kwsplat(self_ptr builder, const token *dstar, foreign_ptr arg) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->kwsplat(dstar, build->cast_node(arg)));
}

foreign_ptr line_literal(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->line_literal(tok));
}

foreign_ptr logical_and(self_ptr builder, foreign_ptr lhs, const token *op, foreign_ptr rhs) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->logical_and(build->cast_node(lhs), op, build->cast_node(rhs)));
}

foreign_ptr logical_or(self_ptr builder, foreign_ptr lhs, const token *op, foreign_ptr rhs) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->logical_or(build->cast_node(lhs), op, build->cast_node(rhs)));
}

foreign_ptr loop_until(self_ptr builder, const token *keyword, foreign_ptr cond, const token *do_, foreign_ptr body,
                       const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->loop_until(keyword, build->cast_node(cond), do_, build->cast_node(body), end));
}

foreign_ptr loop_until_mod(self_ptr builder, foreign_ptr body, foreign_ptr cond) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->loop_until_mod(build->cast_node(body), build->cast_node(cond)));
}

foreign_ptr loop_while(self_ptr builder, const token *keyword, foreign_ptr cond, const token *do_, foreign_ptr body,
                       const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->loop_while(keyword, build->cast_node(cond), do_, build->cast_node(body), end));
}

foreign_ptr loop_while_mod(self_ptr builder, foreign_ptr body, foreign_ptr cond) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->loop_while_mod(build->cast_node(body), build->cast_node(cond)));
}

foreign_ptr match_op(self_ptr builder, foreign_ptr receiver, const token *oper, foreign_ptr arg) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->match_op(build->cast_node(receiver), oper, build->cast_node(arg)));
}

foreign_ptr multi_assign(self_ptr builder, foreign_ptr mlhs, foreign_ptr rhs) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->multi_assign(build->cast_node(mlhs), build->cast_node(rhs)));
}

foreign_ptr multi_lhs(self_ptr builder, const token *begin, const node_list *items, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->multi_lhs(begin, build->convert_node_list(items), end));
}

foreign_ptr multi_lhs1(self_ptr builder, const token *begin, foreign_ptr item, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->multi_lhs1(begin, build->cast_node(item), end));
}

foreign_ptr nil(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->nil(tok));
}

foreign_ptr not_op(self_ptr builder, const token *not_, const token *begin, foreign_ptr receiver, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->not_op(not_, begin, build->cast_node(receiver), end));
}

foreign_ptr nth_ref(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->nth_ref(tok));
}

foreign_ptr op_assign(self_ptr builder, foreign_ptr lhs, const token *op, foreign_ptr rhs) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->op_assign(build->cast_node(lhs), op, build->cast_node(rhs)));
}

foreign_ptr optarg_(self_ptr builder, const token *name, const token *eql, foreign_ptr value) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->optarg_(name, eql, build->cast_node(value)));
}

foreign_ptr pair(self_ptr builder, foreign_ptr key, const token *assoc, foreign_ptr value) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->pair(build->cast_node(key), assoc, build->cast_node(value)));
}

foreign_ptr pair_keyword(self_ptr builder, const token *key, foreign_ptr value) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->pair_keyword(key, build->cast_node(value)));
}

foreign_ptr pair_quoted(self_ptr builder, const token *begin, const node_list *parts, const token *end,
                        foreign_ptr value) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->pair_quoted(begin, build->convert_node_list(parts), end, build->cast_node(value)));
}

foreign_ptr postexe(self_ptr builder, const token *begin, foreign_ptr node, const token *rbrace) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->postexe(begin, build->cast_node(node), rbrace));
}

foreign_ptr preexe(self_ptr builder, const token *begin, foreign_ptr node, const token *rbrace) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->preexe(begin, build->cast_node(node), rbrace));
}

foreign_ptr procarg0(self_ptr builder, foreign_ptr arg) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->procarg0(build->cast_node(arg)));
}

foreign_ptr range_exclusive(self_ptr builder, foreign_ptr lhs, const token *oper, foreign_ptr rhs) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->range_exclusive(build->cast_node(lhs), oper, build->cast_node(rhs)));
}

foreign_ptr range_inclusive(self_ptr builder, foreign_ptr lhs, const token *oper, foreign_ptr rhs) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->range_inclusive(build->cast_node(lhs), oper, build->cast_node(rhs)));
}

foreign_ptr rational(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->rational(tok));
}

foreign_ptr rational_complex(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->rational_complex(tok));
}

foreign_ptr regexp_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end,
                           foreign_ptr options) {
    auto build = cast_builder(builder);
    return build->to_foreign(
        build->regexp_compose(begin, build->convert_node_list(parts), end, build->cast_node(options)));
}

foreign_ptr regexp_options(self_ptr builder, const token *regopt) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->regexp_options(regopt));
}

foreign_ptr rescue_body(self_ptr builder, const token *rescue, foreign_ptr exc_list, const token *assoc,
                        foreign_ptr exc_var, const token *then, foreign_ptr body) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->rescue_body(rescue, build->cast_node(exc_list), assoc, build->cast_node(exc_var),
                                                then, build->cast_node(body)));
}

foreign_ptr restarg(self_ptr builder, const token *star, const token *name) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->restarg(star, name));
}

foreign_ptr self_(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->self_(tok));
}

foreign_ptr shadowarg(self_ptr builder, const token *name) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->shadowarg(name));
}

foreign_ptr splat(self_ptr builder, const token *star, foreign_ptr arg) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->splat(star, build->cast_node(arg)));
}

foreign_ptr splat_mlhs(self_ptr builder, const token *star, foreign_ptr arg) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->splat_mlhs(star, build->cast_node(arg)));
}

foreign_ptr string_(self_ptr builder, const token *string_) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->string(string_));
}

foreign_ptr string_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->string_compose(begin, build->convert_node_list(parts), end));
}

foreign_ptr string_internal(self_ptr builder, const token *string_) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->string_internal(string_));
}

foreign_ptr symbol(self_ptr builder, const token *symbol) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->symbol(symbol));
}

foreign_ptr symbol_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->symbol_compose(begin, build->convert_node_list(parts), end));
}

foreign_ptr symbol_internal(self_ptr builder, const token *symbol) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->symbol_internal(symbol));
}

foreign_ptr symbols_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->symbols_compose(begin, build->convert_node_list(parts), end));
}

foreign_ptr ternary(self_ptr builder, foreign_ptr cond, const token *question, foreign_ptr if_true, const token *colon,
                    foreign_ptr if_false) {
    auto build = cast_builder(builder);
    return build->to_foreign(
        build->ternary(build->cast_node(cond), question, build->cast_node(if_true), colon, build->cast_node(if_false)));
}

foreign_ptr true_(self_ptr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->true_(tok));
}

foreign_ptr unary_op(self_ptr builder, const token *oper, foreign_ptr receiver) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->unary_op(oper, build->cast_node(receiver)));
}

foreign_ptr undef_method(self_ptr builder, const token *undef, const node_list *name_list) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->undef_method(undef, build->convert_node_list(name_list)));
}

foreign_ptr when(self_ptr builder, const token *when, const node_list *patterns, const token *then, foreign_ptr body) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->when(when, build->convert_node_list(patterns), then, build->cast_node(body)));
}

foreign_ptr word(self_ptr builder, const node_list *parts) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->word(build->convert_node_list(parts)));
}

foreign_ptr words_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->words_compose(begin, build->convert_node_list(parts), end));
}

foreign_ptr xstring_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->xstring_compose(begin, build->convert_node_list(parts), end));
}
}; // namespace

namespace sorbet::parser {

unique_ptr<Node> Builder::build(ruby_parser::base_driver *driver) {
    impl_->driver_ = driver;
    return impl_->cast_node(driver->parse(impl_.get()));
}

struct ruby_parser::builder Builder::interface = {
    accessible,
    alias,
    arg,
    args,
    array,
    assign,
    assignable,
    associate,
    attr_asgn,
    back_ref,
    begin,
    begin_body,
    begin_keyword,
    binary_op,
    block,
    block_pass,
    blockarg,
    call_lambda,
    call_method,
    case_,
    character,
    complex,
    compstmt,
    condition,
    condition_mod,
    const_,
    const_fetch,
    const_global,
    const_op_assignable,
    cvar,
    dedent_string,
    def_class,
    def_method,
    def_module,
    def_sclass,
    def_singleton,
    encoding_literal,
    false_,
    file_literal,
    float_,
    float_complex,
    for_,
    gvar,
    ident,
    index,
    index_asgn,
    integer,
    ivar,
    keyword_break,
    keyword_defined,
    keyword_next,
    keyword_redo,
    keyword_retry,
    keyword_return,
    keyword_super,
    keyword_yield,
    keyword_zsuper,
    kwarg,
    kwoptarg,
    kwrestarg,
    kwsplat,
    line_literal,
    logical_and,
    logical_or,
    loop_until,
    loop_until_mod,
    loop_while,
    loop_while_mod,
    match_op,
    multi_assign,
    multi_lhs,
    multi_lhs1,
    nil,
    not_op,
    nth_ref,
    op_assign,
    optarg_,
    pair,
    pair_keyword,
    pair_quoted,
    postexe,
    preexe,
    procarg0,
    range_exclusive,
    range_inclusive,
    rational,
    rational_complex,
    regexp_compose,
    regexp_options,
    rescue_body,
    restarg,
    self_,
    shadowarg,
    splat,
    splat_mlhs,
    string_,
    string_compose,
    string_internal,
    symbol,
    symbol_compose,
    symbol_internal,
    symbols_compose,
    ternary,
    true_,
    unary_op,
    undef_method,
    when,
    word,
    words_compose,
    xstring_compose,
};
} // namespace sorbet::parser
