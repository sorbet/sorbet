#include "parser/Builder.h"
#include "absl/strings/str_split.h"
#include "common/common.h"
#include "common/typecase.h"
#include "core/Names.h"
#include "core/errors/parser.h"
#include "parser/Dedenter.h"
#include "parser/parser.h"

#include "ruby_parser/builder.hh"
#include "ruby_parser/diagnostic.hh"

#include "absl/algorithm/container.h"
#include <algorithm>
#include <regex>
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

    auto lines = absl::StrSplit(str, "\\\n");

    for (auto line : lines) {
        spacesToRemove = dedentLevel;

        for (auto ch : line) {
            if (spacesToRemove > 0) {
                switch (ch) {
                    case ' ':
                        spacesToRemove--;
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
    }

    if (!out.empty() && out.back() == '\n') {
        spacesToRemove = dedentLevel;
    }
    return out;
}

class Builder::Impl {
public:
    Impl(GlobalState &gs, core::FileRef file) : gs_(gs), file_(file) {
        this->maxOff_ = file.data(gs).source().size();
        foreignNodes_.emplace_back();
    }

    GlobalState &gs_;
    core::FileRef file_;
    u2 uniqueCounter_ = 1;
    u4 maxOff_;
    ruby_parser::base_driver *driver_;

    vector<unique_ptr<Node>> foreignNodes_;

    u4 clamp(u4 off) {
        return std::min(off, maxOff_);
    }

    core::LocOffsets tokLoc(const token *tok) {
        return core::LocOffsets{clamp((u4)tok->start()), clamp((u4)tok->end())};
    }

    core::LocOffsets maybe_loc(unique_ptr<Node> &node) {
        if (node == nullptr) {
            return core::LocOffsets::none();
        }
        return node->loc;
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
        return elts.front()->loc.join(elts.back()->loc);
    }

    core::LocOffsets collectionLoc(sorbet::parser::NodeVec &elts) {
        return collectionLoc(nullptr, elts, nullptr);
    }

    unique_ptr<Node> transformCondition(unique_ptr<Node> cond) {
        if (auto *b = parser::cast_node<Begin>(cond.get())) {
            if (b->stmts.size() == 1) {
                b->stmts[0] = transformCondition(std::move(b->stmts[0]));
            }
        } else if (auto *a = parser::cast_node<And>(cond.get())) {
            a->left = transformCondition(std::move(a->left));
            a->right = transformCondition(std::move(a->right));
        } else if (auto *o = parser::cast_node<Or>(cond.get())) {
            o->left = transformCondition(std::move(o->left));
            o->right = transformCondition(std::move(o->right));
        } else if (auto *ir = parser::cast_node<IRange>(cond.get())) {
            return make_unique<IFlipflop>(ir->loc, transformCondition(std::move(ir->from)),
                                          transformCondition(std::move(ir->to)));
        } else if (auto *er = parser::cast_node<ERange>(cond.get())) {
            return make_unique<EFlipflop>(er->loc, transformCondition(std::move(er->from)),
                                          transformCondition(std::move(er->to)));
        } else if (auto *re = parser::cast_node<Regexp>(cond.get())) {
            return make_unique<MatchCurLine>(re->loc, std::move(cond));
        }
        return cond;
    }

    void error(ruby_parser::dclass err, core::LocOffsets loc, std::string data = "") {
        driver_->external_diagnostic(ruby_parser::dlevel::ERROR, err, loc.beginPos(), loc.endPos(), data);
    }

    /* Begin callback methods */

    unique_ptr<Node> accessible(unique_ptr<Node> node) {
        if (auto *id = parser::cast_node<Ident>(node.get())) {
            ENFORCE(id->name.kind() == core::NameKind::UTF8);
            auto name_str = id->name.show(gs_);
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
                            error(ruby_parser::dclass::NumparamUsedInOuterScope, node->loc);
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
                auto intro = make_unique<LVar>(node->loc, id->name);
                auto decls = driver_->alloc.node_list();
                decls->emplace_back(toForeign(std::move(intro)));
                driver_->numparam_stack.regis(name_str[1] - 48, std::move(decls));
            }

            if (driver_->lex.is_declared(name_str)) {
                checkCircularArgumentReferences(node.get(), name_str);
                return make_unique<LVar>(node->loc, id->name);
            } else {
                return make_unique<Send>(node->loc, nullptr, id->name, sorbet::parser::NodeVec());
            }
        }
        return node;
    }

    unique_ptr<Node> alias(const token *alias, unique_ptr<Node> to, unique_ptr<Node> from) {
        return make_unique<Alias>(tokLoc(alias).join(from->loc), std::move(to), std::move(from));
    }

    unique_ptr<Node> arg(const token *name) {
        core::LocOffsets loc = tokLoc(name);
        checkReservedForNumberedParameters(name->string(), loc);

        return make_unique<Arg>(loc, gs_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> args(const token *begin, sorbet::parser::NodeVec args, const token *end, bool check_args) {
        if (check_args) {
            UnorderedMap<std::string, core::LocOffsets> map;
            checkDuplicateArgs(args, map);
        }

        if (begin == nullptr && args.empty() && end == nullptr) {
            return nullptr;
        }
        return make_unique<Args>(collectionLoc(begin, args, end), std::move(args));
    }

    unique_ptr<Node> array(const token *begin, sorbet::parser::NodeVec elements, const token *end) {
        return make_unique<Array>(collectionLoc(begin, elements, end), std::move(elements));
    }

    unique_ptr<Node> array_pattern(const token *begin, sorbet::parser::NodeVec elements, const token *end) {
        auto trailingComma = false;

        sorbet::parser::NodeVec res;
        for (auto &elem : elements) {
            if (auto *trailing = parser::cast_node<MatchWithTrailingComma>(elem.get())) {
                trailingComma = true;
                res.emplace_back(std::move(trailing->match));
            } else {
                trailingComma = false;
                res.emplace_back(std::move(elem));
            }
        }

        auto loc = collectionLoc(res);
        if (begin != nullptr) {
            loc = tokLoc(begin).join(loc);
        }
        if (end != nullptr) {
            loc = loc.join(tokLoc(begin));
        }
        if (trailingComma) {
            return make_unique<ArrayPatternWithTail>(loc, std::move(res));
        } else {
            return make_unique<ArrayPattern>(loc, std::move(res));
        }
    }

    unique_ptr<Node> assign(unique_ptr<Node> lhs, const token *eql, unique_ptr<Node> rhs) {
        core::LocOffsets loc = lhs->loc.join(rhs->loc);

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
            auto name_str = id->name.show(gs_);
            checkReservedForNumberedParameters(name_str, id->loc);

            driver_->lex.declare(name_str);
            return make_unique<LVarLhs>(id->loc, id->name);
        } else if (auto *iv = parser::cast_node<IVar>(node.get())) {
            return make_unique<IVarLhs>(iv->loc, iv->name);
        } else if (auto *c = parser::cast_node<Const>(node.get())) {
            if (!driver_->lex.context.dynamicConstDefintinionAllowed()) {
                error(ruby_parser::dclass::DynamicConst, node->loc);
            }
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

    static bool isKeywordHashElement(sorbet::parser::Node *nd) {
        if (parser::isa_node<Kwsplat>(nd)) {
            return true;
        }

        if (auto *pair = parser::cast_node<Pair>(nd)) {
            return parser::isa_node<Symbol>(pair->key.get());
        }

        return false;
    }

    unique_ptr<Node> associate(const token *begin, sorbet::parser::NodeVec pairs, const token *end) {
        ENFORCE((begin == nullptr && end == nullptr) || (begin != nullptr && end != nullptr));
        auto isKwargs = begin == nullptr && end == nullptr &&
                        absl::c_all_of(pairs, [](const auto &nd) { return isKeywordHashElement(nd.get()); });
        return make_unique<Hash>(collectionLoc(begin, pairs, end), isKwargs, std::move(pairs));
    }

    unique_ptr<Node> attrAsgn(unique_ptr<Node> receiver, const token *dot, const token *selector, bool masgn) {
        core::NameRef method = gs_.enterNameUTF8(selector->string() + "=");
        core::LocOffsets loc = receiver->loc.join(tokLoc(selector));
        if ((dot != nullptr) && dot->string() == "&.") {
            if (masgn) {
                error(ruby_parser::dclass::CSendInLHSOfMAsgn, tokLoc(dot));
            }
            return make_unique<CSend>(loc, std::move(receiver), method, sorbet::parser::NodeVec());
        }
        return make_unique<Send>(loc, std::move(receiver), method, sorbet::parser::NodeVec());
    }

    unique_ptr<Node> backRef(const token *tok) {
        return make_unique<Backref>(tokLoc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> begin(const token *begin, unique_ptr<Node> body, const token *end) {
        core::LocOffsets loc;
        if (begin != nullptr) {
            loc = tokLoc(begin).join(tokLoc(end));
        } else {
            loc = body->loc;
        }

        if (body == nullptr) {
            return make_unique<Begin>(loc, sorbet::parser::NodeVec());
        }
        if (auto *b = parser::cast_node<Begin>(body.get())) {
            if (begin == nullptr && end == nullptr) {
                // Synthesized (begin) from compstmt "a; b" or (mlhs)
                // from multi_lhs "(a, b) = *foo".
                return body;
            }
        }
        if (auto *m = parser::cast_node<Mlhs>(body.get())) {
            return body;
        }
        sorbet::parser::NodeVec stmts;
        stmts.emplace_back(std::move(body));
        return make_unique<Begin>(loc, std::move(stmts));
    }

    unique_ptr<Node> beginBody(unique_ptr<Node> body, sorbet::parser::NodeVec rescueBodies, const token *elseTok,
                               unique_ptr<Node> else_, const token *ensure_tok, unique_ptr<Node> ensure) {
        if (!rescueBodies.empty()) {
            if (else_ == nullptr) {
                body = make_unique<Rescue>(maybe_loc(body).join(rescueBodies.back()->loc), std::move(body),
                                           std::move(rescueBodies), nullptr);
            } else {
                body = make_unique<Rescue>(maybe_loc(body).join(else_->loc), std::move(body), std::move(rescueBodies),
                                           std::move(else_));
            }
        } else if (elseTok != nullptr) {
            sorbet::parser::NodeVec stmts;
            if (body != nullptr) {
                if (auto *b = parser::cast_node<Begin>(body.get())) {
                    stmts = std::move(b->stmts);
                } else {
                    stmts.emplace_back(std::move(body));
                }
            }
            sorbet::parser::NodeVec else_stmts;
            auto elseLoc = tokLoc(elseTok).join(maybe_loc(else_));
            else_stmts.emplace_back(std::move(else_));
            stmts.emplace_back(make_unique<Begin>(elseLoc, std::move(else_stmts)));

            body = make_unique<Begin>(collectionLoc(stmts), std::move(stmts));
        }

        if (ensure_tok != nullptr) {
            core::LocOffsets loc;
            if (body != nullptr) {
                loc = body->loc;
            } else {
                loc = tokLoc(ensure_tok);
            }
            loc = loc.join(maybe_loc(ensure));
            body = make_unique<Ensure>(loc, std::move(body), std::move(ensure));
        }

        return body;
    }

    unique_ptr<Node> beginKeyword(const token *begin, unique_ptr<Node> body, const token *end) {
        core::LocOffsets loc = tokLoc(begin).join(tokLoc(end));
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

    unique_ptr<Node> binaryOp(unique_ptr<Node> receiver, const token *oper, unique_ptr<Node> arg) {
        core::LocOffsets loc = receiver->loc.join(arg->loc);

        sorbet::parser::NodeVec args;
        args.emplace_back(std::move(arg));

        return make_unique<Send>(loc, std::move(receiver), gs_.enterNameUTF8(oper->string()), std::move(args));
    }

    unique_ptr<Node> block(unique_ptr<Node> methodCall, const token *begin, unique_ptr<Node> args,
                           unique_ptr<Node> body, const token *end) {
        if (auto *y = parser::cast_node<Yield>(methodCall.get())) {
            error(ruby_parser::dclass::BlockGivenToYield, y->loc);
            return make_unique<Yield>(y->loc, sorbet::parser::NodeVec());
        }

        sorbet::parser::NodeVec *callargs = nullptr;
        if (auto *s = parser::cast_node<Send>(methodCall.get())) {
            callargs = &s->args;
        }
        if (auto *s = parser::cast_node<CSend>(methodCall.get())) {
            callargs = &s->args;
        }
        if (auto *s = parser::cast_node<Super>(methodCall.get())) {
            callargs = &s->args;
        }
        if (callargs != nullptr && !callargs->empty()) {
            if (auto *bp = parser::cast_node<BlockPass>(callargs->back().get())) {
                error(ruby_parser::dclass::BlockAndBlockarg, bp->loc);
            } else if (auto *fa = parser::cast_node<ForwardedArgs>(callargs->back().get())) {
                error(ruby_parser::dclass::BlockAndBlockarg, fa->loc);
            }
        }

        bool isNumblock = false;
        if (auto *numparams = parser::cast_node<NumParams>(args.get())) {
            isNumblock = true;
        }

        Node &n = *methodCall;
        const type_info &ty = typeid(n);
        if (ty == typeid(Send) || ty == typeid(CSend) || ty == typeid(Super) || ty == typeid(ZSuper)) {
            if (isNumblock) {
                return make_unique<NumBlock>(methodCall->loc.join(tokLoc(end)), std::move(methodCall), std::move(args),
                                             std::move(body));
            }
            return make_unique<Block>(methodCall->loc.join(tokLoc(end)), std::move(methodCall), std::move(args),
                                      std::move(body));
        }

        sorbet::parser::NodeVec *exprs;
        typecase(
            methodCall.get(), [&](Break *b) { exprs = &b->exprs; },

            [&](Return *r) { exprs = &r->exprs; },

            [&](Next *n) { exprs = &n->exprs; },

            [&](Node *n) { Exception::raise("Unexpected send node: {}", n->nodeName()); });

        auto &send = exprs->front();
        core::LocOffsets blockLoc = send->loc.join(tokLoc(end));
        unique_ptr<Node> block;
        if (isNumblock) {
            block = make_unique<NumBlock>(blockLoc, std::move(send), std::move(args), std::move(body));
        } else {
            block = make_unique<Block>(blockLoc, std::move(send), std::move(args), std::move(body));
        }
        exprs->front().swap(block);
        return methodCall;
    }

    unique_ptr<Node> blockPass(const token *amper, unique_ptr<Node> arg) {
        return make_unique<BlockPass>(tokLoc(amper).join(arg->loc), std::move(arg));
    }

    unique_ptr<Node> blockarg(const token *amper, const token *name) {
        core::LocOffsets loc;
        core::NameRef nm;

        if (name != nullptr) {
            loc = tokLoc(name);
            nm = gs_.enterNameUTF8(name->string());
            checkReservedForNumberedParameters(name->string(), loc);
        } else {
            loc = tokLoc(amper);
            nm = gs_.freshNameUnique(core::UniqueNameKind::Parser, core::Names::ampersand(), ++uniqueCounter_);
        }

        return make_unique<Blockarg>(loc, nm);
    }

    unique_ptr<Node> callLambda(const token *lambda) {
        auto loc = tokLoc(lambda);
        auto kernel = make_unique<Const>(loc, nullptr, core::Names::Constants::Kernel());
        return make_unique<Send>(loc, std::move(kernel), core::Names::lambda(), NodeVec());
    }

    unique_ptr<Node> call_method(unique_ptr<Node> receiver, const token *dot, const token *selector,
                                 const token *lparen, sorbet::parser::NodeVec args, const token *rparen) {
        core::LocOffsets selectorLoc, startLoc;
        if (selector != nullptr) {
            selectorLoc = tokLoc(selector);
        } else {
            selectorLoc = tokLoc(dot);
        }
        if (receiver == nullptr) {
            startLoc = selectorLoc;
        } else {
            startLoc = receiver->loc;
        }

        core::LocOffsets loc;
        if (rparen != nullptr) {
            loc = startLoc.join(tokLoc(rparen));
        } else if (!args.empty()) {
            loc = startLoc.join(args.back()->loc);
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
            return make_unique<CSend>(loc, std::move(receiver), method, std::move(args));
        } else {
            return make_unique<Send>(loc, std::move(receiver), method, std::move(args));
        }
    }

    unique_ptr<Node> case_(const token *case_, unique_ptr<Node> expr, sorbet::parser::NodeVec whenBodies,
                           const token *elseTok, unique_ptr<Node> elseBody, const token *end) {
        return make_unique<Case>(tokLoc(case_).join(tokLoc(end)), std::move(expr), std::move(whenBodies),
                                 std::move(elseBody));
    }

    unique_ptr<Node> case_match(const token *case_, unique_ptr<Node> expr, sorbet::parser::NodeVec inBodies,
                                const token *elseTok, unique_ptr<Node> elseBody, const token *end) {
        if (elseTok != nullptr && elseBody == nullptr) {
            elseBody = empty_else(elseTok);
        }
        return make_unique<CaseMatch>(tokLoc(case_).join(tokLoc(end)), std::move(expr), std::move(inBodies),
                                      std::move(elseBody));
    }

    unique_ptr<Node> character(const token *char_) {
        return make_unique<String>(tokLoc(char_), gs_.enterNameUTF8(char_->string()));
    }

    unique_ptr<Node> complex(const token *tok) {
        return make_unique<Complex>(tokLoc(tok), tok->string());
    }

    unique_ptr<Node> compstmt(sorbet::parser::NodeVec nodes) {
        switch (nodes.size()) {
            case 0:
                return nullptr;
            case 1:
                return std::move(nodes.back());
            default:
                return make_unique<Begin>(collectionLoc(nodes), std::move(nodes));
        }
    }

    unique_ptr<Node> condition(const token *cond_tok, unique_ptr<Node> cond, const token *then, unique_ptr<Node> ifTrue,
                               const token *else_, unique_ptr<Node> ifFalse, const token *end) {
        core::LocOffsets loc = tokLoc(cond_tok).join(cond->loc);
        if (then != nullptr) {
            loc = loc.join(tokLoc(then));
        }
        if (ifTrue != nullptr) {
            loc = loc.join(ifTrue->loc);
        }
        if (else_ != nullptr) {
            loc = loc.join(tokLoc(else_));
        }
        if (ifFalse != nullptr) {
            loc = loc.join(ifFalse->loc);
        }
        if (end != nullptr) {
            loc = loc.join(tokLoc(end));
        }
        return make_unique<If>(loc, transformCondition(std::move(cond)), std::move(ifTrue), std::move(ifFalse));
    }

    unique_ptr<Node> conditionMod(unique_ptr<Node> ifTrue, unique_ptr<Node> ifFalse, unique_ptr<Node> cond) {
        core::LocOffsets loc = cond->loc;
        if (ifTrue != nullptr) {
            loc = loc.join(ifTrue->loc);
        } else {
            loc = loc.join(ifFalse->loc);
        }
        return make_unique<If>(loc, transformCondition(std::move(cond)), std::move(ifTrue), std::move(ifFalse));
    }

    unique_ptr<Node> const_(const token *name) {
        return make_unique<Const>(tokLoc(name), nullptr, gs_.enterNameConstant(name->string()));
    }

    unique_ptr<Node> const_pattern(unique_ptr<Node> const_, const token *begin, unique_ptr<Node> pattern,
                                   const token *end) {
        return make_unique<ConstPattern>(tokLoc(begin).join(tokLoc(end)), std::move(const_), std::move(pattern));
    }

    unique_ptr<Node> constFetch(unique_ptr<Node> scope, const token *colon, const token *name) {
        return make_unique<Const>(scope->loc.join(tokLoc(name)), std::move(scope),
                                  gs_.enterNameConstant(name->string()));
    }

    unique_ptr<Node> constGlobal(const token *colon, const token *name) {
        return make_unique<Const>(tokLoc(colon).join(tokLoc(name)), make_unique<Cbase>(tokLoc(colon)),
                                  gs_.enterNameConstant(name->string()));
    }

    unique_ptr<Node> constOpAssignable(unique_ptr<Node> node) {
        if (auto *c = parser::cast_node<Const>(node.get())) {
            return make_unique<ConstLhs>(c->loc, std::move(c->scope), c->name);
        }

        return node;
    }

    unique_ptr<Node> cvar(const token *tok) {
        return make_unique<CVar>(tokLoc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> dedentString(unique_ptr<Node> node, size_t dedentLevel) {
        Dedenter dedenter(dedentLevel);
        unique_ptr<Node> result;

        typecase(
            node.get(),

            [&](String *s) {
                std::string dedented = dedenter.dedent(s->val.shortName(gs_));
                result = make_unique<String>(s->loc, gs_.enterNameUTF8(dedented));
            },

            [&](DString *d) {
                sorbet::parser::NodeVec parts;
                for (auto &p : d->nodes) {
                    if (auto *s = parser::cast_node<String>(p.get())) {
                        std::string dedented = dedenter.dedent(s->val.shortName(gs_));
                        if (dedented.empty()) {
                            continue;
                        }
                        unique_ptr<Node> newstr = make_unique<String>(s->loc, gs_.enterNameUTF8(dedented));
                        parts.emplace_back(std::move(newstr));
                    } else {
                        parts.emplace_back(std::move(p));
                    }
                }
                result = make_unique<DString>(d->loc, std::move(parts));
            },

            [&](XString *d) {
                sorbet::parser::NodeVec parts;
                for (auto &p : d->nodes) {
                    if (auto *s = parser::cast_node<String>(p.get())) {
                        std::string dedented = dedenter.dedent(s->val.shortName(gs_));
                        if (dedented.empty()) {
                            continue;
                        }
                        unique_ptr<Node> newstr = make_unique<String>(s->loc, gs_.enterNameUTF8(dedented));
                        parts.emplace_back(std::move(newstr));
                    } else {
                        parts.emplace_back(std::move(p));
                    }
                }
                result = make_unique<XString>(d->loc, std::move(parts));
            },

            [&](Node *n) { Exception::raise("Unexpected dedent node: {}", n->nodeName()); });

        return result;
    }

    unique_ptr<Node> def_class(const token *class_, unique_ptr<Node> name, const token *lt_,
                               unique_ptr<Node> superclass, unique_ptr<Node> body, const token *end_) {
        core::LocOffsets declLoc = tokLoc(class_).join(maybe_loc(name)).join(maybe_loc(superclass));
        core::LocOffsets loc = tokLoc(class_, end_);

        return make_unique<Class>(loc, declLoc, std::move(name), std::move(superclass), std::move(body));
    }

    unique_ptr<Node> defEndlessMethod(const token *def, const token *tname, unique_ptr<Node> args, const token *equal,
                                      unique_ptr<Node> body) {
        core::LocOffsets declLoc = tokLoc(def, tname).join(maybe_loc(args));
        core::LocOffsets loc = tokLoc(def).join(body->loc);
        std::string name = tname->string();

        checkEndlessSetter(name, declLoc);

        return make_unique<DefMethod>(loc, declLoc, gs_.enterNameUTF8(name), std::move(args), std::move(body));
    }

    unique_ptr<Node> defEndlessSingleton(unique_ptr<Node> defHead, unique_ptr<Node> args, const token *equal,
                                         unique_ptr<Node> body) {
        auto *head = parser::cast_node<DefsHead>(defHead.get());
        core::LocOffsets declLoc = head->loc.join(maybe_loc(args));
        core::LocOffsets loc = head->loc.join(body->loc);
        std::string name = head->name.toString(gs_);

        checkEndlessSetter(name, declLoc);

        return make_unique<DefS>(loc, declLoc, std::move(head->definee), head->name, std::move(args), std::move(body));
    }

    unique_ptr<Node> defMethod(const token *def, const token *name, unique_ptr<Node> args, unique_ptr<Node> body,
                               const token *end) {
        core::LocOffsets declLoc = tokLoc(def, name).join(maybe_loc(args));
        core::LocOffsets loc = tokLoc(def, end);

        checkReservedForNumberedParameters(name->string(), declLoc);

        return make_unique<DefMethod>(loc, declLoc, gs_.enterNameUTF8(name->string()), std::move(args),
                                      std::move(body));
    }

    unique_ptr<Node> defModule(const token *module, unique_ptr<Node> name, unique_ptr<Node> body, const token *end_) {
        core::LocOffsets declLoc = tokLoc(module).join(maybe_loc(name));
        core::LocOffsets loc = tokLoc(module, end_);
        return make_unique<Module>(loc, declLoc, std::move(name), std::move(body));
    }

    unique_ptr<Node> def_sclass(const token *class_, const token *lshft_, unique_ptr<Node> expr, unique_ptr<Node> body,
                                const token *end_) {
        core::LocOffsets declLoc = tokLoc(class_);
        core::LocOffsets loc = tokLoc(class_, end_);
        return make_unique<SClass>(loc, declLoc, std::move(expr), std::move(body));
    }

    unique_ptr<Node> defsHead(const token *def, unique_ptr<Node> definee, const token *dot, const token *name) {
        core::LocOffsets declLoc = tokLoc(def, name);

        return make_unique<DefsHead>(declLoc, std::move(definee), gs_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> defSingleton(unique_ptr<Node> defHead, unique_ptr<Node> args, unique_ptr<Node> body,
                                  const token *end) {
        auto *head = parser::cast_node<DefsHead>(defHead.get());
        core::LocOffsets declLoc = head->loc.join(maybe_loc(args));
        core::LocOffsets loc = head->loc.join(tokLoc(end));

        if (isLiteralNode(*(head->definee.get()))) {
            error(ruby_parser::dclass::SingletonLiteral, head->definee->loc);
        }
        checkReservedForNumberedParameters(name->string(), declLoc);

        return make_unique<DefS>(loc, declLoc, std::move(head->definee), head->name, std::move(args), std::move(body));
    }

    unique_ptr<Node> empty_else(const token *tok) {
        return make_unique<EmptyElse>(tokLoc(tok));
    }

    unique_ptr<Node> encodingLiteral(const token *tok) {
        return make_unique<EncodingLiteral>(tokLoc(tok));
    }

    unique_ptr<Node> false_(const token *tok) {
        return make_unique<False>(tokLoc(tok));
    }

    unique_ptr<Node> find_pattern(const token *lbrack_t, sorbet::parser::NodeVec elements, const token *rbrack_t) {
        auto loc = collectionLoc(elements);

        if (lbrack_t != nullptr) {
            loc = tokLoc(lbrack_t).join(loc);
        }

        if (rbrack_t != nullptr) {
            loc = loc.join(tokLoc(rbrack_t));
        }

        return make_unique<FindPattern>(loc, std::move(elements));
    }

    unique_ptr<Node> fileLiteral(const token *tok) {
        return make_unique<FileLiteral>(tokLoc(tok));
    }

    unique_ptr<Node> float_(const token *tok) {
        return make_unique<Float>(tokLoc(tok), tok->string());
    }

    unique_ptr<Node> floatComplex(const token *tok) {
        return make_unique<Complex>(tokLoc(tok), tok->string());
    }

    unique_ptr<Node> for_(const token *for_, unique_ptr<Node> iterator, const token *in_, unique_ptr<Node> iteratee,
                          const token *do_, unique_ptr<Node> body, const token *end) {
        return make_unique<For>(tokLoc(for_).join(tokLoc(end)), std::move(iterator), std::move(iteratee),
                                std::move(body));
    }

    unique_ptr<Node> forward_arg(const token *begin, const token *dots, const token *end) {
        return make_unique<ForwardArg>(tokLoc(dots));
    }

    unique_ptr<Node> forwarded_args(const token *dots) {
        if (!driver_->lex.is_declared_forward_args()) {
            error(ruby_parser::dclass::UnexpectedToken, tokLoc(dots), "\"...\"");
        }
        return make_unique<ForwardedArgs>(tokLoc(dots));
    }

    unique_ptr<Node> gvar(const token *tok) {
        return make_unique<GVar>(tokLoc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> hash_pattern(const token *begin, sorbet::parser::NodeVec kwargs, const token *end) {
        UnorderedMap<std::string, core::LocOffsets> map;
        checkDuplicateArgs(kwargs, map);
        auto loc = collectionLoc(kwargs);
        if (begin != nullptr) {
            loc = tokLoc(begin).join(loc);
        }
        if (end != nullptr) {
            loc = loc.join(tokLoc(begin));
        }
        return make_unique<HashPattern>(loc, std::move(kwargs));
    }

    unique_ptr<Node> ident(const token *tok) {
        return make_unique<Ident>(tokLoc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> if_guard(const token *tok, unique_ptr<Node> if_body) {
        return make_unique<IfGuard>(tokLoc(tok).join(if_body->loc), std::move(if_body));
    }

    unique_ptr<Node> in_pattern(const token *inTok, unique_ptr<Node> pattern, unique_ptr<Node> guard,
                                const token *thenTok, unique_ptr<Node> body) {
        return make_unique<InPattern>(tokLoc(inTok).join(maybe_loc(body)), std::move(pattern), std::move(guard),
                                      std::move(body));
    }

    unique_ptr<Node> index(unique_ptr<Node> receiver, const token *lbrack, sorbet::parser::NodeVec indexes,
                           const token *rbrack) {
        return make_unique<Send>(receiver->loc.join(tokLoc(rbrack)), std::move(receiver), core::Names::squareBrackets(),
                                 std::move(indexes));
    }

    unique_ptr<Node> indexAsgn(unique_ptr<Node> receiver, const token *lbrack, sorbet::parser::NodeVec indexes,
                               const token *rbrack) {
        return make_unique<Send>(receiver->loc.join(tokLoc(rbrack)), std::move(receiver),
                                 core::Names::squareBracketsEq(), std::move(indexes));
    }

    unique_ptr<Node> integer(const token *tok) {
        return make_unique<Integer>(tokLoc(tok), tok->string());
    }

    unique_ptr<Node> ivar(const token *tok) {
        return make_unique<IVar>(tokLoc(tok), gs_.enterNameUTF8(tok->string()));
    }

    unique_ptr<Node> keywordBreak(const token *keyword, const token *lparen, sorbet::parser::NodeVec args,
                                  const token *rparen) {
        core::LocOffsets loc = tokLoc(keyword).join(collectionLoc(lparen, args, rparen));
        return make_unique<Break>(loc, std::move(args));
    }

    unique_ptr<Node> keywordDefined(const token *keyword, unique_ptr<Node> arg) {
        return make_unique<Defined>(tokLoc(keyword).join(arg->loc), std::move(arg));
    }

    unique_ptr<Node> keywordNext(const token *keyword, const token *lparen, sorbet::parser::NodeVec args,
                                 const token *rparen) {
        return make_unique<Next>(tokLoc(keyword).join(collectionLoc(lparen, args, rparen)), std::move(args));
    }

    unique_ptr<Node> keywordRedo(const token *keyword) {
        return make_unique<Redo>(tokLoc(keyword));
    }

    unique_ptr<Node> keywordRetry(const token *keyword) {
        return make_unique<Retry>(tokLoc(keyword));
    }

    unique_ptr<Node> keywordReturn(const token *keyword, const token *lparen, sorbet::parser::NodeVec args,
                                   const token *rparen) {
        core::LocOffsets loc = tokLoc(keyword).join(collectionLoc(lparen, args, rparen));
        return make_unique<Return>(loc, std::move(args));
    }

    unique_ptr<Node> keywordSuper(const token *keyword, const token *lparen, sorbet::parser::NodeVec args,
                                  const token *rparen) {
        core::LocOffsets loc = tokLoc(keyword);
        core::LocOffsets argloc = collectionLoc(lparen, args, rparen);
        if (argloc.exists()) {
            loc = loc.join(argloc);
        }
        return make_unique<Super>(loc, std::move(args));
    }

    unique_ptr<Node> keywordYield(const token *keyword, const token *lparen, sorbet::parser::NodeVec args,
                                  const token *rparen) {
        core::LocOffsets loc = tokLoc(keyword).join(collectionLoc(lparen, args, rparen));
        if (!args.empty() && parser::isa_node<BlockPass>(args.back().get())) {
            error(ruby_parser::dclass::BlockGivenToYield, loc);
        }
        return make_unique<Yield>(loc, std::move(args));
    }

    unique_ptr<Node> keywordZsuper(const token *keyword) {
        return make_unique<ZSuper>(tokLoc(keyword));
    }

    unique_ptr<Node> kwarg(const token *name) {
        core::LocOffsets loc = tokLoc(name);
        checkReservedForNumberedParameters(name->string(), loc);

        return make_unique<Kwarg>(loc, gs_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> kwoptarg(const token *name, unique_ptr<Node> value) {
        core::LocOffsets loc = tokLoc(name);
        checkReservedForNumberedParameters(name->string(), loc);

        return make_unique<Kwoptarg>(loc.join(value->loc), gs_.enterNameUTF8(name->string()), tokLoc(name),
                                     std::move(value));
    }

    unique_ptr<Node> kwnilarg(const token *dstar, const token *nil) {
        return make_unique<Kwnilarg>(tokLoc(dstar).join(tokLoc(nil)));
    }

    unique_ptr<Node> kwrestarg(const token *dstar, const token *name) {
        core::LocOffsets loc = tokLoc(dstar);
        core::NameRef nm;

        if (name != nullptr) {
            loc = loc.join(tokLoc(name));
            nm = gs_.enterNameUTF8(name->string());
            checkReservedForNumberedParameters(name->string(), loc);
        } else {
            nm = gs_.freshNameUnique(core::UniqueNameKind::Parser, core::Names::starStar(), ++uniqueCounter_);
        }

        return make_unique<Kwrestarg>(loc, nm);
    }

    unique_ptr<Node> kwsplat(const token *dstar, unique_ptr<Node> arg) {
        return make_unique<Kwsplat>(tokLoc(dstar).join(arg->loc), std::move(arg));
    }

    unique_ptr<Node> line_literal(const token *tok) {
        return make_unique<LineLiteral>(tokLoc(tok));
    }

    unique_ptr<Node> logicalAnd(unique_ptr<Node> lhs, const token *op, unique_ptr<Node> rhs) {
        return make_unique<And>(lhs->loc.join(rhs->loc), std::move(lhs), std::move(rhs));
    }

    unique_ptr<Node> logicalOr(unique_ptr<Node> lhs, const token *op, unique_ptr<Node> rhs) {
        return make_unique<Or>(lhs->loc.join(rhs->loc), std::move(lhs), std::move(rhs));
    }

    unique_ptr<Node> loopUntil(const token *keyword, unique_ptr<Node> cond, const token *do_, unique_ptr<Node> body,
                               const token *end) {
        return make_unique<Until>(tokLoc(keyword).join(tokLoc(end)), std::move(cond), std::move(body));
    }

    unique_ptr<Node> loopUntil_mod(unique_ptr<Node> body, unique_ptr<Node> cond) {
        if (parser::isa_node<Kwbegin>(body.get())) {
            return make_unique<UntilPost>(body->loc.join(cond->loc), std::move(cond), std::move(body));
        }

        return make_unique<Until>(body->loc.join(cond->loc), std::move(cond), std::move(body));
    }

    unique_ptr<Node> loop_while(const token *keyword, unique_ptr<Node> cond, const token *do_, unique_ptr<Node> body,
                                const token *end) {
        return make_unique<While>(tokLoc(keyword).join(tokLoc(end)), std::move(cond), std::move(body));
    }

    unique_ptr<Node> loop_while_mod(unique_ptr<Node> body, unique_ptr<Node> cond) {
        if (parser::isa_node<Kwbegin>(body.get())) {
            return make_unique<WhilePost>(body->loc.join(cond->loc), std::move(cond), std::move(body));
        }

        return make_unique<While>(body->loc.join(cond->loc), std::move(cond), std::move(body));
    }

    unique_ptr<Node> match_alt(unique_ptr<Node> left, const token *pipe, unique_ptr<Node> right) {
        return make_unique<MatchAlt>(left->loc.join(right->loc), std::move(left), std::move(right));
    }

    unique_ptr<Node> match_as(unique_ptr<Node> value, const token *assoc, unique_ptr<Node> as) {
        return make_unique<MatchAs>(value->loc.join(as->loc), std::move(value), std::move(as));
    }

    unique_ptr<Node> match_label(unique_ptr<Node> label) {
        if (auto *pair = parser::cast_node<Pair>(label.get())) {
            if (auto *key = parser::cast_node<Symbol>(pair->key.get())) {
                // Label key is a symbol `sym: val`
                return match_var_hash(label->loc, key->val.show(gs_));
            } else if (auto *key = parser::cast_node<DSymbol>(pair->key.get())) {
                // Label key is a quoted string `"sym": val`
                return match_var_hash_from_str(std::move(key->nodes));
            }
            ENFORCE(false, "MatchLabel label key is not a Symbol nor a quoted one");
        }
        ENFORCE(false, "MatchLabel label is not a Pair node");
        return nullptr;
    }

    unique_ptr<Node> match_op(unique_ptr<Node> receiver, const token *oper, unique_ptr<Node> arg) {
        // TODO(nelhage): If the LHS here is a regex literal with (?<...>..)
        // groups, Ruby will autovivify the match groups as locals. If we were
        // to support that, we'd need to analyze that here and call
        // `driver_->lex.declare`.
        core::LocOffsets loc = receiver->loc.join(arg->loc);
        sorbet::parser::NodeVec args;
        args.emplace_back(std::move(arg));
        return make_unique<Send>(loc, std::move(receiver), gs_.enterNameUTF8(oper->string()), std::move(args));
    }

    unique_ptr<Node> match_pattern(unique_ptr<Node> lhs, const token *tok, unique_ptr<Node> rhs) {
        return make_unique<MatchPattern>(lhs->loc.join(rhs->loc), std::move(lhs), std::move(rhs));
    }

    unique_ptr<Node> match_pattern_p(unique_ptr<Node> lhs, const token *tok, unique_ptr<Node> rhs) {
        return make_unique<MatchPatternP>(lhs->loc.join(rhs->loc), std::move(lhs), std::move(rhs));
    }

    unique_ptr<Node> match_nil_pattern(const token *dstar, const token *nil) {
        return make_unique<MatchNilPattern>(tokLoc(dstar).join(tokLoc(nil)));
    }

    unique_ptr<Node> match_pair(unique_ptr<Node> label, unique_ptr<Node> value) {
        if (auto *pair = parser::cast_node<Pair>(label.get())) {
            pair->value = std::move(value);
            if (auto *key = parser::cast_node<Symbol>(pair->key.get())) {
                checkDuplicatePatternKey(key->val.show(gs_), label->loc);
                return label;
            } else if (auto *key = parser::cast_node<DSymbol>(pair->key.get())) {
                auto name_str = collapseSymbolStrings(&key->nodes, collectionLoc(key->nodes));
                checkDuplicatePatternKey(name_str, collectionLoc(key->nodes));
                return label;
            }
            ENFORCE(false, "MatchPair label key is not a Symbol nor a quoted one");
        }
        ENFORCE(false, "MatchPair label is not a Pair node");
        return nullptr;
    }

    unique_ptr<Node> match_rest(const token *star, const token *name) {
        if (name != nullptr) {
            return make_unique<MatchRest>(tokLoc(star).join(tokLoc(name)), match_var(name));
        } else {
            return make_unique<MatchRest>(tokLoc(star), nullptr);
        }
    }

    unique_ptr<Node> match_var(const token *name) {
        return match_var_hash(tokLoc(name), name->string());
    }

    unique_ptr<Node> match_var_hash(core::LocOffsets loc, const std::string name_str) {
        checkLVarName(name_str, loc);
        checkDuplicatePatternVariable(name_str, loc);
        driver_->lex.declare(name_str);
        return make_unique<MatchVar>(loc, gs_.enterNameUTF8(name_str));
    }

    unique_ptr<Node> match_var_hash_from_str(sorbet::parser::NodeVec strings) {
        auto loc = collectionLoc(strings);
        if (strings.size() > 1) {
            error(ruby_parser::dclass::PatternInterpInVarName, loc);
        }
        auto &node = strings.at(0);
        if (auto *str = parser::cast_node<String>(node.get())) {
            auto name_str = str->val.show(gs_);
            checkLVarName(name_str, loc);
            checkDuplicatePatternVariable(name_str, loc);
            driver_->lex.declare(name_str);
            return make_unique<MatchVar>(loc, gs_.enterNameUTF8(name_str));
        }
        // If we get here, the string contains an interpolation
        // collapseSymbolStrings will emit an error
        auto name_str = collapseSymbolStrings(&strings, loc);
        return make_unique<MatchVar>(loc, gs_.enterNameUTF8(name_str));
    }

    unique_ptr<Node> match_with_trailing_comma(unique_ptr<Node> match) {
        return make_unique<MatchWithTrailingComma>(match->loc, std::move(match));
    }

    unique_ptr<Node> multi_assign(unique_ptr<Node> mlhs, unique_ptr<Node> rhs) {
        return make_unique<Masgn>(mlhs->loc.join(rhs->loc), std::move(mlhs), std::move(rhs));
    }

    unique_ptr<Node> multi_lhs(const token *begin, sorbet::parser::NodeVec items, const token *end) {
        return make_unique<Mlhs>(collectionLoc(begin, items, end), std::move(items));
    }

    unique_ptr<Node> multi_lhs1(const token *begin, unique_ptr<Node> item, const token *end) {
        if (auto *mlhs = parser::cast_node<Mlhs>(item.get())) {
            return item;
        }
        sorbet::parser::NodeVec args;
        args.emplace_back(std::move(item));
        return make_unique<Mlhs>(collectionLoc(begin, args, end), std::move(args));
    }

    unique_ptr<Node> nil(const token *tok) {
        return make_unique<Nil>(tokLoc(tok));
    }

    unique_ptr<Node> not_op(const token *not_, const token *begin, unique_ptr<Node> receiver, const token *end) {
        if (receiver != nullptr) {
            core::LocOffsets loc;
            if (end != nullptr) {
                loc = tokLoc(not_).join(tokLoc(end));
            } else {
                loc = tokLoc(not_).join(receiver->loc);
            }
            return make_unique<Send>(loc, transformCondition(std::move(receiver)), core::Names::bang(),
                                     sorbet::parser::NodeVec());
        }

        ENFORCE(begin != nullptr && end != nullptr);
        auto body = make_unique<Begin>(tokLoc(begin).join(tokLoc(end)), sorbet::parser::NodeVec());
        return make_unique<Send>(tokLoc(not_).join(body->loc), std::move(body), core::Names::bang(),
                                 sorbet::parser::NodeVec());
    }

    unique_ptr<Node> nth_ref(const token *tok) {
        return make_unique<NthRef>(tokLoc(tok), atoi(tok->string().c_str()));
    }

    unique_ptr<Node> numparams(sorbet::parser::NodeVec declaringNodes) {
        ENFORCE(!declaringNodes.empty(), "NumParams node created without declaring node.");
        // During desugar we will create implicit arguments for the block based on on the highest
        // numparam used in it's body.
        // We will use the first node declaring a numbered parameter for the location of the implicit arg.
        // In the meantime, we need a loc for the NumParams node to pass the sanity check at the end of the
        // parsing phase so we arbitrary pick the first one from the node list (we know there is at least one).
        auto dummyLoc = declaringNodes.at(0)->loc;
        return make_unique<NumParams>(dummyLoc, std::move(declaringNodes));
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
        core::LocOffsets loc = tokLoc(name);
        checkReservedForNumberedParameters(name->string(), loc);

        return make_unique<Optarg>(loc.join(value->loc), gs_.enterNameUTF8(name->string()), tokLoc(name),
                                   std::move(value));
    }

    unique_ptr<Node> p_ident(const token *tok) {
        auto name_str = tok->string();
        if (!driver_->lex.is_declared(name_str)) {
            error(ruby_parser::dclass::PatternLVarUndefined, tokLoc(tok), name_str);
        }
        return ident(tok);
    }

    unique_ptr<Node> pair(unique_ptr<Node> key, const token *assoc, unique_ptr<Node> value) {
        return make_unique<Pair>(key->loc.join(value->loc), std::move(key), std::move(value));
    }

    unique_ptr<Node> pair_keyword(const token *key, unique_ptr<Node> value) {
        auto keyLoc = core::LocOffsets{clamp((u4)key->start()), clamp((u4)key->end() - 1)}; // drop the trailing :

        return make_unique<Pair>(tokLoc(key).join(maybe_loc(value)),
                                 make_unique<Symbol>(keyLoc, gs_.enterNameUTF8(key->string())), std::move(value));
    }

    unique_ptr<Node> pair_quoted(const token *begin, sorbet::parser::NodeVec parts, const token *end,
                                 unique_ptr<Node> value) {
        auto key = symbol_compose(begin, std::move(parts), end);
        return make_unique<Pair>(tokLoc(begin).join(tokLoc(end)).join(maybe_loc(value)), std::move(key),
                                 std::move(value));
    }

    unique_ptr<Node> pin(const token *tok, unique_ptr<Node> var) {
        return make_unique<Pin>(tokLoc(tok).join(var->loc), std::move(var));
    }

    unique_ptr<Node> postexe(const token *begin, unique_ptr<Node> node, const token *rbrace) {
        return make_unique<Postexe>(tokLoc(begin).join(tokLoc(rbrace)), std::move(node));
    }

    unique_ptr<Node> preexe(const token *begin, unique_ptr<Node> node, const token *rbrace) {
        return make_unique<Preexe>(tokLoc(begin).join(tokLoc(rbrace)), std::move(node));
    }

    unique_ptr<Node> procarg0(unique_ptr<Node> arg) {
        return arg;
    }

    unique_ptr<Node> range_exclusive(unique_ptr<Node> lhs, const token *oper, unique_ptr<Node> rhs) {
        core::LocOffsets loc = maybe_loc(lhs).join(tokLoc(oper)).join(maybe_loc(rhs));
        return make_unique<ERange>(loc, std::move(lhs), std::move(rhs));
    }

    unique_ptr<Node> range_inclusive(unique_ptr<Node> lhs, const token *oper, unique_ptr<Node> rhs) {
        core::LocOffsets loc = maybe_loc(lhs).join(tokLoc(oper)).join(maybe_loc(rhs));
        return make_unique<IRange>(loc, std::move(lhs), std::move(rhs));
    }

    unique_ptr<Node> rational(const token *tok) {
        return make_unique<Rational>(tokLoc(tok), tok->string());
    }

    unique_ptr<Node> rational_complex(const token *tok) {
        // TODO(nelhage): We're losing this information that this was marked as
        // a Rational in the source.
        return make_unique<Complex>(tokLoc(tok), tok->string());
    }

    unique_ptr<Node> regexp_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end,
                                    unique_ptr<Node> options) {
        core::LocOffsets loc = tokLoc(begin).join(tokLoc(end)).join(maybe_loc(options));
        return make_unique<Regexp>(loc, std::move(parts), std::move(options));
    }

    unique_ptr<Node> regexp_options(const token *regopt) {
        return make_unique<Regopt>(tokLoc(regopt), regopt->string());
    }

    unique_ptr<Node> rescue_body(const token *rescue, unique_ptr<Node> excList, const token *assoc,
                                 unique_ptr<Node> excVar, const token *then, unique_ptr<Node> body) {
        core::LocOffsets loc = tokLoc(rescue);
        if (excList != nullptr) {
            loc = loc.join(excList->loc);
        }
        if (excVar != nullptr) {
            loc = loc.join(excVar->loc);
        }
        if (body != nullptr) {
            loc = loc.join(body->loc);
        }
        return make_unique<Resbody>(loc, std::move(excList), std::move(excVar), std::move(body));
    }

    unique_ptr<Node> restarg(const token *star, const token *name) {
        core::LocOffsets loc = tokLoc(star);
        core::NameRef nm;
        core::LocOffsets nameLoc = loc;

        if (name != nullptr) {
            nameLoc = tokLoc(name);
            loc = loc.join(nameLoc);
            nm = gs_.enterNameUTF8(name->string());
            checkReservedForNumberedParameters(name->string(), nameLoc);
        } else {
            // case like 'def m(*); end'
            nm = gs_.freshNameUnique(core::UniqueNameKind::Parser, core::Names::star(), ++uniqueCounter_);
        }

        return make_unique<Restarg>(loc, nm, nameLoc);
    }

    unique_ptr<Node> self_(const token *tok) {
        return make_unique<Self>(tokLoc(tok));
    }

    unique_ptr<Node> shadowarg(const token *name) {
        core::LocOffsets loc = tokLoc(name);
        checkReservedForNumberedParameters(name->string(), loc);

        return make_unique<Shadowarg>(loc, gs_.enterNameUTF8(name->string()));
    }

    unique_ptr<Node> splat(const token *star, unique_ptr<Node> arg) {
        return make_unique<Splat>(tokLoc(star).join(arg->loc), std::move(arg));
    }

    unique_ptr<Node> splat_mlhs(const token *star, unique_ptr<Node> arg) {
        core::LocOffsets loc = tokLoc(star).join(maybe_loc(arg));
        return make_unique<SplatLhs>(loc, std::move(arg));
    }

    unique_ptr<Node> string(const token *string_) {
        return make_unique<String>(tokLoc(string_), gs_.enterNameUTF8(string_->string()));
    }

    unique_ptr<Node> string_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        if (collapseStringParts(parts)) {
            // only 1 child, either String or DString
            auto firstPart = parts.front().get();

            if (begin == nullptr || end == nullptr) {
                if (auto *s = parser::cast_node<String>(firstPart)) {
                    return make_unique<String>(s->loc, s->val);
                } else if (auto *d = parser::cast_node<DString>(firstPart)) {
                    return make_unique<DString>(d->loc, std::move(d->nodes));
                } else {
                    return nullptr;
                }
            } else {
                auto *s = parser::cast_node<String>(firstPart);
                return make_unique<String>(s->loc, s->val);
            }
        } else {
            core::LocOffsets loc = collectionLoc(begin, parts, end);
            return make_unique<DString>(loc, std::move(parts));
        }
    }

    unique_ptr<Node> string_internal(const token *string_) {
        return make_unique<String>(tokLoc(string_), gs_.enterNameUTF8(string_->string()));
    }

    unique_ptr<Node> symbol(const token *symbol) {
        return make_unique<Symbol>(tokLoc(symbol), gs_.enterNameUTF8(symbol->string()));
    }

    unique_ptr<Node> symbol_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        if (collapseStringParts(parts)) {
            // only 1 child: String
            auto firstPart = parts.front().get();
            if (auto *s = parser::cast_node<String>(firstPart)) {
                return make_unique<Symbol>(s->loc, s->val);
            } else {
                return nullptr;
            }
        } else {
            return make_unique<DSymbol>(collectionLoc(begin, parts, end), std::move(parts));
        }
    }

    unique_ptr<Node> symbol_internal(const token *symbol) {
        return make_unique<Symbol>(tokLoc(symbol), gs_.enterNameUTF8(symbol->string()));
    }

    unique_ptr<Node> symbols_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        core::LocOffsets loc = collectionLoc(begin, parts, end);
        sorbet::parser::NodeVec outParts;
        outParts.reserve(parts.size());
        for (auto &p : parts) {
            if (auto *s = parser::cast_node<String>(p.get())) {
                outParts.emplace_back(make_unique<Symbol>(s->loc, s->val));
            } else if (auto *d = parser::cast_node<DString>(p.get())) {
                outParts.emplace_back(make_unique<DSymbol>(d->loc, std::move(d->nodes)));
            } else {
                outParts.emplace_back(std::move(p));
            }
        }
        return make_unique<Array>(loc, std::move(outParts));
    }

    unique_ptr<Node> ternary(unique_ptr<Node> cond, const token *question, unique_ptr<Node> ifTrue, const token *colon,
                             unique_ptr<Node> ifFalse) {
        core::LocOffsets loc = cond->loc.join(ifFalse->loc);
        return make_unique<If>(loc, transformCondition(std::move(cond)), std::move(ifTrue), std::move(ifFalse));
    }

    unique_ptr<Node> true_(const token *tok) {
        return make_unique<True>(tokLoc(tok));
    }

    unique_ptr<Node> unary_op(const token *oper, unique_ptr<Node> receiver) {
        core::LocOffsets loc = tokLoc(oper).join(receiver->loc);

        if (auto *num = parser::cast_node<Integer>(receiver.get())) {
            return make_unique<Integer>(loc, oper->string() + num->val);
        }

        if (oper->type() != ruby_parser::token_type::tTILDE) {
            if (auto *num = parser::cast_node<Float>(receiver.get())) {
                return make_unique<Float>(loc, oper->string() + num->val);
            }
            if (auto *num = parser::cast_node<Rational>(receiver.get())) {
                return make_unique<Rational>(loc, oper->string() + num->val);
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

        return make_unique<Send>(loc, std::move(receiver), op, sorbet::parser::NodeVec());
    }

    unique_ptr<Node> undefMethod(const token *undef, sorbet::parser::NodeVec name_list) {
        core::LocOffsets loc = tokLoc(undef);
        if (!name_list.empty()) {
            loc = loc.join(name_list.back()->loc);
        }
        return make_unique<Undef>(loc, std::move(name_list));
    }

    unique_ptr<Node> unless_guard(const token *tok, unique_ptr<Node> unless_body) {
        return make_unique<UnlessGuard>(tokLoc(tok).join(unless_body->loc), std::move(unless_body));
    }

    unique_ptr<Node> when(const token *when, sorbet::parser::NodeVec patterns, const token *then,
                          unique_ptr<Node> body) {
        core::LocOffsets loc = tokLoc(when);
        if (body != nullptr) {
            loc = loc.join(body->loc);
        } else if (then != nullptr) {
            loc = loc.join(tokLoc(then));
        } else {
            loc = loc.join(patterns.back()->loc);
        }
        return make_unique<When>(loc, std::move(patterns), std::move(body));
    }

    unique_ptr<Node> word(sorbet::parser::NodeVec parts) {
        core::LocOffsets loc = collectionLoc(parts);
        if (collapseStringParts(parts)) {
            // a single String child
            auto firstPart = parts.front().get();

            if (auto *s = parser::cast_node<String>(firstPart)) {
                return make_unique<String>(s->loc, s->val);
            } else {
                return nullptr;
            }
        } else {
            return make_unique<DString>(loc, std::move(parts));
        }
    }

    unique_ptr<Node> words_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        return make_unique<Array>(collectionLoc(begin, parts, end), std::move(parts));
    }

    unique_ptr<Node> xstring_compose(const token *begin, sorbet::parser::NodeVec parts, const token *end) {
        return make_unique<XString>(collectionLoc(begin, parts, end), std::move(parts));
    }

    /* End callback methods */

    /* methods for marshalling to and from the parser's foreign pointers */

    unique_ptr<Node> cast_node(ForeignPtr node) {
        auto off = reinterpret_cast<size_t>(node);
        if (off == 0) {
            return nullptr;
        }

        ENFORCE(foreignNodes_[off] != nullptr);
        return std::move(foreignNodes_[off]);
    }

    ForeignPtr toForeign(unique_ptr<Node> node) {
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

        auto firstPart = parts.front().get();

        return parser::isa_node<String>(firstPart) || parser::isa_node<DString>(firstPart);
    }

    void checkCircularArgumentReferences(const Node *node, std::string name) {
        if (name == driver_->current_arg_stack.top()) {
            error(ruby_parser::dclass::CircularArgumentReference, node->loc, name);
        }
    }

    void checkDuplicateArgs(sorbet::parser::NodeVec &args, UnorderedMap<std::string, core::LocOffsets> &map) {
        for (auto &this_arg : args) {
            if (auto *arg = parser::cast_node<Arg>(this_arg.get())) {
                checkDuplicateArg(arg->name.toString(gs_), arg->loc, map);
            } else if (auto *optarg = parser::cast_node<Optarg>(this_arg.get())) {
                checkDuplicateArg(optarg->name.toString(gs_), optarg->loc, map);
            } else if (auto *restarg = parser::cast_node<Restarg>(this_arg.get())) {
                checkDuplicateArg(restarg->name.toString(gs_), restarg->loc, map);
            } else if (auto *blockarg = parser::cast_node<Blockarg>(this_arg.get())) {
                checkDuplicateArg(blockarg->name.toString(gs_), blockarg->loc, map);
            } else if (auto *kwarg = parser::cast_node<Kwarg>(this_arg.get())) {
                checkDuplicateArg(kwarg->name.toString(gs_), kwarg->loc, map);
            } else if (auto *kwoptarg = parser::cast_node<Kwoptarg>(this_arg.get())) {
                checkDuplicateArg(kwoptarg->name.toString(gs_), kwoptarg->loc, map);
            } else if (auto *kwrestarg = parser::cast_node<Kwrestarg>(this_arg.get())) {
                checkDuplicateArg(kwrestarg->name.toString(gs_), kwrestarg->loc, map);
            } else if (auto *shadowarg = parser::cast_node<Shadowarg>(this_arg.get())) {
                checkDuplicateArg(shadowarg->name.toString(gs_), shadowarg->loc, map);
            } else if (auto *mlhs = parser::cast_node<Mlhs>(this_arg.get())) {
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

    void checkDuplicatePatternVariable(std::string name, core::LocOffsets loc) {
        ENFORCE(name.size() > 0, "Empty pattern variable name");
        if (name[0] == '_') {
            return;
        }

        if (driver_->pattern_variables.declared(name)) {
            error(ruby_parser::dclass::PatternDuplicateVariable, loc, name);
        }

        driver_->pattern_variables.declare(name);
    }

    void checkDuplicatePatternKey(std::string name, core::LocOffsets loc) {
        if (driver_->pattern_hash_keys.declared(name)) {
            error(ruby_parser::dclass::PatternDuplicateKey, loc, name);
        }

        driver_->pattern_hash_keys.declare(name);
    }

    void checkEndlessSetter(std::string name, core::LocOffsets loc) {
        if (name != "===" && name != "==" && name != "!=" && name != "<=" && name != ">=" &&
            name[name.length() - 1] == '=') {
            error(ruby_parser::dclass::EndlessSetter, loc);
        }
    }

    void checkLVarName(std::string name, core::LocOffsets loc) {
        std::regex lvar_regex("^[a-z_][a-zA-Z0-9_]*$");
        if (!std::regex_match(name, lvar_regex)) {
            error(ruby_parser::dclass::PatternLVarName, loc, name);
        }
    }

    std::string collapseSymbolStrings(sorbet::parser::NodeVec *parts, core::LocOffsets loc) {
        std::string res;
        for (auto &p : *parts) {
            if (auto *s = parser::cast_node<String>(p.get())) {
                res += s->val.show(gs_);
            } else if (auto *d = parser::cast_node<DString>(p.get())) {
                res += collapseSymbolStrings(&d->nodes, loc);
            } else if (auto *d = parser::cast_node<DSymbol>(p.get())) {
                res += collapseSymbolStrings(&d->nodes, loc);
            } else if (auto *b = parser::cast_node<Begin>(p.get())) {
                res += collapseSymbolStrings(&b->stmts, loc);
            } else {
                error(ruby_parser::dclass::PatternInterpInVarName, loc);
            }
        }
        return res;
    }

    bool argNameCollides(std::string name) {
        // Ignore everything beginning with underscore.
        return (name[0] != '_');
    }

    bool isLiteralNode(parser::Node &node) {
        return parser::isa_node<Integer>(&node) || parser::isa_node<String>(&node) ||
               parser::isa_node<DString>(&node) || parser::isa_node<Symbol>(&node) ||
               parser::isa_node<DSymbol>(&node) || parser::isa_node<Regexp>(&node) || parser::isa_node<Array>(&node) ||
               parser::isa_node<Hash>(&node);
    }

    void checkReservedForNumberedParameters(std::string name, core::LocOffsets loc) {
        if (isNumberedParameterName(name)) {
            core::Loc location = core::Loc(file_, loc);

            if (auto e = gs_.beginError(location, core::errors::Parser::ParserError)) {
                std::string replacement = fmt::format("arg{}", name[1]);

                e.setHeader("{} is reserved for numbered parameter", name);
                e.addAutocorrect(
                    core::AutocorrectSuggestion{fmt::format("Replace `{}` with `{}`", name, replacement),
                                                {core::AutocorrectSuggestion::Edit{location, replacement}}});
            }

            error(ruby_parser::dclass::ReservedForNumparam, loc, name);
        }
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
using sorbet::parser::Node;

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

ForeignPtr array_pattern(SelfPtr builder, const token *begin, const node_list *elements, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->array_pattern(begin, build->convertNodeList(elements), end));
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

ForeignPtr case_match(SelfPtr builder, const token *case_, ForeignPtr expr, const node_list *inBodies,
                      const token *elseTok, ForeignPtr elseBody, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->case_match(case_, build->cast_node(expr), build->convertNodeList(inBodies), elseTok,
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

ForeignPtr const_pattern(SelfPtr builder, ForeignPtr const_, const token *begin, ForeignPtr pattern, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->const_pattern(build->cast_node(const_), begin, build->cast_node(pattern), end));
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

ForeignPtr defsHead(SelfPtr builder, const token *def, ForeignPtr definee, const token *dot, const token *name) {
    auto build = cast_builder(builder);
    return build->toForeign(build->defsHead(def, build->cast_node(definee), dot, name));
}

ForeignPtr defEndlessMethod(SelfPtr builder, const token *def, const token *name, ForeignPtr args, const token *equal,
                            ForeignPtr body) {
    auto build = cast_builder(builder);
    return build->toForeign(build->defEndlessMethod(def, name, build->cast_node(args), equal, build->cast_node(body)));
}

ForeignPtr defEndlessSingleton(SelfPtr builder, ForeignPtr defHead, ForeignPtr args, const token *equal,
                               ForeignPtr body) {
    auto build = cast_builder(builder);
    return build->toForeign(
        build->defEndlessSingleton(build->cast_node(defHead), build->cast_node(args), equal, build->cast_node(body)));
}

ForeignPtr defSingleton(SelfPtr builder, ForeignPtr defHead, ForeignPtr args, ForeignPtr body, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(
        build->defSingleton(build->cast_node(defHead), build->cast_node(args), build->cast_node(body), end));
}

ForeignPtr encodingLiteral(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->encodingLiteral(tok));
}

ForeignPtr false_(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->false_(tok));
}

ForeignPtr find_pattern(SelfPtr builder, const token *lbrack_t, const node_list *elements, const token *rbrack_t) {
    auto build = cast_builder(builder);
    return build->toForeign(build->find_pattern(lbrack_t, build->convertNodeList(elements), rbrack_t));
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

ForeignPtr forward_arg(SelfPtr builder, const token *begin, const token *dots, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->forward_arg(begin, dots, end));
}

ForeignPtr forwarded_args(SelfPtr builder, const token *dots) {
    auto build = cast_builder(builder);
    return build->toForeign(build->forwarded_args(dots));
}

ForeignPtr gvar(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->gvar(tok));
}

ForeignPtr hash_pattern(SelfPtr builder, const token *begin, const node_list *kwargs, const token *end) {
    auto build = cast_builder(builder);
    return build->toForeign(build->hash_pattern(begin, build->convertNodeList(kwargs), end));
}

ForeignPtr if_guard(SelfPtr builder, const token *tok, ForeignPtr ifBody) {
    auto build = cast_builder(builder);
    return build->toForeign(build->if_guard(tok, build->cast_node(ifBody)));
}

ForeignPtr ident(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->ident(tok));
}

ForeignPtr in_pattern(SelfPtr builder, const token *tok, ForeignPtr pattern, ForeignPtr guard, const token *thenToken,
                      ForeignPtr body) {
    auto build = cast_builder(builder);
    return build->toForeign(
        build->in_pattern(tok, build->cast_node(pattern), build->cast_node(guard), thenToken, build->cast_node(body)));
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

ForeignPtr match_alt(SelfPtr builder, ForeignPtr left, const token *pipe, ForeignPtr right) {
    auto build = cast_builder(builder);
    return build->toForeign(build->match_alt(build->cast_node(left), pipe, build->cast_node(right)));
}

ForeignPtr match_as(SelfPtr builder, ForeignPtr value, const token *assoc, ForeignPtr as) {
    auto build = cast_builder(builder);
    return build->toForeign(build->match_as(build->cast_node(value), assoc, build->cast_node(as)));
}

ForeignPtr match_label(SelfPtr builder, ForeignPtr label) {
    auto build = cast_builder(builder);
    return build->toForeign(build->match_label(build->cast_node(label)));
}

ForeignPtr match_pattern(SelfPtr builder, ForeignPtr lhs, const token *tok, ForeignPtr rhs) {
    auto build = cast_builder(builder);
    return build->toForeign(build->match_pattern(build->cast_node(lhs), tok, build->cast_node(rhs)));
}

ForeignPtr match_pattern_p(SelfPtr builder, ForeignPtr lhs, const token *tok, ForeignPtr rhs) {
    auto build = cast_builder(builder);
    return build->toForeign(build->match_pattern_p(build->cast_node(lhs), tok, build->cast_node(rhs)));
}

ForeignPtr match_nil_pattern(SelfPtr builder, const token *dstar, const token *nil) {
    auto build = cast_builder(builder);
    return build->toForeign(build->match_nil_pattern(dstar, nil));
}

ForeignPtr match_op(SelfPtr builder, ForeignPtr receiver, const token *oper, ForeignPtr arg) {
    auto build = cast_builder(builder);
    return build->toForeign(build->match_op(build->cast_node(receiver), oper, build->cast_node(arg)));
}

ForeignPtr match_pair(SelfPtr builder, ForeignPtr label, ForeignPtr value) {
    auto build = cast_builder(builder);
    return build->toForeign(build->match_pair(build->cast_node(label), build->cast_node(value)));
}

ForeignPtr match_rest(SelfPtr builder, const token *star, const token *name) {
    auto build = cast_builder(builder);
    return build->toForeign(build->match_rest(star, name));
}

ForeignPtr match_var(SelfPtr builder, const token *name) {
    auto build = cast_builder(builder);
    return build->toForeign(build->match_var(name));
}

ForeignPtr match_with_trailing_comma(SelfPtr builder, ForeignPtr match) {
    auto build = cast_builder(builder);
    return build->toForeign(build->match_with_trailing_comma(build->cast_node(match)));
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

ForeignPtr p_ident(SelfPtr builder, const token *tok) {
    auto build = cast_builder(builder);
    return build->toForeign(build->p_ident(tok));
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

ForeignPtr pin(SelfPtr builder, const token *tok, ForeignPtr var) {
    auto build = cast_builder(builder);
    return build->toForeign(build->pin(tok, build->cast_node(var)));
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

ForeignPtr unless_guard(SelfPtr builder, const token *unlessGuard, ForeignPtr unlessBody) {
    auto build = cast_builder(builder);
    return build->toForeign(build->unless_guard(unlessGuard, build->cast_node(unlessBody)));
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
    array_pattern,
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
    case_match,
    character,
    complex,
    compstmt,
    condition,
    conditionMod,
    const_,
    const_pattern,
    constFetch,
    constGlobal,
    constOpAssignable,
    cvar,
    dedentString,
    def_class,
    defEndlessMethod,
    defEndlessSingleton,
    defMethod,
    defModule,
    def_sclass,
    defsHead,
    defSingleton,
    encodingLiteral,
    false_,
    find_pattern,
    fileLiteral,
    float_,
    floatComplex,
    for_,
    forward_arg,
    forwarded_args,
    gvar,
    hash_pattern,
    ident,
    if_guard,
    in_pattern,
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
    match_alt,
    match_as,
    match_label,
    match_pattern,
    match_pattern_p,
    match_nil_pattern,
    match_op,
    match_pair,
    match_rest,
    match_var,
    match_with_trailing_comma,
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
    p_ident,
    pair,
    pair_keyword,
    pair_quoted,
    pin,
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
    unless_guard,
    when,
    word,
    words_compose,
    xstring_compose,
};
} // namespace sorbet::parser
