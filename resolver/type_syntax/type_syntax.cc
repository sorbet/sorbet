#include "resolver/type_syntax/type_syntax.h"
#include "common/typecase.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TypeErrorDiagnostics.h"
#include "core/errors/resolver.h"

using namespace std;

namespace sorbet::resolver {

// Forward declarations for the local versions of getResultType, getResultTypeAndBind, and parseSig that skolemize type
// members.
namespace {
optional<core::TypePtr> getResultTypeWithSelfTypeParams(core::Context ctx, const ast::ExpressionPtr &expr,
                                                        const ParsedSig &sigBeingParsed, TypeSyntaxArgs args);

optional<TypeSyntax::ResultType> getResultTypeAndBindWithSelfTypeParams(core::Context ctx,
                                                                        const ast::ExpressionPtr &expr,
                                                                        const ParsedSig &sigBeingParsed,
                                                                        TypeSyntaxArgs args);

optional<ParsedSig> parseSigWithSelfTypeParams(core::Context ctx, const ast::Send &sigSend, const ParsedSig *parent,
                                               TypeSyntaxArgs args);
} // namespace

ParsedSig TypeSyntax::parseSigTop(core::Context ctx, const ast::Send &sigSend, core::SymbolRef blameSymbol) {
    auto args = TypeSyntaxArgs{
        /* allowSelfType */ true,
        /* allowRebind */ false,
        TypeSyntaxArgs::TypeMember::Allowed,
        /* allowUnspecifiedTypeParameter */ false,
        blameSymbol,
    };
    // Because allowUnspecifiedTypeParameter is always `false`, we know that `parseSig` will never return nullopt.
    return move(TypeSyntax::parseSig(ctx, sigSend, nullptr, args).value());
}

optional<ParsedSig> TypeSyntax::parseSig(core::Context ctx, const ast::Send &sigSend, const ParsedSig *parent,
                                         TypeSyntaxArgs args) {
    auto maybeResult = parseSigWithSelfTypeParams(ctx, sigSend, parent, args);
    if (!maybeResult.has_value()) {
        return nullopt;
    }
    auto result = move(maybeResult.value());

    for (auto &arg : result.argTypes) {
        arg.type = core::Types::unwrapSelfTypeParam(ctx, arg.type);
    }

    if (result.returns != nullptr) {
        result.returns = core::Types::unwrapSelfTypeParam(ctx, result.returns);
    }

    return result;
}

core::TypePtr TypeSyntax::getResultType(core::Context ctx, const ast::ExpressionPtr &expr,
                                        const ParsedSig &sigBeingParsed, TypeSyntaxArgs args) {
    if (auto result = getResultTypeWithSelfTypeParams(ctx, expr, sigBeingParsed, args.withoutRebind())) {
        return core::Types::unwrapSelfTypeParam(ctx, *result);
    } else {
        return core::Types::todo();
    }
}

namespace {

// Parse a literal type for use with `T.deprecated_enum`. If the type is indeed a literal, this will return the
// `underlying` of that literal. The effect of using `underlying` is that `T.deprecated_enum(["a", "b", true])` will be
// parsed as the type `T.any(String, TrueClass)`.
core::TypePtr getResultLiteral(core::Context ctx, const ast::ExpressionPtr &expr) {
    if (auto lit = ast::cast_tree<ast::Literal>(expr)) {
        return core::Types::widen(ctx, lit->value);
    }

    if (auto e = ctx.beginError(expr.loc(), core::errors::Resolver::InvalidTypeDeclaration)) {
        e.setHeader("Unsupported type literal");
    }

    return core::Types::untypedUntracked();
}

bool isTProc(core::Context ctx, const ast::Send *send) {
    while (send != nullptr) {
        if (send->fun == core::Names::proc()) {
            if (auto rcv = ast::cast_tree<ast::ConstantLit>(send->recv)) {
                return rcv->symbol() == core::Symbols::T();
            }
        }
        send = ast::cast_tree<ast::Send>(send->recv);
    }
    return false;
}

} // namespace

bool TypeSyntax::isSig(core::Context ctx, const ast::Send &send) {
    if (send.fun != core::Names::sig()) {
        return false;
    }
    if (!send.hasBlock()) {
        return false;
    }

    auto recv = ast::cast_tree<ast::ConstantLit>(send.recv);

    auto nargs = send.numPosArgs();
    if (!(nargs == 1 || nargs == 2)) {
        return false;
    }

    if (recv != nullptr && recv->symbol() == core::Symbols::Sorbet_Private_Static()) {
        return true;
    }

    return false;
}

namespace {

// When a sig was given with multiple statements, autocorrect it to a single chained send.
void addMultiStatementSigAutocorrect(core::Context ctx, core::ErrorBuilder &e, const ast::ExpressionPtr &blockBody) {
    auto insseq = ast::cast_tree<ast::InsSeq>(blockBody);
    if (insseq == nullptr) {
        return;
    }

    vector<core::LocOffsets> locs;
    for (auto &expr : insseq->stats) {
        auto send = ast::cast_tree<ast::Send>(expr);
        if (send == nullptr || !send->loc.exists()) {
            return;
        }

        locs.emplace_back(send->loc);
    }

    {
        auto send = ast::cast_tree<ast::Send>(insseq->expr);
        if (send == nullptr || !send->loc.exists()) {
            return;
        }

        locs.emplace_back(send->loc);
    }

    auto source = ctx.file.data(ctx).source();

    bool first = true;
    string replacement;
    for (auto loc : locs) {
        if (!first) {
            replacement.append(".");
        }

        auto len = loc.endLoc - loc.beginLoc;
        replacement.append(source.substr(loc.beginLoc, len));

        first = false;
    }

    e.replaceWith("Use a chained sig builder", ctx.locAt(insseq->loc), "{}", replacement);
}

void checkTypeFunArity(core::Context ctx, const ast::Send &send, size_t minArity, size_t maxArity) {
    const auto &file = ctx.file.data(ctx);
    if (!(file.isRBI() || file.strictLevel < core::StrictLevel::True)) {
        // We want to rely on the infer error here, because it will be more descriptive.
        // We do still need to report an error in `# typed: false` files and RBI files where
        // inference will not run.
        return;
    }

    if (send.numPosArgs() < minArity || send.numPosArgs() > maxArity) {
        auto errLoc = send.numPosArgs() > 0 ? send.argsLoc() : send.loc;
        if (auto e = ctx.beginError(errLoc, core::errors::Resolver::InvalidTypeDeclaration)) {
            auto howMany = minArity == maxArity ? "exactly" : "at least";
            auto plural = minArity == 1 ? "" : "s";
            e.setHeader("`{}` expects {} `{}` argument{}, but got `{}`", send.fun.show(ctx), howMany, minArity, plural,
                        send.numPosArgs());
        }
    }
}

core::TypeMemberRef checkValidAttachedClass(core::Context ctx, core::LocOffsets errLoc) {
    ENFORCE(ctx.owner.isClassOrModule());
    auto owner = ctx.owner.asClassOrModuleRef();
    auto ownerData = owner.data(ctx);

    auto maybeAttachedClass = ownerData->findMember(ctx, core::Names::Constants::AttachedClass());
    if (!maybeAttachedClass.exists() ||
        (ownerData->isSingletonClass(ctx) && ownerData->attachedClass(ctx).data(ctx)->isModule())) {
        // ^ Excluding the isModule case because it's not clear that the user should ever do this
        // (We could technically parse it, but it almost certainly indicates the user is
        // doing something they didn't mean to, so let's see an example of this firing
        // before we actually support it.)
        if (auto e = ctx.beginError(errLoc, core::errors::Resolver::InvalidTypeDeclaration)) {
            auto hasAttachedClass = core::Names::declareHasAttachedClass().show(ctx);
            if (ownerData->isModule()) {
                e.setHeader("`{}` must declare `{}` before module instance methods can use `{}`", owner.show(ctx),
                            hasAttachedClass, "T.attached_class");
                // TODO(jez) Autocorrect to insert `has_attached_class!`
            } else if (ownerData->isSingletonClass(ctx)) {
                e.setHeader("`{}` cannot be used in singleton methods on modules, because modules cannot be "
                            "instantiated",
                            "T.attached_class");
            } else {
                e.setHeader("`{}` may only be used in singleton methods on classes or instance methods on `{}` modules",
                            "T.attached_class", hasAttachedClass);
                e.addErrorNote("Current context is `{}`, which is an instance class not a singleton class",
                               owner.show(ctx));
            }
        }
        return core::Symbols::noTypeMember();
    } else {
        return maybeAttachedClass.asTypeMemberRef();
    }
}

optional<ParsedSig> parseSigWithSelfTypeParams(core::Context ctx, const ast::Send &sigSend, const ParsedSig *parent,
                                               TypeSyntaxArgs args) {
    ParsedSig sig;

    const ast::Send *send = nullptr;
    bool isProc = false;
    if (isTProc(ctx, &sigSend)) {
        send = &sigSend;
        isProc = true;
    } else {
        sig.seen.sig = sigSend.loc;
        ENFORCE(sigSend.fun == core::Names::sig());
        auto *block = sigSend.block();
        ENFORCE(block);
        auto blockBody = ast::cast_tree<ast::Send>(block->body);
        if (blockBody) {
            send = blockBody;
        } else {
            if (auto e = ctx.beginError(sigSend.loc, core::errors::Resolver::MultipleStatementsInSig)) {
                e.setHeader("Malformed `{}`: Signature blocks must contain a single statement", "sig");
                addMultiStatementSigAutocorrect(ctx, e, block->body);
            }

            return sig;
        }
    }
    ENFORCE(send != nullptr);

    if (sigSend.numPosArgs() == 2) {
        auto lit = ast::cast_tree<ast::Literal>(sigSend.getPosArg(1));
        if (lit != nullptr && lit->isSymbol() && lit->asSymbol() == core::Names::final_()) {
            sig.seen.final = lit->loc;
        }
    }

    const ast::Send *tsend = send;
    // extract type parameters early
    while (tsend != nullptr) {
        if (tsend->fun == core::Names::typeParameters()) {
            if (parent != nullptr) {
                if (auto e = ctx.beginError(tsend->loc, core::errors::Resolver::InvalidMethodSignature)) {
                    e.setHeader("Malformed `{}`: Type parameters can only be specified in outer sig", "sig");
                }
                break;
            }
            for (auto &arg : tsend->posArgs()) {
                auto c = ast::cast_tree<ast::Literal>(arg);
                if (c == nullptr) {
                    if (auto e = ctx.beginError(arg.loc(), core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Unexpected `{}`: Type parameters are specified with symbols", arg.nodeName());
                    }
                    continue;
                }

                if (!c->isSymbol()) {
                    if (auto e = ctx.beginError(arg.loc(), core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`: Type parameters are specified with symbols", "sig");
                    }
                    continue;
                }

                auto name = c->asSymbol();
                auto &typeArgSpec = sig.enterTypeParamByName(name);
                if (typeArgSpec.type) {
                    if (auto e = ctx.beginError(arg.loc(), core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`: Type argument `{}` was specified twice", "sig", name.show(ctx));
                    }
                }
                typeArgSpec.type = core::make_type<core::TypeVar>(core::Symbols::todoTypeParameter());
                typeArgSpec.loc = arg.loc();
            }

            for (auto [key, _value] : tsend->kwArgPairs()) {
                if (auto e = ctx.beginError(key.loc(), core::errors::Resolver::InvalidMethodSignature)) {
                    e.setHeader("Malformed `{}`: Type parameters are specified with symbols", "sig");
                }
            }

            if (tsend->kwSplat()) {
                if (auto e = ctx.beginError(tsend->kwSplat()->loc(), core::errors::Resolver::InvalidMethodSignature)) {
                    e.setHeader("Malformed `{}`: Type parameters are specified with symbols", "sig");
                }
            }
        }
        tsend = ast::cast_tree<ast::Send>(tsend->recv);
    }
    if (parent == nullptr) {
        parent = &sig;
    }

    while (send != nullptr) {
        // so we don't report multiple "method does not exist" errors arising from the same expression
        bool reportedInvalidMethod = false;
        switch (send->fun.rawId()) {
            case core::Names::proc().rawId(): {
                checkTypeFunArity(ctx, *send, 0, 0);
                sig.seen.proc = send->funLoc;
                break;
            }
            case core::Names::bind().rawId(): {
                if (sig.seen.bind.exists()) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`: Multiple calls to `.bind`", send->fun.show(ctx));
                    }
                    sig.bind = core::Symbols::noClassOrModule();
                }
                sig.seen.bind = send->funLoc;

                if (send->numPosArgs() != 1) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Wrong number of args to `{}`. Expected: `{}`, got: `{}`", "bind", 1,
                                    send->numPosArgs());
                    }
                    break;
                }

                bool validBind = false;
                if (auto arg = ast::cast_tree<ast::Send>(send->getPosArg(0))) {
                    if (arg->fun == core::Names::selfType()) {
                        sig.bind = core::Symbols::MagicBindToSelfType();
                        validBind = true;
                    } else if (arg->fun == core::Names::attachedClass()) {
                        auto attachedClass = checkValidAttachedClass(ctx, arg->loc);
                        if (!attachedClass.exists()) {
                            break;
                        }

                        sig.bind = core::Symbols::MagicBindToAttachedClass();
                        validBind = true;
                    }
                }

                core::TypePtr bind;
                if (!validBind) {
                    auto maybeBind = getResultTypeWithSelfTypeParams(ctx, send->getPosArg(0), *parent, args);
                    if (!maybeBind.has_value()) {
                        validBind = false;
                    } else {
                        bind = move(maybeBind.value());
                    }
                }

                if (core::isa_type<core::ClassType>(bind)) {
                    auto classType = core::cast_type_nonnull<core::ClassType>(bind);
                    sig.bind = classType.symbol;
                    validBind = true;
                } else if (auto appType = core::cast_type<core::AppliedType>(bind)) {
                    // When `T.proc.bind` is used with `T.class_of`, pass it
                    // through as long as it only has the AttachedClass type
                    // member.
                    if (appType->klass.data(ctx)->isSingletonClass(ctx) &&
                        appType->klass.data(ctx)->typeMembers().size() == 1) {
                        sig.bind = appType->klass;
                        validBind = true;
                    }
                }

                if (!validBind) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`: Can only bind to simple class names", send->fun.show(ctx));
                    }
                }

                break;
            }
            case core::Names::params().rawId(): {
                if (sig.seen.params.exists()) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`: Multiple calls to `.params`", send->fun.show(ctx));
                    }
                    sig.argTypes.clear();
                }
                sig.seen.params = send->funLoc;

                if (!send->hasKwArgs() && !send->hasPosArgs()) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        auto paramsStr = send->fun.show(ctx);
                        e.setHeader("`{}` must be given arguments", paramsStr);

                        core::Loc loc{ctx.file, send->loc};
                        if (auto orig = loc.source(ctx)) {
                            auto dot = orig->rfind(".");
                            if (orig->rfind(".") == string::npos) {
                                e.replaceWith("Remove this use of `params`", loc, "");
                            } else {
                                e.replaceWith("Remove this use of `params`", loc, "{}", orig->substr(0, dot));
                            }
                        }
                    }
                    break;
                }

                // `params` only accepts keyword args
                if (send->numPosArgs() != 0) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        auto paramsStr = send->fun.show(ctx);
                        e.setHeader("`{}` expects keyword arguments", paramsStr);
                        e.addErrorNote("All parameters must be given names in `{}` even if they are positional",
                                       paramsStr);

                        // when the first argument is a hash, emit an autocorrect to remove the braces
                        if (send->numPosArgs() == 1) {
                            if (auto hash = ast::cast_tree<ast::Hash>(send->getPosArg(0))) {
                                // TODO(jez) Use Loc::adjust here
                                auto loc = core::Loc(ctx.file, hash->loc.beginPos(), hash->loc.endPos());
                                if (auto locSource = loc.source(ctx)) {
                                    e.replaceWith("Remove braces from keyword args", loc, "{}",
                                                  locSource->substr(1, locSource->size() - 2));
                                }
                            }
                        }
                    }
                    break;
                }

                if (send->hasKwSplat()) {
                    // TODO(trevor) add an error for this
                }

                for (auto [key, value] : send->kwArgPairs()) {
                    auto lit = ast::cast_tree<ast::Literal>(key);
                    if (lit && lit->isSymbol()) {
                        core::NameRef name = lit->asSymbol();
                        auto maybeResultAndBind = getResultTypeAndBindWithSelfTypeParams(
                            ctx, value, *parent, isProc ? args.withoutRebind() : args.withRebind());
                        if (!maybeResultAndBind.has_value()) {
                            return nullopt;
                        }
                        auto resultAndBind = move(maybeResultAndBind.value());

                        sig.argTypes.emplace_back(
                            ParsedSig::ArgSpec{key.loc(), name, value.loc(), resultAndBind.rebind, resultAndBind.type});
                    }
                }
                break;
            }
            case core::Names::typeParameters().rawId():
                // was handled above
                break;
            case core::Names::abstract().rawId():
                if (sig.seen.final.exists()) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Method that is both `{}` and `{}` cannot be implemented", "final", "abstract");
                    }
                }
                if (sig.seen.override_.exists()) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("`{}` cannot be combined with `{}`", "abstract", "override");
                    }
                }
                sig.seen.abstract = send->funLoc;
                break;
            case core::Names::override_().rawId(): {
                if (sig.seen.abstract.exists()) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("`{}` cannot be combined with `{}`", "override", "abstract");
                    }
                }
                sig.seen.override_ = send->funLoc.join(send->loc.copyEndWithZeroLength());

                if (send->hasPosArgs()) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("`{}` expects keyword arguments", send->fun.show(ctx));
                    }
                    break;
                }

                if (send->hasKwArgs()) {
                    for (auto [key, value] : send->kwArgPairs()) {
                        auto lit = ast::cast_tree<ast::Literal>(key);
                        if (lit && lit->isSymbol()) {
                            if (lit->asSymbol() == core::Names::allowIncompatible()) {
                                auto val = ast::cast_tree<ast::Literal>(value);
                                if (val && val->isTrue(ctx)) {
                                    sig.seen.incompatibleOverride = key.loc().join(value.loc());
                                } else if (val && val->isFalse(ctx)) {
                                    if (auto e =
                                            ctx.beginError(val->loc, core::errors::Resolver::InvalidMethodSignature)) {
                                        e.setHeader("`{}` expects either `{}` or `{}`",
                                                    "override(allow_incompatible: ...)", "true", ":visibility");
                                        e.replaceWith("Remove allow_incompatible",
                                                      ctx.locAt(key.loc().join(value.loc())), "");
                                    }
                                } else if (val && val->isSymbol()) {
                                    if (val->asSymbol() == core::Names::visibility()) {
                                        sig.seen.incompatibleOverrideVisibility = key.loc().join(value.loc());
                                    } else {
                                        if (auto e = ctx.beginError(val->loc,
                                                                    core::errors::Resolver::InvalidMethodSignature)) {
                                            e.setHeader("`{}` expects either `{}` or `{}`",
                                                        "override(allow_incompatible: ...)", "true", ":visibility");
                                        }
                                    }
                                }
                                // Other errors are caught with type checking, but since the sig says
                                // `T.any(Symbol, ...)` we have to explicitly check the allowed symbol literals.
                            }
                        }
                    }
                }

                break;
            }
            case core::Names::implementation().rawId():
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::ImplementationDeprecated)) {
                    e.setHeader("Use of `{}` has been replaced by `{}`", "implementation", "override");
                    auto loc = ctx.locAt(send->funLoc);
                    e.replaceWith("Replace with `override`", loc, "override");
                }
                break;
            case core::Names::overridable().rawId():
                if (sig.seen.final.exists()) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Method that is both `{}` and `{}` cannot be implemented", "final", "overridable");
                    }
                }
                sig.seen.overridable = send->funLoc;
                break;
            case core::Names::returns().rawId(): {
                sig.seen.returns = send->funLoc;
                if (send->hasKwArgs()) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("`{}` does not accept keyword arguments", send->fun.show(ctx));
                        if (!send->hasKwSplat()) {
                            auto numKwArgs = send->numKwArgs();
                            auto start = send->getKwKey(0).loc();
                            auto end = send->getKwValue(numKwArgs - 1).loc();
                            core::Loc argsLoc(ctx.file, start.beginPos(), end.endPos());
                            if (argsLoc.exists()) {
                                e.replaceWith("Wrap in braces to make a shape type", argsLoc, "{{{}}}",
                                              argsLoc.source(ctx).value());
                            }
                        }
                    }
                    break;
                }

                if (send->numPosArgs() != 1) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Wrong number of args to `{}`. Expected: `{}`, got: `{}`", "returns", 1,
                                    send->numPosArgs());
                    }
                    break;
                }

                auto maybeReturns = getResultTypeWithSelfTypeParams(ctx, send->getPosArg(0), *parent, args);
                if (!maybeReturns.has_value()) {
                    return nullopt;
                }
                sig.returns = move(maybeReturns.value());
                sig.returnsLoc = send->loc;

                break;
            }
            case core::Names::void_().rawId(): {
                checkTypeFunArity(ctx, *send, 0, 0);
                sig.seen.void_ = send->funLoc;
                sig.returns = core::Types::void_();
                sig.returnsLoc = send->loc;
                break;
            }
            case core::Names::checked().rawId(): {
                checkTypeFunArity(ctx, *send, 1, 1);
                sig.seen.checked = send->funLoc;
                break;
            }
            case core::Names::onFailure().rawId():
                break;
            case core::Names::final_().rawId():
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                    reportedInvalidMethod = true;
                    e.setHeader("The syntax for declaring a method final is `sig(:final) {{...}}`, not `sig "
                                "{{final. ...}}`");
                }
                break;
            default:
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                    reportedInvalidMethod = true;
                    e.setHeader("Malformed `{}`: `{}` is invalid in this context", "sig", send->fun.show(ctx));
                    e.addErrorLine(ctx.locAt(send->loc), "Consult https://sorbet.org/docs/sigs for signature syntax");
                }
        }
        auto recv = ast::cast_tree<ast::Send>(send->recv);

        // we only report this error if we haven't reported another unknown method error
        if (!recv && !reportedInvalidMethod) {
            if (!send->recv.isSelfReference()) {
                if (!sig.seen.proc.exists()) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`: `{}` being invoked on an invalid receiver", "sig",
                                    send->fun.show(ctx));
                    }
                }
            }
            break;
        }

        send = recv;
    }
    ENFORCE(sig.seen.sig.exists() || sig.seen.proc.exists());

    return sig;
}

// This function recurses through an OrType, and accumulates all the class names,
// wrapped in T.class_of, and checks if the type is only made up of Classes and OrTypes
bool recurseOrType(core::Context ctx, core::TypePtr type, vector<string> &v) {
    if (auto o = core::cast_type<core::OrType>(type)) {
        return recurseOrType(ctx, o->left, v) && recurseOrType(ctx, o->right, v);
    } else if (core::isa_type<core::ClassType>(type)) {
        v.push_back(fmt::format("T.class_of({})", type.show(ctx)));
        return true;
    } else {
        return false;
    }
}

void checkUnexpectedKwargs(core::Context ctx, const ast::Send &send) {
    if (!send.hasKwArgs()) {
        return;
    }

    if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidMethodSignature)) {
        string name = "T.";
        name.append(send.fun.shortName(ctx));
        e.setHeader("`{}` does not accept keyword arguments", name);

        if (send.hasKwSplat()) {
            // There's not _really_ a good autocorrect we can do here. Something is completely wrong.
            // Simply deleting the whole kwsplat might mean that we're left with no args entirely,
            // which is not obviously better than having the splat there.
        } else {
            auto start = send.getKwKey(0).loc();
            auto end = send.getKwValue(send.numKwArgs() - 1).loc();
            if (start.exists() && end.exists()) {
                core::Loc loc{ctx.file, start.join(end)};
                if (auto keywords = loc.source(ctx)) {
                    e.replaceWith("Remove keyword args", core::Loc{ctx.file, start.join(end)}, "{{{}}}", *keywords);
                }
            }
        }
    }
}

core::ClassOrModuleRef sendLooksLikeBadTypeApplication(core::Context ctx, const ast::Send &send) {
    core::SymbolRef maybeScopeClass;
    if (auto recv = ast::cast_tree<ast::ConstantLit>(send.recv)) {
        maybeScopeClass = recv->symbol();
    } else if (send.recv.isSelfReference()) {
        // Let's not try to reinvent constant resolution here and just pick a heuristic that tends to
        // work in some cases and is simple.
        maybeScopeClass = core::Symbols::root();
    } else {
        return core::Symbols::noClassOrModule();
    }

    if (!maybeScopeClass.isClassOrModule()) {
        return core::Symbols::noClassOrModule();
    }

    auto scope = maybeScopeClass.asClassOrModuleRef();

    auto className = ctx.state.lookupNameConstant(send.fun);
    if (!className.exists()) {
        // The name itself doesn't even exist, so definitely no class with this name can exist
        return core::Symbols::noClassOrModule();
    }

    auto maybeSym = scope.data(ctx)->findMember(ctx, className);
    if (!maybeSym.exists()) {
        return core::Symbols::noClassOrModule();
    }

    if (!maybeSym.isClassOrModule()) {
        return core::Symbols::noClassOrModule();
    }

    auto klass = maybeSym.asClassOrModuleRef();

    if (klass.data(ctx)->typeArity(ctx) == 0) {
        return core::Symbols::noClassOrModule();
    }

    return klass;
}

void maybeSuggestTClass(core::Context ctx, core::ErrorBuilder &e, core::LocOffsets sendLoc, core::LocOffsets argLoc) {
    e.addErrorNote("You may wish to use `{}`, which doesn't have this restriction.\n"
                   "    For more information, see https://sorbet.org/docs/class-of#tclass-vs-tclass_of",
                   "T::Class");
    if (sendLoc.exists() && !sendLoc.empty() && argLoc.exists() && !argLoc.empty()) {
        e.replaceWith("Use `T::Class` instead", ctx.locAt(sendLoc), "T::Class[{}]",
                      ctx.locAt(argLoc).source(ctx).value());
    }
}

optional<core::ClassOrModuleRef> parseTClassOf(core::Context ctx, const ast::Send &send, const ParsedSig &sig,
                                               TypeSyntaxArgs args) {
    if (send.numPosArgs() != 1 || send.hasKwArgs()) {
        checkTypeFunArity(ctx, send, 1, 1);
        checkUnexpectedKwargs(ctx, send);
        return core::Symbols::untyped();
    }

    auto obj = ast::cast_tree<ast::ConstantLit>(send.getPosArg(0));
    if (!obj) {
        if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
            auto maybeType = getResultTypeWithSelfTypeParams(ctx, send.getPosArg(0), sig, args);
            if (!maybeType.has_value()) {
                return nullopt;
            }
            auto type = move(maybeType.value());
            vector<string> classes;
            auto shouldAutoCorrect = recurseOrType(ctx, type, classes);
            if (core::isa_type<core::OrType>(type) && shouldAutoCorrect) {
                auto autocorrect = fmt::format("T.any({})", fmt::join(classes, ", "));
                e.setHeader("`{}` must wrap each individual class type, not the outer `{}`", "T.class_of", "T.any");
                e.replaceWith("Distribute `T.class_of`", ctx.locAt(send.loc), "{}", autocorrect);
            } else {
                e.setHeader("`{}` needs a class or module as its argument", "T.class_of");
                maybeSuggestTClass(ctx, e, send.loc, send.getPosArg(0).loc());
            }
        }
        return core::Symbols::untyped();
    }
    auto maybeAliased = obj->symbol();
    if (maybeAliased.isTypeAlias(ctx)) {
        if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
            e.setHeader("T.class_of can't be used with a T.type_alias");
            maybeSuggestTClass(ctx, e, send.loc, obj->loc());
        }
        return core::Symbols::untyped();
    }
    if (maybeAliased.isTypeMember()) {
        if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
            e.setHeader("T.class_of can't be used with a T.type_member");
            maybeSuggestTClass(ctx, e, send.loc, obj->loc());
        }
        return core::Symbols::untyped();
    }
    auto sym = maybeAliased.dealias(ctx);
    if (sym.isStaticField(ctx)) {
        if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
            e.setHeader("T.class_of can't be used with a constant field");
            maybeSuggestTClass(ctx, e, send.loc, obj->loc());
        }
        return core::Symbols::untyped();
    }

    auto singleton = sym.asClassOrModuleRef().data(ctx)->lookupSingletonClass(ctx);
    if (!singleton.exists()) {
        if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
            e.setHeader("Unknown class");
        }
        return core::Symbols::untyped();
    }
    return singleton;
}

void checkTNilableArity(core::Context ctx, const ast::Send &send) {
    const auto &file = ctx.file.data(ctx);
    auto willReportInferError = !(file.isRBI() || file.strictLevel < core::StrictLevel::True);
    auto canWrapWithTAny = send.numPosArgs() > 1 && !send.hasKwArgs() && send.argsLoc().exists();

    if (willReportInferError && !canWrapWithTAny) {
        // No use reporting a double error
        return;
    }

    // Reports a double error when `willReportInferError && canWrapWithTAny`, so that we can attach
    // an autocorrect even in files that have infer run on them.

    auto errLoc = send.numPosArgs() > 0 ? send.argsLoc() : send.loc;
    if (auto e = ctx.beginError(errLoc, core::errors::Resolver::TNilableArity)) {
        e.setHeader("`{}` expects exactly `{}` arguments, but got `{}`", "T.nilable", 1, send.numPosArgs());
        if (canWrapWithTAny) {
            e.addErrorNote("Did you mean to use `{}` around the inner arguments?", "T.any");
            auto replaceLoc = ctx.locAt(send.argsLoc());
            e.replaceWith("Wrap args with T.any", replaceLoc, "T.any({})", replaceLoc.source(ctx).value());
        }
    }
}

optional<TypeSyntax::ResultType> interpretTCombinator(core::Context ctx, const ast::Send &send, const ParsedSig &sig,
                                                      TypeSyntaxArgs args) {
    auto &recvi = ast::cast_tree_nonnull<ast::ConstantLit>(send.recv);

    switch (send.fun.rawId()) {
        case core::Names::nilable().rawId(): {
            if (send.numPosArgs() != 1 || send.hasKwArgs()) {
                checkTNilableArity(ctx, send);
                checkUnexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto maybeResult = getResultTypeAndBindWithSelfTypeParams(ctx, send.getPosArg(0), sig, args);
            if (!maybeResult.has_value()) {
                return nullopt;
            }
            auto result = move(maybeResult.value());
            if (result.type.isUntyped()) {
                // As a compromise, only raise the error when the argument is syntactically `T.untyped`, as arguments of
                // type `T.untyped` can arise through other errors. This isn't ideal as it allows cases like:
                //
                // > X = T.type_alias {T.untyped}
                // > Y = T.type_alias {T.nilable(X)}
                //
                auto arg = ast::cast_tree<ast::Send>(send.getPosArg(0));
                if (arg != nullptr && arg->fun == core::Names::untyped() && !arg->hasPosArgs() && !arg->hasKwArgs()) {
                    if (auto e = ctx.beginError(send.loc, core::errors::Resolver::NilableUntyped)) {
                        e.setHeader("`{}` is the same as `{}`", "T.nilable(T.untyped)", "T.untyped");
                        e.replaceWith("Replace with `T.untyped`", ctx.locAt(send.loc), "T.untyped");
                    }
                }
                return result;
            }

            return TypeSyntax::ResultType{core::Types::any(ctx, result.type, core::Types::nilClass()), result.rebind};
        }
        case core::Names::all().rawId(): {
            auto posArgs = send.posArgs();
            if (posArgs.size() < 2 || send.hasKwArgs()) {
                checkUnexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto maybeResult = getResultTypeWithSelfTypeParams(ctx, posArgs[0], sig, args);
            if (!maybeResult.has_value()) {
                return nullopt;
            }
            auto result = move(maybeResult.value());
            auto remainingArgs = posArgs.subspan(1);
            for (auto &arg : remainingArgs) {
                auto maybeResult = getResultTypeWithSelfTypeParams(ctx, arg, sig, args);
                if (!maybeResult.has_value()) {
                    return nullopt;
                }
                result = core::Types::all(ctx, result, move(maybeResult.value()));
            }
            return TypeSyntax::ResultType{result, core::Symbols::noClassOrModule()};
        }
        case core::Names::any().rawId(): {
            auto posArgs = send.posArgs();
            if (posArgs.size() < 2 || send.hasKwArgs()) {
                checkTypeFunArity(ctx, send, 2, SIZE_MAX);
                checkUnexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto maybeResult = getResultTypeWithSelfTypeParams(ctx, posArgs[0], sig, args);
            if (!maybeResult.has_value()) {
                return nullopt;
            }
            auto result = move(maybeResult.value());
            auto remainingArgs = posArgs.subspan(1);
            for (auto &arg : remainingArgs) {
                auto maybeResult = getResultTypeWithSelfTypeParams(ctx, arg, sig, args);
                if (!maybeResult.has_value()) {
                    return nullopt;
                }
                result = core::Types::any(ctx, result, move(maybeResult.value()));
            }
            return TypeSyntax::ResultType{result, core::Symbols::noClassOrModule()};
        }
        case core::Names::typeParameter().rawId(): {
            if (send.numPosArgs() != 1 || send.hasKwArgs()) {
                checkTypeFunArity(ctx, send, 1, 1);
                checkUnexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto arr = ast::cast_tree<ast::Literal>(send.getPosArg(0));
            if (!arr || !arr->isSymbol()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("type_parameter requires a symbol");
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto fnd = sig.findTypeParamByName(arr->asSymbol());
            if (!fnd.type) {
                if (args.allowUnspecifiedTypeParameter) {
                    // Return nullopt, which will indicate that we couldn't parse the sig at this time.
                    return nullopt;
                } else {
                    if (auto e = ctx.beginError(arr->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Unspecified type parameter");
                    }
                    return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
                }
            }
            return TypeSyntax::ResultType{fnd.type, core::Symbols::noClassOrModule()};
        }
        case core::Names::enum_().rawId():
        case core::Names::deprecatedEnum().rawId(): {
            if (send.numPosArgs() != 1 || send.hasKwArgs()) {
                checkTypeFunArity(ctx, send, 1, 1);
                checkUnexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }

            if (send.fun == core::Names::enum_()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("`{}` has been renamed to `{}`", "T.enum", "T.deprecated_enum");

                    if (send.funLoc.exists() && !send.funLoc.empty()) {
                        e.replaceWith("Replace with `deprecated_enum`", ctx.locAt(send.funLoc), "{}",
                                      "deprecated_enum");
                    }
                }
            }

            auto arr = ast::cast_tree<ast::Array>(send.getPosArg(0));
            if (arr == nullptr) {
                // TODO(pay-server) unsilence this error and support enums from pay-server
                { return TypeSyntax::ResultType{core::Types::Object(), core::Symbols::noClassOrModule()}; }
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("enum must be passed a literal array. e.g. enum([1,\"foo\",MyClass])");
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }

            auto arrayElements = absl::MakeSpan(arr->elems);
            if (arrayElements.empty()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("enum([]) is invalid");
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto result = core::Types::bottom();
            for (auto &e : arrayElements) {
                result = core::Types::any(ctx, result, getResultLiteral(ctx, e));
            }
            return TypeSyntax::ResultType{result, core::Symbols::noClassOrModule()};
        }
        case core::Names::classOf().rawId(): {
            if (auto parseResult = parseTClassOf(ctx, send, sig, args)) {
                // TODO(jez) At some point, we will want to emit an error for not passing type args
                // to a generic singleton class, like how we report a "Generic class without type arguments"
                // error for normal classes.
                //
                // For the moment, we're punting on introducing that error because it would apply
                // to every use of `T.class_of` on a class, because all class singleton classes are
                // generic in <AttachedClass>. It would be redundant to have to write
                // T.class_of(My::Long::Class::Name)[My::Long::Class::Name], but we also can't
                // settle on a good set of rules for how default generic types should work (too many
                // footguns).
                //
                // At the very least, this is not a _new_ problem--it's been there ever since we
                // added `type_template`--so it can remain unfixed a little while longer.
                //
                // The call to `externalType` below implements certain defaulting rules (based on
                // variance). It's worth noting that those defaulting rules were built at the same
                // time that we added the `<AttachedClass>` type_template, designed to avoid making
                // people provide type arguments for all `T.class_of`.
                return TypeSyntax::ResultType{
                    parseResult.value().data(ctx)->externalType(),
                    core::Symbols::noClassOrModule(),
                };
            } else {
                return nullopt;
            }
        }
        case core::Names::untyped().rawId(): {
            if (send.numPosArgs() != 0 || send.hasKwArgs()) {
                checkTypeFunArity(ctx, send, 0, 0);
                checkUnexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            return TypeSyntax::ResultType{core::Types::untyped(args.untypedBlame), core::Symbols::noClassOrModule()};
        }
        case core::Names::selfType().rawId():
            if (send.numPosArgs() != 0 || send.hasKwArgs()) {
                checkTypeFunArity(ctx, send, 0, 0);
                checkUnexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }

            switch (args.typeMember) {
                case TypeSyntaxArgs::TypeMember::Allowed:
                case TypeSyntaxArgs::TypeMember::BannedInTypeAlias:
                    break;

                case TypeSyntaxArgs::TypeMember::BannedInTypeMember:
                    // It would cause problems in `ClassOrModule::selfType` if we allowed this.
                    //
                    // The naive implementation (what's there now) produces a type that is not fully
                    // defined, despite the point of that function being to produce a fully-defined type.
                    //
                    // An alternative implementation might be to try to recursively expand `T.self_type`
                    // LambdaParam's to `FreshSelfType`, but that would produce an infinite type.
                    //
                    // We'll need a smarter approach, and it's not clear what that is. Since `T.self_type`
                    // in type member bounds already failed (with a use-site error), turning it into a
                    // definition-site error should be more straightforward and prevent crashes in the mean time.
                    if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("`{}` is not supported inside generic type bounds", "T.self_type");
                    }
                    return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }

            if (!args.allowSelfType) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Only top-level `{}` is supported", "T.self_type");
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }

            return TypeSyntax::ResultType{core::make_type<core::SelfType>(), core::Symbols::noClassOrModule()};
        case core::Names::experimentalAttachedClass().rawId():
        case core::Names::attachedClass().rawId(): {
            if (send.fun == core::Names::experimentalAttachedClass()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::ExperimentalAttachedClass)) {
                    e.setHeader("`{}` has been stabilized and is no longer experimental",
                                "T.experimental_attached_class");
                    e.replaceWith("Replace with `T.attached_class`", ctx.locAt(send.loc), "T.attached_class");
                }
            }
            if (recvi.symbol() != core::Symbols::Magic()) {
                // Resolver would have written syntactically-valid `T.attached_class` calls to a call on `<Magic>`
                checkTypeFunArity(ctx, send, 0, 0);
                checkUnexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }

            const auto attachedClass = checkValidAttachedClass(ctx, send.loc);
            if (!attachedClass.exists()) {
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            } else {
                return TypeSyntax::ResultType{core::make_type<core::SelfTypeParam>(attachedClass),
                                              core::Symbols::noClassOrModule()};
            }
        }
        case core::Names::noreturn().rawId(): {
            if (send.numPosArgs() != 0 || send.hasKwArgs()) {
                checkTypeFunArity(ctx, send, 0, 0);
                checkUnexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            return TypeSyntax::ResultType{core::Types::bottom(), core::Symbols::noClassOrModule()};
        }
        case core::Names::anything().rawId(): {
            if (send.numPosArgs() != 0 || send.hasKwArgs()) {
                checkTypeFunArity(ctx, send, 0, 0);
                checkUnexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            return TypeSyntax::ResultType{core::Types::top(), core::Symbols::noClassOrModule()};
        }

        default:
            if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                if (send.numPosArgs() > 0 && send.onlyPosArgs() && send.block() == nullptr && send.argsLoc().exists() &&
                    ctx.locAt(send.funLoc).adjustLen(ctx, -1, 1).source(ctx) == ":") {
                    auto replacement =
                        fmt::format("T::{}[{}]", send.fun.show(ctx), ctx.locAt(send.argsLoc()).source(ctx).value());
                    e.setHeader("Did you mean to use square brackets: `{}`", replacement);
                    e.replaceWith("Use square brackets for type args", ctx.locAt(send.loc), "{}", replacement);
                } else {
                    e.setHeader("Unsupported method `{}`", "T." + send.fun.show(ctx));
                }
            }
            return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
    }
}

optional<core::TypePtr> getResultTypeWithSelfTypeParams(core::Context ctx, const ast::ExpressionPtr &expr,
                                                        const ParsedSig &sigBeingParsed, TypeSyntaxArgs args) {
    if (auto result = getResultTypeAndBindWithSelfTypeParams(ctx, expr, sigBeingParsed, args.withoutRebind())) {
        return result->type;
    } else {
        return nullopt;
    }
}

TypeSyntax::ResultType reportUnknownTypeSyntaxError(core::Context ctx, const ast::Send &s,
                                                    TypeSyntax::ResultType &&result) {
    if (auto e = ctx.beginError(s.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
        auto klass = sendLooksLikeBadTypeApplication(ctx, s);
        if (klass.exists() && s.hasPosArgs()) {
            auto scope =
                s.recv.isSelfReference() ? "" : fmt::format("{}::", ctx.locAt(s.recv.loc()).source(ctx).value());
            auto replacement =
                fmt::format("{}{}[{}]", scope, s.fun.show(ctx), ctx.locAt(s.argsLoc()).source(ctx).value());
            e.setHeader("Did you mean to use square brackets: `{}`", replacement);
            e.replaceWith("Use square brackets for type args", ctx.locAt(s.loc), "{}", replacement);
        } else {
            e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
        }
    }

    result.type = core::Types::untypedUntracked();
    return move(result);
}

optional<TypeSyntax::ResultType> getResultTypeAndBindWithSelfTypeParamsImpl(core::Context ctx,
                                                                            const ast::ExpressionPtr &expr,
                                                                            const ParsedSig &sigBeingParsed,
                                                                            TypeSyntaxArgs args) {
    // Ensure that we only check types from a class context
    ENFORCE(ctx.owner.isClassOrModule(), "getResultTypeAndBind wasn't called with a class owner");
    auto ctxOwnerData = ctx.owner.asClassOrModuleRef().data(ctx);

    TypeSyntax::ResultType result;
    if (ast::isa_tree<ast::Array>(expr)) {
        const auto &arr = ast::cast_tree_nonnull<ast::Array>(expr);
        vector<core::TypePtr> elems;
        for (auto &el : arr.elems) {
            auto maybeElem = getResultTypeWithSelfTypeParams(ctx, el, sigBeingParsed, args.withoutSelfType());
            if (!maybeElem.has_value()) {
                return nullopt;
            }
            elems.emplace_back(move(maybeElem.value()));
        }
        result.type = core::make_type<core::TupleType>(move(elems));
    } else if (ast::isa_tree<ast::Hash>(expr)) {
        const auto &hash = ast::cast_tree_nonnull<ast::Hash>(expr);
        vector<core::TypePtr> keys;
        vector<core::TypePtr> values;

        for (auto [ktree, vtree] : hash.kviter()) {
            auto maybeVal = getResultTypeWithSelfTypeParams(ctx, vtree, sigBeingParsed, args.withoutSelfType());
            if (!maybeVal.has_value()) {
                return nullopt;
            }
            auto val = move(maybeVal.value());
            auto lit = ast::cast_tree<ast::Literal>(ktree);
            if (lit && lit->isName()) {
                ENFORCE(core::isa_type<core::NamedLiteralType>(lit->value));
                keys.emplace_back(lit->value);
                values.emplace_back(val);
            } else {
                if (auto e = ctx.beginError(ktree.loc(), core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed type declaration. Shape keys must be literals");
                }
            }
        }
        result.type = core::make_type<core::ShapeType>(move(keys), move(values));
    } else if (ast::isa_tree<ast::ConstantLit>(expr)) {
        const auto &i = ast::cast_tree_nonnull<ast::ConstantLit>(expr);
        auto maybeAliased = i.symbol();
        ENFORCE(maybeAliased.exists());

        if (maybeAliased.isTypeAlias(ctx)) {
            result.type = maybeAliased.resultType(ctx);
            return result;
        }

        // Only T::Enum singletons are allowed to be in type syntax because there is only one
        // non-alias constant for an instance of a particular T::Enum--it was created with
        // `new` and immediately assigned into a constant. Any further ConstantLits of this enum's
        // type must be either class aliases (banned in type syntax) or type aliases.
        //
        // This is not the case for arbitrary singletons: MySingleton.instance can be called as many
        // times as wanted, and assigned into different constants each time. As much as possible, we
        // want there to be one name for every type; making an alias for a type should always be
        // syntactically declared with T.type_alias.
        if (core::isa_type<core::ClassType>(maybeAliased.resultType(ctx))) {
            auto resultType = core::cast_type_nonnull<core::ClassType>(maybeAliased.resultType(ctx));
            if (resultType.symbol.data(ctx)->derivesFrom(ctx, core::Symbols::T_Enum())) {
                result.type = maybeAliased.resultType(ctx);
                return result;
            }
        }

        auto sym = maybeAliased.dealias(ctx);
        if (sym.isClassOrModule()) {
            auto klass = sym.asClassOrModuleRef();
            // the T::Type generics internally have a typeArity of 0, so this allows us to check against them in the
            // same way that we check against types like `Array`
            if (klass.isBuiltinGenericForwarder() || klass.data(ctx)->typeArity(ctx) > 0) {
                // Class/Module are not isLegacyStdlibGeneric (because their type members don't default to T.untyped),
                // but we want to report this syntax error at `# typed: strict` like other stdlib classes.
                auto level =
                    klass.isLegacyStdlibGeneric() || klass == core::Symbols::Class() || klass == core::Symbols::Module()
                        ? core::errors::Resolver::GenericClassWithoutTypeArgsStdlib
                        : core::errors::Resolver::GenericClassWithoutTypeArgs;
                if (auto e = ctx.beginError(i.loc(), level)) {
                    e.setHeader("Malformed type declaration. Generic class without type arguments `{}`",
                                klass.show(ctx));
                    core::TypeErrorDiagnostics::insertTypeArguments(ctx, e, klass, ctx.locAt(i.loc()));
                }
            }
            if (klass == core::Symbols::StubModule()) {
                if (maybeAliased != sym) {
                    // There is a bug here, where were don't take the fast path when fixing a
                    // constant resolution error in a class alias.
                    // We can't use our normal trick with fullUnresolvedPath though, because we only
                    // store that on the constant lit that fails to resolve. In this case, the
                    // constant lit itself resolves, but points at something that doesn't resolve,
                    // so there's no resolutionScopes on the constant that we can use to create an
                    // UnresolvedClassType. Just default to untyped.
                    result.type = core::Types::untypedUntracked();
                } else {
                    // Though for normal types _and_ stub types `infer` should use `externalType`,
                    // using `externalType` for stub types here will lead to incorrect handling of global state hashing,
                    // where we won't see difference between two different unresolved stubs(or a mistyped stub). thus,
                    // while normally we would treat stubs as untyped, in `sig`s we treat them as proper types, so that
                    // we can correctly hash them.
                    auto unresolvedPath = i.fullUnresolvedPath(ctx);
                    ENFORCE(unresolvedPath.has_value());
                    result.type =
                        core::make_type<core::UnresolvedClassType>(unresolvedPath->first, move(unresolvedPath->second));
                }
            } else {
                result.type = klass.data(ctx)->externalType();
            }
        } else if (sym.isTypeMember()) {
            auto tm = sym.asTypeMemberRef();
            auto symData = tm.data(ctx);
            auto symOwner = symData->owner.asClassOrModuleRef().data(ctx);

            bool isTypeTemplate = symOwner->isSingletonClass(ctx);

            switch (args.typeMember) {
                case TypeSyntaxArgs::TypeMember::Allowed: {
                    bool ctxIsSingleton = ctxOwnerData->isSingletonClass(ctx);

                    // Check if we're processing a type within the class that
                    // defines this type member by comparing the singleton class of
                    // the context, and the singleton class of the type member's
                    // owner.
                    core::SymbolRef symOwnerSingleton =
                        isTypeTemplate ? symData->owner : symOwner->lookupSingletonClass(ctx);
                    core::SymbolRef ctxSingleton = ctxIsSingleton ? ctx.owner : ctxOwnerData->lookupSingletonClass(ctx);
                    bool usedOnSourceClass = symOwnerSingleton == ctxSingleton;

                    // For this to be a valid use of a member or template type, this
                    // must:
                    //
                    // 1. be used in the context of the class that defines it
                    // 2. if it's a type_template type, be used in a singleton
                    //    method
                    // 3. if it's a type_member type, be used in an instance method
                    if (usedOnSourceClass &&
                        ((isTypeTemplate && ctxIsSingleton) || !(isTypeTemplate || ctxIsSingleton))) {
                        // At this point, we make a skolemized variable that will be unwrapped at the end of type
                        // parsing using Types::unwrapSelfTypeParam. The justification for this is that type
                        // constructors like `Types::any` do not expect to see bound variables, and will panic.
                        result.type = core::make_type<core::SelfTypeParam>(sym);
                    } else {
                        if (auto e = ctx.beginError(i.loc(), core::errors::Resolver::TypeMemberScopeMismatch)) {
                            string typeSource = isTypeTemplate ? "type_template" : "type_member";
                            string typeStr = usedOnSourceClass ? symData->name.show(ctx) : sym.show(ctx);

                            if (usedOnSourceClass) {
                                // Autocorrects here are awkward, because we want to offer the autocorrect at the
                                // definition of the type_member or type_template and we only have access to the use of
                                // the type_member or type_template here.  We could examine the source and attempt to
                                // identify the location for the autocorrect, but that gets messy.
                                //
                                // Plus, it's not absolutely clear that the definition is really at fault: it might be
                                // that the user is using the wrong type_member/type_template constant for the given
                                // context, or they need to change the definition of the method this use is associated
                                // with.  Try to give them enough of a hint to decide what to do on their own.
                                if (ctxIsSingleton) {
                                    e.setHeader("`{}` type `{}` used in a singleton method definition", typeSource,
                                                typeStr);
                                    e.addErrorLine(symData->loc(), "`{}` defined here", typeStr);
                                    e.addErrorNote("Only a `{}` can be used in a singleton method definition.",
                                                   "type_template");
                                } else {
                                    e.setHeader("`{}` type `{}` used in an instance method definition", typeSource,
                                                typeStr);
                                    e.addErrorLine(symData->loc(), "`{}` defined here", typeStr);
                                    e.addErrorNote("Only a `{}` can be used in an instance method definition.",
                                                   "type_member");
                                }
                            } else {
                                e.setHeader("`{}` type `{}` used outside of the class definition", typeSource, typeStr);
                                e.addErrorLine(symData->loc(), "{} defined here", typeStr);
                            }
                        }
                        result.type = core::Types::untypedUntracked();
                    }
                    break;
                }
                case TypeSyntaxArgs::TypeMember::BannedInTypeAlias:
                    if (auto e = ctx.beginError(i.loc(), core::errors::Resolver::TypeAliasToTypeMember)) {
                        const auto &owner = tm.data(ctx)->owner.asClassOrModuleRef().data(ctx);
                        auto memTem = owner->attachedClass(ctx).exists() ? "type_template" : "type_member";
                        e.setHeader("Defining a `{}` to a generic `{}` is not allowed", "type_alias", memTem);
                        e.addErrorLine(tm.data(ctx)->loc(), "`{}` defined as a `{}` here", tm.data(ctx)->name.show(ctx),
                                       memTem);
                        e.addErrorNote("Type aliases to type members and type templates are not allowed: aliases\n"
                                       "    can be referenced from both instance and singleton class methods\n"
                                       "    whereas type members can only be referenced from one or the other.");
                    }
                    result.type = core::Types::untypedUntracked();
                    break;
                case TypeSyntaxArgs::TypeMember::BannedInTypeMember:
                    // a type member has occurred in a context that doesn't allow them
                    if (auto e = ctx.beginError(i.loc(), core::errors::Resolver::InvalidTypeDeclaration)) {
                        auto flavor = isTypeTemplate ? "type_template"sv : "type_member"sv;
                        e.setHeader("`{}` `{}` is not allowed in this context", flavor, sym.show(ctx));
                    }
                    result.type = core::Types::untypedUntracked();
                    break;
            }
        } else if (sym.isStaticField(ctx)) {
            if (auto e = ctx.beginError(i.loc(), core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Constant `{}` is not a class or type alias", maybeAliased.show(ctx));
                e.addErrorLine(sym.loc(ctx), "If you are trying to define a type alias, you should use `{}` here",
                               "T.type_alias");
            }
            result.type = core::Types::untypedUntracked();
        } else {
            if (auto e = ctx.beginError(i.loc(), core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Malformed type declaration. Not a class type `{}`", maybeAliased.show(ctx));
            }
            result.type = core::Types::untypedUntracked();
        }
    } else if (ast::isa_tree<ast::Send>(expr)) {
        const auto &s = ast::cast_tree_nonnull<ast::Send>(expr);
        if (isTProc(ctx, &s)) {
            auto maybeSig = parseSigWithSelfTypeParams(ctx, s, &sigBeingParsed, args.withoutSelfType());
            if (!maybeSig.has_value()) {
                return nullopt;
            }
            auto sig = move(maybeSig.value());
            if (sig.bind.exists()) {
                if (!args.allowRebind) {
                    if (auto e = ctx.beginError(s.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Using `{}` is not permitted here", "bind");
                    }
                } else {
                    result.rebind = sig.bind;
                }
            }

            vector<core::TypePtr> targs;

            if (sig.returns == nullptr) {
                if (auto e = ctx.beginError(s.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed T.proc: You must specify a return type");
                }
                targs.emplace_back(core::Types::untypedUntracked());
            } else {
                targs.emplace_back(sig.returns);
            }

            for (auto &arg : sig.argTypes) {
                targs.emplace_back(arg.type);
            }

            auto arity = targs.size() - 1;
            if (arity > core::Symbols::MAX_PROC_ARITY) {
                if (auto e = ctx.beginError(s.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed T.proc: Too many arguments (max `{}`)", core::Symbols::MAX_PROC_ARITY);
                }
                result.type = core::Types::untypedUntracked();
                return result;
            }
            auto sym = core::Symbols::Proc(arity);

            result.type = core::make_type<core::AppliedType>(sym, move(targs));
            return result;
        }

        core::SymbolRef appliedKlass;
        if (auto recvi = ast::cast_tree<ast::ConstantLit>(s.recv)) {
            if (recvi->symbol() == core::Symbols::T()) {
                return interpretTCombinator(ctx, s, sigBeingParsed, args);
            }

            if (recvi->symbol() == core::Symbols::Magic()) {
                switch (s.fun.rawId()) {
                    case core::Names::callWithSplat().rawId(): {
                        if (auto e = ctx.beginError(s.recv.loc(), core::errors::Resolver::InvalidTypeDeclaration)) {
                            e.setHeader("Malformed type declaration: splats cannot be used in types");
                        }
                        result.type = core::Types::untypedUntracked();
                        return result;
                    }

                    case core::Names::attachedClass().rawId(): {
                        return interpretTCombinator(ctx, s, sigBeingParsed, args);
                    }
                }
            }

            appliedKlass = recvi->symbol();
        } else if (auto recvi = ast::cast_tree<ast::Send>(s.recv)) {
            if (recvi->fun != core::Names::classOf() ||
                (s.fun != core::Names::squareBrackets() && s.fun != core::Names::syntheticSquareBrackets())) {
                return reportUnknownTypeSyntaxError(ctx, s, move(result));
            }

            auto recviRecvi = ast::cast_tree<ast::ConstantLit>(recvi->recv);
            if (recviRecvi == nullptr || recviRecvi->symbol() != core::Symbols::T()) {
                return reportUnknownTypeSyntaxError(ctx, s, move(result));
            }

            auto tClassOfResult = parseTClassOf(ctx, *recvi, sigBeingParsed, args);
            if (!tClassOfResult.has_value()) {
                return nullopt;
            }

            appliedKlass = tClassOfResult.value();
        } else {
            return reportUnknownTypeSyntaxError(ctx, s, move(result));
        }

        if (s.fun != core::Names::squareBrackets() && s.fun != core::Names::syntheticSquareBrackets()) {
            return reportUnknownTypeSyntaxError(ctx, s, move(result));
        }

        InlinedVector<core::TypeAndOrigins, 2> holders;
        InlinedVector<const core::TypeAndOrigins *, 2> targs;
        InlinedVector<core::LocOffsets, 2> argLocs;
        const auto argSize = s.numPosArgs() + (2 * s.numKwArgs()) + (s.hasKwSplat() ? 1 : 0);
        targs.reserve(argSize);
        argLocs.reserve(argSize);
        holders.reserve(argSize);

        for (auto &arg : s.posArgs()) {
            auto maybeType = getResultTypeWithSelfTypeParams(ctx, arg, sigBeingParsed, args.withoutSelfType());
            if (!maybeType.has_value()) {
                return nullopt;
            }
            auto type = core::make_type<core::MetaType>(move(maybeType.value()));
            auto &argtao = holders.emplace_back(std::move(type), ctx.locAt(arg.loc()));
            targs.emplace_back(&argtao);
            argLocs.emplace_back(arg.loc());
        }

        for (auto [key, value] : s.kwArgPairs()) {
            // Fill the keyword and val args in with a dummy type. We don't want to parse this as type
            // syntax because we already know it's garbage.
            // But we still want to record some sort of arg (for the loc specifically) so
            // that the calls.cc intrinsic can craft an autocorrect.
            auto &kwtao = holders.emplace_back(core::Types::untypedUntracked(), ctx.locAt(key.loc()));
            targs.emplace_back(&kwtao);
            argLocs.emplace_back(key.loc());

            auto &valtao = holders.emplace_back(core::Types::untypedUntracked(), ctx.locAt(value.loc()));
            targs.emplace_back(&valtao);
            argLocs.emplace_back(value.loc());
        }

        if (auto *splat = s.kwSplat()) {
            auto &splattao = holders.emplace_back(core::Types::untypedUntracked(), ctx.locAt(splat->loc()));
            targs.emplace_back(&splattao);
            argLocs.emplace_back(splat->loc());
        }

        core::SymbolRef corrected;
        if (appliedKlass.isClassOrModule()) {
            corrected = appliedKlass.asClassOrModuleRef().forwarderForBuiltinGeneric();
        }
        if (corrected.exists()) {
            if (auto e = ctx.beginError(s.loc, core::errors::Resolver::BadStdlibGeneric)) {
                e.setHeader("Use `{}`, not `{}` to declare a typed `{}`", corrected.show(ctx) + "[...]",
                            appliedKlass.show(ctx) + "[...]", appliedKlass.show(ctx));
                e.addErrorNote("`{}` will raise at runtime because this generic was defined in the standard library",
                               appliedKlass.show(ctx) + "[...]");
                e.replaceWith(fmt::format("Change `{}` to `{}`", appliedKlass.show(ctx), corrected.show(ctx)),
                              ctx.locAt(s.recv.loc()), "{}", corrected.show(ctx));
            }
            result.type = core::Types::untypedUntracked();
            return result;
        } else {
            corrected = appliedKlass;
        }
        corrected = corrected.dealias(ctx);

        if (!corrected.isClassOrModule()) {
            if (auto e = ctx.beginError(s.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Expected a class or module");
            }
            result.type = core::Types::untypedUntracked();
            return result;
        }

        auto genericClass = corrected.asClassOrModuleRef();
        ENFORCE_NO_TIMER(genericClass.exists());
        core::CallLocs locs{ctx.file, s.loc, s.recv.loc(), s.funLoc, argLocs};
        auto out = core::Types::applyTypeArguments(ctx, locs, s.numPosArgs(), targs, genericClass,
                                                   core::errors::Resolver::GenericArgumentCountMismatch,
                                                   core::errors::Resolver::GenericArgumentKeywordArgs);

        if (out.isUntyped()) {
            // Using a generic untyped type here will lead to incorrect handling of global state hashing,
            // where we won't see difference between types with generic arguments.
            // Thus, while normally we would treat these as untyped, in `sig`s we treat them as proper types, so
            // that we can correctly hash them.
            vector<core::TypePtr> targPtrs;
            targPtrs.reserve(targs.size());
            for (auto &targ : targs) {
                targPtrs.push_back(targ->type);
            }
            result.type = core::make_type<core::UnresolvedAppliedType>(genericClass, move(targPtrs));
            return result;
        }
        if (auto mt = core::cast_type<core::MetaType>(out)) {
            result.type = mt->wrapped;
            return result;
        }

        if (auto e = ctx.beginError(s.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
            e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
        }
        result.type = core::Types::untypedUntracked();
    } else if (ast::isa_tree<ast::Local>(expr)) {
        const auto &slf = ast::cast_tree_nonnull<ast::Local>(expr);
        if (auto e = ctx.beginError(slf.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
            e.setHeader("Unsupported type syntax");
        }
        result.type = core::Types::untypedUntracked();
    } else if (ast::isa_tree<ast::Self>(expr)) {
        if (auto e = ctx.beginError(expr.loc(), core::errors::Resolver::InvalidTypeDeclaration)) {
            e.setHeader("Unsupported type syntax, did you mean `{}`?", "T.self_type");
            e.replaceWith("Replace with T.self_type", ctx.locAt(expr.loc()), "T.self_type");
        }
        result.type = core::Types::untypedUntracked();
    } else if (ast::isa_tree<ast::Literal>(expr)) {
        const auto &lit = ast::cast_tree_nonnull<ast::Literal>(expr);
        core::TypePtr underlying;
        if (core::isa_type<core::NamedLiteralType>(lit.value)) {
            underlying = lit.value.underlying(ctx);
        } else if (core::isa_type<core::IntegerLiteralType>(lit.value)) {
            underlying = lit.value.underlying(ctx);
        } else if (core::isa_type<core::FloatLiteralType>(lit.value)) {
            underlying = lit.value.underlying(ctx);
        } else {
            underlying = lit.value;
        }
        if (auto e = ctx.beginError(lit.loc, core::errors::Resolver::UnsupportedLiteralType)) {
            e.setHeader("Unsupported literal in type syntax", lit.value.show(ctx));
            e.replaceWith("Replace with underlying type", ctx.locAt(lit.loc), "{}", underlying.show(ctx));
            e.addErrorNote("Sorbet does not allow literal values in types. Consider defining a `{}` instead.",
                           "T::Enum");
        }
        result.type = underlying;
    } else {
        if (auto e = ctx.beginError(expr.loc(), core::errors::Resolver::InvalidTypeDeclaration)) {
            e.setHeader("Unsupported type syntax");
        }
        result.type = core::Types::untypedUntracked();
    }
    return result;
}

optional<TypeSyntax::ResultType> getResultTypeAndBindWithSelfTypeParams(core::Context ctx,
                                                                        const ast::ExpressionPtr &expr,
                                                                        const ParsedSig &sigBeingParsed,
                                                                        TypeSyntaxArgs args) {
    if (auto result = getResultTypeAndBindWithSelfTypeParamsImpl(ctx, expr, sigBeingParsed, args)) {
        ENFORCE(result->type != nullptr);
        DEBUG_ONLY(result->type.sanityCheck(ctx));
        return result;
    } else {
        return nullopt;
    }
}

} // namespace

ParsedSig::TypeParamSpec &ParsedSig::enterTypeParamByName(core::NameRef name) {
    for (auto &current : typeParams) {
        if (current.name == name) {
            return current;
        }
    }
    auto &inserted = typeParams.emplace_back();
    inserted.name = name;
    return inserted;
}

const ParsedSig::TypeParamSpec emptyTypeArgSpec;

const ParsedSig::TypeParamSpec &ParsedSig::findTypeParamByName(core::NameRef name) const {
    for (auto &current : typeParams) {
        if (current.name == name) {
            return current;
        }
    }
    return emptyTypeArgSpec;
}
} // namespace sorbet::resolver
