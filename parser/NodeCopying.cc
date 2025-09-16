#include "common/typecase.h"
#include "parser/Node.h"

namespace sorbet::parser {

namespace {

NodeVec deepCopyVec(const NodeVec &vec) {
    NodeVec result;
    result.reserve(vec.size());
    for (const auto &node : vec) {
        result.emplace_back(node->deepCopy());
    }
    return result;
}

std::unique_ptr<Node> deepCopy(const Node *node) {
    if (node == nullptr) {
        return nullptr;
    }

    std::unique_ptr<Node> result;

    typecase(
        node,
        [&](const parser::Alias *alias) {
            result = std::make_unique<Alias>(alias->loc, deepCopy(alias->from.get()), deepCopy(alias->to.get()));
        },
        [&](const parser::And *and_) {
            result = std::make_unique<And>(and_->loc, deepCopy(and_->left.get()), deepCopy(and_->right.get()));
        },
        [&](const parser::AndAsgn *andAsgn) {
            result =
                std::make_unique<AndAsgn>(andAsgn->loc, deepCopy(andAsgn->left.get()), deepCopy(andAsgn->right.get()));
        },
        [&](const parser::Array *array) { result = std::make_unique<Array>(array->loc, deepCopyVec(array->elts)); },
        [&](const parser::ArrayPattern *arrayPattern) {
            result = std::make_unique<ArrayPattern>(arrayPattern->loc, deepCopyVec(arrayPattern->elts));
        },
        [&](const parser::ArrayPatternWithTail *arrayPatternWithTail) {
            result = std::make_unique<ArrayPatternWithTail>(arrayPatternWithTail->loc,
                                                            deepCopyVec(arrayPatternWithTail->elts));
        },
        [&](const parser::Assign *assign) {
            result = std::make_unique<Assign>(assign->loc, deepCopy(assign->lhs.get()), deepCopy(assign->rhs.get()));
        },
        [&](const parser::Backref *backref) { result = std::make_unique<Backref>(backref->loc, backref->name); },
        [&](const parser::Begin *begin) { result = std::make_unique<Begin>(begin->loc, deepCopyVec(begin->stmts)); },
        [&](const parser::Block *block) {
            result = std::make_unique<Block>(block->loc, deepCopy(block->send.get()), deepCopy(block->params.get()),
                                             deepCopy(block->body.get()));
        },
        [&](const parser::Blockarg *blockarg) { result = std::make_unique<Blockarg>(blockarg->loc, blockarg->name); },
        [&](const parser::BlockPass *blockPass) {
            result = std::make_unique<BlockPass>(blockPass->loc, deepCopy(blockPass->block.get()));
        },
        [&](const parser::Break *break_) { result = std::make_unique<Break>(break_->loc, deepCopyVec(break_->exprs)); },
        [&](const parser::Case *case_) {
            result = std::make_unique<Case>(case_->loc, deepCopy(case_->condition.get()), deepCopyVec(case_->whens),
                                            deepCopy(case_->else_.get()));
        },
        [&](const parser::CaseMatch *caseMatch) {
            result = std::make_unique<CaseMatch>(caseMatch->loc, deepCopy(caseMatch->expr.get()),
                                                 deepCopyVec(caseMatch->inBodies), deepCopy(caseMatch->elseBody.get()));
        },
        [&](const parser::Cbase *cbase) { result = std::make_unique<Cbase>(cbase->loc); },
        [&](const parser::Class *class_) {
            result = std::make_unique<Class>(class_->loc, class_->declLoc, deepCopy(class_->name.get()),
                                             deepCopy(class_->superclass.get()), deepCopy(class_->body.get()));
        },
        [&](const parser::Complex *complex) { result = std::make_unique<Complex>(complex->loc, complex->value); },
        [&](const parser::Const *const_) {
            result = std::make_unique<Const>(const_->loc, deepCopy(const_->scope.get()), const_->name);
        },
        [&](const parser::ConstLhs *constLhs) {
            result = std::make_unique<ConstLhs>(constLhs->loc, deepCopy(constLhs->scope.get()), constLhs->name);
        },
        [&](const parser::ConstPattern *constPattern) {
            result = std::make_unique<ConstPattern>(constPattern->loc, deepCopy(constPattern->scope.get()),
                                                    deepCopy(constPattern->pattern.get()));
        },
        [&](const parser::CSend *cSend) {
            result = std::make_unique<CSend>(cSend->loc, deepCopy(cSend->receiver.get()), cSend->method,
                                             cSend->methodLoc, deepCopyVec(cSend->args));
        },
        [&](const parser::CVar *cVar) { result = std::make_unique<CVar>(cVar->loc, cVar->name); },
        [&](const parser::CVarLhs *cVarLhs) { result = std::make_unique<CVarLhs>(cVarLhs->loc, cVarLhs->name); },
        [&](const parser::DefMethod *defMethod) {
            result = std::make_unique<DefMethod>(defMethod->loc, defMethod->declLoc, defMethod->name,
                                                 deepCopy(defMethod->params.get()), deepCopy(defMethod->body.get()));
        },
        [&](const parser::Defined *defined) {
            result = std::make_unique<Defined>(defined->loc, deepCopy(defined->value.get()));
        },
        [&](const parser::DefnHead *defnHead) { result = std::make_unique<DefnHead>(defnHead->loc, defnHead->name); },
        [&](const parser::DefS *defS) {
            result = std::make_unique<DefS>(defS->loc, defS->declLoc, deepCopy(defS->singleton.get()), defS->name,
                                            deepCopy(defS->params.get()), deepCopy(defS->body.get()));
        },
        [&](const parser::DefsHead *defsHead) {
            result = std::make_unique<DefsHead>(defsHead->loc, deepCopy(defsHead->definee.get()), defsHead->name);
        },
        [&](const parser::DString *dString) {
            result = std::make_unique<DString>(dString->loc, deepCopyVec(dString->nodes));
        },
        [&](const parser::DSymbol *dSymbol) {
            result = std::make_unique<DSymbol>(dSymbol->loc, deepCopyVec(dSymbol->nodes));
        },
        [&](const parser::EFlipflop *eFlipflop) {
            result = std::make_unique<EFlipflop>(eFlipflop->loc, deepCopy(eFlipflop->left.get()),
                                                 deepCopy(eFlipflop->right.get()));
        },
        [&](const parser::EmptyElse *emptyElse) { result = std::make_unique<EmptyElse>(emptyElse->loc); },
        [&](const parser::EncodingLiteral *encodingLiteral) {
            result = std::make_unique<EncodingLiteral>(encodingLiteral->loc);
        },
        [&](const parser::Ensure *ensure) {
            result =
                std::make_unique<Ensure>(ensure->loc, deepCopy(ensure->body.get()), deepCopy(ensure->ensure.get()));
        },
        [&](const parser::ERange *eRange) {
            result = std::make_unique<ERange>(eRange->loc, deepCopy(eRange->from.get()), deepCopy(eRange->to.get()));
        },
        [&](const parser::False *false_) { result = std::make_unique<False>(false_->loc); },
        [&](const parser::FindPattern *findPattern) {
            result = std::make_unique<FindPattern>(findPattern->loc, deepCopyVec(findPattern->elements));
        },
        [&](const parser::FileLiteral *fileLiteral) { result = std::make_unique<FileLiteral>(fileLiteral->loc); },
        [&](const parser::For *for_) {
            result = std::make_unique<For>(for_->loc, deepCopy(for_->vars.get()), deepCopy(for_->expr.get()),
                                           deepCopy(for_->body.get()));
        },
        [&](const parser::ForwardArg *forwardArg) { result = std::make_unique<ForwardArg>(forwardArg->loc); },
        [&](const parser::ForwardedArgs *forwardedArgs) {
            result = std::make_unique<ForwardedArgs>(forwardedArgs->loc);
        },
        [&](const parser::ForwardedRestArg *forwardedRestArg) {
            result = std::make_unique<ForwardedRestArg>(forwardedRestArg->loc);
        },
        [&](const parser::ForwardedKwrestArg *forwardedKwrestArg) {
            result = std::make_unique<ForwardedKwrestArg>(forwardedKwrestArg->loc);
        },
        [&](const parser::Float *float_) { result = std::make_unique<Float>(float_->loc, float_->val); },
        [&](const parser::GVar *gVar) { result = std::make_unique<GVar>(gVar->loc, gVar->name); },
        [&](const parser::GVarLhs *gVarLhs) { result = std::make_unique<GVarLhs>(gVarLhs->loc, gVarLhs->name); },
        [&](const parser::Hash *hash) {
            result = std::make_unique<Hash>(hash->loc, hash->kwargs, deepCopyVec(hash->pairs));
        },
        [&](const parser::HashPattern *hashPattern) {
            result = std::make_unique<HashPattern>(hashPattern->loc, deepCopyVec(hashPattern->pairs));
        },
        [&](const parser::Ident *ident) { result = std::make_unique<Ident>(ident->loc, ident->name); },
        [&](const parser::If *if_) {
            result = std::make_unique<If>(if_->loc, deepCopy(if_->condition.get()), deepCopy(if_->then_.get()),
                                          deepCopy(if_->else_.get()));
        },
        [&](const parser::IfGuard *ifGuard) {
            result = std::make_unique<IfGuard>(ifGuard->loc, deepCopy(ifGuard->condition.get()));
        },
        [&](const parser::IFlipflop *iFlipflop) {
            result = std::make_unique<IFlipflop>(iFlipflop->loc, deepCopy(iFlipflop->left.get()),
                                                 deepCopy(iFlipflop->right.get()));
        },
        [&](const parser::InPattern *inPattern) {
            result = std::make_unique<InPattern>(inPattern->loc, deepCopy(inPattern->pattern.get()),
                                                 deepCopy(inPattern->guard.get()), deepCopy(inPattern->body.get()));
        },
        [&](const parser::IRange *iRange) {
            result = std::make_unique<IRange>(iRange->loc, deepCopy(iRange->from.get()), deepCopy(iRange->to.get()));
        },
        [&](const parser::Integer *integer) { result = std::make_unique<Integer>(integer->loc, integer->val); },
        [&](const parser::IVar *iVar) { result = std::make_unique<IVar>(iVar->loc, iVar->name); },
        [&](const parser::IVarLhs *iVarLhs) { result = std::make_unique<IVarLhs>(iVarLhs->loc, iVarLhs->name); },
        [&](const parser::Kwarg *kwarg) { result = std::make_unique<Kwarg>(kwarg->loc, kwarg->name); },
        [&](const parser::Kwnilarg *kwnilarg) { result = std::make_unique<Kwnilarg>(kwnilarg->loc); },
        [&](const parser::Kwbegin *kwbegin) {
            result = std::make_unique<Kwbegin>(kwbegin->loc, deepCopyVec(kwbegin->stmts));
        },
        [&](const parser::Kwoptarg *kwoptarg) {
            result = std::make_unique<Kwoptarg>(kwoptarg->loc, kwoptarg->name, kwoptarg->nameLoc,
                                                deepCopy(kwoptarg->default_.get()));
        },
        [&](const parser::Kwrestarg *kwrestarg) {
            result = std::make_unique<Kwrestarg>(kwrestarg->loc, kwrestarg->name);
        },
        [&](const parser::Kwsplat *kwsplat) {
            result = std::make_unique<Kwsplat>(kwsplat->loc, deepCopy(kwsplat->expr.get()));
        },
        [&](const parser::LineLiteral *lineLiteral) { result = std::make_unique<LineLiteral>(lineLiteral->loc); },
        [&](const parser::LVar *lVar) { result = std::make_unique<LVar>(lVar->loc, lVar->name); },
        [&](const parser::LVarLhs *lVarLhs) { result = std::make_unique<LVarLhs>(lVarLhs->loc, lVarLhs->name); },
        [&](const parser::MatchAlt *matchAlt) {
            result = std::make_unique<MatchAlt>(matchAlt->loc, deepCopy(matchAlt->left.get()),
                                                deepCopy(matchAlt->right.get()));
        },
        [&](const parser::MatchAs *matchAs) {
            result =
                std::make_unique<MatchAs>(matchAs->loc, deepCopy(matchAs->value.get()), deepCopy(matchAs->as.get()));
        },
        [&](const parser::MatchAsgn *matchAsgn) {
            result = std::make_unique<MatchAsgn>(matchAsgn->loc, deepCopy(matchAsgn->regex.get()),
                                                 deepCopy(matchAsgn->expr.get()));
        },
        [&](const parser::MatchCurLine *matchCurLine) {
            result = std::make_unique<MatchCurLine>(matchCurLine->loc, deepCopy(matchCurLine->cond.get()));
        },
        [&](const parser::MatchNilPattern *matchNilPattern) {
            result = std::make_unique<MatchNilPattern>(matchNilPattern->loc);
        },
        [&](const parser::MatchPattern *matchPattern) {
            result = std::make_unique<MatchPattern>(matchPattern->loc, deepCopy(matchPattern->lhs.get()),
                                                    deepCopy(matchPattern->rhs.get()));
        },
        [&](const parser::MatchPatternP *matchPatternP) {
            result = std::make_unique<MatchPatternP>(matchPatternP->loc, deepCopy(matchPatternP->lhs.get()),
                                                     deepCopy(matchPatternP->rhs.get()));
        },
        [&](const parser::MatchRest *matchRest) {
            result = std::make_unique<MatchRest>(matchRest->loc, deepCopy(matchRest->var.get()));
        },
        [&](const parser::MatchVar *matchVar) { result = std::make_unique<MatchVar>(matchVar->loc, matchVar->name); },
        [&](const parser::MatchWithTrailingComma *matchWithTrailingComma) {
            result = std::make_unique<MatchWithTrailingComma>(matchWithTrailingComma->loc,
                                                              deepCopy(matchWithTrailingComma->match.get()));
        },
        [&](const parser::Masgn *masgn) {
            result = std::make_unique<Masgn>(masgn->loc, deepCopy(masgn->lhs.get()), deepCopy(masgn->rhs.get()));
        },
        [&](const parser::Mlhs *mlhs) { result = std::make_unique<Mlhs>(mlhs->loc, deepCopyVec(mlhs->exprs)); },
        [&](const parser::Module *module) {
            result = std::make_unique<Module>(module->loc, module->declLoc, deepCopy(module->name.get()),
                                              deepCopy(module->body.get()));
        },
        [&](const parser::Next *next) { result = std::make_unique<Next>(next->loc, deepCopyVec(next->exprs)); },
        [&](const parser::Nil *nil) { result = std::make_unique<Nil>(nil->loc); },
        [&](const parser::NthRef *nthRef) { result = std::make_unique<NthRef>(nthRef->loc, nthRef->ref); },
        [&](const parser::NumParams *numParams) {
            result = std::make_unique<NumParams>(numParams->loc, deepCopyVec(numParams->decls));
        },
        [&](const parser::OpAsgn *opAsgn) {
            result = std::make_unique<OpAsgn>(opAsgn->loc, deepCopy(opAsgn->left.get()), opAsgn->op, opAsgn->opLoc,
                                              deepCopy(opAsgn->right.get()));
        },
        [&](const parser::Or *or_) {
            result = std::make_unique<Or>(or_->loc, deepCopy(or_->left.get()), deepCopy(or_->right.get()));
        },
        [&](const parser::OrAsgn *orAsgn) {
            result = std::make_unique<OrAsgn>(orAsgn->loc, deepCopy(orAsgn->left.get()), deepCopy(orAsgn->right.get()));
        },
        [&](const parser::OptParam *optarg) {
            result = std::make_unique<OptParam>(optarg->loc, optarg->name, optarg->nameLoc,
                                                deepCopy(optarg->default_.get()));
        },
        [&](const parser::Pair *pair) {
            result = std::make_unique<Pair>(pair->loc, deepCopy(pair->key.get()), deepCopy(pair->value.get()));
        },
        [&](const parser::Param *param) { result = std::make_unique<Param>(param->loc, param->name); },
        [&](const parser::Params *params) {
            result = std::make_unique<Params>(params->loc, deepCopyVec(params->params));
        },
        [&](const parser::Pin *pin) { result = std::make_unique<Pin>(pin->loc, deepCopy(pin->var.get())); },
        [&](const parser::Param *param) { result = std::make_unique<Param>(param->loc, param->name); },
        [&](const parser::Postexe *postexe) {
            result = std::make_unique<Postexe>(postexe->loc, deepCopy(postexe->body.get()));
        },
        [&](const parser::Preexe *preexe) {
            result = std::make_unique<Preexe>(preexe->loc, deepCopy(preexe->body.get()));
        },
        [&](const parser::Procarg0 *procarg0) {
            result = std::make_unique<Procarg0>(procarg0->loc, deepCopy(procarg0->arg.get()));
        },
        [&](const parser::Rational *rational) { result = std::make_unique<Rational>(rational->loc, rational->val); },
        [&](const parser::Redo *redo) { result = std::make_unique<Redo>(redo->loc); },
        [&](const parser::Regexp *regexp) {
            result = std::make_unique<Regexp>(regexp->loc, deepCopyVec(regexp->regex), deepCopy(regexp->opts.get()));
        },
        [&](const parser::Regopt *regopt) { result = std::make_unique<Regopt>(regopt->loc, regopt->opts); },
        [&](const parser::Resbody *resbody) {
            result = std::make_unique<Resbody>(resbody->loc, deepCopy(resbody->exception.get()),
                                               deepCopy(resbody->var.get()), deepCopy(resbody->body.get()));
        },
        [&](const parser::Rescue *rescue) {
            result = std::make_unique<Rescue>(rescue->loc, deepCopy(rescue->body.get()), deepCopyVec(rescue->rescue),
                                              deepCopy(rescue->else_.get()));
        },
        [&](const parser::Restarg *restarg) {
            result = std::make_unique<Restarg>(restarg->loc, restarg->name, restarg->nameLoc);
        },
        [&](const parser::Retry *retry) { result = std::make_unique<Retry>(retry->loc); },
        [&](const parser::Return *return_) {
            result = std::make_unique<Return>(return_->loc, deepCopyVec(return_->exprs));
        },
        [&](const parser::SClass *sClass) {
            result = std::make_unique<SClass>(sClass->loc, sClass->declLoc, deepCopy(sClass->expr.get()),
                                              deepCopy(sClass->body.get()));
        },
        [&](const parser::Self *self) { result = std::make_unique<Self>(self->loc); },
        [&](const parser::Send *send) {
            result = std::make_unique<Send>(send->loc, deepCopy(send->receiver.get()), send->method, send->methodLoc,
                                            deepCopyVec(send->args));
        },
        [&](const parser::Shadowarg *shadowarg) {
            result = std::make_unique<Shadowarg>(shadowarg->loc, shadowarg->name);
        },
        [&](const parser::Splat *splat) { result = std::make_unique<Splat>(splat->loc, deepCopy(splat->var.get())); },
        [&](const parser::SplatLhs *splatLhs) {
            result = std::make_unique<SplatLhs>(splatLhs->loc, deepCopy(splatLhs->var.get()));
        },
        [&](const parser::String *string) { result = std::make_unique<String>(string->loc, string->val); },
        [&](const parser::Super *super) { result = std::make_unique<Super>(super->loc, deepCopyVec(super->args)); },
        [&](const parser::Symbol *symbol) { result = std::make_unique<Symbol>(symbol->loc, symbol->val); },
        [&](const parser::True *true_) { result = std::make_unique<True>(true_->loc); },
        [&](const parser::Undef *undef) { result = std::make_unique<Undef>(undef->loc, deepCopyVec(undef->exprs)); },
        [&](const parser::UnlessGuard *unlessGuard) {
            result = std::make_unique<UnlessGuard>(unlessGuard->loc, deepCopy(unlessGuard->condition.get()));
        },
        [&](const parser::Until *until) {
            result = std::make_unique<Until>(until->loc, deepCopy(until->cond.get()), deepCopy(until->body.get()));
        },
        [&](const parser::UntilPost *untilPost) {
            result = std::make_unique<UntilPost>(untilPost->loc, deepCopy(untilPost->cond.get()),
                                                 deepCopy(untilPost->body.get()));
        },
        [&](const parser::When *when) {
            result = std::make_unique<When>(when->loc, deepCopyVec(when->patterns), deepCopy(when->body.get()));
        },
        [&](const parser::While *while_) {
            result = std::make_unique<While>(while_->loc, deepCopy(while_->cond.get()), deepCopy(while_->body.get()));
        },
        [&](const parser::WhilePost *whilePost) {
            result = std::make_unique<WhilePost>(whilePost->loc, deepCopy(whilePost->cond.get()),
                                                 deepCopy(whilePost->body.get()));
        },
        [&](const parser::XString *xString) {
            result = std::make_unique<XString>(xString->loc, deepCopyVec(xString->nodes));
        },
        [&](const parser::Yield *yield) { result = std::make_unique<Yield>(yield->loc, deepCopyVec(yield->exprs)); },
        [&](const parser::ZSuper *zSuper) { result = std::make_unique<ZSuper>(zSuper->loc); },
        [&](const parser::Node *other) { ENFORCE(false, "Unimplemented node type: {}", other->nodeName()); });

    return result;
}

} // namespace

std::unique_ptr<Node> Node::deepCopy() const {
    return sorbet::parser::deepCopy(this);
}

} // namespace sorbet::parser
