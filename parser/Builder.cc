#include "parser/Builder.h"
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

using ruby_typer::core::GlobalState;
using ruby_typer::core::UTF8Desc;
using std::make_unique;
using std::string;
using std::type_info;
using std::unique_ptr;
using std::vector;

namespace ruby_typer {
namespace parser {

string Dedenter::dedent(const string &str) {
    string out;
    for (auto ch : str) {
        if (spaces_to_remove > 0) {
            switch (ch) {
                case ' ':
                    spaces_to_remove--;
                    break;
                case '\n':
                    spaces_to_remove = dedent_level;
                    break;
                case '\t': {
                    int indent = dedent_level - spaces_to_remove;
                    spaces_to_remove -= (8 - indent % 8);
                    break;
                }
                default:
                    Error::raise("unexpected whitespace: '", ch, "'");
            }
        } else {
            out.push_back(ch);
        }
    }
    if (!out.empty() && out.back() == '\n') {
        spaces_to_remove = dedent_level;
    }
    return out;
}

class Builder::Impl {
public:
    Impl(GlobalState &gs, core::FileRef file) : gs_(gs), file_(file) {
        this->max_off_ = file.file(gs).source().to;
        foreign_nodes_.emplace_back();
    }

    GlobalState &gs_;
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

    Loc loc_join(Loc begin, Loc end) {
        if (begin.is_none()) {
            return end;
        }
        if (end.is_none()) {
            return begin;
        }
        Error::check(begin.file == end.file);

        return Loc{file_, begin.begin_pos, end.end_pos};
    }

    Loc collection_loc(const token *begin, ruby_typer::parser::NodeVec &elts, const token *end) {
        if (begin != nullptr) {
            DEBUG_ONLY(Error::check(end != nullptr));
            return tok_loc(begin, end);
        }
        DEBUG_ONLY(Error::check(end == nullptr));
        if (elts.empty()) {
            return Loc::none(file_);
        }
        return loc_join(elts.front()->loc, elts.back()->loc);
    }

    Loc collection_loc(ruby_typer::parser::NodeVec &elts) {
        return collection_loc(nullptr, elts, nullptr);
    }

    unique_ptr<Node> transform_condition(unique_ptr<Node> cond) {
        if (Begin *b = parser::cast_node<Begin>(cond.get())) {
            if (b->stmts.size() == 1) {
                b->stmts[0] = transform_condition(move(b->stmts[0]));
            }
        } else if (And *a = parser::cast_node<And>(cond.get())) {
            a->left = transform_condition(move(a->left));
            a->right = transform_condition(move(a->right));
        } else if (Or *o = parser::cast_node<Or>(cond.get())) {
            o->left = transform_condition(move(o->left));
            o->right = transform_condition(move(o->right));
        } else if (IRange *ir = parser::cast_node<IRange>(cond.get())) {
            return make_unique<IFlipflop>(ir->loc, transform_condition(move(ir->from)),
                                          transform_condition(move(ir->to)));
        } else if (ERange *er = parser::cast_node<ERange>(cond.get())) {
            return make_unique<EFlipflop>(er->loc, transform_condition(move(er->from)),
                                          transform_condition(move(er->to)));
        } else if (Regexp *re = parser::cast_node<Regexp>(cond.get())) {
            return make_unique<MatchCurLine>(re->loc, move(cond));
        }
        return cond;
    }

    void error(ruby_parser::dclass err, Loc loc) {
        driver_->external_diagnostic(ruby_parser::dlevel::ERROR, err, loc.begin_pos, loc.end_pos, "");
    }

    /* Begin callback methods */

    unique_ptr<Node> accessible(unique_ptr<Node> node) {
        if (Ident *id = parser::cast_node<Ident>(node.get())) {
            core::Name &name = id->name.name(gs_);
            DEBUG_ONLY(Error::check(name.kind == core::UTF8));
            if (driver_->lex.is_declared(name.toString(gs_))) {
                return make_unique<LVar>(node->loc, id->name);
            } else {
                return make_unique<Send>(node->loc, nullptr, id->name, ruby_typer::parser::NodeVec());
            }
        }
        return node;
    }

    unique_ptr<Node> alias(const token *alias, unique_ptr<Node> to, unique_ptr<Node> from) {
        return make_unique<Alias>(loc_join(tok_loc(alias), from->loc), move(to), move(from));
    }

    unique_ptr<Node> arg(const token *name) {
        return make_unique<Arg>(tok_loc(name), gs_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> args(const token *begin, ruby_typer::parser::NodeVec args, const token *end, bool check_args) {
        if (begin == nullptr && args.empty() && end == nullptr) {
            return nullptr;
        }
        return make_unique<Args>(collection_loc(begin, args, end), move(args));
    }

    unique_ptr<Node> array(const token *begin, ruby_typer::parser::NodeVec elements, const token *end) {
        return make_unique<Array>(collection_loc(begin, elements, end), move(elements));
    }

    unique_ptr<Node> assign(unique_ptr<Node> lhs, const token *eql, unique_ptr<Node> rhs) {
        Loc loc = loc_join(lhs->loc, rhs->loc);

        if (Send *s = parser::cast_node<Send>(lhs.get())) {
            s->args.push_back(move(rhs));
            return make_unique<Send>(loc, move(s->receiver), s->method, move(s->args));
        } else if (CSend *s = parser::cast_node<CSend>(lhs.get())) {
            s->args.push_back(move(rhs));
            return make_unique<CSend>(loc, move(s->receiver), s->method, move(s->args));
        } else {
            return make_unique<Assign>(loc, move(lhs), move(rhs));
        }
    }

    unique_ptr<Node> assignable(unique_ptr<Node> node) {
        if (Ident *id = parser::cast_node<Ident>(node.get())) {
            core::Name &name = id->name.name(gs_);
            driver_->lex.declare(name.toString(gs_));
            return make_unique<LVarLhs>(id->loc, id->name);
        } else if (IVar *iv = parser::cast_node<IVar>(node.get())) {
            return make_unique<IVarLhs>(iv->loc, iv->name);
        } else if (Const *c = parser::cast_node<Const>(node.get())) {
            return make_unique<ConstLhs>(c->loc, move(c->scope), c->name);
        } else if (CVar *cv = parser::cast_node<CVar>(node.get())) {
            return make_unique<CVarLhs>(cv->loc, cv->name);
        } else if (GVar *gv = parser::cast_node<GVar>(node.get())) {
            return make_unique<GVarLhs>(gv->loc, gv->name);
        } else if (parser::cast_node<Backref>(node.get()) != nullptr ||
                   parser::cast_node<NthRef>(node.get()) != nullptr) {
            error(ruby_parser::dclass::BackrefAssignment, node->loc);
            return make_unique<Nil>(node->loc);
        } else {
            error(ruby_parser::dclass::InvalidAssignment, node->loc);
            return make_unique<Nil>(node->loc);
        }
    }

    unique_ptr<Node> associate(const token *begin, ruby_typer::parser::NodeVec pairs, const token *end) {
        return make_unique<Hash>(collection_loc(begin, pairs, end), move(pairs));
    }

    unique_ptr<Node> attr_asgn(unique_ptr<Node> receiver, const token *dot, const token *selector) {
        NameRef method = gs_.enterNameUTF8(selector->string() + "=");
        Loc loc = loc_join(receiver->loc, tok_loc(selector));
        if ((dot != nullptr) && dot->string() == "&.") {
            return make_unique<CSend>(loc, move(receiver), method, ruby_typer::parser::NodeVec());
        }
        return make_unique<Send>(loc, move(receiver), method, ruby_typer::parser::NodeVec());
    }

    unique_ptr<Node> back_ref(const token *tok) {
        return make_unique<Backref>(tok_loc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> begin(const token *begin, unique_ptr<Node> body, const token *end) {
        Loc loc;
        if (begin != nullptr) {
            loc = loc_join(tok_loc(begin), tok_loc(end));
        } else {
            loc = body->loc;
        }

        if (body == nullptr) {
            return make_unique<Begin>(loc, ruby_typer::parser::NodeVec());
        }
        if (Begin *b = parser::cast_node<Begin>(body.get())) {
            return body;
        }
        if (Mlhs *m = parser::cast_node<Mlhs>(body.get())) {
            return body;
        }
        ruby_typer::parser::NodeVec stmts;
        stmts.push_back(move(body));
        return make_unique<Begin>(loc, move(stmts));
    }

    unique_ptr<Node> begin_body(unique_ptr<Node> body, ruby_typer::parser::NodeVec rescue_bodies, const token *else_tok,
                                unique_ptr<Node> else_, const token *ensure_tok, unique_ptr<Node> ensure) {
        if (!rescue_bodies.empty()) {
            if (else_ == nullptr) {
                body = make_unique<Rescue>(loc_join(maybe_loc(body), rescue_bodies.back()->loc), move(body),
                                           move(rescue_bodies), nullptr);
            } else {
                body = make_unique<Rescue>(loc_join(maybe_loc(body), else_->loc), move(body), move(rescue_bodies),
                                           move(else_));
            }
        } else if (else_ != nullptr) {
            // TODO: We're losing the source-level information that there was an
            // `else` here.
            ruby_typer::parser::NodeVec stmts;
            if (body != nullptr) {
                stmts.push_back(move(body));
            }
            stmts.push_back(move(else_));
            body = make_unique<Begin>(collection_loc(stmts), move(stmts));
        }

        if (ensure_tok != nullptr) {
            Loc loc;
            if (body != nullptr) {
                loc = body->loc;
            } else {
                loc = tok_loc(ensure_tok);
            }
            loc = loc_join(loc, maybe_loc(ensure));
            body = make_unique<Ensure>(loc, move(body), move(ensure));
        }

        return body;
    }

    unique_ptr<Node> begin_keyword(const token *begin, unique_ptr<Node> body, const token *end) {
        Loc loc = loc_join(tok_loc(begin), tok_loc(end));
        if (body != nullptr) {
            if (Begin *b = parser::cast_node<Begin>(body.get())) {
                return make_unique<Kwbegin>(loc, move(b->stmts));
            } else {
                ruby_typer::parser::NodeVec nodes;
                nodes.push_back(move(body));
                return make_unique<Kwbegin>(loc, move(nodes));
            }
        }
        return make_unique<Kwbegin>(loc, ruby_typer::parser::NodeVec());
    }

    unique_ptr<Node> binary_op(unique_ptr<Node> receiver, const token *oper, unique_ptr<Node> arg) {
        Loc loc = loc_join(receiver->loc, arg->loc);

        ruby_typer::parser::NodeVec args;
        args.push_back(move(arg));

        return make_unique<Send>(loc, move(receiver), gs_.enterNameUTF8(oper->string()), move(args));
    }

    unique_ptr<Node> block(unique_ptr<Node> method_call, const token *begin, unique_ptr<Node> args,
                           unique_ptr<Node> body, const token *end) {
        if (Yield *y = parser::cast_node<Yield>(method_call.get())) {
            error(ruby_parser::dclass::BlockGivenToYield, y->loc);
            return make_unique<Yield>(y->loc, ruby_typer::parser::NodeVec());
        }

        ruby_typer::parser::NodeVec *callargs = nullptr;
        if (Send *s = parser::cast_node<Send>(method_call.get())) {
            callargs = &s->args;
        }
        if (CSend *s = parser::cast_node<CSend>(method_call.get())) {
            callargs = &s->args;
        }
        if (Super *s = parser::cast_node<Super>(method_call.get())) {
            callargs = &s->args;
        }
        if (callargs != nullptr && !callargs->empty()) {
            if (BlockPass *bp = parser::cast_node<BlockPass>(callargs->back().get())) {
                error(ruby_parser::dclass::BlockAndBlockarg, bp->loc);
            }
        }

        Node &n = *method_call;
        const type_info &ty = typeid(n);
        if (ty == typeid(Send) || ty == typeid(CSend) || ty == typeid(Super) || ty == typeid(ZSuper)) {
            return make_unique<Block>(loc_join(method_call->loc, tok_loc(end)), move(method_call), move(args),
                                      move(body));
        }

        ruby_typer::parser::NodeVec *exprs;
        typecase(method_call.get(), [&](Break *b) { exprs = &b->exprs; },

                 [&](Return *r) { exprs = &r->exprs; },

                 [&](Next *n) { exprs = &n->exprs; },

                 [&](Node *n) { Error::raise("Unexpected send node: ", n->nodeName()); });

        auto &send = exprs->front();
        Loc block_loc = loc_join(send->loc, tok_loc(end));
        unique_ptr<Node> block = make_unique<Block>(block_loc, move(send), move(args), move(body));
        exprs->front().swap(block);
        return method_call;
    }

    unique_ptr<Node> block_pass(const token *amper, unique_ptr<Node> arg) {
        return make_unique<BlockPass>(loc_join(tok_loc(amper), arg->loc), move(arg));
    }

    unique_ptr<Node> blockarg(const token *amper, const token *name) {
        Loc loc = tok_loc(amper);
        NameRef nm;

        if (name != nullptr) {
            loc = loc_join(loc, tok_loc(name));
            nm = gs_.enterNameUTF8(name->string());
        } else {
            nm = gs_.freshNameUnique(core::UniqueNameKind::Parser, core::Names::ampersand());
        }
        return make_unique<Blockarg>(loc, nm);
    }

    unique_ptr<Node> call_lambda(const token *lambda) {
        return make_unique<Send>(tok_loc(lambda), nullptr, core::Names::lambda(), NodeVec());
    }

    unique_ptr<Node> call_method(unique_ptr<Node> receiver, const token *dot, const token *selector,
                                 const token *lparen, ruby_typer::parser::NodeVec args, const token *rparen) {
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
            loc = loc_join(start_loc, tok_loc(rparen));
        } else if (!args.empty()) {
            loc = loc_join(start_loc, args.back()->loc);
        } else {
            loc = loc_join(start_loc, selector_loc);
        }

        NameRef method;
        if (selector == nullptr) {
            method = core::Names::bang();
        } else {
            method = gs_.enterNameUTF8(selector->string());
        }

        if ((dot != nullptr) && dot->string() == "&.") {
            return make_unique<CSend>(loc, move(receiver), method, move(args));
        } else {
            return make_unique<Send>(loc, move(receiver), method, move(args));
        }
    }

    unique_ptr<Node> case_(const token *case_, unique_ptr<Node> expr, ruby_typer::parser::NodeVec when_bodies,
                           const token *else_tok, unique_ptr<Node> else_body, const token *end) {
        return make_unique<Case>(loc_join(tok_loc(case_), tok_loc(end)), move(expr), move(when_bodies),
                                 move(else_body));
    }

    unique_ptr<Node> character(const token *char_) {
        return make_unique<String>(tok_loc(char_), gs_.enterNameUTF8(char_->string()));
    }

    unique_ptr<Node> complex(const token *tok) {
        return make_unique<Complex>(tok_loc(tok), tok->string());
    }

    unique_ptr<Node> compstmt(ruby_typer::parser::NodeVec nodes) {
        switch (nodes.size()) {
            case 0:
                return nullptr;
            case 1:
                return move(nodes.back());
            default:
                return make_unique<Begin>(collection_loc(nodes), move(nodes));
        }
    }

    unique_ptr<Node> condition(const token *cond_tok, unique_ptr<Node> cond, const token *then,
                               unique_ptr<Node> if_true, const token *else_, unique_ptr<Node> if_false,
                               const token *end) {
        Loc loc = loc_join(tok_loc(cond_tok), cond->loc);
        if (then != nullptr) {
            loc = loc_join(loc, tok_loc(then));
        }
        if (if_true != nullptr) {
            loc = loc_join(loc, if_true->loc);
        }
        if (else_ != nullptr) {
            loc = loc_join(loc, tok_loc(else_));
        }
        if (if_false != nullptr) {
            loc = loc_join(loc, if_false->loc);
        }
        if (end != nullptr) {
            loc = loc_join(loc, tok_loc(end));
        }
        return make_unique<If>(loc, transform_condition(move(cond)), move(if_true), move(if_false));
    }

    unique_ptr<Node> condition_mod(unique_ptr<Node> if_true, unique_ptr<Node> if_false, unique_ptr<Node> cond) {
        Loc loc = cond->loc;
        if (if_true != nullptr) {
            loc = loc_join(loc, if_true->loc);
        } else {
            loc = loc_join(loc, if_false->loc);
        }
        return make_unique<If>(loc, transform_condition(move(cond)), move(if_true), move(if_false));
    }

    unique_ptr<Node> const_(const token *name) {
        return make_unique<Const>(tok_loc(name), nullptr, gs_.enterNameConstant(name->string()));
    }

    unique_ptr<Node> const_fetch(unique_ptr<Node> scope, const token *colon, const token *name) {
        return make_unique<Const>(loc_join(scope->loc, tok_loc(name)), move(scope),
                                  gs_.enterNameConstant(name->string()));
    }

    unique_ptr<Node> const_global(const token *colon, const token *name) {
        return make_unique<Const>(loc_join(tok_loc(colon), tok_loc(name)), make_unique<Cbase>(tok_loc(colon)),
                                  gs_.enterNameConstant(name->string()));
    }

    unique_ptr<Node> const_op_assignable(unique_ptr<Node> node) {
        return node;
    }

    unique_ptr<Node> cvar(const token *tok) {
        return make_unique<CVar>(tok_loc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> dedent_string(unique_ptr<Node> node, size_t dedent_level) {
        if (dedent_level == 0) {
            return node;
        }

        Dedenter dedenter(dedent_level);
        unique_ptr<Node> result;

        typecase(node.get(),

                 [&](String *s) {
                     std::string dedented = dedenter.dedent(s->val.name(gs_).toString(gs_));
                     result = make_unique<String>(s->loc, gs_.enterNameUTF8(dedented));
                 },

                 [&](DString *d) {
                     for (auto &p : d->nodes) {
                         if (String *s = parser::cast_node<String>(p.get())) {
                             std::string dedented = dedenter.dedent(s->val.name(gs_).toString(gs_));
                             unique_ptr<Node> newstr = make_unique<String>(s->loc, gs_.enterNameUTF8(dedented));
                             p.swap(newstr);
                         }
                     }
                     result = move(node);
                 },

                 [&](Node *n) { Error::raise("Unexpected dedent node: ", n->nodeName()); });

        return result;
    }

    unique_ptr<Node> def_class(const token *class_, unique_ptr<Node> name, const token *lt_,
                               unique_ptr<Node> superclass, unique_ptr<Node> body, const token *end_) {
        return make_unique<Class>(tok_loc(class_, end_), move(name), move(superclass), move(body));
    }

    unique_ptr<Node> def_method(const token *def, const token *name, unique_ptr<Node> args, unique_ptr<Node> body,
                                const token *end) {
        return make_unique<DefMethod>(tok_loc(def, end), gs_.enterNameUTF8(name->string()), move(args), move(body));
    }

    unique_ptr<Node> def_module(const token *module, unique_ptr<Node> name, unique_ptr<Node> body, const token *end_) {
        return make_unique<Module>(tok_loc(module, end_), move(name), move(body));
    }

    unique_ptr<Node> def_sclass(const token *class_, const token *lshft_, unique_ptr<Node> expr, unique_ptr<Node> body,
                                const token *end_) {
        return make_unique<SClass>(loc_join(tok_loc(class_), tok_loc(end_)), move(expr), move(body));
    }

    unique_ptr<Node> def_singleton(const token *def, unique_ptr<Node> definee, const token *dot, const token *name,
                                   unique_ptr<Node> args, unique_ptr<Node> body, const token *end) {
        Loc loc = loc_join(tok_loc(def), tok_loc(end));
        // TODO: Ruby interprets (e.g.) def 1.method as a parser error; Do we
        // need to reject it here, or can we defer that until later analysis?
        return make_unique<DefS>(loc, move(definee), gs_.enterNameUTF8(name->string()), move(args), move(body));
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
        return make_unique<For>(loc_join(tok_loc(for_), tok_loc(end)), move(iterator), move(iteratee), move(body));
    }

    unique_ptr<Node> gvar(const token *tok) {
        return make_unique<GVar>(tok_loc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> ident(const token *tok) {
        return make_unique<Ident>(tok_loc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> index(unique_ptr<Node> receiver, const token *lbrack, ruby_typer::parser::NodeVec indexes,
                           const token *rbrack) {
        return make_unique<Send>(loc_join(receiver->loc, tok_loc(rbrack)), move(receiver),
                                 core::Names::squareBrackets(), move(indexes));
    }

    unique_ptr<Node> index_asgn(unique_ptr<Node> receiver, const token *lbrack, ruby_typer::parser::NodeVec indexes,
                                const token *rbrack) {
        ruby_typer::parser::NodeVec args;
        return make_unique<Send>(loc_join(receiver->loc, tok_loc(rbrack)), move(receiver),
                                 core::Names::squareBracketsEq(), move(args));
    }

    unique_ptr<Node> integer(const token *tok) {
        return make_unique<Integer>(tok_loc(tok), tok->string());
    }

    unique_ptr<Node> ivar(const token *tok) {
        return make_unique<IVar>(tok_loc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> keyword_break(const token *keyword, const token *lparen, ruby_typer::parser::NodeVec args,
                                   const token *rparen) {
        Loc loc = loc_join(tok_loc(keyword), collection_loc(lparen, args, rparen));
        return make_unique<Break>(loc, move(args));
    }

    unique_ptr<Node> keyword_defined(const token *keyword, unique_ptr<Node> arg) {
        return make_unique<Defined>(loc_join(tok_loc(keyword), arg->loc), move(arg));
    }

    unique_ptr<Node> keyword_next(const token *keyword, const token *lparen, ruby_typer::parser::NodeVec args,
                                  const token *rparen) {
        return make_unique<Next>(loc_join(tok_loc(keyword), collection_loc(lparen, args, rparen)), move(args));
    }

    unique_ptr<Node> keyword_redo(const token *keyword) {
        return make_unique<Redo>(tok_loc(keyword));
    }

    unique_ptr<Node> keyword_retry(const token *keyword) {
        return make_unique<Retry>(tok_loc(keyword));
    }

    unique_ptr<Node> keyword_return(const token *keyword, const token *lparen, ruby_typer::parser::NodeVec args,
                                    const token *rparen) {
        Loc loc = loc_join(tok_loc(keyword), collection_loc(lparen, args, rparen));
        return make_unique<Return>(loc, move(args));
    }

    unique_ptr<Node> keyword_super(const token *keyword, const token *lparen, ruby_typer::parser::NodeVec args,
                                   const token *rparen) {
        Loc loc = tok_loc(keyword);
        Loc argloc = collection_loc(lparen, args, rparen);
        if (!argloc.is_none()) {
            loc = loc_join(loc, argloc);
        }
        return make_unique<Super>(loc, move(args));
    }

    unique_ptr<Node> keyword_yield(const token *keyword, const token *lparen, ruby_typer::parser::NodeVec args,
                                   const token *rparen) {
        Loc loc = loc_join(tok_loc(keyword), collection_loc(lparen, args, rparen));
        if (!args.empty() && parser::cast_node<BlockPass>(args.back().get()) != nullptr) {
            error(ruby_parser::dclass::BlockGivenToYield, loc);
        }
        return make_unique<Yield>(loc, move(args));
    }

    unique_ptr<Node> keyword_zsuper(const token *keyword) {
        return make_unique<ZSuper>(tok_loc(keyword));
    }

    unique_ptr<Node> kwarg(const token *name) {
        return make_unique<Kwarg>(tok_loc(name), gs_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> kwoptarg(const token *name, unique_ptr<Node> value) {
        return make_unique<Kwoptarg>(loc_join(tok_loc(name), value->loc), gs_.enterNameUTF8(name->string()),
                                     move(value));
    }

    unique_ptr<Node> kwrestarg(const token *dstar, const token *name) {
        Loc loc = tok_loc(dstar);
        NameRef nm;

        if (name != nullptr) {
            loc = loc_join(loc, tok_loc(name));
            nm = gs_.enterNameUTF8(name->string());
        } else {
            nm = gs_.freshNameUnique(core::UniqueNameKind::Parser, core::Names::starStar());
        }
        return make_unique<Kwrestarg>(loc, nm);
    }

    unique_ptr<Node> kwsplat(const token *dstar, unique_ptr<Node> arg) {
        return make_unique<Kwsplat>(loc_join(tok_loc(dstar), arg->loc), move(arg));
    }

    unique_ptr<Node> line_literal(const token *tok) {
        return make_unique<LineLiteral>(tok_loc(tok));
    }

    unique_ptr<Node> logical_and(unique_ptr<Node> lhs, const token *op, unique_ptr<Node> rhs) {
        return make_unique<And>(loc_join(lhs->loc, rhs->loc), move(lhs), move(rhs));
    }

    unique_ptr<Node> logical_or(unique_ptr<Node> lhs, const token *op, unique_ptr<Node> rhs) {
        return make_unique<Or>(loc_join(lhs->loc, rhs->loc), move(lhs), move(rhs));
    }

    unique_ptr<Node> loop_until(const token *keyword, unique_ptr<Node> cond, const token *do_, unique_ptr<Node> body,
                                const token *end) {
        return make_unique<Until>(loc_join(tok_loc(keyword), tok_loc(end)), move(cond), move(body));
    }

    unique_ptr<Node> loop_until_mod(unique_ptr<Node> body, unique_ptr<Node> cond) {
        return make_unique<UntilPost>(loc_join(body->loc, cond->loc), move(cond), move(body));
    }

    unique_ptr<Node> loop_while(const token *keyword, unique_ptr<Node> cond, const token *do_, unique_ptr<Node> body,
                                const token *end) {
        return make_unique<While>(loc_join(tok_loc(keyword), tok_loc(end)), move(cond), move(body));
    }

    unique_ptr<Node> loop_while_mod(unique_ptr<Node> body, unique_ptr<Node> cond) {
        return make_unique<WhilePost>(loc_join(body->loc, cond->loc), move(cond), move(body));
    }

    unique_ptr<Node> match_op(unique_ptr<Node> receiver, const token *oper, unique_ptr<Node> arg) {
        // TODO(nelhage): If the LHS here is a regex literal with (?<...>..)
        // groups, Ruby will autovivify the match groups as locals. If we were
        // to support that, we'd need to analyze that here and call
        // `driver_->lex.declare`.
        Loc loc = loc_join(receiver->loc, arg->loc);
        ruby_typer::parser::NodeVec args;
        args.emplace_back(move(arg));
        return make_unique<Send>(loc, move(receiver), gs_.enterNameUTF8(oper->string()), move(args));
    }

    unique_ptr<Node> multi_assign(unique_ptr<Node> mlhs, unique_ptr<Node> rhs) {
        return make_unique<Masgn>(loc_join(mlhs->loc, rhs->loc), move(mlhs), move(rhs));
    }

    unique_ptr<Node> multi_lhs(const token *begin, ruby_typer::parser::NodeVec items, const token *end) {
        return make_unique<Mlhs>(collection_loc(begin, items, end), move(items));
    }

    unique_ptr<Node> multi_lhs1(const token *begin, unique_ptr<Node> item, const token *end) {
        if (Mlhs *mlhs = parser::cast_node<Mlhs>(item.get())) {
            return item;
        }
        ruby_typer::parser::NodeVec args;
        args.emplace_back(move(item));
        return make_unique<Mlhs>(collection_loc(begin, args, end), move(args));
    }

    unique_ptr<Node> negate(const token *uminus, unique_ptr<Node> numeric) {
        Loc loc = loc_join(tok_loc(uminus), numeric->loc);
        if (Integer *i = parser::cast_node<Integer>(numeric.get())) {
            return make_unique<Integer>(loc, "-" + i->val);
        }
        if (Float *i = parser::cast_node<Float>(numeric.get())) {
            return make_unique<Float>(loc, "-" + i->val);
        }
        if (Rational *r = parser::cast_node<Rational>(numeric.get())) {
            return make_unique<Float>(loc, "-" + r->val);
        }
        Error::raise("unexpected numeric type: ", numeric->nodeName());
    }

    unique_ptr<Node> nil(const token *tok) {
        return make_unique<Nil>(tok_loc(tok));
    }

    unique_ptr<Node> not_op(const token *not_, const token *begin, unique_ptr<Node> receiver, const token *end) {
        if (receiver != nullptr) {
            Loc loc;
            if (end != nullptr) {
                loc = loc_join(tok_loc(not_), tok_loc(end));
            } else {
                loc = loc_join(tok_loc(not_), receiver->loc);
            }
            return make_unique<Send>(loc, move(receiver), core::Names::bang(), ruby_typer::parser::NodeVec());
        }

        DEBUG_ONLY(Error::check(begin != nullptr && end != nullptr));
        auto body = make_unique<Begin>(loc_join(tok_loc(begin), tok_loc(end)), ruby_typer::parser::NodeVec());
        return make_unique<Send>(loc_join(tok_loc(not_), body->loc), move(body), core::Names::bang(),
                                 ruby_typer::parser::NodeVec());
    }

    unique_ptr<Node> nth_ref(const token *tok) {
        return make_unique<NthRef>(tok_loc(tok), atoi(tok->string().c_str()));
    }

    unique_ptr<Node> op_assign(unique_ptr<Node> lhs, const token *op, unique_ptr<Node> rhs) {
        if (parser::cast_node<Backref>(lhs.get()) != nullptr || parser::cast_node<NthRef>(lhs.get()) != nullptr) {
            error(ruby_parser::dclass::BackrefAssignment, lhs->loc);
        }

        if (op->string() == "&&") {
            return make_unique<AndAsgn>(loc_join(lhs->loc, rhs->loc), move(lhs), move(rhs));
        }
        if (op->string() == "||") {
            return make_unique<OrAsgn>(loc_join(lhs->loc, rhs->loc), move(lhs), move(rhs));
        }
        return make_unique<OpAsgn>(loc_join(lhs->loc, rhs->loc), move(lhs), gs_.enterNameUTF8(op->string()), move(rhs));
    }

    unique_ptr<Node> optarg_(const token *name, const token *eql, unique_ptr<Node> value) {
        return make_unique<Optarg>(loc_join(tok_loc(name), value->loc), gs_.enterNameUTF8(name->string()), move(value));
    }

    unique_ptr<Node> pair(unique_ptr<Node> key, const token *assoc, unique_ptr<Node> value) {
        return make_unique<Pair>(loc_join(key->loc, value->loc), move(key), move(value));
    }

    unique_ptr<Node> pair_keyword(const token *key, unique_ptr<Node> value) {
        return make_unique<Pair>(loc_join(tok_loc(key), value->loc),
                                 make_unique<Symbol>(tok_loc(key), gs_.enterNameUTF8(key->string())), move(value));
    }

    unique_ptr<Node> pair_quoted(const token *begin, ruby_typer::parser::NodeVec parts, const token *end,
                                 unique_ptr<Node> value) {
        auto sym = make_unique<DSymbol>(loc_join(tok_loc(begin), tok_loc(end)), move(parts));
        return make_unique<Pair>(loc_join(tok_loc(begin), value->loc), move(sym), move(value));
    }

    unique_ptr<Node> postexe(const token *begin, unique_ptr<Node> node, const token *rbrace) {
        return make_unique<Postexe>(loc_join(tok_loc(begin), tok_loc(rbrace)), move(node));
    }

    unique_ptr<Node> preexe(const token *begin, unique_ptr<Node> node, const token *rbrace) {
        return make_unique<Preexe>(loc_join(tok_loc(begin), tok_loc(rbrace)), move(node));
    }

    unique_ptr<Node> procarg0(unique_ptr<Node> arg) {
        return arg;
    }

    unique_ptr<Node> range_exclusive(unique_ptr<Node> lhs, const token *oper, unique_ptr<Node> rhs) {
        return make_unique<ERange>(loc_join(lhs->loc, rhs->loc), move(lhs), move(rhs));
    }

    unique_ptr<Node> range_inclusive(unique_ptr<Node> lhs, const token *oper, unique_ptr<Node> rhs) {
        return make_unique<IRange>(loc_join(lhs->loc, rhs->loc), move(lhs), move(rhs));
    }

    unique_ptr<Node> rational(const token *tok) {
        return make_unique<Rational>(tok_loc(tok), tok->string());
    }

    unique_ptr<Node> rational_complex(const token *tok) {
        // TODO(nelhage): We're losing this information that this was marked as
        // a Rational in the source.
        return make_unique<Complex>(tok_loc(tok), tok->string());
    }

    unique_ptr<Node> regexp_compose(const token *begin, ruby_typer::parser::NodeVec parts, const token *end,
                                    unique_ptr<Node> options) {
        Loc loc = loc_join(loc_join(tok_loc(begin), tok_loc(end)), maybe_loc(options));
        return make_unique<Regexp>(loc, move(parts), move(options));
    }

    unique_ptr<Node> regexp_options(const token *regopt) {
        return make_unique<Regopt>(tok_loc(regopt), regopt->string());
    }

    unique_ptr<Node> rescue_body(const token *rescue, unique_ptr<Node> exc_list, const token *assoc,
                                 unique_ptr<Node> exc_var, const token *then, unique_ptr<Node> body) {
        Loc loc = tok_loc(rescue);
        if (exc_list != nullptr) {
            loc = loc_join(loc, exc_list->loc);
        }
        if (exc_var != nullptr) {
            loc = loc_join(loc, exc_var->loc);
        }
        if (body != nullptr) {
            loc = loc_join(loc, body->loc);
        }
        return make_unique<Resbody>(loc, move(exc_list), move(exc_var), move(body));
    }

    unique_ptr<Node> restarg(const token *star, const token *name) {
        Loc loc = tok_loc(star);
        NameRef nm;

        if (name != nullptr) {
            loc = loc_join(loc, tok_loc(name));
            nm = gs_.enterNameUTF8(name->string());
        } else {
            nm = gs_.freshNameUnique(core::UniqueNameKind::Parser, core::Names::star());
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
        return make_unique<Splat>(loc_join(tok_loc(star), arg->loc), move(arg));
    }

    unique_ptr<Node> splat_mlhs(const token *star, unique_ptr<Node> arg) {
        Loc loc = loc_join(tok_loc(star), maybe_loc(arg));
        return make_unique<SplatLhs>(loc, move(arg));
    }

    unique_ptr<Node> string(const token *string_) {
        return make_unique<String>(tok_loc(string_), gs_.enterNameUTF8(string_->string()));
    }

    unique_ptr<Node> string_compose(const token *begin, ruby_typer::parser::NodeVec parts, const token *end) {
        Loc loc = collection_loc(begin, parts, end);
        return make_unique<DString>(loc, move(parts));
    }

    unique_ptr<Node> string_internal(const token *string_) {
        return make_unique<String>(tok_loc(string_), gs_.enterNameUTF8(string_->string()));
    }

    unique_ptr<Node> symbol(const token *symbol) {
        return make_unique<Symbol>(tok_loc(symbol), gs_.enterNameUTF8(symbol->string()));
    }

    unique_ptr<Node> symbol_compose(const token *begin, ruby_typer::parser::NodeVec parts, const token *end) {
        return make_unique<DSymbol>(collection_loc(begin, parts, end), move(parts));
    }

    unique_ptr<Node> symbol_internal(const token *symbol) {
        return make_unique<Symbol>(tok_loc(symbol), gs_.enterNameUTF8(symbol->string()));
    }

    unique_ptr<Node> symbols_compose(const token *begin, ruby_typer::parser::NodeVec parts, const token *end) {
        Loc loc = collection_loc(begin, parts, end);
        ruby_typer::parser::NodeVec out_parts;
        out_parts.reserve(parts.size());
        for (auto &p : parts) {
            if (String *s = parser::cast_node<String>(p.get())) {
                out_parts.emplace_back(make_unique<Symbol>(s->loc, s->val));
            } else if (DString *d = parser::cast_node<DString>(p.get())) {
                out_parts.emplace_back(make_unique<DSymbol>(d->loc, move(d->nodes)));
            } else {
                out_parts.push_back(move(p));
            }
        }
        return make_unique<Array>(loc, move(out_parts));
    }

    unique_ptr<Node> ternary(unique_ptr<Node> cond, const token *question, unique_ptr<Node> if_true, const token *colon,
                             unique_ptr<Node> if_false) {
        Loc loc = loc_join(cond->loc, if_false->loc);
        return make_unique<If>(loc, transform_condition(move(cond)), move(if_true), move(if_false));
    }

    unique_ptr<Node> tr_any(const token *special) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_arg_instance(unique_ptr<Node> base, ruby_typer::parser::NodeVec types, const token *end) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_array(const token *begin, unique_ptr<Node> type_, const token *end) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_cast(const token *begin, unique_ptr<Node> expr, const token *colon, unique_ptr<Node> type_,
                             const token *end) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_class(const token *special) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_consubtype(unique_ptr<Node> sub, unique_ptr<Node> super_) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_conunify(unique_ptr<Node> a, unique_ptr<Node> b) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_cpath(unique_ptr<Node> cpath) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_genargs(const token *begin, ruby_typer::parser::NodeVec genargs,
                                ruby_typer::parser::NodeVec constraints, const token *end) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_gendecl(unique_ptr<Node> cpath, const token *begin, ruby_typer::parser::NodeVec genargs,
                                ruby_typer::parser::NodeVec constraints, const token *end) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_gendeclarg(const token *tok, unique_ptr<Node> constraint) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_geninst(unique_ptr<Node> cpath, const token *begin, ruby_typer::parser::NodeVec genargs,
                                const token *end) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_hash(const token *begin, unique_ptr<Node> key_type, const token *assoc,
                             unique_ptr<Node> value_type, const token *end) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_instance(const token *special) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_ivardecl(const token *def, const token *name, unique_ptr<Node> type_) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_nil(const token *nil) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_nillable(const token *tilde, unique_ptr<Node> type_) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_or(unique_ptr<Node> a, unique_ptr<Node> b) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_paren(const token *begin, unique_ptr<Node> node, const token *end) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_proc(const token *begin, unique_ptr<Node> args, const token *end) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_prototype(unique_ptr<Node> genargs, unique_ptr<Node> args, unique_ptr<Node> return_type) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_returnsig(const token *arrow, unique_ptr<Node> ret) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_self(const token *special) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_tuple(const token *begin, ruby_typer::parser::NodeVec types, const token *end) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> tr_typed_arg(unique_ptr<Node> type_, unique_ptr<Node> arg) {
        Error::raise("Unsupported TypedRuby syntax");
    }

    unique_ptr<Node> true_(const token *tok) {
        return make_unique<True>(tok_loc(tok));
    }

    unique_ptr<Node> unary_op(const token *oper, unique_ptr<Node> receiver) {
        Loc loc = loc_join(tok_loc(oper), receiver->loc);
        NameRef op;
        if (oper->string() == "+") {
            op = core::Names::unaryPlus();
        } else if (oper->string() == "-") {
            op = core::Names::unaryMinus();
        } else {
            op = gs_.enterNameUTF8(oper->string());
        }

        return make_unique<Send>(loc, move(receiver), op, ruby_typer::parser::NodeVec());
    }

    unique_ptr<Node> undef_method(const token *undef, ruby_typer::parser::NodeVec name_list) {
        Loc loc = tok_loc(undef);
        if (!name_list.empty()) {
            loc = loc_join(loc, name_list.back()->loc);
        }
        return make_unique<Undef>(loc, move(name_list));
    }

    unique_ptr<Node> when(const token *when, ruby_typer::parser::NodeVec patterns, const token *then,
                          unique_ptr<Node> body) {
        Loc loc = tok_loc(when);
        if (body != nullptr) {
            loc = loc_join(loc, body->loc);
        } else if (then != nullptr) {
            loc = loc_join(loc, tok_loc(then));
        } else {
            loc = loc_join(loc, patterns.back()->loc);
        }
        return make_unique<When>(loc, move(patterns), move(body));
    }

    unique_ptr<Node> word(ruby_typer::parser::NodeVec parts) {
        Loc loc = collection_loc(parts);
        return make_unique<DString>(loc, move(parts));
    }

    unique_ptr<Node> words_compose(const token *begin, ruby_typer::parser::NodeVec parts, const token *end) {
        return make_unique<Array>(collection_loc(begin, parts, end), move(parts));
    }

    unique_ptr<Node> xstring_compose(const token *begin, ruby_typer::parser::NodeVec parts, const token *end) {
        return make_unique<XString>(collection_loc(begin, parts, end), move(parts));
    }

    /* End callback methods */

    /* methods for marshalling to and from the parser's foreign pointers */

    unique_ptr<Node> cast_node(foreign_ptr node) {
        size_t off = reinterpret_cast<size_t>(node);
        if (off == 0) {
            return nullptr;
        }

        Error::check(foreign_nodes_[off] != nullptr);
        return move(foreign_nodes_[off]);
    }

    foreign_ptr to_foreign(unique_ptr<Node> node) {
        if (node == nullptr) {
            return reinterpret_cast<foreign_ptr>(0);
        }
        foreign_nodes_.emplace_back(move(node));
        return reinterpret_cast<foreign_ptr>(foreign_nodes_.size() - 1);
    }

    ruby_typer::parser::NodeVec convert_node_list(const node_list *cargs) {
        ruby_typer::parser::NodeVec out;
        if (cargs == nullptr) {
            return out;
        }

        node_list *args = const_cast<node_list *>(cargs);
        out.reserve(args->size());
        for (int i = 0; i < args->size(); i++) {
            out.push_back(cast_node(args->at(i)));
        }
        return out;
    }
};

Builder::Builder(GlobalState &ctx, core::FileRef file) : impl_(new Builder::Impl(ctx, file)) {}
Builder::~Builder() = default;

}; // namespace parser
}; // namespace ruby_typer

namespace {

using ruby_typer::parser::Builder;
using ruby_typer::parser::Node;

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

foreign_ptr dedent_string(self_ptr builder, foreign_ptr node, size_t dedent_level) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->dedent_string(build->cast_node(node), dedent_level));
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

foreign_ptr negate(self_ptr builder, const token *uminus, foreign_ptr numeric) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->negate(uminus, build->cast_node(numeric)));
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

foreign_ptr tr_any(self_ptr builder, const token *special) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_any(special));
}

foreign_ptr tr_arg_instance(self_ptr builder, foreign_ptr base, const node_list *types, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_arg_instance(build->cast_node(base), build->convert_node_list(types), end));
}

foreign_ptr tr_array(self_ptr builder, const token *begin, foreign_ptr type_, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_array(begin, build->cast_node(type_), end));
}

foreign_ptr tr_cast(self_ptr builder, const token *begin, foreign_ptr expr, const token *colon, foreign_ptr type_,
                    const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_cast(begin, build->cast_node(expr), colon, build->cast_node(type_), end));
}

foreign_ptr tr_class(self_ptr builder, const token *special) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_class(special));
}

foreign_ptr tr_consubtype(self_ptr builder, foreign_ptr sub, foreign_ptr super_) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_consubtype(build->cast_node(sub), build->cast_node(super_)));
}

foreign_ptr tr_conunify(self_ptr builder, foreign_ptr a, foreign_ptr b) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_conunify(build->cast_node(a), build->cast_node(b)));
}

foreign_ptr tr_cpath(self_ptr builder, foreign_ptr cpath) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_cpath(build->cast_node(cpath)));
}

foreign_ptr tr_genargs(self_ptr builder, const token *begin, const node_list *genargs, const node_list *constraints,
                       const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(
        build->tr_genargs(begin, build->convert_node_list(genargs), build->convert_node_list(constraints), end));
}

foreign_ptr tr_gendecl(self_ptr builder, foreign_ptr cpath, const token *begin, const node_list *genargs,
                       const node_list *constraints, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_gendecl(build->cast_node(cpath), begin, build->convert_node_list(genargs),
                                               build->convert_node_list(constraints), end));
}

foreign_ptr tr_gendeclarg(self_ptr builder, const token *tok, foreign_ptr constraint) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_gendeclarg(tok, build->cast_node(constraint)));
}

foreign_ptr tr_geninst(self_ptr builder, foreign_ptr cpath, const token *begin, const node_list *genargs,
                       const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_geninst(build->cast_node(cpath), begin, build->convert_node_list(genargs), end));
}

foreign_ptr tr_hash(self_ptr builder, const token *begin, foreign_ptr key_type, const token *assoc,
                    foreign_ptr value_type, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(
        build->tr_hash(begin, build->cast_node(key_type), assoc, build->cast_node(value_type), end));
}

foreign_ptr tr_instance(self_ptr builder, const token *special) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_instance(special));
}

foreign_ptr tr_ivardecl(self_ptr builder, const token *def, const token *name, foreign_ptr type_) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_ivardecl(def, name, build->cast_node(type_)));
}

foreign_ptr tr_nil(self_ptr builder, const token *nil) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_nil(nil));
}

foreign_ptr tr_nillable(self_ptr builder, const token *tilde, foreign_ptr type_) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_nillable(tilde, build->cast_node(type_)));
}

foreign_ptr tr_or(self_ptr builder, foreign_ptr a, foreign_ptr b) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_or(build->cast_node(a), build->cast_node(b)));
}

foreign_ptr tr_paren(self_ptr builder, const token *begin, foreign_ptr node, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_paren(begin, build->cast_node(node), end));
}

foreign_ptr tr_proc(self_ptr builder, const token *begin, foreign_ptr args, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_proc(begin, build->cast_node(args), end));
}

foreign_ptr tr_prototype(self_ptr builder, foreign_ptr genargs, foreign_ptr args, foreign_ptr return_type) {
    auto build = cast_builder(builder);
    return build->to_foreign(
        build->tr_prototype(build->cast_node(genargs), build->cast_node(args), build->cast_node(return_type)));
}

foreign_ptr tr_returnsig(self_ptr builder, const token *arrow, foreign_ptr ret) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_returnsig(arrow, build->cast_node(ret)));
}

foreign_ptr tr_self(self_ptr builder, const token *special) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_self(special));
}

foreign_ptr tr_tuple(self_ptr builder, const token *begin, const node_list *types, const token *end) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_tuple(begin, build->convert_node_list(types), end));
}

foreign_ptr tr_typed_arg(self_ptr builder, foreign_ptr type_, foreign_ptr arg) {
    auto build = cast_builder(builder);
    return build->to_foreign(build->tr_typed_arg(build->cast_node(type_), build->cast_node(arg)));
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

namespace ruby_typer {
namespace parser {

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
    negate,
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
    tr_any,
    tr_arg_instance,
    tr_array,
    tr_cast,
    tr_class,
    tr_consubtype,
    tr_conunify,
    tr_cpath,
    tr_genargs,
    tr_gendecl,
    tr_gendeclarg,
    tr_geninst,
    tr_hash,
    tr_instance,
    tr_ivardecl,
    tr_nil,
    tr_nillable,
    tr_or,
    tr_paren,
    tr_proc,
    tr_prototype,
    tr_returnsig,
    tr_self,
    tr_tuple,
    tr_typed_arg,
    true_,
    unary_op,
    undef_method,
    when,
    word,
    words_compose,
    xstring_compose,
};
} // namespace parser
} // namespace ruby_typer
