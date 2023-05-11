#include "resolver/type_syntax/type_syntax.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "common/typecase.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TypeErrorDiagnostics.h"
#include "core/core.h"
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
        /* allowTypeMember */ true,
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

TypeSyntax::ResultType TypeSyntax::getResultTypeAndBind(core::Context ctx, const ast::ExpressionPtr &expr,
                                                        const ParsedSig &sigBeingParsed, TypeSyntaxArgs args) {
    auto result = getResultTypeAndBindWithSelfTypeParams(ctx, expr, sigBeingParsed, args);
    if (result.has_value()) {
        result->type = core::Types::unwrapSelfTypeParam(ctx, result->type);
        return result.value();
    } else {
        return {core::Types::todo(), core::Symbols::noClassOrModule()};
    }
}

namespace {

// Parse a literal type for use with `T.deprecated_enum`. If the type is indeed a literal, this will return the
// `underlying` of that literal. The effect of using `underlying` is that `T.deprecated_enum(["a", "b", true])` will be
// parsed as the type `T.any(String, TrueClass)`.
core::TypePtr getResultLiteral(core::Context ctx, const ast::ExpressionPtr &expr) {
    core::TypePtr result;
    typecase(
        expr,
        [&](const ast::Literal &lit) {
            result = lit.value;
            if (core::isa_type<core::NamedLiteralType>(result)) {
                result = core::cast_type_nonnull<core::NamedLiteralType>(result).underlying(ctx);
            }
        },
        [&](const ast::ExpressionPtr &e) {
            if (auto e = ctx.beginError(expr.loc(), core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Unsupported type literal");
            }
            result = core::Types::untypedUntracked();
        });
    ENFORCE(result != nullptr);
    result.sanityCheck(ctx);
    return result;
}

bool isTProc(core::Context ctx, const ast::Send *send) {
    while (send != nullptr) {
        if (send->fun == core::Names::proc()) {
            if (auto *rcv = ast::cast_tree<ast::ConstantLit>(send->recv)) {
                return rcv->symbol == core::Symbols::T();
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
    if (recv != nullptr && recv->symbol == core::Symbols::Sorbet_Private_Static_ResolvedSig()) {
        // Regardless of how many arguments this method has, we already marked it resolved, so it's good.
        return true;
    }

    auto nargs = send.numPosArgs();
    if (!(nargs == 1 || nargs == 2)) {
        return false;
    }

    if (recv != nullptr && recv->symbol == core::Symbols::Sorbet_Private_Static()) {
        return true;
    }

    return false;
}

namespace {

// When a sig was given with multiple statements, autocorrect it to a single chained send.
void addMultiStatementSigAutocorrect(core::Context ctx, core::ErrorBuilder &e, const ast::ExpressionPtr &blockBody) {
    auto *insseq = ast::cast_tree<ast::InsSeq>(blockBody);
    if (insseq == nullptr) {
        return;
    }

    vector<core::LocOffsets> locs;
    for (auto &expr : insseq->stats) {
        auto *send = ast::cast_tree<ast::Send>(expr);
        if (send == nullptr || !send->loc.exists()) {
            return;
        }

        locs.emplace_back(send->loc);
    }

    {
        auto *send = ast::cast_tree<ast::Send>(insseq->expr);
        if (send == nullptr || !send->loc.exists()) {
            return;
        }

        locs.emplace_back(send->loc);
    }

    auto source = ctx.file.data(ctx).source();

    bool first = true;
    std::string replacement;
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

optional<ParsedSig> parseSigWithSelfTypeParams(core::Context ctx, const ast::Send &sigSend, const ParsedSig *parent,
                                               TypeSyntaxArgs args) {
    ParsedSig sig;
    sig.origSend = const_cast<ast::Send *>(&sigSend);

    const ast::Send *send = nullptr;
    bool isProc = false;
    if (isTProc(ctx, &sigSend)) {
        send = &sigSend;
        isProc = true;
    } else {
        sig.seen.sig = true;
        ENFORCE(sigSend.fun == core::Names::sig());
        auto *block = sigSend.block();
        ENFORCE(block);
        auto *blockBody = ast::cast_tree<ast::Send>(block->body);
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
            sig.seen.final = true;
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
                auto *c = ast::cast_tree<ast::Literal>(arg);
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
                auto &typeArgSpec = sig.enterTypeArgByName(name);
                if (typeArgSpec.type) {
                    if (auto e = ctx.beginError(arg.loc(), core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`: Type argument `{}` was specified twice", "sig", name.show(ctx));
                    }
                }
                typeArgSpec.type = core::make_type<core::TypeVar>(core::Symbols::todoTypeArgument());
                typeArgSpec.loc = ctx.locAt(arg.loc());
            }

            const auto numKwArgs = tsend->numKwArgs();
            for (auto i = 0; i < numKwArgs; ++i) {
                auto &kwkey = tsend->getKwKey(i);
                if (auto e = ctx.beginError(kwkey.loc(), core::errors::Resolver::InvalidMethodSignature)) {
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
            case core::Names::proc().rawId():
                sig.seen.proc = true;
                break;
            case core::Names::bind().rawId(): {
                if (sig.seen.bind) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`: Multiple calls to `.bind`", send->fun.show(ctx));
                    }
                    sig.bind = core::Symbols::noClassOrModule();
                }
                sig.seen.bind = true;

                if (send->numPosArgs() != 1) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Wrong number of args to `{}`. Expected: `{}`, got: `{}`", "bind", 1,
                                    send->numPosArgs());
                    }
                    break;
                }

                bool validBind = false;
                auto maybeBind = getResultTypeWithSelfTypeParams(ctx, send->getPosArg(0), *parent, args);
                core::TypePtr bind;
                if (!maybeBind.has_value()) {
                    validBind = false;
                } else {
                    bind = move(maybeBind.value());
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
                } else if (auto arg = ast::cast_tree<ast::Send>(send->getPosArg(0))) {
                    if (arg->fun == core::Names::selfType()) {
                        sig.bind = core::Symbols::MagicBindToSelfType();
                        validBind = true;
                    } else if (arg->fun == core::Names::attachedClass()) {
                        sig.bind = core::Symbols::MagicBindToAttachedClass();
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
                if (sig.seen.params) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`: Multiple calls to `.params`", send->fun.show(ctx));
                    }
                    sig.argTypes.clear();
                }
                sig.seen.params = true;

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
                            if (auto *hash = ast::cast_tree<ast::Hash>(send->getPosArg(0))) {
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

                auto end = send->numKwArgs();
                for (auto i = 0; i < end; ++i) {
                    auto &key = send->getKwKey(i);
                    auto &value = send->getKwValue(i);
                    auto *lit = ast::cast_tree<ast::Literal>(key);
                    if (lit && lit->isSymbol()) {
                        core::NameRef name = lit->asSymbol();
                        auto maybeResultAndBind = getResultTypeAndBindWithSelfTypeParams(
                            ctx, value, *parent, isProc ? args.withoutRebind() : args.withRebind());
                        if (!maybeResultAndBind.has_value()) {
                            return nullopt;
                        }
                        auto resultAndBind = move(maybeResultAndBind.value());

                        sig.argTypes.emplace_back(ParsedSig::ArgSpec{ctx.locAt(key.loc()), name, ctx.locAt(value.loc()),
                                                                     resultAndBind.rebind, resultAndBind.type});
                    }
                }
                break;
            }
            case core::Names::typeParameters().rawId():
                // was handled above
                break;
            case core::Names::abstract().rawId():
                if (sig.seen.final) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Method that is both `{}` and `{}` cannot be implemented", "final", "abstract");
                    }
                }
                if (sig.seen.override_) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("`{}` cannot be combined with `{}`", "abstract", "override");
                    }
                }
                sig.seen.abstract = true;
                break;
            case core::Names::override_().rawId(): {
                if (sig.seen.abstract) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("`{}` cannot be combined with `{}`", "override", "abstract");
                    }
                }
                sig.seen.override_ = true;

                if (send->hasPosArgs()) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("`{}` expects keyword arguments", send->fun.show(ctx));
                    }
                    break;
                }

                if (send->hasKwArgs()) {
                    auto end = send->numKwArgs();
                    for (auto i = 0; i < end; ++i) {
                        auto &key = send->getKwKey(i);
                        auto &value = send->getKwValue(i);
                        auto lit = ast::cast_tree<ast::Literal>(key);
                        if (lit && lit->isSymbol()) {
                            if (lit->asSymbol() == core::Names::allowIncompatible()) {
                                auto val = ast::cast_tree<ast::Literal>(value);
                                if (val && val->isTrue(ctx)) {
                                    sig.seen.incompatibleOverride = true;
                                }
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
                if (sig.seen.final) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Method that is both `{}` and `{}` cannot be implemented", "final", "overridable");
                    }
                }
                sig.seen.overridable = true;
                break;
            case core::Names::returns().rawId(): {
                sig.seen.returns = true;
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
                sig.returnsLoc = ctx.locAt(send->loc);

                break;
            }
            case core::Names::void_().rawId():
                sig.seen.void_ = true;
                sig.returns = core::Types::void_();
                sig.returnsLoc = ctx.locAt(send->loc);
                break;
            case core::Names::checked().rawId():
                sig.seen.checked = true;
                break;
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
                if (!sig.seen.proc) {
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
    ENFORCE(sig.seen.sig || sig.seen.proc);

    return sig;
}

// This function recurses through an OrType, and accumlates all the class names,
// wrapped in T.class_of, and checks if the type is only made up of Classes and OrTypes
bool recurseOrType(core::Context ctx, core::TypePtr type, std::vector<std::string> &v) {
    if (auto *o = core::cast_type<core::OrType>(type)) {
        return recurseOrType(ctx, o->left, v) && recurseOrType(ctx, o->right, v);
    } else if (core::isa_type<core::ClassType>(type)) {
        v.push_back(fmt::format("T.class_of({})", type.show(ctx)));
        return true;
    } else {
        return false;
    }
}

void unexpectedKwargs(core::Context ctx, const ast::Send &send) {
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
    if (auto *recv = ast::cast_tree<ast::ConstantLit>(send.recv)) {
        maybeScopeClass = recv->symbol;
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

optional<TypeSyntax::ResultType> interpretTCombinator(core::Context ctx, const ast::Send &send, const ParsedSig &sig,
                                                      TypeSyntaxArgs args) {
    switch (send.fun.rawId()) {
        case core::Names::nilable().rawId(): {
            if (send.numPosArgs() != 1 || send.hasKwArgs()) {
                unexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(),
                                              core::Symbols::noClassOrModule()}; // error will be reported in infer.
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
                auto *arg = ast::cast_tree<ast::Send>(send.getPosArg(0));
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
            if (send.numPosArgs() == 0 || send.hasKwArgs()) {
                unexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto maybeResult = getResultTypeWithSelfTypeParams(ctx, send.getPosArg(0), sig, args);
            if (!maybeResult.has_value()) {
                return nullopt;
            }
            auto result = move(maybeResult.value());
            int i = 1;
            while (i < send.numPosArgs()) {
                auto maybeResult = getResultTypeWithSelfTypeParams(ctx, send.getPosArg(i), sig, args);
                if (!maybeResult.has_value()) {
                    return nullopt;
                }
                result = core::Types::all(ctx, result, move(maybeResult.value()));
                i++;
            }
            return TypeSyntax::ResultType{result, core::Symbols::noClassOrModule()};
        }
        case core::Names::any().rawId(): {
            if (send.numPosArgs() == 0 || send.hasKwArgs()) {
                unexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto maybeResult = getResultTypeWithSelfTypeParams(ctx, send.getPosArg(0), sig, args);
            if (!maybeResult.has_value()) {
                return nullopt;
            }
            auto result = move(maybeResult.value());
            int i = 1;
            while (i < send.numPosArgs()) {
                auto maybeResult = getResultTypeWithSelfTypeParams(ctx, send.getPosArg(i), sig, args);
                if (!maybeResult.has_value()) {
                    return nullopt;
                }
                result = core::Types::any(ctx, result, move(maybeResult.value()));
                i++;
            }
            return TypeSyntax::ResultType{result, core::Symbols::noClassOrModule()};
        }
        case core::Names::typeParameter().rawId(): {
            if (send.numPosArgs() != 1 || send.hasKwArgs()) {
                unexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto arr = ast::cast_tree<ast::Literal>(send.getPosArg(0));
            if (!arr || !arr->isSymbol()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("type_parameter requires a symbol");
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto fnd = sig.findTypeArgByName(arr->asSymbol());
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
                unexpectedKwargs(ctx, send);
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
            if (arr->elems.empty()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("enum([]) is invalid");
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto result = getResultLiteral(ctx, arr->elems[0]);
            int i = 1;
            while (i < arr->elems.size()) {
                result = core::Types::any(ctx, result, getResultLiteral(ctx, arr->elems[i]));
                i++;
            }
            return TypeSyntax::ResultType{result, core::Symbols::noClassOrModule()};
        }
        case core::Names::classOf().rawId(): {
            if (send.numPosArgs() != 1 || send.hasKwArgs()) {
                unexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }

            auto *obj = ast::cast_tree<ast::ConstantLit>(send.getPosArg(0));
            if (!obj) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    auto maybeType = getResultTypeWithSelfTypeParams(ctx, send.getPosArg(0), sig, args);
                    if (!maybeType.has_value()) {
                        return nullopt;
                    }
                    auto type = move(maybeType.value());
                    std::vector<std::string> classes;
                    auto shouldAutoCorrect = recurseOrType(ctx, type, classes);
                    if (core::isa_type<core::OrType>(type) && shouldAutoCorrect) {
                        auto autocorrect = fmt::format("T.any({})", fmt::join(classes, ", "));
                        e.setHeader("`{}` must wrap each individual class type, not the outer `{}`", "T.class_of",
                                    "T.any");
                        e.replaceWith("Distribute `T.class_of`", ctx.locAt(send.loc), "{}", autocorrect);
                    } else {
                        e.setHeader("`{}` needs a class or module as its argument", "T.class_of");
                    }
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto maybeAliased = obj->symbol;
            if (maybeAliased.isTypeAlias(ctx)) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("T.class_of can't be used with a T.type_alias");
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            if (maybeAliased.isTypeMember()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("T.class_of can't be used with a T.type_member");
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto sym = maybeAliased.dealias(ctx);
            if (sym.isStaticField(ctx)) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("T.class_of can't be used with a constant field");
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }

            auto singleton = sym.asClassOrModuleRef().data(ctx)->lookupSingletonClass(ctx);
            if (!singleton.exists()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Unknown class");
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            return TypeSyntax::ResultType{singleton.data(ctx)->externalType(), core::Symbols::noClassOrModule()};
        }
        case core::Names::untyped().rawId():
            return TypeSyntax::ResultType{core::Types::untyped(ctx, args.untypedBlame),
                                          core::Symbols::noClassOrModule()};
        case core::Names::selfType().rawId():
            if (args.allowSelfType) {
                return TypeSyntax::ResultType{core::make_type<core::SelfType>(), core::Symbols::noClassOrModule()};
            }
            if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Only top-level T.self_type is supported");
            }
            return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
        case core::Names::experimentalAttachedClass().rawId():
        case core::Names::attachedClass().rawId(): {
            if (send.fun == core::Names::experimentalAttachedClass()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::ExperimentalAttachedClass)) {
                    e.setHeader("`{}` has been stabilized and is no longer experimental",
                                "T.experimental_attached_class");
                    e.replaceWith("Replace with `T.attached_class`", ctx.locAt(send.loc), "T.attached_class");
                }
            }

            ENFORCE(ctx.owner.isClassOrModule());
            auto owner = ctx.owner.asClassOrModuleRef();
            auto ownerData = owner.data(ctx);

            auto maybeAttachedClass = ownerData->findMember(ctx, core::Names::Constants::AttachedClass());
            if (!maybeAttachedClass.exists()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    auto hasAttachedClass = core::Names::declareHasAttachedClass().show(ctx);
                    if (ownerData->isModule()) {
                        e.setHeader("`{}` must declare `{}` before module instance methods can use `{}`",
                                    owner.show(ctx), hasAttachedClass, "T.attached_class");
                        // TODO(jez) Autocorrect to insert `has_attached_class!`
                    } else if (ownerData->isSingletonClass(ctx)) {
                        // Combination of `isSingletonClass` and `<AttachedClass>` missing means
                        // this is the singleton class of a module.
                        ENFORCE(ownerData->attachedClass(ctx).data(ctx)->isModule());
                        e.setHeader("`{}` cannot be used in singleton methods on modules, because modules cannot be "
                                    "instantiated",
                                    "T.attached_class");
                    } else {
                        e.setHeader(
                            "`{}` may only be used in singleton methods on classes or instance methods on `{}` modules",
                            "T.attached_class", hasAttachedClass);
                        e.addErrorNote("Current context is `{}`, which is an instance class not a singleton class",
                                       owner.show(ctx));
                    }
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            } else {
                ENFORCE(
                    // T::Class[...] support
                    owner == core::Symbols::Class() ||
                    // isModule is never true for a singleton class, which implies this is a module instance method
                    ownerData->isModule() ||
                    // In classes, can only use `T.attached_class` on singleton methods
                    (ownerData->isSingletonClass(ctx) && ownerData->attachedClass(ctx).data(ctx)->isClass()));

                const auto attachedClass = maybeAttachedClass.asTypeMemberRef();
                return TypeSyntax::ResultType{core::make_type<core::SelfTypeParam>(attachedClass),
                                              core::Symbols::noClassOrModule()};
            }
        }
        case core::Names::noreturn().rawId():
            return TypeSyntax::ResultType{core::Types::bottom(), core::Symbols::noClassOrModule()};
        case core::Names::anything().rawId():
            return TypeSyntax::ResultType{core::Types::top(), core::Symbols::noClassOrModule()};

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

        for (auto &ktree : hash.keys) {
            auto &vtree = hash.values[&ktree - &hash.keys.front()];
            auto maybeVal = getResultTypeWithSelfTypeParams(ctx, vtree, sigBeingParsed, args.withoutSelfType());
            if (!maybeVal.has_value()) {
                return nullopt;
            }
            auto val = move(maybeVal.value());
            auto lit = ast::cast_tree<ast::Literal>(ktree);
            if (lit && (lit->isSymbol() || lit->isString())) {
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
        auto maybeAliased = i.symbol;
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
            //
            // TODO(jez) After T::Class change: fix the payload, fix all the codebases, and remove this check.
            // (Leaving at least one version in between, so that there is a published version that
            // supports both `Class` and `T::Class` as valid syntax.)
            if (klass != core::Symbols::Class() &&
                (klass.isBuiltinGenericForwarder() || klass.data(ctx)->typeArity(ctx) > 0)) {
                // Class is not isLegacyStdlibGeneric (because its type members don't default to T.untyped),
                // but we want to report this syntax error at `# typed: strict` like other stdlib classes.
                auto level = klass.isLegacyStdlibGeneric() || klass == core::Symbols::Class()
                                 ? core::errors::Resolver::GenericClassWithoutTypeArgsStdlib
                                 : core::errors::Resolver::GenericClassWithoutTypeArgs;
                if (auto e = ctx.beginError(i.loc, level)) {
                    e.setHeader("Malformed type declaration. Generic class without type arguments `{}`",
                                klass.show(ctx));
                    core::TypeErrorDiagnostics::insertUntypedTypeArguments(ctx, e, klass, ctx.locAt(i.loc));
                }
            }
            if (klass == core::Symbols::StubModule()) {
                // Though for normal types _and_ stub types `infer` should use `externalType`,
                // using `externalType` for stub types here will lead to incorrect handling of global state hashing,
                // where we won't see difference between two different unresolved stubs(or a mistyped stub). thus,
                // while normally we would treat stubs as untyped, in `sig`s we treat them as proper types, so that
                // we can correctly hash them.
                auto unresolvedPath = i.fullUnresolvedPath(ctx);
                ENFORCE(unresolvedPath.has_value());
                result.type =
                    core::make_type<core::UnresolvedClassType>(unresolvedPath->first, move(unresolvedPath->second));
            } else {
                result.type = klass.data(ctx)->externalType();
            }
        } else if (sym.isTypeMember()) {
            auto tm = sym.asTypeMemberRef();
            auto symData = tm.data(ctx);
            auto symOwner = symData->owner.asClassOrModuleRef().data(ctx);

            bool isTypeTemplate = symOwner->isSingletonClass(ctx);

            if (args.allowTypeMember) {
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
                if (usedOnSourceClass && ((isTypeTemplate && ctxIsSingleton) || !(isTypeTemplate || ctxIsSingleton))) {
                    // At this point, we maake a skolemized variable that will be unwrapped at the end of type
                    // parsing using Types::unwrapSkolemVariables. The justification for this is that type
                    // constructors like `Types::any` do not expect to see bound variables, and will panic.
                    result.type = core::make_type<core::SelfTypeParam>(sym);
                } else {
                    if (auto e = ctx.beginError(i.loc, core::errors::Resolver::TypeMemberScopeMismatch)) {
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
            } else {
                // a type member has occurred in a context that doesn't allow them
                if (auto e = ctx.beginError(i.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    auto flavor = isTypeTemplate ? "type_template"sv : "type_member"sv;
                    e.setHeader("`{}` `{}` is not allowed in this context", flavor, sym.show(ctx));
                }
                result.type = core::Types::untypedUntracked();
            }
        } else if (sym.isStaticField(ctx)) {
            if (auto e = ctx.beginError(i.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Constant `{}` is not a class or type alias", maybeAliased.show(ctx));
                e.addErrorLine(sym.loc(ctx), "If you are trying to define a type alias, you should use `{}` here",
                               "T.type_alias");
            }
            result.type = core::Types::untypedUntracked();
        } else {
            if (auto e = ctx.beginError(i.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Malformed type declaration. Not a class type `{}`", maybeAliased.show(ctx));
            }
            result.type = core::Types::untypedUntracked();
        }
    } else if (ast::isa_tree<ast::Send>(expr)) {
        const auto &s = ast::cast_tree_nonnull<ast::Send>(expr);
        if (isTProc(ctx, &s)) {
            auto maybeSig = parseSigWithSelfTypeParams(ctx, s, &sigBeingParsed, args);
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

        auto *recvi = ast::cast_tree<ast::ConstantLit>(s.recv);
        if (recvi == nullptr) {
            if (auto e = ctx.beginError(s.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                auto klass = sendLooksLikeBadTypeApplication(ctx, s);
                if (klass.exists()) {
                    auto scope = s.recv.isSelfReference()
                                     ? ""
                                     : fmt::format("{}::", ctx.locAt(s.recv.loc()).source(ctx).value());
                    auto replacement =
                        fmt::format("{}{}[{}]", scope, s.fun.show(ctx), ctx.locAt(s.argsLoc()).source(ctx).value());
                    e.setHeader("Did you mean to use square brackets: `{}`", replacement);
                    e.replaceWith("Use square brackets for type args", ctx.locAt(s.loc), "{}", replacement);
                } else {
                    e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
                }
            }
            result.type = core::Types::untypedUntracked();
            return result;
        }
        if (recvi->symbol == core::Symbols::T()) {
            if (auto res = interpretTCombinator(ctx, s, sigBeingParsed, args)) {
                return move(res.value());
            } else {
                return nullopt;
            }
        }

        if (recvi->symbol == core::Symbols::Magic() && s.fun == core::Names::callWithSplat()) {
            if (auto e = ctx.beginError(recvi->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Malformed type declaration: splats cannot be used in types");
            }
            result.type = core::Types::untypedUntracked();
            return result;
        }

        if (s.fun != core::Names::squareBrackets()) {
            if (auto e = ctx.beginError(s.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                auto klass = sendLooksLikeBadTypeApplication(ctx, s);
                if (klass.exists()) {
                    auto scope = s.recv.isSelfReference()
                                     ? ""
                                     : fmt::format("{}::", ctx.locAt(s.recv.loc()).source(ctx).value());
                    auto replacement =
                        fmt::format("{}{}[{}]", scope, s.fun.show(ctx), ctx.locAt(s.argsLoc()).source(ctx).value());
                    e.setHeader("Did you mean to use square brackets: `{}`", replacement);
                    e.replaceWith("Use square brackets for type args", ctx.locAt(s.loc), "{}", replacement);
                } else {
                    e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
                }
            }
            result.type = core::Types::untypedUntracked();
            return result;
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

        const auto numKwArgs = s.numKwArgs();
        for (auto i = 0; i < numKwArgs; ++i) {
            auto &kw = s.getKwKey(i);
            auto &val = s.getKwValue(i);

            // Fill the keyword and val args in with a dummy type. We don't want to parse this as type
            // syntax because we already know it's garbage.
            // But we still want to record some sort of arg (for the loc specifically) so
            // that the calls.cc intrinsic can craft an autocorrect.
            auto &kwtao = holders.emplace_back(core::Types::untypedUntracked(), ctx.locAt(kw.loc()));
            targs.emplace_back(&kwtao);
            argLocs.emplace_back(kw.loc());

            auto &valtao = holders.emplace_back(core::Types::untypedUntracked(), ctx.locAt(val.loc()));
            targs.emplace_back(&valtao);
            argLocs.emplace_back(val.loc());
        }

        if (auto *splat = s.kwSplat()) {
            auto &splattao = holders.emplace_back(core::Types::untypedUntracked(), ctx.locAt(splat->loc()));
            targs.emplace_back(&splattao);
            argLocs.emplace_back(splat->loc());
        }

        core::SymbolRef corrected;
        if (recvi->symbol.isClassOrModule()) {
            corrected = recvi->symbol.asClassOrModuleRef().forwarderForBuiltinGeneric();
        }
        if (corrected.exists()) {
            if (auto e = ctx.beginError(s.loc, core::errors::Resolver::BadStdlibGeneric)) {
                e.setHeader("Use `{}`, not `{}` to declare a typed `{}`", corrected.show(ctx) + "[...]",
                            recvi->symbol.show(ctx) + "[...]", recvi->symbol.show(ctx));
                e.addErrorNote("`{}` will raise at runtime because this generic was defined in the standard library",
                               recvi->symbol.show(ctx) + "[...]");
                e.replaceWith(fmt::format("Change `{}` to `{}`", recvi->symbol.show(ctx), corrected.show(ctx)),
                              ctx.locAt(recvi->loc), "{}", corrected.show(ctx));
            }
            result.type = core::Types::untypedUntracked();
            return result;
        } else {
            corrected = recvi->symbol;
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
        core::CallLocs locs{ctx.file, s.loc, recvi->loc, s.funLoc, argLocs};
        auto out = core::Types::applyTypeArguments(ctx, locs, s.numPosArgs(), targs, genericClass);

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
        if (auto *mt = core::cast_type<core::MetaType>(out)) {
            result.type = mt->wrapped;
            return result;
        }

        if (auto e = ctx.beginError(s.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
            e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
        }
        result.type = core::Types::untypedUntracked();
    } else if (ast::isa_tree<ast::Local>(expr)) {
        const auto &slf = ast::cast_tree_nonnull<ast::Local>(expr);
        if (expr.isSelfReference()) {
            result.type = ctxOwnerData->selfType(ctx);
        } else {
            if (auto e = ctx.beginError(slf.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Unsupported type syntax");
            }
            result.type = core::Types::untypedUntracked();
        }
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
        if (auto e = ctx.beginError(lit.loc, core::errors::Resolver::InvalidMethodSignature)) {
            e.setHeader("Unsupported literal in type syntax", lit.value.show(ctx));
            e.replaceWith("Replace with underlying type", ctx.locAt(lit.loc), "{}", underlying.show(ctx));
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

ParsedSig::TypeArgSpec &ParsedSig::enterTypeArgByName(core::NameRef name) {
    for (auto &current : typeArgs) {
        if (current.name == name) {
            return current;
        }
    }
    auto &inserted = typeArgs.emplace_back();
    inserted.name = name;
    return inserted;
}

const ParsedSig::TypeArgSpec emptyTypeArgSpec;

const ParsedSig::TypeArgSpec &ParsedSig::findTypeArgByName(core::NameRef name) const {
    for (auto &current : typeArgs) {
        if (current.name == name) {
            return current;
        }
    }
    return emptyTypeArgSpec;
}
} // namespace sorbet::resolver
