#include "parser/Builder.h"
#include "common/common.h"
#include "common/typecase.h"
#include "core/Names.h"
#include "parser/Dedenter.h"
#include "parser/parser.h"

#include "ruby_parser/builder.hh"
#include "ruby_parser/diagnostic.hh"

#include <algorithm>
#include <typeinfo>

using ruby_parser::ForeignPtr;
using ruby_parser::node_list;
using ruby_parser::SelfPtr;
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
                    int delta = 8 - indent % 8;
                    if (delta > spacesToRemove) {
                        // Prevent against underflow on unsigned integer.
                        // In this case, the tab doesn't get chomped.
                        out.push_back(ch);
                        spacesToRemove = 0;
                    } else {
                        spacesToRemove -= delta;
                    }
                    break;
                }
                default:
                    // String does not have anymore whitespace left to remove.
                    out.push_back(ch);
                    spacesToRemove = 0;
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
    Impl(GlobalState &gs, core::FileRef file) : gs_(gs) {
        this->maxOff_ = file.data(gs).source().size();
        foreignNodes_.emplace_back();
    }

    GlobalState &gs_;
    u2 uniqueCounter_ = 1;
    u4 maxOff_;
    ruby_parser::base_driver *driver_;

    vector<NodePtr> foreignNodes_;

    u4 clamp(u4 off) {
        return std::min(off, maxOff_);
    }

    core::LocOffsets tokLoc(const token *tok) {
        return core::LocOffsets{clamp((u4)tok->start()), clamp((u4)tok->end())};
    }

    core::LocOffsets tokLoc(const token *begin, const token *end) {
        return core::LocOffsets{clamp((u4)begin->start()), clamp((u4)end->end())};
    }

    core::LocOffsets collectionLoc(const token *begin, sorbet::parser::NodeVec &elts, const token *end) {
        if (begin != nullptr) {
            ENFORCE(end != nullptr);
            return tokLoc(begin, end);
        }
        ENFORCE(end == nullptr);
        if (elts.empty()) {
            return core::LocOffsets::none();
        }
        return elts.front().loc().join(elts.back().loc());
    }

    core::LocOffsets collectionLoc(sorbet::parser::NodeVec &elts) {
        return collectionLoc(nullptr, elts, nullptr);
    }

    NodePtr transformCondition(NodePtr cond) {
        if (auto *b = parser::cast_node<Begin>(cond)) {
            if (b->stmts.size() == 1) {
                b->stmts[0] = transformCondition(std::move(b->stmts[0]));
            }
        } else if (auto *a = parser::cast_node<And>(cond)) {
            a->left = transformCondition(std::move(a->left));
            a->right = transformCondition(std::move(a->right));
        } else if (auto *o = parser::cast_node<Or>(cond)) {
            o->left = transformCondition(std::move(o->left));
            o->right = transformCondition(std::move(o->right));
        } else if (auto *ir = parser::cast_node<IRange>(cond)) {
            return make_node<IFlipflop>(ir->loc, transformCondition(std::move(ir->from)),
                                        transformCondition(std::move(ir->to)));
        } else if (auto *er = parser::cast_node<ERange>(cond)) {
            return make_node<EFlipflop>(er->loc, transformCondition(std::move(er->from)),
                                        transformCondition(std::move(er->to)));
        } else if (auto *re = parser::cast_node<Regexp>(cond)) {
            return make_node<MatchCurLine>(re->loc, std::move(cond));
        }
        return cond;
    }

    void error(ruby_parser::dclass err, core::LocOffsets loc, std::string data = "") {
        driver_->external_diagnostic(ruby_parser::dlevel::ERROR, err, loc.beginPos(), loc.endPos(), data);
    }

    /* Begin callback methods */

    NodePtr accessible(NodePtr node) {
        if (auto *id = parser::cast_node<Ident>(node)) {
            auto name = id->name.data(gs_);
            ENFORCE(name->kind == core::NameKind::UTF8);
            auto name_str = name->show(gs_);
            if (isNumberedParameterName(name_str) && driver_->lex.context.inDynamicBlock()) {
                if (driver_->numparam_stack.seen_ordinary_params()) {
                    error(ruby_parser::dclass::OrdinaryParamDefined, id->loc);
                }

                auto raw_context = driver_->lex.context.stackCopy();
                auto raw_numparam_stack = driver_->numparam_stack.stackCopy();

                // ignore current block scope
                raw_context.pop_back();
                raw_numparam_stack.pop_back();

                for (auto outer_scope : raw_context) {
                    if (outer_scope == ruby_parser::Context::State::BLOCK ||
                        outer_scope == ruby_parser::Context::State::LAMBDA) {
                        auto outer_scope_has_numparams = raw_numparam_stack.back().max > 0;
                        raw_numparam_stack.pop_back();

                        if (outer_scope_has_numparams) {
                            error(ruby_parser::dclass::NumparamUsedInOuterScope, id->loc);
                        } else {
                            // for now it's ok, but an outer scope can also be a block
                            // with numparams, so we need to continue
                        }
                    } else {
                        // found an outer scope that can't have numparams
                        // like def/class/etc
                        break;
                    }
                }

                driver_->lex.declare(name_str);
                auto intro = make_node<LVar>(id->loc, id->name);
                auto decls = driver_->alloc.node_list();
                decls->emplace_back(toForeign(std::move(intro)));
                driver_->numparam_stack.regis(name_str[1] - 48, std::move(decls));
            }

            if (driver_->lex.is_declared(name_str)) {
                checkCircularArgumentReferences(node, name_str);
                return make_node<LVar>(id->loc, id->name);
            } else {
                return make_node<Send>(id->loc, nullptr, id->name, sorbet::parser::NodeVec());
            }
        }
        return node;
    }

    NodePtr alias(const token *alias, NodePtr to, NodePtr from) {
        return make_node<Alias>(tokLoc(alias).join(from.loc()), std::move(to), std::move(from));
    }

    NodePtr arg(const token *name) {
        return make_node<Arg>(tokLoc(name), gs_.enterNameUTF8(name->string()));
    }

    NodePtr args(const token *begin, sorbet::parser::NodeVec args, const token *end, bool check_args) {
        if (check_args) {
            UnorderedMap<std::string, core::LocOffsets> map;
            checkDuplicateArgs(args, map);
        }

        if (begin == nullptr && args.empty() && end == nullptr) {
            return nullptr;
        }
        return make_node<Args>(collectionLoc(begin, args, end), std::move(args));
    }

    NodePtr array(const token *begin, sorbet::parser::NodeVec elements, const token *end) {
        return make_node<Array>(collectionLoc(begin, elements, end), std::move(elements));
    }

    NodePtr assign(NodePtr lhs, const token *eql, NodePtr rhs) {
        core::LocOffsets loc = lhs.loc().join(rhs.loc());

        if (auto *s = parser::cast_node<Send>(lhs)) {
            s->args.emplace_back(std::move(rhs));
            return make_node<Send>(loc, std::move(s->receiver), s->method, std::move(s->args));
        } else if (auto *s = parser::cast_node<CSend>(lhs)) {
            s->args.emplace_back(std::move(rhs));
            return make_node<CSend>(loc, std::move(s->receiver), s->method, std::move(s->args));
        } else {
            return make_node<Assign>(loc, std::move(lhs), std::move(rhs));
        }
    }

    NodePtr assignable(NodePtr node) {
        if (auto *id = parser::cast_node<Ident>(node)) {
            auto name = id->name.data(gs_);
            auto name_str = name->show(gs_);
            if (isNumberedParameterName(name_str) && driver_->lex.context.inDynamicBlock()) {
                error(ruby_parser::dclass::CantAssignToNumparam, id->loc, name_str);
            }
            driver_->lex.declare(name_str);
            return make_node<LVarLhs>(id->loc, id->name);
        } else if (auto *iv = parser::cast_node<IVar>(node)) {
            return make_node<IVarLhs>(iv->loc, iv->name);
        } else if (auto *c = parser::cast_node<Const>(node)) {
            if (!driver_->lex.context.dynamicConstDefintinionAllowed()) {
                error(ruby_parser::dclass::DynamicConst, c->loc);
            }
            return make_node<ConstLhs>(c->loc, std::move(c->scope), c->name);
        } else if (auto *cv = parser::cast_node<CVar>(node)) {
            return make_node<CVarLhs>(cv->loc, cv->name);
        } else if (auto *gv = parser::cast_node<GVar>(node)) {
            return make_node<GVarLhs>(gv->loc, gv->name);
        } else if (parser::isa_node<Backref>(node) || parser::isa_node<NthRef>(node)) {
            error(ruby_parser::dclass::BackrefAssignment, node.loc());
            return make_node<Nil>(node.loc());
        } else {
            error(ruby_parser::dclass::InvalidAssignment, node.loc());
            return make_node<Nil>(node.loc());
        }
    }

    NodePtr associate(const token *begin, sorbet::parser::NodeVec pairs, const token *end) {
        ENFORCE((begin == nullptr && end == nullptr) || (begin != nullptr && end != nullptr));
        auto isKwargs = begin == nullptr && end == nullptr;
        return make_node<Hash>(collectionLoc(begin, pairs, end), isKwargs, std::move(pairs));
    }

    NodePtr attrAsgn(NodePtr receiver, const token *dot, const token *selector, bool masgn) {
        core::NameRef method = gs_.enterNameUTF8(selector->string() + "=");
        core::LocOffsets loc = receiver.loc().join(tokLoc(selector));
        if ((dot != nullptr) && dot->string() == "&.") {
            if (masgn) {
                error(ruby_parser::dclass::CSendInLHSOfMAsgn, tokLoc(dot));
            }
            return make_node<CSend>(loc, std::move(receiver), method, sorbet::parser::NodeVec());
        }
        return make_node<Send>(loc, std::move(receiver), method, sorbet::parser::NodeVec());
    }

    NodePtr backRef(const token *tok) {
        return make_node<Backref>(tokLoc(tok), gs_.enterNameUTF8(tok->string()));
    }

    NodePtr begin(const token *begin, NodePtr body, const token *end) {
        core::LocOffsets loc;
        if (begin != nullptr) {
            loc = tokLoc(begin).join(tokLoc(end));
        } else {
            loc = body.loc();
        }

        if (body == nullptr) {
            return make_node<Begin>(loc, sorbet::parser::NodeVec());
        }
        if (auto *b = parser::cast_node<Begin>(body)) {
            if (begin == nullptr && end == nullptr) {
                // Synthesized (begin) from compstmt "a; b" or (mlhs)
                // from multi_lhs "(a, b) = *foo".
                return body;
            }
        }
        if (auto *m = parser::cast_node<Mlhs>(body)) {
            return body;
        }
        sorbet::parser::NodeVec stmts;
        stmts.emplace_back(std::move(body));
        return make_node<Begin>(loc, std::move(stmts));
    }

    NodePtr beginBody(NodePtr body, sorbet::parser::NodeVec rescueBodies, const token *elseTok, NodePtr else_,
                      const token *ensure_tok, NodePtr ensure) {
        if (!rescueBodies.empty()) {
            if (else_ == nullptr) {
                body = make_node<Rescue>(body.loc().join(rescueBodies.back().loc()), std::move(body),
                                         std::move(rescueBodies), nullptr);
            } else {
                body = make_node<Rescue>(body.loc().join(else_.loc()), std::move(body), std::move(rescueBodies),
                                         std::move(else_));
            }
        } else if (elseTok != nullptr) {
            sorbet::parser::NodeVec stmts;
            if (body != nullptr) {
                if (auto *b = parser::cast_node<Begin>(body)) {
                    stmts = std::move(b->stmts);
                } else {
                    stmts.emplace_back(std::move(body));
                }
            }
            sorbet::parser::NodeVec else_stmts;
            auto elseLoc = tokLoc(elseTok).join(else_.loc());
            else_stmts.emplace_back(std::move(else_));
            stmts.emplace_back(make_node<Begin>(elseLoc, std::move(else_stmts)));

            body = make_node<Begin>(collectionLoc(stmts), std::move(stmts));
        }

        if (ensure_tok != nullptr) {
            core::LocOffsets loc;
            if (body != nullptr) {
                loc = body.loc();
            } else {
                loc = tokLoc(ensure_tok);
            }
            loc = loc.join(ensure.loc());
            body = make_node<Ensure>(loc, std::move(body), std::move(ensure));
        }

        return body;
    }

    NodePtr beginKeyword(const token *begin, NodePtr body, const token *end) {
        core::LocOffsets loc = tokLoc(begin).join(tokLoc(end));
        if (body != nullptr) {
            if (auto *b = parser::cast_node<Begin>(body)) {
                return make_node<Kwbegin>(loc, std::move(b->stmts));
            } else {
                sorbet::parser::NodeVec nodes;
                nodes.emplace_back(std::move(body));
                return make_node<Kwbegin>(loc, std::move(nodes));
            }
        }
        return make_node<Kwbegin>(loc, sorbet::parser::NodeVec());
    }

    NodePtr binaryOp(NodePtr receiver, const token *oper, NodePtr arg) {
        core::LocOffsets loc = receiver.loc().join(arg.loc());

        sorbet::parser::NodeVec args;
        args.emplace_back(std::move(arg));

        return make_node<Send>(loc, std::move(receiver), gs_.enterNameUTF8(oper->string()), std::move(args));
    }

    NodePtr block(NodePtr methodCall, const token *begin, NodePtr args, NodePtr body, const token *end) {
        if (auto *y = parser::cast_node<Yield>(methodCall)) {
            error(ruby_parser::dclass::BlockGivenToYield, y->loc);
            return make_node<Yield>(y->loc, sorbet::parser::NodeVec());
        }

        sorbet::parser::NodeVec *callargs = nullptr;
        if (auto *s = parser::cast_node<Send>(methodCall)) {
            callargs = &s->args;
        }
        if (auto *s = parser::cast_node<CSend>(methodCall)) {
            callargs = &s->args;
        }
        if (auto *s = parser::cast_node<Super>(methodCall)) {
            callargs = &s->args;
        }
        if (callargs != nullptr && !callargs->empty()) {
            if (auto *bp = parser::cast_node<BlockPass>(callargs->back())) {
                error(ruby_parser::dclass::BlockAndBlockarg, bp->loc);
            }
        }

        bool isNumblock = false;
        if (auto *numparams = parser::cast_node<NumParams>(args)) {
            isNumblock = true;
        }

        auto ty = methodCall.tag();
        if (ty == NodePtr::Tag::Send || ty == NodePtr::Tag::CSend || ty == NodePtr::Tag::Super ||
            ty == NodePtr::Tag::ZSuper) {
            if (isNumblock) {
                return make_node<NumBlock>(methodCall.loc().join(tokLoc(end)), std::move(methodCall), std::move(args),
                                           std::move(body));
            }
            return make_node<Block>(methodCall.loc().join(tokLoc(end)), std::move(methodCall), std::move(args),
                                    std::move(body));
        }

        sorbet::parser::NodeVec *exprs;
        typecase(
            methodCall, [&](Break &b) { exprs = &b.exprs; },

            [&](Return &r) { exprs = &r.exprs; },

            [&](Next &n) { exprs = &n.exprs; },

            [&](NodePtr &n) { Exception::raise("Unexpected send node: {}", n.nodeName()); });

        auto &send = exprs->front();
        core::LocOffsets blockLoc = send.loc().join(tokLoc(end));
        NodePtr block;
        if (isNumblock) {
            block = make_node<NumBlock>(blockLoc, std::move(send), std::move(args), std::move(body));
        } else {
            block = make_node<Block>(blockLoc, std::move(send), std::move(args), std::move(body));
        }
        exprs->front().swap(block);
        return methodCall;
    }

    NodePtr blockPass(const token *amper, NodePtr arg) {
        return make_node<BlockPass>(tokLoc(amper).join(arg.loc()), std::move(arg));
    }

    NodePtr blockarg(const token *amper, const token *name) {
        core::LocOffsets loc;
        core::NameRef nm;

        if (name != nullptr) {
            loc = tokLoc(name);
            nm = gs_.enterNameUTF8(name->string());
        } else {
            loc = tokLoc(amper);
            nm = gs_.freshNameUnique(core::UniqueNameKind::Parser, core::Names::ampersand(), ++uniqueCounter_);
        }
        return make_node<Blockarg>(loc, nm);
    }

    NodePtr callLambda(const token *lambda) {
        auto loc = tokLoc(lambda);
        auto kernel = make_node<Const>(loc, nullptr, core::Names::Constants::Kernel());
        return make_node<Send>(loc, std::move(kernel), core::Names::lambda(), NodeVec());
    }

    NodePtr call_method(NodePtr receiver, const token *dot, const token *selector, const token *lparen,
                        sorbet::parser::NodeVec args, const token *rparen) {
        core::LocOffsets selectorLoc, startLoc;
        if (selector != nullptr) {
            selectorLoc = tokLoc(selector);
        } else {
            selectorLoc = tokLoc(dot);
        }
        if (receiver == nullptr) {
            startLoc = selectorLoc;
        } else {
            startLoc = receiver.loc();
        }

        core::LocOffsets loc;
        if (rparen != nullptr) {
            loc = startLoc.join(tokLoc(rparen));
        } else if (!args.empty()) {
            loc = startLoc.join(args.back().loc());
        } else {
            loc = startLoc.join(selectorLoc);
        }

        core::NameRef method;
        if (selector == nullptr) {
            // when the selector is missing, this is a use of the `call` method.
            method = core::Names::call();
        } else {
            method = gs_.enterNameUTF8(selector->string());
        }

        if ((dot != nullptr) && dot->string() == "&.") {
            return make_node<CSend>(loc, std::move(receiver), method, std::move(args));
        } else {
            return make_node<Send>(loc, std::move(receiver), method, std::move(args));
        }
    }

    NodePtr case_(const token *case_, NodePtr expr, sorbet::parser::NodeVec whenBodies, const token *elseTok,
                  NodePtr elseBody, const token *end) {
        return make_node<Case>(tokLoc(case_).join(tokLoc(end)), std::move(expr), std::move(whenBodies),
                               std::move(elseBody));
    }

    NodePtr character(const token *char_) {
        return make_node<String>(tokLoc(char_), gs_.enterNameUTF8(char_->string()));
    }

    NodePtr complex(const token *tok) {
        return make_node<Complex>(tokLoc(tok), tok->string());
    }

    NodePtr compstmt(sorbet::parser::NodeVec nodes) {
        switch (nodes.size()) {
            case 0:
                return nullptr;
            case 1:
                return std::move(nodes.back());
            default:
                return make_node<Begin>(collectionLoc(nodes), std::move(nodes));
        }
    }

    NodePtr condition(const token *cond_tok, NodePtr cond, const token *then, NodePtr ifTrue, const token *else_,
                      NodePtr ifFalse, const token *end) {
        core::LocOffsets loc = tokLoc(cond_tok).join(cond.loc());
        if (then != nullptr) {
            loc = loc.join(tokLoc(then));
        }
        if (ifTrue != nullptr) {
            loc = loc.join(ifTrue.loc());
        }
        if (else_ != nullptr) {
            loc = loc.join(tokLoc(else_));
        }
        if (ifFalse != nullptr) {
            loc = loc.join(ifFalse.loc());
        }
        if (end != nullptr) {
            loc = loc.join(tokLoc(end));
        }
        return make_node<If>(loc, transformCondition(std::move(cond)), std::move(ifTrue), std::move(ifFalse));
    }

    NodePtr conditionMod(NodePtr ifTrue, NodePtr ifFalse, NodePtr cond) {
        core::LocOffsets loc = cond.loc();
        if (ifTrue != nullptr) {
            loc = loc.join(ifTrue.loc());
        } else {
            loc = loc.join(ifFalse.loc());
        }
        return make_node<If>(loc, transformCondition(std::move(cond)), std::move(ifTrue), std::move(ifFalse));
    }

    NodePtr const_(const token *name) {
        return make_node<Const>(tokLoc(name), nullptr, gs_.enterNameConstant(name->string()));
    }

    NodePtr constFetch(NodePtr scope, const token *colon, const token *name) {
        return make_node<Const>(scope.loc().join(tokLoc(name)), std::move(scope),
                                gs_.enterNameConstant(name->string()));
    }

    NodePtr constGlobal(const token *colon, const token *name) {
        return make_node<Const>(tokLoc(colon).join(tokLoc(name)), make_node<Cbase>(tokLoc(colon)),
                                gs_.enterNameConstant(name->string()));
    }

    NodePtr constOpAssignable(NodePtr node) {
        if (auto *c = parser::cast_node<Const>(node)) {
            return make_node<ConstLhs>(c->loc, std::move(c->scope), c->name);
        }

        return node;
    }

    NodePtr cvar(const token *tok) {
        return make_node<CVar>(tokLoc(tok), gs_.enterNameUTF8(tok->string()));
    }

    NodePtr dedentString(NodePtr node, size_t dedentLevel) {
        if (dedentLevel == 0) {
            return node;
        }

        Dedenter dedenter(dedentLevel);
        NodePtr result;

        typecase(
            node,

            [&](String &s) {
                std::string dedented = dedenter.dedent(s.val.data(gs_)->shortName(gs_));
                result = make_node<String>(s.loc, gs_.enterNameUTF8(dedented));
            },

            [&](DString &d) {
                for (auto &p : d.nodes) {
                    if (auto *s = parser::cast_node<String>(p)) {
                        std::string dedented = dedenter.dedent(s->val.data(gs_)->shortName(gs_));
                        NodePtr newstr = make_node<String>(s->loc, gs_.enterNameUTF8(dedented));
                        p.swap(newstr);
                    }
                }
                result = std::move(node);
            },

            [&](XString &d) {
                for (auto &p : d.nodes) {
                    if (auto *s = parser::cast_node<String>(p)) {
                        std::string dedented = dedenter.dedent(s->val.data(gs_)->shortName(gs_));
                        NodePtr newstr = make_node<String>(s->loc, gs_.enterNameUTF8(dedented));
                        p.swap(newstr);
                    }
                }
                result = std::move(node);
            },

            [&](NodePtr &n) { Exception::raise("Unexpected dedent node: {}", n.nodeName()); });

        return result;
    }

    NodePtr def_class(const token *class_, NodePtr name, const token *lt_, NodePtr superclass, NodePtr body,
                      const token *end_) {
        core::LocOffsets declLoc = tokLoc(class_).join(name.loc()).join(superclass.loc());
        core::LocOffsets loc = tokLoc(class_, end_);

        return make_node<Class>(loc, declLoc, std::move(name), std::move(superclass), std::move(body));
    }

    NodePtr defMethod(const token *def, const token *name, NodePtr args, NodePtr body, const token *end) {
        core::LocOffsets declLoc = tokLoc(def, name).join(args.loc());
        core::LocOffsets loc = tokLoc(def, end);

        return make_node<DefMethod>(loc, declLoc, gs_.enterNameUTF8(name->string()), std::move(args), std::move(body));
    }

    NodePtr defModule(const token *module, NodePtr name, NodePtr body, const token *end_) {
        core::LocOffsets declLoc = tokLoc(module).join(name.loc());
        core::LocOffsets loc = tokLoc(module, end_);
        return make_node<Module>(loc, declLoc, std::move(name), std::move(body));
    }

    NodePtr def_sclass(const token *class_, const token *lshft_, NodePtr expr, NodePtr body, const token *end_) {
        core::LocOffsets declLoc = tokLoc(class_);
        core::LocOffsets loc = tokLoc(class_, end_);
        return make_node<SClass>(loc, declLoc, std::move(expr), std::move(body));
    }

    NodePtr defSingleton(const token *def, NodePtr definee, const token *dot, const token *name, NodePtr args,
                         NodePtr body, const token *end) {
        core::LocOffsets declLoc = tokLoc(def, name).join(args.loc());
        core::LocOffsets loc = tokLoc(def, end);

        if (isLiteralNode(definee)) {
            error(ruby_parser::dclass::SingletonLiteral, definee.loc());
        }

        return make_node<DefS>(loc, declLoc, std::move(definee), gs_.enterNameUTF8(name->string()), std::move(args),
                               std::move(body));
    }

    NodePtr encodingLiteral(const token *tok) {
        return make_node<EncodingLiteral>(tokLoc(tok));
    }

    NodePtr false_(const token *tok) {
        return make_node<False>(tokLoc(tok));
    }

    NodePtr fileLiteral(const token *tok) {
        return make_node<FileLiteral>(tokLoc(tok));
    }

    NodePtr float_(const token *tok) {
        return make_node<Float>(tokLoc(tok), tok->string());
    }

    NodePtr floatComplex(const token *tok) {
        return make_node<Complex>(tokLoc(tok), tok->string());
    }

    NodePtr for_(const token *for_, NodePtr iterator, const token *in_, NodePtr iteratee, const token *do_,
                 NodePtr body, const token *end) {
        return make_node<For>(tokLoc(for_).join(tokLoc(end)), std::move(iterator), std::move(iteratee),
                              std::move(body));
    }

    NodePtr gvar(const token *tok) {
        return make_node<GVar>(tokLoc(tok), gs_.enterNameUTF8(tok->string()));
    }

    NodePtr ident(const token *tok) {
        return make_node<Ident>(tokLoc(tok), gs_.enterNameUTF8(tok->string()));
    }

    NodePtr index(NodePtr receiver, const token *lbrack, sorbet::parser::NodeVec indexes, const token *rbrack) {
        return make_node<Send>(receiver.loc().join(tokLoc(rbrack)), std::move(receiver), core::Names::squareBrackets(),
                               std::move(indexes));
    }

    NodePtr indexAsgn(NodePtr receiver, const token *lbrack, sorbet::parser::NodeVec indexes, const token *rbrack) {
        return make_node<Send>(receiver.loc().join(tokLoc(rbrack)), std::move(receiver),
                               core::Names::squareBracketsEq(), std::move(indexes));
    }

    NodePtr integer(const token *tok) {
        return make_node<Integer>(tokLoc(tok), tok->string());
    }

    NodePtr ivar(const token *tok) {
        return make_node<IVar>(tokLoc(tok), gs_.enterNameUTF8(tok->string()));
    }

    NodePtr keywordBreak(const token *keyword, const token *lparen, sorbet::parser::NodeVec args, const token *rparen) {
        core::LocOffsets loc = tokLoc(keyword).join(collectionLoc(lparen, args, rparen));
        return make_node<Break>(loc, std::move(args));
    }

    NodePtr keywordDefined(const token *keyword, NodePtr arg) {
        return make_node<Defined>(tokLoc(keyword).join(arg.loc()), std::move(arg));
    }

    NodePtr keywordNext(const token *keyword, const token *lparen, sorbet::parser::NodeVec args, const token *rparen) {
        return make_node<Next>(tokLoc(keyword).join(collectionLoc(lparen, args, rparen)), std::move(args));
    }

    NodePtr keywordRedo(const token *keyword) {
        return make_node<Redo>(tokLoc(keyword));
    }

    NodePtr keywordRetry(const token *keyword) {
        return make_node<Retry>(tokLoc(keyword));
    }

    NodePtr keywordReturn(const token *keyword, const token *lparen, sorbet::parser::NodeVec args,
                          const token *rparen) {
        core::LocOffsets loc = tokLoc(keyword).join(collectionLoc(lparen, args, rparen));
        return make_node<Return>(loc, std::move(args));
    }

    NodePtr keywordSuper(const token *keyword, const token *lparen, sorbet::parser::NodeVec args, const token *rparen) {
        core::LocOffsets loc = tokLoc(keyword);
        core::LocOffsets argloc = collectionLoc(lparen, args, rparen);
        if (argloc.exists()) {
            loc = loc.join(argloc);
        }
        return make_node<Super>(loc, std::move(args));
    }

    NodePtr keywordYield(const token *keyword, const token *lparen, sorbet::parser::NodeVec args, const token *rparen) {
        core::LocOffsets loc = tokLoc(keyword).join(collectionLoc(lparen, args, rparen));
        if (!args.empty() && parser::isa_node<BlockPass>(args.back())) {
            error(ruby_parser::dclass::BlockGivenToYield, loc);
        }
        return make_node<Yield>(loc, std::move(args));
    }

    NodePtr keywordZsuper(const token *keyword) {
        return make_node<ZSuper>(tokLoc(keyword));
    }

    NodePtr kwarg(const token *name) {
        return make_node<Kwarg>(tokLoc(name), gs_.enterNameUTF8(name->string()));
    }

    NodePtr kwoptarg(const token *name, NodePtr value) {
        return make_node<Kwoptarg>(tokLoc(name).join(value.loc()), gs_.enterNameUTF8(name->string()), tokLoc(name),
                                   std::move(value));
    }

    NodePtr kwnilarg(const token *dstar, const token *nil) {
        return make_node<Kwnilarg>(tokLoc(dstar).join(tokLoc(nil)));
    }

    NodePtr kwrestarg(const token *dstar, const token *name) {
        core::LocOffsets loc = tokLoc(dstar);
        core::NameRef nm;

        if (name != nullptr) {
            loc = loc.join(tokLoc(name));
            nm = gs_.enterNameUTF8(name->string());
        } else {
            nm = gs_.freshNameUnique(core::UniqueNameKind::Parser, core::Names::starStar(), ++uniqueCounter_);
        }
        return make_node<Kwrestarg>(loc, nm);
    }

    NodePtr kwsplat(const token *dstar, NodePtr arg) {
        return make_node<Kwsplat>(tokLoc(dstar).join(arg.loc()), std::move(arg));
    }

    NodePtr line_literal(const token *tok) {
        return make_node<LineLiteral>(tokLoc(tok));
    }

    NodePtr logicalAnd(NodePtr lhs, const token *op, NodePtr rhs) {
        return make_node<And>(lhs.loc().join(rhs.loc()), std::move(lhs), std::move(rhs));
    }

    NodePtr logicalOr(NodePtr lhs, const token *op, NodePtr rhs) {
        return make_node<Or>(lhs.loc().join(rhs.loc()), std::move(lhs), std::move(rhs));
    }

    NodePtr loopUntil(const token *keyword, NodePtr cond, const token *do_, NodePtr body, const token *end) {
        return make_node<Until>(tokLoc(keyword).join(tokLoc(end)), std::move(cond), std::move(body));
    }

    NodePtr loopUntil_mod(NodePtr body, NodePtr cond) {
        if (parser::isa_node<Kwbegin>(body)) {
            return make_node<UntilPost>(body.loc().join(cond.loc()), std::move(cond), std::move(body));
        }

        return make_node<Until>(body.loc().join(cond.loc()), std::move(cond), std::move(body));
    }

    NodePtr loop_while(const token *keyword, NodePtr cond, const token *do_, NodePtr body, const token *end) {
        return make_node<While>(tokLoc(keyword).join(tokLoc(end)), std::move(cond), std::move(body));
    }

    NodePtr loop_while_mod(NodePtr body, NodePtr cond) {
        if (parser::isa_node<Kwbegin>(body)) {
            return make_node<WhilePost>(body.loc().join(cond.loc()), std::move(cond), std::move(body));
        }

        return make_node<While>(body.loc().join(cond.loc()), std::move(cond), std::move(body));
    }

    NodePtr match_op(NodePtr receiver, const token *oper, NodePtr arg) {
        // TODO(nelhage): If the LHS here is a regex literal with (?<...>..)
        // groups, Ruby will autovivify the match groups as locals. If we were
        // to support that, we'd need to analyze that here and call
        // `driver_->lex.declare`.
        core::LocOffsets loc = receiver.loc().join(arg.loc());
        sorbet::parser::NodeVec args;
        args.emplace_back(std::move(arg));
        return make_node<Send>(loc, std::move(receiver), gs_.enterNameUTF8(oper->string()), std::move(args));
    }

    NodePtr multi_assign(NodePtr mlhs, NodePtr rhs) {
        return make_node<Masgn>(mlhs.loc().join(rhs.loc()), std::move(mlhs), std::move(rhs));
    }

    NodePtr multi_lhs(const token *begin, sorbet::parser::NodeVec items, const token *end) {
        return make_node<Mlhs>(collectionLoc(begin, items, end), std::move(items));
    }

    NodePtr multi_lhs1(const token *begin, NodePtr item, const token *end) {
        if (auto *mlhs = parser::cast_node<Mlhs>(item)) {
            return item;
        }
        sorbet::parser::NodeVec args;
        args.emplace_back(std::move(item));
        return make_node<Mlhs>(collectionLoc(begin, args, end), std::move(args));
    }

    NodePtr nil(const token *tok) {
        return make_node<Nil>(tokLoc(tok));
    }

    NodePtr not_op(const token *not_, const token *begin, NodePtr receiver, const token *end) {
        if (receiver != nullptr) {
            core::LocOffsets loc;
            if (end != nullptr) {
                loc = tokLoc(not_).join(tokLoc(end));
            } else {
                loc = tokLoc(not_).join(receiver.loc());
            }
            return make_node<Send>(loc, transformCondition(std::move(receiver)), core::Names::bang(),
                                   sorbet::parser::NodeVec());
        }

        ENFORCE(begin != nullptr && end != nullptr);
        auto body = make_node<Begin>(tokLoc(begin).join(tokLoc(end)), sorbet::parser::NodeVec());
        return make_node<Send>(tokLoc(not_).join(body.loc()), std::move(body), core::Names::bang(),
                               sorbet::parser::NodeVec());
    }

    NodePtr nth_ref(const token *tok) {
        return make_node<NthRef>(tokLoc(tok), atoi(tok->string().c_str()));
    }

    NodePtr numparams(sorbet::parser::NodeVec declaringNodes) {
        ENFORCE(!declaringNodes.empty(), "NumParams node created without declaring node.");
        // During desugar we will create implicit arguments for the block based on on the highest
        // numparam used in it's body.
        // We will use the first node declaring a numbered parameter for the location of the implicit arg.
        // In the meantime, we need a loc for the NumParams node to pass the sanity check at the end of the
        // parsing phase so we arbitrary pick the first one from the node list (we know there is at least one).
        auto dummyLoc = declaringNodes.at(0).loc();
        return make_node<NumParams>(dummyLoc, std::move(declaringNodes));
    }

    NodePtr op_assign(NodePtr lhs, const token *op, NodePtr rhs) {
        if (parser::isa_node<Backref>(lhs) || parser::isa_node<NthRef>(lhs)) {
            error(ruby_parser::dclass::BackrefAssignment, lhs.loc());
        }

        if (op->string() == "&&") {
            return make_node<AndAsgn>(lhs.loc().join(rhs.loc()), std::move(lhs), std::move(rhs));
        }
        if (op->string() == "||") {
            return make_node<OrAsgn>(lhs.loc().join(rhs.loc()), std::move(lhs), std::move(rhs));
        }
        return make_node<OpAsgn>(lhs.loc().join(rhs.loc()), std::move(lhs), gs_.enterNameUTF8(op->string()),
                                 std::move(rhs));
    }

    NodePtr optarg_(const token *name, const token *eql, NodePtr value) {
        return make_node<Optarg>(tokLoc(name).join(value.loc()), gs_.enterNameUTF8(name->string()), tokLoc(name),
                                 std::move(value));
    }

    NodePtr pair(NodePtr key, const token *assoc, NodePtr value) {
        return make_node<Pair>(key.loc().join(value.loc()), std::move(key), std::move(value));
    }

    NodePtr pair_keyword(const token *key, NodePtr value) {
        auto keyLoc = core::LocOffsets{clamp((u4)key->start()), clamp((u4)key->end() - 1)}; // drop the trailing :

        return make_node<Pair>(tokLoc(key).join(value.loc()),
                               make_node<Symbol>(keyLoc, gs_.enterNameUTF8(key->string())), std::move(value));
    }

    NodePtr pair_quoted(const token *begin, sorbet::parser::NodeVec parts, const token *end, NodePtr value) {
        auto key = symbol_compose(begin, std::move(parts), end);
        return make_node<Pair>(tokLoc(begin).join(value.loc()), std::move(key), std::move(value));
    }

    NodePtr postexe(const token *begin, NodePtr node, const token *rbrace) {
        return make_node<Postexe>(tokLoc(begin).join(tokLoc(rbrace)), std::move(node));
    }

    NodePtr preexe(const token *begin, NodePtr node, const token *rbrace) {
        return make_node<Preexe>(tokLoc(begin).join(tokLoc(rbrace)), std::move(node));
    }

    NodePtr procarg0(NodePtr arg) {
        return arg;
    }

    NodePtr range_exclusive(NodePtr lhs, const token *oper, NodePtr rhs) {
        core::LocOffsets loc = lhs.loc().join(tokLoc(oper)).join(rhs.loc());
        return make_node<ERange>(loc, std::move(lhs), std::move(rhs));
    }

    NodePtr range_inclusive(NodePtr lhs, const token *oper, NodePtr rhs) {
        core::LocOffsets loc = lhs.loc().join(tokLoc(oper)).join(rhs.loc());
        return make_node<IRange>(loc, std::move(lhs), std::move(rhs));
    }

    NodePtr rational(const token *tok) {
        return make_node<Rational>(tokLoc(tok), tok->string());
    }

    NodePtr rational_complex(const token *tok) {
        // TODO(nelhage): We're losing this information that this was marked as
        // a Rational in the source.
        return make_node<Complex>(tokLoc(tok), tok->string());
    }

    NodePtr regexp_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end, NodePtr options) {
        core::LocOffsets loc = tokLoc(begin).join(tokLoc(end)).join(options.loc());
        return make_node<Regexp>(loc, std::move(parts), std::move(options));
    }

    NodePtr regexp_options(const token *regopt) {
        return make_node<Regopt>(tokLoc(regopt), regopt->string());
    }

    NodePtr rescue_body(const token *rescue, NodePtr excList, const token *assoc, NodePtr excVar, const token *then,
                        NodePtr body) {
        core::LocOffsets loc = tokLoc(rescue);
        if (excList != nullptr) {
            loc = loc.join(excList.loc());
        }
        if (excVar != nullptr) {
            loc = loc.join(excVar.loc());
        }
        if (body != nullptr) {
            loc = loc.join(body.loc());
        }
        return make_node<Resbody>(loc, std::move(excList), std::move(excVar), std::move(body));
    }

    NodePtr restarg(const token *star, const token *name) {
        core::LocOffsets loc = tokLoc(star);
        core::NameRef nm;
        core::LocOffsets nameLoc = loc;

        if (name != nullptr) {
            nameLoc = tokLoc(name);
            loc = loc.join(nameLoc);
            nm = gs_.enterNameUTF8(name->string());
        } else {
            // case like 'def m(*); end'
            nm = gs_.freshNameUnique(core::UniqueNameKind::Parser, core::Names::star(), ++uniqueCounter_);
        }
        return make_node<Restarg>(loc, nm, nameLoc);
    }

    NodePtr self_(const token *tok) {
        return make_node<Self>(tokLoc(tok));
    }

    NodePtr shadowarg(const token *name) {
        return make_node<Shadowarg>(tokLoc(name), gs_.enterNameUTF8(name->string()));
    }

    NodePtr splat(const token *star, NodePtr arg) {
        return make_node<Splat>(tokLoc(star).join(arg.loc()), std::move(arg));
    }

    NodePtr splat_mlhs(const token *star, NodePtr arg) {
        core::LocOffsets loc = tokLoc(star).join(arg.loc());
        return make_node<SplatLhs>(loc, std::move(arg));
    }

    NodePtr string(const token *string_) {
        return make_node<String>(tokLoc(string_), gs_.enterNameUTF8(string_->string()));
    }

    NodePtr string_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        if (collapseStringParts(parts)) {
            // only 1 child, either String or DString
            auto &firstPart = parts.front();

            if (begin == nullptr || end == nullptr) {
                if (auto *s = parser::cast_node<String>(firstPart)) {
                    return make_node<String>(s->loc, s->val);
                } else if (auto *d = parser::cast_node<DString>(firstPart)) {
                    return make_node<DString>(d->loc, std::move(d->nodes));
                } else {
                    return nullptr;
                }
            } else {
                auto *s = parser::cast_node<String>(firstPart);
                return make_node<String>(s->loc, s->val);
            }
        } else {
            core::LocOffsets loc = collectionLoc(begin, parts, end);
            return make_node<DString>(loc, std::move(parts));
        }
    }

    NodePtr string_internal(const token *string_) {
        return make_node<String>(tokLoc(string_), gs_.enterNameUTF8(string_->string()));
    }

    NodePtr symbol(const token *symbol) {
        return make_node<Symbol>(tokLoc(symbol), gs_.enterNameUTF8(symbol->string()));
    }

    NodePtr symbol_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        if (collapseStringParts(parts)) {
            // only 1 child: String
            auto &firstPart = parts.front();
            if (auto *s = parser::cast_node<String>(firstPart)) {
                return make_node<Symbol>(s->loc, s->val);
            } else {
                return nullptr;
            }
        } else {
            return make_node<DSymbol>(collectionLoc(begin, parts, end), std::move(parts));
        }
    }

    NodePtr symbol_internal(const token *symbol) {
        return make_node<Symbol>(tokLoc(symbol), gs_.enterNameUTF8(symbol->string()));
    }

    NodePtr symbols_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        core::LocOffsets loc = collectionLoc(begin, parts, end);
        sorbet::parser::NodeVec outParts;
        outParts.reserve(parts.size());
        for (auto &p : parts) {
            if (auto *s = parser::cast_node<String>(p)) {
                outParts.emplace_back(make_node<Symbol>(s->loc, s->val));
            } else if (auto *d = parser::cast_node<DString>(p)) {
                outParts.emplace_back(make_node<DSymbol>(d->loc, std::move(d->nodes)));
            } else {
                outParts.emplace_back(std::move(p));
            }
        }
        return make_node<Array>(loc, std::move(outParts));
    }

    NodePtr ternary(NodePtr cond, const token *question, NodePtr ifTrue, const token *colon, NodePtr ifFalse) {
        core::LocOffsets loc = cond.loc().join(ifFalse.loc());
        return make_node<If>(loc, transformCondition(std::move(cond)), std::move(ifTrue), std::move(ifFalse));
    }

    NodePtr true_(const token *tok) {
        return make_node<True>(tokLoc(tok));
    }

    NodePtr unary_op(const token *oper, NodePtr receiver) {
        core::LocOffsets loc = tokLoc(oper).join(receiver.loc());

        if (auto *num = parser::cast_node<Integer>(receiver)) {
            return make_node<Integer>(loc, oper->string() + num->val);
        }

        if (oper->type() != ruby_parser::token_type::tTILDE) {
            if (auto *num = parser::cast_node<Float>(receiver)) {
                return make_node<Float>(loc, oper->string() + num->val);
            }
            if (auto *num = parser::cast_node<Rational>(receiver)) {
                return make_node<Rational>(loc, oper->string() + num->val);
            }
        }

        core::NameRef op;
        if (oper->string() == "+") {
            op = core::Names::unaryPlus();
        } else if (oper->string() == "-") {
            op = core::Names::unaryMinus();
        } else {
            op = gs_.enterNameUTF8(oper->string());
        }

        return make_node<Send>(loc, std::move(receiver), op, sorbet::parser::NodeVec());
    }

    NodePtr undefMethod(const token *undef, sorbet::parser::NodeVec name_list) {
        core::LocOffsets loc = tokLoc(undef);
        if (!name_list.empty()) {
            loc = loc.join(name_list.back().loc());
        }
        return make_node<Undef>(loc, std::move(name_list));
    }

    NodePtr when(const token *when, sorbet::parser::NodeVec patterns, const token *then, NodePtr body) {
        core::LocOffsets loc = tokLoc(when);
        if (body != nullptr) {
            loc = loc.join(body.loc());
        } else if (then != nullptr) {
            loc = loc.join(tokLoc(then));
        } else {
            loc = loc.join(patterns.back().loc());
        }
        return make_node<When>(loc, std::move(patterns), std::move(body));
    }

    NodePtr word(sorbet::parser::NodeVec parts) {
        core::LocOffsets loc = collectionLoc(parts);
        if (collapseStringParts(parts)) {
            // a single String child
            auto &firstPart = parts.front();

            if (auto *s = parser::cast_node<String>(firstPart)) {
                return make_node<String>(s->loc, s->val);
            } else {
                return nullptr;
            }
        } else {
            return make_node<DString>(loc, std::move(parts));
        }
    }

    NodePtr words_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        return make_node<Array>(collectionLoc(begin, parts, end), std::move(parts));
    }

    NodePtr xstring_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        return make_node<XString>(collectionLoc(begin, parts, end), std::move(parts));
    }

    /* End callback methods */

    /* methods for marshalling to and from the parser's foreign pointers */

    NodePtr cast_node(ForeignPtr node) {
        auto off = reinterpret_cast<size_t>(node);
        if (off == 0) {
            return nullptr;
        }

        ENFORCE(foreignNodes_[off] != nullptr);
        return std::move(foreignNodes_[off]);
    }

    ForeignPtr toForeign(NodePtr node) {
        if (node == nullptr) {
            return reinterpret_cast<ForeignPtr>(0);
        }
        foreignNodes_.emplace_back(std::move(node));
        return reinterpret_cast<ForeignPtr>(foreignNodes_.size() - 1);
    }

    sorbet::parser::NodeVec convertNodeList(const node_list *cargs) {
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

    bool collapseStringParts(sorbet::parser::NodeVec &parts) {
        if (parts.size() != 1) {
            return false;
        }

        auto &firstPart = parts.front();

        return parser::isa_node<String>(firstPart) || parser::isa_node<DString>(firstPart);
    }

    void checkCircularArgumentReferences(const NodePtr &node, std::string name) {
        if (name == driver_->current_arg_stack.top()) {
            error(ruby_parser::dclass::CircularArgumentReference, node.loc(), name);
        }
    }

    void checkDuplicateArgs(sorbet::parser::NodeVec &args, UnorderedMap<std::string, core::LocOffsets> &map) {
        for (auto &this_arg : args) {
            if (auto *arg = parser::cast_node<Arg>(this_arg)) {
                checkDuplicateArg(arg->name.toString(gs_), arg->loc, map);
            } else if (auto *optarg = parser::cast_node<Optarg>(this_arg)) {
                checkDuplicateArg(optarg->name.toString(gs_), optarg->loc, map);
            } else if (auto *restarg = parser::cast_node<Restarg>(this_arg)) {
                checkDuplicateArg(restarg->name.toString(gs_), restarg->loc, map);
            } else if (auto *blockarg = parser::cast_node<Blockarg>(this_arg)) {
                checkDuplicateArg(blockarg->name.toString(gs_), blockarg->loc, map);
            } else if (auto *kwarg = parser::cast_node<Kwarg>(this_arg)) {
                checkDuplicateArg(kwarg->name.toString(gs_), kwarg->loc, map);
            } else if (auto *kwoptarg = parser::cast_node<Kwoptarg>(this_arg)) {
                checkDuplicateArg(kwoptarg->name.toString(gs_), kwoptarg->loc, map);
            } else if (auto *kwrestarg = parser::cast_node<Kwrestarg>(this_arg)) {
                checkDuplicateArg(kwrestarg->name.toString(gs_), kwrestarg->loc, map);
            } else if (auto *shadowarg = parser::cast_node<Shadowarg>(this_arg)) {
                checkDuplicateArg(shadowarg->name.toString(gs_), shadowarg->loc, map);
            } else if (auto *mlhs = parser::cast_node<Mlhs>(this_arg)) {
                checkDuplicateArgs(mlhs->exprs, map);
            }
        }
    }

    void checkDuplicateArg(std::string this_name, core::LocOffsets this_loc,
                           UnorderedMap<std::string, core::LocOffsets> &map) {
        auto that_arg_loc_it = map.find(this_name);

        if (that_arg_loc_it == map.end()) {
            map[this_name] = this_loc;
        } else if (argNameCollides(this_name)) {
            error(ruby_parser::dclass::DuplicateArgument, this_loc, this_name);
        }
    }

    bool argNameCollides(std::string name) {
        // Ignore everything beginning with underscore.
        return (name[0] != '_');
    }

    bool isLiteralNode(parser::NodePtr &node) {
        return parser::isa_node<Integer>(node) || parser::isa_node<String>(node) || parser::isa_node<DString>(node) ||
               parser::isa_node<Symbol>(node) || parser::isa_node<DSymbol>(node) || parser::isa_node<Regexp>(node) ||
               parser::isa_node<Array>(node) || parser::isa_node<Hash>(node);
    }

    bool isNumberedParameterName(std::string_view name) {
        return name.length() == 2 && name[0] == '_' && name[1] >= '1' && name[1] <= '9';
    }
};

Builder::Builder(GlobalState &gs, core::FileRef file) : impl_(new Builder::Impl(gs, file)) {}
Builder::~Builder() = default;

}; // namespace sorbet::parser

namespace {

using sorbet::parser::Builder;

Builder::Impl *cast_builder(SelfPtr builder) {
    return const_cast<Builder::Impl *>(reinterpret_cast<const Builder::Impl *>(builder));
}

ForeignPtr accessible(SelfPtr builder, ForeignPtr node) {
    auto build = cast_builder(builder);
    return build->toForeign(build->accessible(build->cast_node(node)));
}

ForeignPtr alias(SelfPtr builder, const token *alias, ForeignPtr to, ForeignPtr from) {
    auto build = cast_builder(builder);
    return build->toForeign(build->alias(alias, build->cast_node(to), build->cast_node(from)));
}

ForeignPtr arg(SelfPtr builder, const token *name) {
    auto build = cast_builder(builder);
    return build->toForeign(build->arg(name));
}

ForeignPtr args(SelfPtr builder, const token *begin, const node_list *args, const token *end, bool check_args) {
    auto build = cast_builder(builder);
    return build->toForeign(build->args(begin, build->convertNodeList(args), end, check_args));
}

ForeignPtr array(SelfPtr builder, const token *begin, const node_list *elements, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->array(begin, build->convertNodeList(elements), end));
}

ForeignPtr assign(SelfPtr builder, ForeignPtr lhs, const token *eql, ForeignPtr rhs) {
    auto build = cast_builder(builder);
    return build->toForeign(build->assign(build->cast_node(lhs), eql, build->cast_node(rhs)));
}

ForeignPtr assignable(SelfPtr builder, ForeignPtr node) {
    auto build = cast_builder(builder);
    return build->toForeign(build->assignable(build->cast_node(node)));
}

ForeignPtr associate(SelfPtr builder, const token *begin, const node_list *pairs, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->associate(begin, build->convertNodeList(pairs), end));
}

ForeignPtr attrAsgn(SelfPtr builder, ForeignPtr receiver, const token *dot, const token *selector, bool masgn) {
    auto build = cast_builder(builder);
    return build->toForeign(build->attrAsgn(build->cast_node(receiver), dot, selector, masgn));
}

ForeignPtr backRef(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->backRef(tok));
}

ForeignPtr begin(SelfPtr builder, const token *begin, ForeignPtr body, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->begin(begin, build->cast_node(body), end));
}

ForeignPtr beginBody(SelfPtr builder, ForeignPtr body, const node_list *rescueBodies, const token *elseTok,
                     ForeignPtr else_, const token *ensure_tok, ForeignPtr ensure) {
    auto build = cast_builder(builder);
    return build->toForeign(build->beginBody(build->cast_node(body), build->convertNodeList(rescueBodies), elseTok,
                                             build->cast_node(else_), ensure_tok, build->cast_node(ensure)));
}

ForeignPtr beginKeyword(SelfPtr builder, const token *begin, ForeignPtr body, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->beginKeyword(begin, build->cast_node(body), end));
}

ForeignPtr binaryOp(SelfPtr builder, ForeignPtr receiver, const token *oper, ForeignPtr arg) {
    auto build = cast_builder(builder);
    return build->toForeign(build->binaryOp(build->cast_node(receiver), oper, build->cast_node(arg)));
}

ForeignPtr block(SelfPtr builder, ForeignPtr methodCall, const token *begin, ForeignPtr args, ForeignPtr body,
                 const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(
        build->block(build->cast_node(methodCall), begin, build->cast_node(args), build->cast_node(body), end));
}

ForeignPtr blockPass(SelfPtr builder, const token *amper, ForeignPtr arg) {
    auto build = cast_builder(builder);
    return build->toForeign(build->blockPass(amper, build->cast_node(arg)));
}

ForeignPtr blockarg(SelfPtr builder, const token *amper, const token *name) {
    auto build = cast_builder(builder);
    return build->toForeign(build->blockarg(amper, name));
}

ForeignPtr callLambda(SelfPtr builder, const token *lambda) {
    auto build = cast_builder(builder);
    return build->toForeign(build->callLambda(lambda));
}

ForeignPtr call_method(SelfPtr builder, ForeignPtr receiver, const token *dot, const token *selector,
                       const token *lparen, const node_list *args, const token *rparen) {
    auto build = cast_builder(builder);
    return build->toForeign(
        build->call_method(build->cast_node(receiver), dot, selector, lparen, build->convertNodeList(args), rparen));
}

ForeignPtr case_(SelfPtr builder, const token *case_, ForeignPtr expr, const node_list *whenBodies,
                 const token *elseTok, ForeignPtr elseBody, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->case_(case_, build->cast_node(expr), build->convertNodeList(whenBodies), elseTok,
                                         build->cast_node(elseBody), end));
}

ForeignPtr character(SelfPtr builder, const token *char_) {
    auto build = cast_builder(builder);
    return build->toForeign(build->character(char_));
}

ForeignPtr complex(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->complex(tok));
}

ForeignPtr compstmt(SelfPtr builder, const node_list *node) {
    auto build = cast_builder(builder);
    return build->toForeign(build->compstmt(build->convertNodeList(node)));
}

ForeignPtr condition(SelfPtr builder, const token *cond_tok, ForeignPtr cond, const token *then, ForeignPtr ifTrue,
                     const token *else_, ForeignPtr ifFalse, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->condition(cond_tok, build->cast_node(cond), then, build->cast_node(ifTrue), else_,
                                             build->cast_node(ifFalse), end));
}

ForeignPtr conditionMod(SelfPtr builder, ForeignPtr ifTrue, ForeignPtr ifFalse, ForeignPtr cond) {
    auto build = cast_builder(builder);
    return build->toForeign(
        build->conditionMod(build->cast_node(ifTrue), build->cast_node(ifFalse), build->cast_node(cond)));
}

ForeignPtr const_(SelfPtr builder, const token *name) {
    auto build = cast_builder(builder);
    return build->toForeign(build->const_(name));
}

ForeignPtr constFetch(SelfPtr builder, ForeignPtr scope, const token *colon, const token *name) {
    auto build = cast_builder(builder);
    return build->toForeign(build->constFetch(build->cast_node(scope), colon, name));
}

ForeignPtr constGlobal(SelfPtr builder, const token *colon, const token *name) {
    auto build = cast_builder(builder);
    return build->toForeign(build->constGlobal(colon, name));
}

ForeignPtr constOpAssignable(SelfPtr builder, ForeignPtr node) {
    auto build = cast_builder(builder);
    return build->toForeign(build->constOpAssignable(build->cast_node(node)));
}

ForeignPtr cvar(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->cvar(tok));
}

ForeignPtr dedentString(SelfPtr builder, ForeignPtr node, size_t dedentLevel) {
    auto build = cast_builder(builder);
    return build->toForeign(build->dedentString(build->cast_node(node), dedentLevel));
}

ForeignPtr def_class(SelfPtr builder, const token *class_, ForeignPtr name, const token *lt_, ForeignPtr superclass,
                     ForeignPtr body, const token *end_) {
    auto build = cast_builder(builder);
    return build->toForeign(build->def_class(class_, build->cast_node(name), lt_, build->cast_node(superclass),
                                             build->cast_node(body), end_));
}

ForeignPtr defMethod(SelfPtr builder, const token *def, const token *name, ForeignPtr args, ForeignPtr body,
                     const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->defMethod(def, name, build->cast_node(args), build->cast_node(body), end));
}

ForeignPtr defModule(SelfPtr builder, const token *module, ForeignPtr name, ForeignPtr body, const token *end_) {
    auto build = cast_builder(builder);
    return build->toForeign(build->defModule(module, build->cast_node(name), build->cast_node(body), end_));
}

ForeignPtr def_sclass(SelfPtr builder, const token *class_, const token *lshft_, ForeignPtr expr, ForeignPtr body,
                      const token *end_) {
    auto build = cast_builder(builder);
    return build->toForeign(build->def_sclass(class_, lshft_, build->cast_node(expr), build->cast_node(body), end_));
}

ForeignPtr defSingleton(SelfPtr builder, const token *def, ForeignPtr definee, const token *dot, const token *name,
                        ForeignPtr args, ForeignPtr body, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->defSingleton(def, build->cast_node(definee), dot, name, build->cast_node(args),
                                                build->cast_node(body), end));
}

ForeignPtr encodingLiteral(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->encodingLiteral(tok));
}

ForeignPtr false_(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->false_(tok));
}

ForeignPtr fileLiteral(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->fileLiteral(tok));
}

ForeignPtr float_(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->float_(tok));
}

ForeignPtr floatComplex(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->floatComplex(tok));
}

ForeignPtr for_(SelfPtr builder, const token *for_, ForeignPtr iterator, const token *in_, ForeignPtr iteratee,
                const token *do_, ForeignPtr body, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->for_(for_, build->cast_node(iterator), in_, build->cast_node(iteratee), do_,
                                        build->cast_node(body), end));
}

ForeignPtr gvar(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->gvar(tok));
}

ForeignPtr ident(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->ident(tok));
}

ForeignPtr index(SelfPtr builder, ForeignPtr receiver, const token *lbrack, const node_list *indexes,
                 const token *rbrack) {
    auto build = cast_builder(builder);
    return build->toForeign(build->index(build->cast_node(receiver), lbrack, build->convertNodeList(indexes), rbrack));
}

ForeignPtr indexAsgn(SelfPtr builder, ForeignPtr receiver, const token *lbrack, const node_list *indexes,
                     const token *rbrack) {
    auto build = cast_builder(builder);
    return build->toForeign(
        build->indexAsgn(build->cast_node(receiver), lbrack, build->convertNodeList(indexes), rbrack));
}

ForeignPtr integer(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->integer(tok));
}

ForeignPtr ivar(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->ivar(tok));
}

ForeignPtr keywordBreak(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                        const token *rparen) {
    auto build = cast_builder(builder);
    return build->toForeign(build->keywordBreak(keyword, lparen, build->convertNodeList(args), rparen));
}

ForeignPtr keywordDefined(SelfPtr builder, const token *keyword, ForeignPtr arg) {
    auto build = cast_builder(builder);
    return build->toForeign(build->keywordDefined(keyword, build->cast_node(arg)));
}

ForeignPtr keywordNext(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                       const token *rparen) {
    auto build = cast_builder(builder);
    return build->toForeign(build->keywordNext(keyword, lparen, build->convertNodeList(args), rparen));
}

ForeignPtr keywordRedo(SelfPtr builder, const token *keyword) {
    auto build = cast_builder(builder);
    return build->toForeign(build->keywordRedo(keyword));
}

ForeignPtr keywordRetry(SelfPtr builder, const token *keyword) {
    auto build = cast_builder(builder);
    return build->toForeign(build->keywordRetry(keyword));
}

ForeignPtr keywordReturn(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                         const token *rparen) {
    auto build = cast_builder(builder);
    return build->toForeign(build->keywordReturn(keyword, lparen, build->convertNodeList(args), rparen));
}

ForeignPtr keywordSuper(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                        const token *rparen) {
    auto build = cast_builder(builder);
    return build->toForeign(build->keywordSuper(keyword, lparen, build->convertNodeList(args), rparen));
}

ForeignPtr keywordYield(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                        const token *rparen) {
    auto build = cast_builder(builder);
    return build->toForeign(build->keywordYield(keyword, lparen, build->convertNodeList(args), rparen));
}

ForeignPtr keywordZsuper(SelfPtr builder, const token *keyword) {
    auto build = cast_builder(builder);
    return build->toForeign(build->keywordZsuper(keyword));
}

ForeignPtr kwarg(SelfPtr builder, const token *name) {
    auto build = cast_builder(builder);
    return build->toForeign(build->kwarg(name));
}

ForeignPtr kwoptarg(SelfPtr builder, const token *name, ForeignPtr value) {
    auto build = cast_builder(builder);
    return build->toForeign(build->kwoptarg(name, build->cast_node(value)));
}

ForeignPtr kwnilarg(SelfPtr builder, const token *dstar, const token *nil) {
    auto build = cast_builder(builder);
    return build->toForeign(build->kwnilarg(dstar, nil));
}

ForeignPtr kwrestarg(SelfPtr builder, const token *dstar, const token *name) {
    auto build = cast_builder(builder);
    return build->toForeign(build->kwrestarg(dstar, name));
}

ForeignPtr kwsplat(SelfPtr builder, const token *dstar, ForeignPtr arg) {
    auto build = cast_builder(builder);
    return build->toForeign(build->kwsplat(dstar, build->cast_node(arg)));
}

ForeignPtr line_literal(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->line_literal(tok));
}

ForeignPtr logicalAnd(SelfPtr builder, ForeignPtr lhs, const token *op, ForeignPtr rhs) {
    auto build = cast_builder(builder);
    return build->toForeign(build->logicalAnd(build->cast_node(lhs), op, build->cast_node(rhs)));
}

ForeignPtr logicalOr(SelfPtr builder, ForeignPtr lhs, const token *op, ForeignPtr rhs) {
    auto build = cast_builder(builder);
    return build->toForeign(build->logicalOr(build->cast_node(lhs), op, build->cast_node(rhs)));
}

ForeignPtr loopUntil(SelfPtr builder, const token *keyword, ForeignPtr cond, const token *do_, ForeignPtr body,
                     const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->loopUntil(keyword, build->cast_node(cond), do_, build->cast_node(body), end));
}

ForeignPtr loopUntil_mod(SelfPtr builder, ForeignPtr body, ForeignPtr cond) {
    auto build = cast_builder(builder);
    return build->toForeign(build->loopUntil_mod(build->cast_node(body), build->cast_node(cond)));
}

ForeignPtr loop_while(SelfPtr builder, const token *keyword, ForeignPtr cond, const token *do_, ForeignPtr body,
                      const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->loop_while(keyword, build->cast_node(cond), do_, build->cast_node(body), end));
}

ForeignPtr loop_while_mod(SelfPtr builder, ForeignPtr body, ForeignPtr cond) {
    auto build = cast_builder(builder);
    return build->toForeign(build->loop_while_mod(build->cast_node(body), build->cast_node(cond)));
}

ForeignPtr match_op(SelfPtr builder, ForeignPtr receiver, const token *oper, ForeignPtr arg) {
    auto build = cast_builder(builder);
    return build->toForeign(build->match_op(build->cast_node(receiver), oper, build->cast_node(arg)));
}

ForeignPtr multi_assign(SelfPtr builder, ForeignPtr mlhs, ForeignPtr rhs) {
    auto build = cast_builder(builder);
    return build->toForeign(build->multi_assign(build->cast_node(mlhs), build->cast_node(rhs)));
}

ForeignPtr multi_lhs(SelfPtr builder, const token *begin, const node_list *items, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->multi_lhs(begin, build->convertNodeList(items), end));
}

ForeignPtr multi_lhs1(SelfPtr builder, const token *begin, ForeignPtr item, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->multi_lhs1(begin, build->cast_node(item), end));
}

ForeignPtr nil(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->nil(tok));
}

ForeignPtr not_op(SelfPtr builder, const token *not_, const token *begin, ForeignPtr receiver, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->not_op(not_, begin, build->cast_node(receiver), end));
}

ForeignPtr nth_ref(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->nth_ref(tok));
}

ForeignPtr numparams(SelfPtr builder, const node_list *declaringNodes) {
    auto build = cast_builder(builder);
    return build->toForeign(build->numparams(build->convertNodeList(declaringNodes)));
}

ForeignPtr numblock(SelfPtr builder, ForeignPtr methodCall, const token *begin, ForeignPtr args, ForeignPtr body,
                    const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(
        build->block(build->cast_node(methodCall), begin, build->cast_node(args), build->cast_node(body), end));
}

ForeignPtr op_assign(SelfPtr builder, ForeignPtr lhs, const token *op, ForeignPtr rhs) {
    auto build = cast_builder(builder);
    return build->toForeign(build->op_assign(build->cast_node(lhs), op, build->cast_node(rhs)));
}

ForeignPtr optarg_(SelfPtr builder, const token *name, const token *eql, ForeignPtr value) {
    auto build = cast_builder(builder);
    return build->toForeign(build->optarg_(name, eql, build->cast_node(value)));
}

ForeignPtr pair(SelfPtr builder, ForeignPtr key, const token *assoc, ForeignPtr value) {
    auto build = cast_builder(builder);
    return build->toForeign(build->pair(build->cast_node(key), assoc, build->cast_node(value)));
}

ForeignPtr pair_keyword(SelfPtr builder, const token *key, ForeignPtr value) {
    auto build = cast_builder(builder);
    return build->toForeign(build->pair_keyword(key, build->cast_node(value)));
}

ForeignPtr pair_quoted(SelfPtr builder, const token *begin, const node_list *parts, const token *end,
                       ForeignPtr value) {
    auto build = cast_builder(builder);
    return build->toForeign(build->pair_quoted(begin, build->convertNodeList(parts), end, build->cast_node(value)));
}

ForeignPtr postexe(SelfPtr builder, const token *begin, ForeignPtr node, const token *rbrace) {
    auto build = cast_builder(builder);
    return build->toForeign(build->postexe(begin, build->cast_node(node), rbrace));
}

ForeignPtr preexe(SelfPtr builder, const token *begin, ForeignPtr node, const token *rbrace) {
    auto build = cast_builder(builder);
    return build->toForeign(build->preexe(begin, build->cast_node(node), rbrace));
}

ForeignPtr procarg0(SelfPtr builder, ForeignPtr arg) {
    auto build = cast_builder(builder);
    return build->toForeign(build->procarg0(build->cast_node(arg)));
}

ForeignPtr range_exclusive(SelfPtr builder, ForeignPtr lhs, const token *oper, ForeignPtr rhs) {
    auto build = cast_builder(builder);
    return build->toForeign(build->range_exclusive(build->cast_node(lhs), oper, build->cast_node(rhs)));
}

ForeignPtr range_inclusive(SelfPtr builder, ForeignPtr lhs, const token *oper, ForeignPtr rhs) {
    auto build = cast_builder(builder);
    return build->toForeign(build->range_inclusive(build->cast_node(lhs), oper, build->cast_node(rhs)));
}

ForeignPtr rational(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->rational(tok));
}

ForeignPtr rational_complex(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->rational_complex(tok));
}

ForeignPtr regexp_compose(SelfPtr builder, const token *begin, const node_list *parts, const token *end,
                          ForeignPtr options) {
    auto build = cast_builder(builder);
    return build->toForeign(
        build->regexp_compose(begin, build->convertNodeList(parts), end, build->cast_node(options)));
}

ForeignPtr regexp_options(SelfPtr builder, const token *regopt) {
    auto build = cast_builder(builder);
    return build->toForeign(build->regexp_options(regopt));
}

ForeignPtr rescue_body(SelfPtr builder, const token *rescue, ForeignPtr excList, const token *assoc, ForeignPtr excVar,
                       const token *then, ForeignPtr body) {
    auto build = cast_builder(builder);
    return build->toForeign(build->rescue_body(rescue, build->cast_node(excList), assoc, build->cast_node(excVar), then,
                                               build->cast_node(body)));
}

ForeignPtr restarg(SelfPtr builder, const token *star, const token *name) {
    auto build = cast_builder(builder);
    return build->toForeign(build->restarg(star, name));
}

ForeignPtr self_(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->self_(tok));
}

ForeignPtr shadowarg(SelfPtr builder, const token *name) {
    auto build = cast_builder(builder);
    return build->toForeign(build->shadowarg(name));
}

ForeignPtr splat(SelfPtr builder, const token *star, ForeignPtr arg) {
    auto build = cast_builder(builder);
    return build->toForeign(build->splat(star, build->cast_node(arg)));
}

ForeignPtr splat_mlhs(SelfPtr builder, const token *star, ForeignPtr arg) {
    auto build = cast_builder(builder);
    return build->toForeign(build->splat_mlhs(star, build->cast_node(arg)));
}

ForeignPtr string_(SelfPtr builder, const token *string_) {
    auto build = cast_builder(builder);
    return build->toForeign(build->string(string_));
}

ForeignPtr string_compose(SelfPtr builder, const token *begin, const node_list *parts, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->string_compose(begin, build->convertNodeList(parts), end));
}

ForeignPtr string_internal(SelfPtr builder, const token *string_) {
    auto build = cast_builder(builder);
    return build->toForeign(build->string_internal(string_));
}

ForeignPtr symbol(SelfPtr builder, const token *symbol) {
    auto build = cast_builder(builder);
    return build->toForeign(build->symbol(symbol));
}

ForeignPtr symbol_compose(SelfPtr builder, const token *begin, const node_list *parts, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->symbol_compose(begin, build->convertNodeList(parts), end));
}

ForeignPtr symbol_internal(SelfPtr builder, const token *symbol) {
    auto build = cast_builder(builder);
    return build->toForeign(build->symbol_internal(symbol));
}

ForeignPtr symbols_compose(SelfPtr builder, const token *begin, const node_list *parts, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->symbols_compose(begin, build->convertNodeList(parts), end));
}

ForeignPtr ternary(SelfPtr builder, ForeignPtr cond, const token *question, ForeignPtr ifTrue, const token *colon,
                   ForeignPtr ifFalse) {
    auto build = cast_builder(builder);
    return build->toForeign(
        build->ternary(build->cast_node(cond), question, build->cast_node(ifTrue), colon, build->cast_node(ifFalse)));
}

ForeignPtr true_(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->true_(tok));
}

ForeignPtr unary_op(SelfPtr builder, const token *oper, ForeignPtr receiver) {
    auto build = cast_builder(builder);
    return build->toForeign(build->unary_op(oper, build->cast_node(receiver)));
}

ForeignPtr undefMethod(SelfPtr builder, const token *undef, const node_list *name_list) {
    auto build = cast_builder(builder);
    return build->toForeign(build->undefMethod(undef, build->convertNodeList(name_list)));
}

ForeignPtr when(SelfPtr builder, const token *when, const node_list *patterns, const token *then, ForeignPtr body) {
    auto build = cast_builder(builder);
    return build->toForeign(build->when(when, build->convertNodeList(patterns), then, build->cast_node(body)));
}

ForeignPtr word(SelfPtr builder, const node_list *parts) {
    auto build = cast_builder(builder);
    return build->toForeign(build->word(build->convertNodeList(parts)));
}

ForeignPtr words_compose(SelfPtr builder, const token *begin, const node_list *parts, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->words_compose(begin, build->convertNodeList(parts), end));
}

ForeignPtr xstring_compose(SelfPtr builder, const token *begin, const node_list *parts, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->xstring_compose(begin, build->convertNodeList(parts), end));
}
}; // namespace

namespace sorbet::parser {

NodePtr Builder::build(ruby_parser::base_driver *driver) {
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
    attrAsgn,
    backRef,
    begin,
    beginBody,
    beginKeyword,
    binaryOp,
    block,
    blockPass,
    blockarg,
    callLambda,
    call_method,
    case_,
    character,
    complex,
    compstmt,
    condition,
    conditionMod,
    const_,
    constFetch,
    constGlobal,
    constOpAssignable,
    cvar,
    dedentString,
    def_class,
    defMethod,
    defModule,
    def_sclass,
    defSingleton,
    encodingLiteral,
    false_,
    fileLiteral,
    float_,
    floatComplex,
    for_,
    gvar,
    ident,
    index,
    indexAsgn,
    integer,
    ivar,
    keywordBreak,
    keywordDefined,
    keywordNext,
    keywordRedo,
    keywordRetry,
    keywordReturn,
    keywordSuper,
    keywordYield,
    keywordZsuper,
    kwarg,
    kwoptarg,
    kwnilarg,
    kwrestarg,
    kwsplat,
    line_literal,
    logicalAnd,
    logicalOr,
    loopUntil,
    loopUntil_mod,
    loop_while,
    loop_while_mod,
    match_op,
    multi_assign,
    multi_lhs,
    multi_lhs1,
    nil,
    not_op,
    nth_ref,
    numparams,
    numblock,
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
    undefMethod,
    when,
    word,
    words_compose,
    xstring_compose,
};
} // namespace sorbet::parser
