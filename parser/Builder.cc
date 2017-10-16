#include "parser/Builder.h"
#include "parser/parser.h"

#include "ruby_parser/builder.hh"
#include "ruby_parser/diagnostic.hh"

#include <typeinfo>

using ruby_parser::foreign_ptr;
using ruby_parser::node_list;
using ruby_parser::self_ptr;
using ruby_parser::token;

using ruby_typer::ast::ContextBase;
using ruby_typer::ast::UTF8Desc;
using std::make_unique;
using std::unique_ptr;
using std::unique_ptr;
using std::vector;
using std::vector;

#define BUILDER_UNIMPLEMENTED()                                      \
    ({                                                               \
        ctx_.logger.critical("unimplemented builder: {}", __func__); \
        Error::notImplemented();                                     \
    })

namespace ruby_typer {
namespace parser {

class Builder::Impl {
public:
    Impl(ContextBase &ctx, Result &r) : ctx_(ctx), result_(r) {}

    ContextBase &ctx_;
    ruby_parser::base_driver *driver_;
    Result &result_;

    Loc tok_loc(const token *tok) {
        return Loc{(u4)tok->start(), (u4)tok->end()};
    }

    Loc maybe_loc(unique_ptr<Node> &node) {
        if (node == nullptr) {
            return Loc::none();
        }
        return node->loc;
    }

    Loc tok_loc(const token *begin, const token *end) {
        return Loc{(u4)begin->start(), (u4)end->end()};
    }

    Loc loc_join(Loc begin, Loc end) {
        if (begin.is_none())
            return end;
        if (end.is_none())
            return begin;

        return Loc{begin.begin_pos, end.end_pos};
    }

    Loc collection_loc(const token *begin, vector<unique_ptr<Node>> &elts, const token *end) {
        if (begin != nullptr) {
            DEBUG_ONLY(Error::check(end != nullptr));
            return tok_loc(begin, end);
        }
        DEBUG_ONLY(Error::check(end == nullptr));
        if (elts.size() == 0)
            return Loc::none();
        return loc_join(elts.front()->loc, elts.back()->loc);
    }

    Loc collection_loc(vector<unique_ptr<Node>> &elts) {
        return collection_loc(nullptr, elts, nullptr);
    }

    unique_ptr<Node> transform_condition(unique_ptr<Node> cond) {
        if (Begin *b = dynamic_cast<Begin *>(cond.get())) {
            if (b->stmts.size() == 1)
                b->stmts[0] = transform_condition(move(b->stmts[0]));
        } else if (And *a = dynamic_cast<And *>(cond.get())) {
            a->left = transform_condition(move(a->right));
            a->left = transform_condition(move(a->right));
        } else if (Or *o = dynamic_cast<Or *>(cond.get())) {
            o->left = transform_condition(move(o->right));
            o->left = transform_condition(move(o->right));
        } else if (IRange *ir = dynamic_cast<IRange *>(cond.get())) {
            return make_unique<IFlipflop>(ir->loc, transform_condition(move(ir->from)),
                                          transform_condition(move(ir->to)));
        } else if (ERange *er = dynamic_cast<ERange *>(cond.get())) {
            return make_unique<EFlipflop>(er->loc, transform_condition(move(er->from)),
                                          transform_condition(move(er->to)));
        } else if (Regexp *re = dynamic_cast<Regexp *>(cond.get())) {
            return make_unique<MatchCurLine>(re->loc, move(cond));
        }
        return cond;
    }

    void error(ruby_parser::dclass err, Loc loc) {
        driver_->external_diagnostic(ruby_parser::dlevel::ERROR, err, loc.begin_pos, loc.end_pos, "");
    }

    /* Begin callback methods */

    unique_ptr<Node> accessible(unique_ptr<Node> node) {
        if (Ident *id = dynamic_cast<Ident *>(node.get())) {
            auto &name = id->name.name(ctx_);
            DEBUG_ONLY(Error::check(name.kind == ast::UTF8));
            if (driver_->lex.is_declared(name.toString(ctx_))) {
                return make_unique<LVar>(node->loc, id->name);
            } else {
                return make_unique<Send>(node->loc, nullptr, id->name, vector<unique_ptr<Node>>());
            }
        }
        return node;
    }

    unique_ptr<Node> alias(const token *alias, unique_ptr<Node> to, unique_ptr<Node> from) {
        return make_unique<Alias>(loc_join(tok_loc(alias), from->loc), move(to), move(from));
    }

    unique_ptr<Node> arg(const token *name) {
        return make_unique<Arg>(tok_loc(name), ctx_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> args(const token *begin, vector<unique_ptr<Node>> args, const token *end, bool check_args) {
        if (begin == nullptr && args.size() == 0 && end == nullptr)
            return nullptr;
        return make_unique<Args>(collection_loc(begin, args, end), move(args));
    }

    unique_ptr<Node> array(const token *begin, vector<unique_ptr<Node>> elements, const token *end) {
        return make_unique<Array>(collection_loc(begin, elements, end), move(elements));
    }

    unique_ptr<Node> assign(unique_ptr<Node> lhs, const token *eql, unique_ptr<Node> rhs) {
        Loc loc = loc_join(lhs->loc, rhs->loc);

        if (Send *s = dynamic_cast<Send *>(lhs.get())) {
            s->args.push_back(move(rhs));
            return make_unique<Send>(loc, move(s->receiver), s->method, move(s->args));
        } else if (CSend *s = dynamic_cast<CSend *>(lhs.get())) {
            s->args.push_back(move(rhs));
            return make_unique<CSend>(loc, move(s->receiver), s->method, move(s->args));
        } else if (LVarLhs *l = dynamic_cast<LVarLhs *>(lhs.get())) {
            return make_unique<LVarAsgn>(loc, l->name, move(rhs));
        } else if (ConstLhs *c = dynamic_cast<ConstLhs *>(lhs.get())) {
            return make_unique<ConstAsgn>(loc, move(c->scope), c->name, move(rhs));
        } else if (CVarLhs *c = dynamic_cast<CVarLhs *>(lhs.get())) {
            return make_unique<CVarAsgn>(loc, c->name, move(rhs));
        } else if (IVarLhs *c = dynamic_cast<IVarLhs *>(lhs.get())) {
            return make_unique<IVarAsgn>(loc, c->name, move(rhs));
        } else if (GVarLhs *c = dynamic_cast<GVarLhs *>(lhs.get())) {
            return make_unique<GVarAsgn>(loc, c->name, move(rhs));
        }
        Error::raise("unimplemented lhs: ", lhs->nodeName());
    }

    unique_ptr<Node> assignable(unique_ptr<Node> node) {
        if (Ident *id = dynamic_cast<Ident *>(node.get())) {
            auto &name = id->name.name(ctx_);
            driver_->lex.declare(name.toString(ctx_));
            return make_unique<LVarLhs>(id->loc, id->name);
        } else if (IVar *iv = dynamic_cast<IVar *>(node.get())) {
            return make_unique<IVarLhs>(iv->loc, iv->name);
        } else if (Const *c = dynamic_cast<Const *>(node.get())) {
            return make_unique<ConstLhs>(c->loc, move(c->scope), move(c->name));
        } else if (CVar *cv = dynamic_cast<CVar *>(node.get())) {
            return make_unique<CVarLhs>(cv->loc, cv->name);
        } else if (GVar *gv = dynamic_cast<GVar *>(node.get())) {
            return make_unique<GVarLhs>(gv->loc, gv->name);
        } else if (dynamic_cast<Backref *>(node.get()) != nullptr || dynamic_cast<NthRef *>(node.get()) != nullptr) {
            error(ruby_parser::dclass::BackrefAssignment, node->loc);
            return make_unique<Nil>(node->loc);
        } else {
            error(ruby_parser::dclass::InvalidAssignment, node->loc);
            return make_unique<Nil>(node->loc);
        }
    }

    unique_ptr<Node> associate(const token *begin, vector<unique_ptr<Node>> pairs, const token *end) {
        return make_unique<Hash>(collection_loc(begin, pairs, end), move(pairs));
    }

    unique_ptr<Node> attr_asgn(unique_ptr<Node> receiver, const token *dot, const token *selector) {
        auto method = ctx_.enterNameUTF8(selector->string() + "=");
        Loc loc = loc_join(receiver->loc, tok_loc(selector));
        if (dot && dot->string() == "&.") {
            return make_unique<CSend>(loc, move(receiver), method, vector<unique_ptr<Node>>());
        }
        return make_unique<Send>(loc, move(receiver), method, vector<unique_ptr<Node>>());
    }

    unique_ptr<Node> back_ref(const token *tok) {
        return make_unique<Backref>(tok_loc(tok), ctx_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> begin(const token *begin, unique_ptr<Node> body, const token *end) {
        Loc loc;
        if (begin != nullptr)
            loc = loc_join(tok_loc(begin), tok_loc(end));
        else
            loc = body->loc;

        if (body == nullptr) {
            return make_unique<Begin>(loc, vector<unique_ptr<Node>>());
        }
        if (Begin *b = dynamic_cast<Begin *>(body.get()))
            return body;
        if (Mlhs *m = dynamic_cast<Mlhs *>(body.get()))
            return body;
        vector<unique_ptr<Node>> stmts;
        stmts.push_back(move(body));
        return make_unique<Begin>(loc, move(stmts));
    }

    unique_ptr<Node> begin_body(unique_ptr<Node> body, vector<unique_ptr<Node>> rescue_bodies, const token *else_tok,
                                unique_ptr<Node> else_, const token *ensure_tok, unique_ptr<Node> ensure) {
        if (rescue_bodies.size() > 0) {
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
            vector<unique_ptr<Node>> stmts;
            stmts.push_back(move(body));
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
            if (Begin *b = dynamic_cast<Begin *>(body.get())) {
                return make_unique<Kwbegin>(loc, move(b->stmts));
            } else {
                vector<unique_ptr<Node>> nodes;
                nodes.push_back(move(body));
                return make_unique<Kwbegin>(loc, move(nodes));
            }
        }
        return make_unique<Kwbegin>(loc, vector<unique_ptr<Node>>());
    }

    unique_ptr<Node> binary_op(unique_ptr<Node> receiver, const token *oper, unique_ptr<Node> arg) {
        Loc loc = loc_join(receiver->loc, arg->loc);

        vector<unique_ptr<Node>> args;
        args.push_back(move(arg));

        return make_unique<Send>(loc, move(receiver), ctx_.enterNameUTF8(oper->string()), move(args));
    }

    unique_ptr<Node> block(unique_ptr<Node> method_call, const token *begin, unique_ptr<Node> args,
                           unique_ptr<Node> body, const token *end) {
        if (Yield *y = dynamic_cast<Yield *>(method_call.get())) {
            error(ruby_parser::dclass::BlockGivenToYield, y->loc);
            return make_unique<Yield>(y->loc, vector<unique_ptr<Node>>());
        }

        vector<unique_ptr<Node>> *callargs = nullptr;
        if (Send *s = dynamic_cast<Send *>(method_call.get())) {
            callargs = &s->args;
        }
        if (CSend *s = dynamic_cast<CSend *>(method_call.get())) {
            callargs = &s->args;
        }
        if (Super *s = dynamic_cast<Super *>(method_call.get())) {
            callargs = &s->args;
        }
        if (callargs != nullptr && callargs->size() > 0) {
            if (BlockPass *bp = dynamic_cast<BlockPass *>(callargs->back().get()))
                error(ruby_parser::dclass::BlockAndBlockarg, bp->loc);
        }

        Node &n = *method_call;
        const std::type_info &ty = typeid(n);
        if (ty == typeid(Send) || ty == typeid(CSend) || ty == typeid(Super) || ty == typeid(ZSuper) ||
            ty == typeid(Lambda)) {
            return make_unique<Block>(loc_join(method_call->loc, tok_loc(end)), move(method_call), move(args),
                                      move(body));
        }

        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> block_pass(const token *amper, unique_ptr<Node> arg) {
        return make_unique<BlockPass>(loc_join(tok_loc(amper), arg->loc), move(arg));
    }

    unique_ptr<Node> blockarg(const token *amper, const token *name) {
        return make_unique<Blockarg>(loc_join(tok_loc(amper), tok_loc(name)), ctx_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> call_lambda(const token *lambda) {
        return make_unique<Lambda>(tok_loc(lambda));
    }

    unique_ptr<Node> call_method(unique_ptr<Node> receiver, const token *dot, const token *selector,
                                 const token *lparen, vector<unique_ptr<Node>> args, const token *rparen) {
        Loc selector_loc, start_loc;
        if (selector != nullptr)
            selector_loc = tok_loc(selector);
        else
            selector_loc = tok_loc(dot);
        if (receiver == nullptr)
            start_loc = selector_loc;
        else
            start_loc = receiver->loc;

        Loc loc;
        if (rparen != nullptr)
            loc = loc_join(start_loc, tok_loc(rparen));
        else if (args.size() > 0)
            loc = loc_join(start_loc, args.back()->loc);
        else
            loc = loc_join(start_loc, selector_loc);

        NameRef method;
        if (selector == nullptr)
            method = ast::Names::bang();
        else
            method = ctx_.enterNameUTF8(selector->string());

        if (dot && dot->string() == "&.")
            return make_unique<CSend>(loc, move(receiver), method, move(args));
        else
            return make_unique<Send>(loc, move(receiver), method, move(args));
    }

    unique_ptr<Node> case_(const token *case_, unique_ptr<Node> expr, vector<unique_ptr<Node>> when_bodies,
                           const token *else_tok, unique_ptr<Node> else_body, const token *end) {
        return make_unique<Case>(loc_join(tok_loc(case_), tok_loc(end)), move(expr), move(when_bodies),
                                 move(else_body));
    }

    unique_ptr<Node> character(const token *char_) {
        return make_unique<String>(tok_loc(char_), char_->string());
    }

    unique_ptr<Node> complex(const token *tok) {
        return make_unique<Complex>(tok_loc(tok), tok->string());
    }

    unique_ptr<Node> compstmt(vector<unique_ptr<Node>> nodes) {
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
        if (then != nullptr)
            loc = loc_join(loc, tok_loc(then));
        if (if_true != nullptr)
            loc = loc_join(loc, if_true->loc);
        if (else_ != nullptr)
            loc = loc_join(loc, tok_loc(else_));
        if (if_false != nullptr)
            loc = loc_join(loc, if_false->loc);
        if (end != nullptr)
            loc = loc_join(loc, tok_loc(end));
        return make_unique<If>(loc, transform_condition(move(cond)), move(if_true), move(if_false));
    }

    unique_ptr<Node> condition_mod(unique_ptr<Node> if_true, unique_ptr<Node> if_false, unique_ptr<Node> cond) {
        Loc loc = cond->loc;
        if (if_true != nullptr)
            loc = loc_join(loc, if_true->loc);
        else
            loc = loc_join(loc, if_false->loc);
        return make_unique<If>(loc, transform_condition(move(cond)), move(if_true), move(if_false));
    }

    unique_ptr<Node> const_(const token *name) {
        return make_unique<Const>(tok_loc(name), nullptr, ctx_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> const_fetch(unique_ptr<Node> scope, const token *colon, const token *name) {
        return make_unique<Const>(loc_join(scope->loc, tok_loc(name)), move(scope), ctx_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> const_global(const token *colon, const token *name) {
        return make_unique<Const>(loc_join(tok_loc(colon), tok_loc(name)), make_unique<Cbase>(tok_loc(colon)),
                                  ctx_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> const_op_assignable(unique_ptr<Node> node) {
        return node;
    }

    unique_ptr<Node> cvar(const token *tok) {
        return make_unique<CVar>(tok_loc(tok), ctx_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> dedent_string(unique_ptr<Node> node, size_t dedent_level) {
        // TODO: implement dedenting for dedent_level != 0
        Error::check(dedent_level == 0);
        return node;
    }

    unique_ptr<Node> def_class(const token *class_, unique_ptr<Node> name, const token *lt_,
                               unique_ptr<Node> superclass, unique_ptr<Node> body, const token *end_) {
        return make_unique<Class>(tok_loc(class_, end_), move(name), move(superclass), move(body));
    }

    unique_ptr<Node> def_method(const token *def, const token *name, unique_ptr<Node> args, unique_ptr<Node> body,
                                const token *end) {
        return make_unique<DefMethod>(tok_loc(def, end), ctx_.enterNameUTF8(name->string()), move(args), move(body));
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
        return make_unique<DefS>(loc, move(definee), ctx_.enterNameUTF8(name->string()), move(args), move(body));
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
        return make_unique<GVar>(tok_loc(tok), ctx_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> ident(const token *tok) {
        return make_unique<Ident>(tok_loc(tok), ctx_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> index(unique_ptr<Node> receiver, const token *lbrack, vector<unique_ptr<Node>> indexes,
                           const token *rbrack) {
        return make_unique<Send>(loc_join(receiver->loc, tok_loc(rbrack)), move(receiver), ast::Names::squareBrackets(),
                                 move(indexes));
    }

    unique_ptr<Node> index_asgn(unique_ptr<Node> receiver, const token *lbrack, vector<unique_ptr<Node>> indexes,
                                const token *rbrack) {
        vector<unique_ptr<Node>> args;
        return make_unique<Send>(loc_join(receiver->loc, tok_loc(rbrack)), move(receiver),
                                 ast::Names::squareBracketsEq(), move(args));
    }

    unique_ptr<Node> integer(const token *tok) {
        return make_unique<Integer>(tok_loc(tok), tok->string());
    }

    unique_ptr<Node> ivar(const token *tok) {
        return make_unique<IVar>(tok_loc(tok), ctx_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> keyword_break(const token *keyword, const token *lparen, vector<unique_ptr<Node>> args,
                                   const token *rparen) {
        Loc loc = loc_join(tok_loc(keyword), collection_loc(lparen, args, rparen));
        return make_unique<Break>(loc, move(args));
    }

    unique_ptr<Node> keyword_defined(const token *keyword, unique_ptr<Node> arg) {
        return make_unique<Defined>(loc_join(tok_loc(keyword), arg->loc), move(arg));
    }

    unique_ptr<Node> keyword_next(const token *keyword, const token *lparen, vector<unique_ptr<Node>> args,
                                  const token *rparen) {
        return make_unique<Next>(loc_join(tok_loc(keyword), collection_loc(lparen, args, rparen)), move(args));
    }

    unique_ptr<Node> keyword_redo(const token *keyword) {
        return make_unique<Redo>(tok_loc(keyword));
    }

    unique_ptr<Node> keyword_retry(const token *keyword) {
        return make_unique<Retry>(tok_loc(keyword));
    }

    unique_ptr<Node> keyword_return(const token *keyword, const token *lparen, vector<unique_ptr<Node>> args,
                                    const token *rparen) {
        Loc loc = loc_join(tok_loc(keyword), collection_loc(lparen, args, rparen));
        return make_unique<Return>(loc, move(args));
    }

    unique_ptr<Node> keyword_super(const token *keyword, const token *lparen, vector<unique_ptr<Node>> args,
                                   const token *rparen) {
        Loc loc = tok_loc(keyword);
        Loc argloc = collection_loc(lparen, args, rparen);
        if (!argloc.is_none())
            loc = loc_join(loc, argloc);
        return make_unique<Super>(loc, move(args));
    }

    unique_ptr<Node> keyword_yield(const token *keyword, const token *lparen, vector<unique_ptr<Node>> args,
                                   const token *rparen) {
        Loc loc = loc_join(tok_loc(keyword), collection_loc(lparen, args, rparen));
        if (args.size() > 0 && dynamic_cast<BlockPass *>(args.back().get()) != nullptr) {
            error(ruby_parser::dclass::BlockGivenToYield, loc);
        }
        return make_unique<Yield>(loc, move(args));
    }

    unique_ptr<Node> keyword_zsuper(const token *keyword) {
        return make_unique<ZSuper>(tok_loc(keyword));
    }

    unique_ptr<Node> kwarg(const token *name) {
        return make_unique<Kwarg>(tok_loc(name), ctx_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> kwoptarg(const token *name, unique_ptr<Node> value) {
        return make_unique<Kwoptarg>(loc_join(tok_loc(name), value->loc), ctx_.enterNameUTF8(name->string()),
                                     move(value));
    }

    unique_ptr<Node> kwrestarg(const token *dstar, const token *name) {
        return make_unique<Kwrestarg>(loc_join(tok_loc(dstar), tok_loc(name)), ctx_.enterNameUTF8(name->string()));
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
        Loc loc = loc_join(receiver->loc, arg->loc);
        vector<unique_ptr<Node>> args;
        args.emplace_back(move(arg));
        return make_unique<Send>(loc, move(receiver), ctx_.enterNameUTF8(oper->string()), move(args));
    }

    unique_ptr<Node> multi_assign(unique_ptr<Node> mlhs, unique_ptr<Node> rhs) {
        return make_unique<Masgn>(loc_join(mlhs->loc, rhs->loc), move(mlhs), move(rhs));
    }

    unique_ptr<Node> multi_lhs(const token *begin, vector<unique_ptr<Node>> items, const token *end) {
        return make_unique<Mlhs>(collection_loc(begin, items, end), move(items));
    }

    unique_ptr<Node> multi_lhs1(const token *begin, unique_ptr<Node> item, const token *end) {
        if (Mlhs *mlhs = dynamic_cast<Mlhs *>(item.get())) {
            return item;
        }
        vector<unique_ptr<Node>> args;
        args.emplace_back(move(item));
        return make_unique<Mlhs>(collection_loc(begin, args, end), move(args));
    }

    unique_ptr<Node> negate(const token *uminus, unique_ptr<Node> numeric) {
        Loc loc = loc_join(tok_loc(uminus), numeric->loc);
        if (Integer *i = dynamic_cast<Integer *>(numeric.get()))
            return make_unique<Integer>(loc, "-" + i->val);
        if (Float *i = dynamic_cast<Float *>(numeric.get()))
            return make_unique<Float>(loc, "-" + i->val);
        Error::raise("unexpected numeric type");
    }

    unique_ptr<Node> nil(const token *tok) {
        return make_unique<Nil>(tok_loc(tok));
    }

    unique_ptr<Node> not_op(const token *not_, const token *begin, unique_ptr<Node> receiver, const token *end) {
        if (receiver != nullptr) {
            Loc loc;
            if (end != nullptr)
                loc = loc_join(tok_loc(not_), tok_loc(end));
            else
                loc = loc_join(tok_loc(not_), receiver->loc);
            return make_unique<Send>(loc, move(receiver), ast::Names::bang(), vector<unique_ptr<Node>>());
        }

        DEBUG_ONLY(Error::check(begin != nullptr && end != nullptr));
        auto body = make_unique<Begin>(loc_join(tok_loc(begin), tok_loc(end)), vector<unique_ptr<Node>>());
        return make_unique<Send>(loc_join(tok_loc(not_), body->loc), move(body), ast::Names::bang(),
                                 vector<unique_ptr<Node>>());
    }

    unique_ptr<Node> nth_ref(const token *tok) {
        return make_unique<NthRef>(tok_loc(tok), std::atoi(tok->string().c_str()));
    }

    unique_ptr<Node> op_assign(unique_ptr<Node> lhs, const token *op, unique_ptr<Node> rhs) {
        if (dynamic_cast<Backref *>(lhs.get()) != nullptr || dynamic_cast<NthRef *>(lhs.get()) != nullptr)
            error(ruby_parser::dclass::BackrefAssignment, lhs->loc);

        if (op->string() == "&&")
            return make_unique<AndAsgn>(loc_join(lhs->loc, rhs->loc), move(lhs), move(rhs));
        if (op->string() == "||")
            return make_unique<OrAsgn>(loc_join(lhs->loc, rhs->loc), move(lhs), move(rhs));
        return make_unique<OpAsgn>(loc_join(lhs->loc, rhs->loc), move(lhs), ctx_.enterNameUTF8(op->string()),
                                   move(rhs));
    }

    unique_ptr<Node> optarg_(const token *name, const token *eql, unique_ptr<Node> value) {
        return make_unique<Optarg>(loc_join(tok_loc(name), value->loc), ctx_.enterNameUTF8(name->string()),
                                   move(value));
    }

    unique_ptr<Node> pair(unique_ptr<Node> key, const token *assoc, unique_ptr<Node> value) {
        return make_unique<Pair>(loc_join(key->loc, value->loc), move(key), move(value));
    }

    unique_ptr<Node> pair_keyword(const token *key, unique_ptr<Node> value) {
        return make_unique<Pair>(loc_join(tok_loc(key), value->loc), make_unique<Symbol>(tok_loc(key), key->string()),
                                 move(value));
    }

    unique_ptr<Node> pair_quoted(const token *begin, vector<unique_ptr<Node>> parts, const token *end,
                                 unique_ptr<Node> value) {
        auto sym = make_unique<DSymbol>(loc_join(tok_loc(begin), tok_loc(end)), move(parts));
        return make_unique<Pair>(loc_join(tok_loc(begin), value->loc), move(sym), move(value));
    }

    unique_ptr<Node> postexe(const token *begin, unique_ptr<Node> node, const token *rbrace) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> preexe(const token *begin, unique_ptr<Node> node, const token *rbrace) {
        BUILDER_UNIMPLEMENTED();
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
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> rational_complex(const token *tok) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> regexp_compose(const token *begin, vector<unique_ptr<Node>> parts, const token *end,
                                    unique_ptr<Node> options) {
        Loc loc = loc_join(loc_join(tok_loc(begin), tok_loc(end)), maybe_loc(options));
        return make_unique<Regexp>(loc, move(parts), move(options));
    }

    unique_ptr<Node> regexp_options(const token *regopt) {
        return make_unique<Regopt>(tok_loc(regopt), regopt->string());
    }

    unique_ptr<Node> rescue_body(const token *rescue, unique_ptr<Node> exc_list, const token *assoc,
                                 unique_ptr<Node> exc_var, const token *then, unique_ptr<Node> body) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> restarg(const token *star, const token *name) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> self_(const token *tok) {
        return make_unique<Self>(tok_loc(tok));
    }

    unique_ptr<Node> shadowarg(const token *name) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> splat(const token *star, unique_ptr<Node> arg) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> splat_mlhs(const token *star, unique_ptr<Node> arg) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> string(const token *string_) {
        return make_unique<String>(tok_loc(string_), string_->string());
    }

    unique_ptr<Node> string_compose(const token *begin, vector<unique_ptr<Node>> parts, const token *end) {
        Loc loc = collection_loc(begin, parts, end);
        return make_unique<DString>(loc, move(parts));
    }

    unique_ptr<Node> string_internal(const token *string_) {
        return make_unique<String>(tok_loc(string_), string_->string());
    }

    unique_ptr<Node> symbol(const token *symbol) {
        return make_unique<Symbol>(tok_loc(symbol), symbol->string());
    }

    unique_ptr<Node> symbol_compose(const token *begin, vector<unique_ptr<Node>> parts, const token *end) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> symbol_internal(const token *symbol) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> symbols_compose(const token *begin, vector<unique_ptr<Node>> parts, const token *end) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> ternary(unique_ptr<Node> cond, const token *question, unique_ptr<Node> if_true, const token *colon,
                             unique_ptr<Node> if_false) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_any(const token *special) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_arg_instance(unique_ptr<Node> base, vector<unique_ptr<Node>> types, const token *end) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_array(const token *begin, unique_ptr<Node> type_, const token *end) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_cast(const token *begin, unique_ptr<Node> expr, const token *colon, unique_ptr<Node> type_,
                             const token *end) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_class(const token *special) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_consubtype(unique_ptr<Node> sub, unique_ptr<Node> super_) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_conunify(unique_ptr<Node> a, unique_ptr<Node> b) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_cpath(unique_ptr<Node> cpath) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_genargs(const token *begin, vector<unique_ptr<Node>> genargs,
                                vector<unique_ptr<Node>> constraints, const token *end) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_gendecl(unique_ptr<Node> cpath, const token *begin, vector<unique_ptr<Node>> genargs,
                                vector<unique_ptr<Node>> constraints, const token *end) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_gendeclarg(const token *tok, unique_ptr<Node> constraint) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_geninst(unique_ptr<Node> cpath, const token *begin, vector<unique_ptr<Node>> genargs,
                                const token *end) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_hash(const token *begin, unique_ptr<Node> key_type, const token *assoc,
                             unique_ptr<Node> value_type, const token *end) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_instance(const token *special) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_ivardecl(const token *def, const token *name, unique_ptr<Node> type_) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_nil(const token *nil) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_nillable(const token *tilde, unique_ptr<Node> type_) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_or(unique_ptr<Node> a, unique_ptr<Node> b) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_proc(const token *begin, unique_ptr<Node> args, const token *end) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_prototype(unique_ptr<Node> genargs, unique_ptr<Node> args, unique_ptr<Node> return_type) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_returnsig(const token *arrow, unique_ptr<Node> ret) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_self(const token *special) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_tuple(const token *begin, vector<unique_ptr<Node>> types, const token *end) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> tr_typed_arg(unique_ptr<Node> type_, unique_ptr<Node> arg) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> true_(const token *tok) {
        return make_unique<True>(tok_loc(tok));
    }

    unique_ptr<Node> unary_op(const token *oper, unique_ptr<Node> receiver) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> undef_method(const token *undef, vector<unique_ptr<Node>> name_list) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> when(const token *when, vector<unique_ptr<Node>> patterns, const token *then,
                          unique_ptr<Node> body) {
        Loc loc = tok_loc(when);
        if (body != nullptr)
            loc = loc_join(loc, body->loc);
        else if (then != nullptr)
            loc = loc_join(loc, tok_loc(then));
        else
            loc = loc_join(loc, patterns.back()->loc);
        return make_unique<When>(loc, move(patterns), move(body));
    }

    unique_ptr<Node> word(vector<unique_ptr<Node>> parts) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> words_compose(const token *begin, vector<unique_ptr<Node>> parts, const token *end) {
        BUILDER_UNIMPLEMENTED();
    }

    unique_ptr<Node> xstring_compose(const token *begin, vector<unique_ptr<Node>> parts, const token *end) {
        BUILDER_UNIMPLEMENTED();
    }
};

Builder::Builder(ContextBase &ctx, Result &r) : impl_(new Builder::Impl(ctx, r)) {}
Builder::~Builder() {}

}; // namespace parser
}; // namespace ruby_typer

namespace {

using ruby_typer::parser::Builder;
using ruby_typer::parser::Node;

Builder::Impl *cast_builder(self_ptr builder) {
    return const_cast<Builder::Impl *>(reinterpret_cast<const Builder::Impl *>(builder));
}

unique_ptr<Node> cast_node(foreign_ptr node) {
    return unique_ptr<Node>(const_cast<Node *>(reinterpret_cast<const Node *>(node)));
}

vector<unique_ptr<Node>> convert_node_list(const node_list *cargs) {
    vector<unique_ptr<Node>> out;
    if (cargs == nullptr)
        return out;

    node_list *args = const_cast<node_list *>(cargs);
    for (int i = 0; i < args->size(); i++) {
        out.push_back(cast_node(args->at(i)));
    }
    return out;
}

foreign_ptr accessible(self_ptr builder, foreign_ptr node) {
    return cast_builder(builder)->accessible(cast_node(node)).release();
}

foreign_ptr alias(self_ptr builder, const token *alias, foreign_ptr to, foreign_ptr from) {
    return cast_builder(builder)->alias(alias, cast_node(to), cast_node(from)).release();
}

foreign_ptr arg(self_ptr builder, const token *name) {
    return cast_builder(builder)->arg(name).release();
}

foreign_ptr args(self_ptr builder, const token *begin, const node_list *args, const token *end, bool check_args) {
    return cast_builder(builder)->args(begin, convert_node_list(args), end, check_args).release();
}

foreign_ptr array(self_ptr builder, const token *begin, const node_list *elements, const token *end) {
    return cast_builder(builder)->array(begin, convert_node_list(elements), end).release();
}

foreign_ptr assign(self_ptr builder, foreign_ptr lhs, const token *eql, foreign_ptr rhs) {
    return cast_builder(builder)->assign(cast_node(lhs), eql, cast_node(rhs)).release();
}

foreign_ptr assignable(self_ptr builder, foreign_ptr node) {
    return cast_builder(builder)->assignable(cast_node(node)).release();
}

foreign_ptr associate(self_ptr builder, const token *begin, const node_list *pairs, const token *end) {
    return cast_builder(builder)->associate(begin, convert_node_list(pairs), end).release();
}

foreign_ptr attr_asgn(self_ptr builder, foreign_ptr receiver, const token *dot, const token *selector) {
    return cast_builder(builder)->attr_asgn(cast_node(receiver), dot, selector).release();
}

foreign_ptr back_ref(self_ptr builder, const token *tok) {
    return cast_builder(builder)->back_ref(tok).release();
}

foreign_ptr begin(self_ptr builder, const token *begin, foreign_ptr body, const token *end) {
    return cast_builder(builder)->begin(begin, cast_node(body), end).release();
}

foreign_ptr begin_body(self_ptr builder, foreign_ptr body, const node_list *rescue_bodies, const token *else_tok,
                       foreign_ptr else_, const token *ensure_tok, foreign_ptr ensure) {
    return cast_builder(builder)
        ->begin_body(cast_node(body), convert_node_list(rescue_bodies), else_tok, cast_node(else_), ensure_tok,
                     cast_node(ensure))
        .release();
}

foreign_ptr begin_keyword(self_ptr builder, const token *begin, foreign_ptr body, const token *end) {
    return cast_builder(builder)->begin_keyword(begin, cast_node(body), end).release();
}

foreign_ptr binary_op(self_ptr builder, foreign_ptr receiver, const token *oper, foreign_ptr arg) {
    return cast_builder(builder)->binary_op(cast_node(receiver), oper, cast_node(arg)).release();
}

foreign_ptr block(self_ptr builder, foreign_ptr method_call, const token *begin, foreign_ptr args, foreign_ptr body,
                  const token *end) {
    return cast_builder(builder)->block(cast_node(method_call), begin, cast_node(args), cast_node(body), end).release();
}

foreign_ptr block_pass(self_ptr builder, const token *amper, foreign_ptr arg) {
    return cast_builder(builder)->block_pass(amper, cast_node(arg)).release();
}

foreign_ptr blockarg(self_ptr builder, const token *amper, const token *name) {
    return cast_builder(builder)->blockarg(amper, name).release();
}

foreign_ptr call_lambda(self_ptr builder, const token *lambda) {
    return cast_builder(builder)->call_lambda(lambda).release();
}

foreign_ptr call_method(self_ptr builder, foreign_ptr receiver, const token *dot, const token *selector,
                        const token *lparen, const node_list *args, const token *rparen) {
    return cast_builder(builder)
        ->call_method(cast_node(receiver), dot, selector, lparen, convert_node_list(args), rparen)
        .release();
}

foreign_ptr case_(self_ptr builder, const token *case_, foreign_ptr expr, const node_list *when_bodies,
                  const token *else_tok, foreign_ptr else_body, const token *end) {
    return cast_builder(builder)
        ->case_(case_, cast_node(expr), convert_node_list(when_bodies), else_tok, cast_node(else_body), end)
        .release();
}

foreign_ptr character(self_ptr builder, const token *char_) {
    return cast_builder(builder)->character(char_).release();
}

foreign_ptr complex(self_ptr builder, const token *tok) {
    return cast_builder(builder)->complex(tok).release();
}

foreign_ptr compstmt(self_ptr builder, const node_list *node) {
    return cast_builder(builder)->compstmt(convert_node_list(node)).release();
}

foreign_ptr condition(self_ptr builder, const token *cond_tok, foreign_ptr cond, const token *then, foreign_ptr if_true,
                      const token *else_, foreign_ptr if_false, const token *end) {
    return cast_builder(builder)
        ->condition(cond_tok, cast_node(cond), then, cast_node(if_true), else_, cast_node(if_false), end)
        .release();
}

foreign_ptr condition_mod(self_ptr builder, foreign_ptr if_true, foreign_ptr if_false, foreign_ptr cond) {
    return cast_builder(builder)->condition_mod(cast_node(if_true), cast_node(if_false), cast_node(cond)).release();
}

foreign_ptr const_(self_ptr builder, const token *name) {
    return cast_builder(builder)->const_(name).release();
}

foreign_ptr const_fetch(self_ptr builder, foreign_ptr scope, const token *colon, const token *name) {
    return cast_builder(builder)->const_fetch(cast_node(scope), colon, name).release();
}

foreign_ptr const_global(self_ptr builder, const token *colon, const token *name) {
    return cast_builder(builder)->const_global(colon, name).release();
}

foreign_ptr const_op_assignable(self_ptr builder, foreign_ptr node) {
    return cast_builder(builder)->const_op_assignable(cast_node(node)).release();
}

foreign_ptr cvar(self_ptr builder, const token *tok) {
    return cast_builder(builder)->cvar(tok).release();
}

foreign_ptr dedent_string(self_ptr builder, foreign_ptr node, size_t dedent_level) {
    return cast_builder(builder)->dedent_string(cast_node(node), dedent_level).release();
}

foreign_ptr def_class(self_ptr builder, const token *class_, foreign_ptr name, const token *lt_, foreign_ptr superclass,
                      foreign_ptr body, const token *end_) {
    return cast_builder(builder)
        ->def_class(class_, cast_node(name), lt_, cast_node(superclass), cast_node(body), end_)
        .release();
}

foreign_ptr def_method(self_ptr builder, const token *def, const token *name, foreign_ptr args, foreign_ptr body,
                       const token *end) {
    return cast_builder(builder)->def_method(def, name, cast_node(args), cast_node(body), end).release();
}

foreign_ptr def_module(self_ptr builder, const token *module, foreign_ptr name, foreign_ptr body, const token *end_) {
    return cast_builder(builder)->def_module(module, cast_node(name), cast_node(body), end_).release();
}

foreign_ptr def_sclass(self_ptr builder, const token *class_, const token *lshft_, foreign_ptr expr, foreign_ptr body,
                       const token *end_) {
    return cast_builder(builder)->def_sclass(class_, lshft_, cast_node(expr), cast_node(body), end_).release();
}

foreign_ptr def_singleton(self_ptr builder, const token *def, foreign_ptr definee, const token *dot, const token *name,
                          foreign_ptr args, foreign_ptr body, const token *end) {
    return cast_builder(builder)
        ->def_singleton(def, cast_node(definee), dot, name, cast_node(args), cast_node(body), end)
        .release();
}

foreign_ptr encoding_literal(self_ptr builder, const token *tok) {
    return cast_builder(builder)->encoding_literal(tok).release();
}

foreign_ptr false_(self_ptr builder, const token *tok) {
    return cast_builder(builder)->false_(tok).release();
}

foreign_ptr file_literal(self_ptr builder, const token *tok) {
    return cast_builder(builder)->file_literal(tok).release();
}

foreign_ptr float_(self_ptr builder, const token *tok) {
    return cast_builder(builder)->float_(tok).release();
}

foreign_ptr float_complex(self_ptr builder, const token *tok) {
    return cast_builder(builder)->float_complex(tok).release();
}

foreign_ptr for_(self_ptr builder, const token *for_, foreign_ptr iterator, const token *in_, foreign_ptr iteratee,
                 const token *do_, foreign_ptr body, const token *end) {
    return cast_builder(builder)
        ->for_(for_, cast_node(iterator), in_, cast_node(iteratee), do_, cast_node(body), end)
        .release();
}

foreign_ptr gvar(self_ptr builder, const token *tok) {
    return cast_builder(builder)->gvar(tok).release();
}

foreign_ptr ident(self_ptr builder, const token *tok) {
    return cast_builder(builder)->ident(tok).release();
}

foreign_ptr index(self_ptr builder, foreign_ptr receiver, const token *lbrack, const node_list *indexes,
                  const token *rbrack) {
    return cast_builder(builder)->index(cast_node(receiver), lbrack, convert_node_list(indexes), rbrack).release();
}

foreign_ptr index_asgn(self_ptr builder, foreign_ptr receiver, const token *lbrack, const node_list *indexes,
                       const token *rbrack) {
    return cast_builder(builder)->index_asgn(cast_node(receiver), lbrack, convert_node_list(indexes), rbrack).release();
}

foreign_ptr integer(self_ptr builder, const token *tok) {
    return cast_builder(builder)->integer(tok).release();
}

foreign_ptr ivar(self_ptr builder, const token *tok) {
    return cast_builder(builder)->ivar(tok).release();
}

foreign_ptr keyword_break(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                          const token *rparen) {
    return cast_builder(builder)->keyword_break(keyword, lparen, convert_node_list(args), rparen).release();
}

foreign_ptr keyword_defined(self_ptr builder, const token *keyword, foreign_ptr arg) {
    return cast_builder(builder)->keyword_defined(keyword, cast_node(arg)).release();
}

foreign_ptr keyword_next(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                         const token *rparen) {
    return cast_builder(builder)->keyword_next(keyword, lparen, convert_node_list(args), rparen).release();
}

foreign_ptr keyword_redo(self_ptr builder, const token *keyword) {
    return cast_builder(builder)->keyword_redo(keyword).release();
}

foreign_ptr keyword_retry(self_ptr builder, const token *keyword) {
    return cast_builder(builder)->keyword_retry(keyword).release();
}

foreign_ptr keyword_return(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                           const token *rparen) {
    return cast_builder(builder)->keyword_return(keyword, lparen, convert_node_list(args), rparen).release();
}

foreign_ptr keyword_super(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                          const token *rparen) {
    return cast_builder(builder)->keyword_super(keyword, lparen, convert_node_list(args), rparen).release();
}

foreign_ptr keyword_yield(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                          const token *rparen) {
    return cast_builder(builder)->keyword_yield(keyword, lparen, convert_node_list(args), rparen).release();
}

foreign_ptr keyword_zsuper(self_ptr builder, const token *keyword) {
    return cast_builder(builder)->keyword_zsuper(keyword).release();
}

foreign_ptr kwarg(self_ptr builder, const token *name) {
    return cast_builder(builder)->kwarg(name).release();
}

foreign_ptr kwoptarg(self_ptr builder, const token *name, foreign_ptr value) {
    return cast_builder(builder)->kwoptarg(name, cast_node(value)).release();
}

foreign_ptr kwrestarg(self_ptr builder, const token *dstar, const token *name) {
    return cast_builder(builder)->kwrestarg(dstar, name).release();
}

foreign_ptr kwsplat(self_ptr builder, const token *dstar, foreign_ptr arg) {
    return cast_builder(builder)->kwsplat(dstar, cast_node(arg)).release();
}

foreign_ptr line_literal(self_ptr builder, const token *tok) {
    return cast_builder(builder)->line_literal(tok).release();
}

foreign_ptr logical_and(self_ptr builder, foreign_ptr lhs, const token *op, foreign_ptr rhs) {
    return cast_builder(builder)->logical_and(cast_node(lhs), op, cast_node(rhs)).release();
}

foreign_ptr logical_or(self_ptr builder, foreign_ptr lhs, const token *op, foreign_ptr rhs) {
    return cast_builder(builder)->logical_or(cast_node(lhs), op, cast_node(rhs)).release();
}

foreign_ptr loop_until(self_ptr builder, const token *keyword, foreign_ptr cond, const token *do_, foreign_ptr body,
                       const token *end) {
    return cast_builder(builder)->loop_until(keyword, cast_node(cond), do_, cast_node(body), end).release();
}

foreign_ptr loop_until_mod(self_ptr builder, foreign_ptr body, foreign_ptr cond) {
    return cast_builder(builder)->loop_until_mod(cast_node(body), cast_node(cond)).release();
}

foreign_ptr loop_while(self_ptr builder, const token *keyword, foreign_ptr cond, const token *do_, foreign_ptr body,
                       const token *end) {
    return cast_builder(builder)->loop_while(keyword, cast_node(cond), do_, cast_node(body), end).release();
}

foreign_ptr loop_while_mod(self_ptr builder, foreign_ptr body, foreign_ptr cond) {
    return cast_builder(builder)->loop_while_mod(cast_node(body), cast_node(cond)).release();
}

foreign_ptr match_op(self_ptr builder, foreign_ptr receiver, const token *oper, foreign_ptr arg) {
    return cast_builder(builder)->match_op(cast_node(receiver), oper, cast_node(arg)).release();
}

foreign_ptr multi_assign(self_ptr builder, foreign_ptr mlhs, foreign_ptr rhs) {
    return cast_builder(builder)->multi_assign(cast_node(mlhs), cast_node(rhs)).release();
}

foreign_ptr multi_lhs(self_ptr builder, const token *begin, const node_list *items, const token *end) {
    return cast_builder(builder)->multi_lhs(begin, convert_node_list(items), end).release();
}

foreign_ptr multi_lhs1(self_ptr builder, const token *begin, foreign_ptr item, const token *end) {
    return cast_builder(builder)->multi_lhs1(begin, cast_node(item), end).release();
}

foreign_ptr negate(self_ptr builder, const token *uminus, foreign_ptr numeric) {
    return cast_builder(builder)->negate(uminus, cast_node(numeric)).release();
}

foreign_ptr nil(self_ptr builder, const token *tok) {
    return cast_builder(builder)->nil(tok).release();
}

foreign_ptr not_op(self_ptr builder, const token *not_, const token *begin, foreign_ptr receiver, const token *end) {
    return cast_builder(builder)->not_op(not_, begin, cast_node(receiver), end).release();
}

foreign_ptr nth_ref(self_ptr builder, const token *tok) {
    return cast_builder(builder)->nth_ref(tok).release();
}

foreign_ptr op_assign(self_ptr builder, foreign_ptr lhs, const token *op, foreign_ptr rhs) {
    return cast_builder(builder)->op_assign(cast_node(lhs), op, cast_node(rhs)).release();
}

foreign_ptr optarg_(self_ptr builder, const token *name, const token *eql, foreign_ptr value) {
    return cast_builder(builder)->optarg_(name, eql, cast_node(value)).release();
}

foreign_ptr pair(self_ptr builder, foreign_ptr key, const token *assoc, foreign_ptr value) {
    return cast_builder(builder)->pair(cast_node(key), assoc, cast_node(value)).release();
}

foreign_ptr pair_keyword(self_ptr builder, const token *key, foreign_ptr value) {
    return cast_builder(builder)->pair_keyword(key, cast_node(value)).release();
}

foreign_ptr pair_quoted(self_ptr builder, const token *begin, const node_list *parts, const token *end,
                        foreign_ptr value) {
    return cast_builder(builder)->pair_quoted(begin, convert_node_list(parts), end, cast_node(value)).release();
}

foreign_ptr postexe(self_ptr builder, const token *begin, foreign_ptr node, const token *rbrace) {
    return cast_builder(builder)->postexe(begin, cast_node(node), rbrace).release();
}

foreign_ptr preexe(self_ptr builder, const token *begin, foreign_ptr node, const token *rbrace) {
    return cast_builder(builder)->preexe(begin, cast_node(node), rbrace).release();
}

foreign_ptr procarg0(self_ptr builder, foreign_ptr arg) {
    return cast_builder(builder)->procarg0(cast_node(arg)).release();
}

foreign_ptr range_exclusive(self_ptr builder, foreign_ptr lhs, const token *oper, foreign_ptr rhs) {
    return cast_builder(builder)->range_exclusive(cast_node(lhs), oper, cast_node(rhs)).release();
}

foreign_ptr range_inclusive(self_ptr builder, foreign_ptr lhs, const token *oper, foreign_ptr rhs) {
    return cast_builder(builder)->range_inclusive(cast_node(lhs), oper, cast_node(rhs)).release();
}

foreign_ptr rational(self_ptr builder, const token *tok) {
    return cast_builder(builder)->rational(tok).release();
}

foreign_ptr rational_complex(self_ptr builder, const token *tok) {
    return cast_builder(builder)->rational_complex(tok).release();
}

foreign_ptr regexp_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end,
                           foreign_ptr options) {
    return cast_builder(builder)->regexp_compose(begin, convert_node_list(parts), end, cast_node(options)).release();
}

foreign_ptr regexp_options(self_ptr builder, const token *regopt) {
    return cast_builder(builder)->regexp_options(regopt).release();
}

foreign_ptr rescue_body(self_ptr builder, const token *rescue, foreign_ptr exc_list, const token *assoc,
                        foreign_ptr exc_var, const token *then, foreign_ptr body) {
    return cast_builder(builder)
        ->rescue_body(rescue, cast_node(exc_list), assoc, cast_node(exc_var), then, cast_node(body))
        .release();
}

foreign_ptr restarg(self_ptr builder, const token *star, const token *name) {
    return cast_builder(builder)->restarg(star, name).release();
}

foreign_ptr self_(self_ptr builder, const token *tok) {
    return cast_builder(builder)->self_(tok).release();
}

foreign_ptr shadowarg(self_ptr builder, const token *name) {
    return cast_builder(builder)->shadowarg(name).release();
}

foreign_ptr splat(self_ptr builder, const token *star, foreign_ptr arg) {
    return cast_builder(builder)->splat(star, cast_node(arg)).release();
}

foreign_ptr splat_mlhs(self_ptr builder, const token *star, foreign_ptr arg) {
    return cast_builder(builder)->splat_mlhs(star, cast_node(arg)).release();
}

foreign_ptr string(self_ptr builder, const token *string_) {
    return cast_builder(builder)->string(string_).release();
}

foreign_ptr string_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    return cast_builder(builder)->string_compose(begin, convert_node_list(parts), end).release();
}

foreign_ptr string_internal(self_ptr builder, const token *string_) {
    return cast_builder(builder)->string_internal(string_).release();
}

foreign_ptr symbol(self_ptr builder, const token *symbol) {
    return cast_builder(builder)->symbol(symbol).release();
}

foreign_ptr symbol_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    return cast_builder(builder)->symbol_compose(begin, convert_node_list(parts), end).release();
}

foreign_ptr symbol_internal(self_ptr builder, const token *symbol) {
    return cast_builder(builder)->symbol_internal(symbol).release();
}

foreign_ptr symbols_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    return cast_builder(builder)->symbols_compose(begin, convert_node_list(parts), end).release();
}

foreign_ptr ternary(self_ptr builder, foreign_ptr cond, const token *question, foreign_ptr if_true, const token *colon,
                    foreign_ptr if_false) {
    return cast_builder(builder)
        ->ternary(cast_node(cond), question, cast_node(if_true), colon, cast_node(if_false))
        .release();
}

foreign_ptr tr_any(self_ptr builder, const token *special) {
    return cast_builder(builder)->tr_any(special).release();
}

foreign_ptr tr_arg_instance(self_ptr builder, foreign_ptr base, const node_list *types, const token *end) {
    return cast_builder(builder)->tr_arg_instance(cast_node(base), convert_node_list(types), end).release();
}

foreign_ptr tr_array(self_ptr builder, const token *begin, foreign_ptr type_, const token *end) {
    return cast_builder(builder)->tr_array(begin, cast_node(type_), end).release();
}

foreign_ptr tr_cast(self_ptr builder, const token *begin, foreign_ptr expr, const token *colon, foreign_ptr type_,
                    const token *end) {
    return cast_builder(builder)->tr_cast(begin, cast_node(expr), colon, cast_node(type_), end).release();
}

foreign_ptr tr_class(self_ptr builder, const token *special) {
    return cast_builder(builder)->tr_class(special).release();
}

foreign_ptr tr_consubtype(self_ptr builder, foreign_ptr sub, foreign_ptr super_) {
    return cast_builder(builder)->tr_consubtype(cast_node(sub), cast_node(super_)).release();
}

foreign_ptr tr_conunify(self_ptr builder, foreign_ptr a, foreign_ptr b) {
    return cast_builder(builder)->tr_conunify(cast_node(a), cast_node(b)).release();
}

foreign_ptr tr_cpath(self_ptr builder, foreign_ptr cpath) {
    return cast_builder(builder)->tr_cpath(cast_node(cpath)).release();
}

foreign_ptr tr_genargs(self_ptr builder, const token *begin, const node_list *genargs, const node_list *constraints,
                       const token *end) {
    return cast_builder(builder)
        ->tr_genargs(begin, convert_node_list(genargs), convert_node_list(constraints), end)
        .release();
}

foreign_ptr tr_gendecl(self_ptr builder, foreign_ptr cpath, const token *begin, const node_list *genargs,
                       const node_list *constraints, const token *end) {
    return cast_builder(builder)
        ->tr_gendecl(cast_node(cpath), begin, convert_node_list(genargs), convert_node_list(constraints), end)
        .release();
}

foreign_ptr tr_gendeclarg(self_ptr builder, const token *tok, foreign_ptr constraint) {
    return cast_builder(builder)->tr_gendeclarg(tok, cast_node(constraint)).release();
}

foreign_ptr tr_geninst(self_ptr builder, foreign_ptr cpath, const token *begin, const node_list *genargs,
                       const token *end) {
    return cast_builder(builder)->tr_geninst(cast_node(cpath), begin, convert_node_list(genargs), end).release();
}

foreign_ptr tr_hash(self_ptr builder, const token *begin, foreign_ptr key_type, const token *assoc,
                    foreign_ptr value_type, const token *end) {
    return cast_builder(builder)->tr_hash(begin, cast_node(key_type), assoc, cast_node(value_type), end).release();
}

foreign_ptr tr_instance(self_ptr builder, const token *special) {
    return cast_builder(builder)->tr_instance(special).release();
}

foreign_ptr tr_ivardecl(self_ptr builder, const token *def, const token *name, foreign_ptr type_) {
    return cast_builder(builder)->tr_ivardecl(def, name, cast_node(type_)).release();
}

foreign_ptr tr_nil(self_ptr builder, const token *nil) {
    return cast_builder(builder)->tr_nil(nil).release();
}

foreign_ptr tr_nillable(self_ptr builder, const token *tilde, foreign_ptr type_) {
    return cast_builder(builder)->tr_nillable(tilde, cast_node(type_)).release();
}

foreign_ptr tr_or(self_ptr builder, foreign_ptr a, foreign_ptr b) {
    return cast_builder(builder)->tr_or(cast_node(a), cast_node(b)).release();
}

foreign_ptr tr_proc(self_ptr builder, const token *begin, foreign_ptr args, const token *end) {
    return cast_builder(builder)->tr_proc(begin, cast_node(args), end).release();
}

foreign_ptr tr_prototype(self_ptr builder, foreign_ptr genargs, foreign_ptr args, foreign_ptr return_type) {
    return cast_builder(builder)->tr_prototype(cast_node(genargs), cast_node(args), cast_node(return_type)).release();
}

foreign_ptr tr_returnsig(self_ptr builder, const token *arrow, foreign_ptr ret) {
    return cast_builder(builder)->tr_returnsig(arrow, cast_node(ret)).release();
}

foreign_ptr tr_self(self_ptr builder, const token *special) {
    return cast_builder(builder)->tr_self(special).release();
}

foreign_ptr tr_tuple(self_ptr builder, const token *begin, const node_list *types, const token *end) {
    return cast_builder(builder)->tr_tuple(begin, convert_node_list(types), end).release();
}

foreign_ptr tr_typed_arg(self_ptr builder, foreign_ptr type_, foreign_ptr arg) {
    return cast_builder(builder)->tr_typed_arg(cast_node(type_), cast_node(arg)).release();
}

foreign_ptr true_(self_ptr builder, const token *tok) {
    return cast_builder(builder)->true_(tok).release();
}

foreign_ptr unary_op(self_ptr builder, const token *oper, foreign_ptr receiver) {
    return cast_builder(builder)->unary_op(oper, cast_node(receiver)).release();
}

foreign_ptr undef_method(self_ptr builder, const token *undef, const node_list *name_list) {
    return cast_builder(builder)->undef_method(undef, convert_node_list(name_list)).release();
}

foreign_ptr when(self_ptr builder, const token *when, const node_list *patterns, const token *then, foreign_ptr body) {
    return cast_builder(builder)->when(when, convert_node_list(patterns), then, cast_node(body)).release();
}

foreign_ptr word(self_ptr builder, const node_list *parts) {
    return cast_builder(builder)->word(convert_node_list(parts)).release();
}

foreign_ptr words_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    return cast_builder(builder)->words_compose(begin, convert_node_list(parts), end).release();
}

foreign_ptr xstring_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    return cast_builder(builder)->xstring_compose(begin, convert_node_list(parts), end).release();
}
}; // namespace

namespace ruby_typer {
namespace parser {

unique_ptr<Node> Builder::build(ruby_parser::base_driver *driver) {
    impl_->driver_ = driver;
    return cast_node(driver->parse(impl_.get()));
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
    string,
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
