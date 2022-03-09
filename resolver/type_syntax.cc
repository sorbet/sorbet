#include "resolver/type_syntax.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "common/typecase.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/core.h"
#include "core/errors/resolver.h"

using namespace std;

namespace sorbet::resolver {

// Forward declarations for the local versions of getResultType, getResultTypeAndBind, and parseSig that skolemize type
// members.
namespace {
core::TypePtr getResultTypeWithSelfTypeParams(core::Context ctx, const ast::ExpressionPtr &expr,
                                              const ParsedSig &sigBeingParsed, TypeSyntaxArgs args);

TypeSyntax::ResultType getResultTypeAndBindWithSelfTypeParams(core::Context ctx, const ast::ExpressionPtr &expr,
                                                              const ParsedSig &sigBeingParsed, TypeSyntaxArgs args);

ParsedSig parseSigWithSelfTypeParams(core::Context ctx, const ast::Send &sigSend, const ParsedSig *parent,
                                     TypeSyntaxArgs args);
} // namespace

ParsedSig TypeSyntax::parseSig(core::Context ctx, const ast::Send &sigSend, const ParsedSig *parent,
                               TypeSyntaxArgs args) {
    auto result = parseSigWithSelfTypeParams(ctx, sigSend, parent, args);

    for (auto &arg : result.argTypes) {
        arg.type = core::Types::unwrapSelfTypeParam(ctx, arg.type);
    }

    if (result.returns != nullptr) {
        result.returns = core::Types::unwrapSelfTypeParam(ctx, result.returns);
    }

    return result;
}

core::TypePtr TypeSyntax::getResultType(core::Context ctx, ast::ExpressionPtr &expr, const ParsedSig &sigBeingParsed,
                                        TypeSyntaxArgs args) {
    return core::Types::unwrapSelfTypeParam(
        ctx, getResultTypeWithSelfTypeParams(ctx, expr, sigBeingParsed, args.withoutRebind()));
}

TypeSyntax::ResultType TypeSyntax::getResultTypeAndBind(core::Context ctx, ast::ExpressionPtr &expr,
                                                        const ParsedSig &sigBeingParsed, TypeSyntaxArgs args) {
    auto result = getResultTypeAndBindWithSelfTypeParams(ctx, expr, sigBeingParsed, args);
    result.type = core::Types::unwrapSelfTypeParam(ctx, result.type);
    return result;
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
            if (core::isa_type<core::LiteralType>(result)) {
                result = core::cast_type_nonnull<core::LiteralType>(result).underlying(ctx);
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
    // NB: this only needs to check for Sorbet::Private::Static.sig and not
    // Sorbet::Private::Static::ResolvedSig.sig, because this function is only
    // used during resolver to identify potential sigs.  We don't create
    // Sorbet::Private::Static::ResolvedSig.sig until after resolver is run.
    auto nargs = send.numPosArgs();
    if (!(nargs == 1 || nargs == 2)) {
        return false;
    }

    auto recv = ast::cast_tree<ast::ConstantLit>(send.recv);
    if (recv && recv->symbol == core::Symbols::Sorbet_Private_Static()) {
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

    e.replaceWith("Use a chained sig builder", core::Loc{ctx.file, insseq->loc}, "{}", replacement);
}

ParsedSig parseSigWithSelfTypeParams(core::Context ctx, const ast::Send &sigSend, const ParsedSig *parent,
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
        if (lit != nullptr && lit->isSymbol(ctx) && lit->asSymbol(ctx) == core::Names::final_()) {
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
                if (auto c = ast::cast_tree<ast::Literal>(arg)) {
                    if (c->isSymbol(ctx)) {
                        auto name = c->asSymbol(ctx);
                        auto &typeArgSpec = sig.enterTypeArgByName(name);
                        if (typeArgSpec.type) {
                            if (auto e = ctx.beginError(arg.loc(), core::errors::Resolver::InvalidMethodSignature)) {
                                e.setHeader("Malformed `{}`: Type argument `{}` was specified twice", "sig",
                                            name.show(ctx));
                            }
                        }
                        typeArgSpec.type = core::make_type<core::TypeVar>(core::Symbols::todoTypeArgument());
                        typeArgSpec.loc = core::Loc(ctx.file, arg.loc());
                    } else {
                        if (auto e = ctx.beginError(arg.loc(), core::errors::Resolver::InvalidMethodSignature)) {
                            e.setHeader("Malformed `{}`: Type parameters are specified with symbols", "sig");
                        }
                    }
                } else {
                    if (auto e = ctx.beginError(arg.loc(), core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Unexpected `{}`: Type parameters are specified with symbols", arg.nodeName());
                    }
                }
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
                auto bind = getResultTypeWithSelfTypeParams(ctx, send->getPosArg(0), *parent, args);
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
                    if (lit && lit->isSymbol(ctx)) {
                        core::NameRef name = lit->asSymbol(ctx);
                        TypeSyntax::ResultType resultAndBind;

                        if (isProc) {
                            resultAndBind =
                                getResultTypeAndBindWithSelfTypeParams(ctx, value, *parent, args.withoutRebind());
                        } else {
                            resultAndBind =
                                getResultTypeAndBindWithSelfTypeParams(ctx, value, *parent, args.withRebind());
                        }

                        sig.argTypes.emplace_back(ParsedSig::ArgSpec{core::Loc(ctx.file, key.loc()), name,
                                                                     resultAndBind.type, resultAndBind.rebind});
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
                        if (lit && lit->isSymbol(ctx)) {
                            if (lit->asSymbol(ctx) == core::Names::allowIncompatible()) {
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
                    auto loc = core::Loc(ctx.file, send->funLoc);
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

                sig.returns = getResultTypeWithSelfTypeParams(ctx, send->getPosArg(0), *parent, args);

                break;
            }
            case core::Names::void_().rawId():
                sig.seen.void_ = true;
                sig.returns = core::Types::void_();
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
                    e.addErrorLine(core::Loc(ctx.file, send->loc),
                                   "Consult https://sorbet.org/docs/sigs for signature syntax");
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

TypeSyntax::ResultType interpretTCombinator(core::Context ctx, const ast::Send &send, const ParsedSig &sig,
                                            TypeSyntaxArgs args) {
    switch (send.fun.rawId()) {
        case core::Names::nilable().rawId(): {
            if (send.numPosArgs() != 1 || send.hasKwArgs()) {
                unexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(),
                                              core::Symbols::noClassOrModule()}; // error will be reported in infer.
            }

            auto result = getResultTypeAndBindWithSelfTypeParams(ctx, send.getPosArg(0), sig, args);
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
                        e.replaceWith("Replace with `T.untyped`", core::Loc{ctx.file, send.loc}, "T.untyped");
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
            auto result = getResultTypeWithSelfTypeParams(ctx, send.getPosArg(0), sig, args);
            int i = 1;
            while (i < send.numPosArgs()) {
                result =
                    core::Types::all(ctx, result, getResultTypeWithSelfTypeParams(ctx, send.getPosArg(i), sig, args));
                i++;
            }
            return TypeSyntax::ResultType{result, core::Symbols::noClassOrModule()};
        }
        case core::Names::any().rawId(): {
            if (send.numPosArgs() == 0 || send.hasKwArgs()) {
                unexpectedKwargs(ctx, send);
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto result = getResultTypeWithSelfTypeParams(ctx, send.getPosArg(0), sig, args);
            int i = 1;
            while (i < send.numPosArgs()) {
                result =
                    core::Types::any(ctx, result, getResultTypeWithSelfTypeParams(ctx, send.getPosArg(i), sig, args));
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
            if (!arr || !arr->isSymbol(ctx)) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("type_parameter requires a symbol");
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            }
            auto fnd = sig.findTypeArgByName(arr->asSymbol(ctx));
            if (!fnd.type) {
                if (auto e = ctx.beginError(arr->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Unspecified type parameter");
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
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

                    auto prefix = 0;
                    auto sendSource = core::Loc(ctx.file, send.loc).source(ctx);
                    if (sendSource.has_value() && absl::StartsWith(sendSource.value(), "::")) {
                        prefix += 2;
                    }

                    if (send.funLoc.exists() && !send.funLoc.empty()) {
                        e.replaceWith("Replace with `deprecated_enum`", core::Loc(ctx.file, send.funLoc), "{}",
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
                    auto type = getResultTypeWithSelfTypeParams(ctx, send.getPosArg(0), sig, args);
                    std::vector<std::string> classes;
                    auto shouldAutoCorrect = recurseOrType(ctx, type, classes);
                    if (core::isa_type<core::OrType>(type) && shouldAutoCorrect) {
                        auto autocorrect = fmt::format("T.any({})", fmt::join(classes, ", "));
                        e.setHeader("`{}` must wrap each individual class type, not the outer `{}`", "T.class_of",
                                    "T.any");
                        e.replaceWith("Distribute `T.class_of`", core::Loc(ctx.file, send.loc), "{}", autocorrect);
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
        case core::Names::attachedClass().rawId():
            if (send.fun == core::Names::experimentalAttachedClass()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::ExperimentalAttachedClass)) {
                    e.setHeader("`{}` has been stabilized and is no longer experimental",
                                "T.experimental_attached_class");
                    e.replaceWith("Replace with `T.attached_class`", core::Loc(ctx.file, send.loc), "T.attached_class");
                }
            }

            if (!ctx.owner.isClassOrModule() || !ctx.owner.asClassOrModuleRef().data(ctx)->isSingletonClass(ctx)) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("`{}` may only be used in a singleton class method context",
                                "T." + core::Names::attachedClass().show(ctx));
                }
                return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
            } else {
                // All singletons have an AttachedClass type member, created by
                // `singletonClass`
                auto attachedClass = ctx.owner.asClassOrModuleRef()
                                         .data(ctx)
                                         ->findMember(ctx, core::Names::Constants::AttachedClass())
                                         .asTypeMemberRef();
                return TypeSyntax::ResultType{attachedClass.data(ctx)->resultType, core::Symbols::noClassOrModule()};
            }
        case core::Names::noreturn().rawId():
            return TypeSyntax::ResultType{core::Types::bottom(), core::Symbols::noClassOrModule()};

        default:
            if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Unsupported method `{}`", "T." + send.fun.show(ctx));
            }
            return TypeSyntax::ResultType{core::Types::untypedUntracked(), core::Symbols::noClassOrModule()};
    }
}

core::TypePtr getResultTypeWithSelfTypeParams(core::Context ctx, const ast::ExpressionPtr &expr,
                                              const ParsedSig &sigBeingParsed, TypeSyntaxArgs args) {
    return getResultTypeAndBindWithSelfTypeParams(ctx, expr, sigBeingParsed, args.withoutRebind()).type;
}

unique_ptr<core::TypeAndOrigins> makeTypeAndOrigins(core::Context ctx, core::LocOffsets origin, core::TypePtr type) {
    auto ty = make_unique<core::TypeAndOrigins>();
    ty->origins.emplace_back(core::Loc(ctx.file, origin));
    ty->type = move(type);
    return ty;
}

TypeSyntax::ResultType getResultTypeAndBindWithSelfTypeParams(core::Context ctx, const ast::ExpressionPtr &expr,
                                                              const ParsedSig &sigBeingParsed, TypeSyntaxArgs args) {
    // Ensure that we only check types from a class context
    ENFORCE(ctx.owner.isClassOrModule(), "getResultTypeAndBind wasn't called with a class owner");
    auto ctxOwnerData = ctx.owner.asClassOrModuleRef().data(ctx);

    TypeSyntax::ResultType result;
    typecase(
        expr,
        [&](const ast::Array &arr) {
            vector<core::TypePtr> elems;
            for (auto &el : arr.elems) {
                elems.emplace_back(getResultTypeWithSelfTypeParams(ctx, el, sigBeingParsed, args.withoutSelfType()));
            }
            result.type = core::make_type<core::TupleType>(move(elems));
        },
        [&](const ast::Hash &hash) {
            vector<core::TypePtr> keys;
            vector<core::TypePtr> values;

            for (auto &ktree : hash.keys) {
                auto &vtree = hash.values[&ktree - &hash.keys.front()];
                auto val = getResultTypeWithSelfTypeParams(ctx, vtree, sigBeingParsed, args.withoutSelfType());
                auto lit = ast::cast_tree<ast::Literal>(ktree);
                if (lit && (lit->isSymbol(ctx) || lit->isString(ctx))) {
                    ENFORCE(core::isa_type<core::LiteralType>(lit->value));
                    keys.emplace_back(lit->value);
                    values.emplace_back(val);
                } else {
                    if (auto e = ctx.beginError(ktree.loc(), core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Malformed type declaration. Shape keys must be literals");
                    }
                }
            }
            result.type = core::make_type<core::ShapeType>(move(keys), move(values));
        },
        [&](const ast::ConstantLit &i) {
            auto maybeAliased = i.symbol;
            ENFORCE(maybeAliased.exists());

            if (maybeAliased.isTypeAlias(ctx)) {
                result.type = maybeAliased.resultType(ctx);
                return;
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
                    return;
                }
            }

            auto sym = maybeAliased.dealias(ctx);
            if (sym.isClassOrModule()) {
                auto klass = sym.asClassOrModuleRef();
                // the T::Type generics internally have a typeArity of 0, so this allows us to check against them in the
                // same way that we check against types like `Array`
                bool isBuiltinGeneric = klass == core::Symbols::T_Hash() || klass == core::Symbols::T_Array() ||
                                        klass == core::Symbols::T_Set() || klass == core::Symbols::T_Range() ||
                                        klass == core::Symbols::T_Enumerable() ||
                                        klass == core::Symbols::T_Enumerator();

                if (isBuiltinGeneric || klass.data(ctx)->typeArity(ctx) > 0) {
                    // This set **should not** grow over time.
                    bool isStdlibWhitelisted = klass == core::Symbols::Hash() || klass == core::Symbols::Array() ||
                                               klass == core::Symbols::Set() || klass == core::Symbols::Range() ||
                                               klass == core::Symbols::Enumerable() ||
                                               klass == core::Symbols::Enumerator();
                    auto level = isStdlibWhitelisted ? core::errors::Resolver::GenericClassWithoutTypeArgsStdlib
                                                     : core::errors::Resolver::GenericClassWithoutTypeArgs;
                    if (auto e = ctx.beginError(i.loc, level)) {
                        e.setHeader("Malformed type declaration. Generic class without type arguments `{}`",
                                    klass.show(ctx));
                        // if we're looking at `Array`, we want the autocorrect to include `T::`, but we don't need to
                        // if we're already looking at `T::Array` instead.
                        auto typePrefix = isBuiltinGeneric ? "" : "T::";

                        auto loc = core::Loc{ctx.file, i.loc};
                        if (auto locSource = loc.source(ctx)) {
                            if (klass == core::Symbols::Hash() || klass == core::Symbols::T_Hash()) {
                                // Hash is special because it has arity 3 but you're only supposed to write the first 2
                                e.replaceWith("Add type arguments", loc, "{}{}[T.untyped, T.untyped]", typePrefix,
                                              locSource.value());
                            } else if (isStdlibWhitelisted || isBuiltinGeneric) {
                                // the default provided here for builtin generic types is 1, and that might need to
                                // change if we add other builtin generics (but ideally we should never need to do so!)
                                auto numTypeArgs = isBuiltinGeneric ? 1 : klass.data(ctx)->typeArity(ctx);
                                vector<string> untypeds;
                                for (int i = 0; i < numTypeArgs; i++) {
                                    untypeds.emplace_back("T.untyped");
                                }
                                e.replaceWith("Add type arguments", loc, "{}{}[{}]", typePrefix, locSource.value(),
                                              absl::StrJoin(untypeds, ", "));
                            }
                        }
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
                    if (usedOnSourceClass &&
                        ((isTypeTemplate && ctxIsSingleton) || !(isTypeTemplate || ctxIsSingleton))) {
                        // At this point, we maake a skolemized variable that will be unwrapped at the end of type
                        // parsing using Types::unwrapSkolemVariables. The justification for this is that type
                        // constructors like `Types::any` do not expect to see bound variables, and will panic.
                        result.type = core::make_type<core::SelfTypeParam>(sym);
                    } else {
                        if (auto e = ctx.beginError(i.loc, core::errors::Resolver::InvalidTypeDeclarationTyped)) {
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
        },
        [&](const ast::Send &s) {
            if (isTProc(ctx, &s)) {
                auto sig = parseSigWithSelfTypeParams(ctx, s, &sigBeingParsed, args);
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
                    return;
                }
                auto sym = core::Symbols::Proc(arity);

                result.type = core::make_type<core::AppliedType>(sym, move(targs));
                return;
            }

            auto *recvi = ast::cast_tree<ast::ConstantLit>(s.recv);
            if (recvi == nullptr) {
                if (auto e = ctx.beginError(s.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
                }
                result.type = core::Types::untypedUntracked();
                return;
            }
            if (recvi->symbol == core::Symbols::T()) {
                result = interpretTCombinator(ctx, s, sigBeingParsed, args);
                return;
            }

            if (recvi->symbol == core::Symbols::Magic() && s.fun == core::Names::callWithSplat()) {
                // TODO(pay-server) remove this block
                if (auto e = ctx.beginError(recvi->loc, core::errors::Resolver::InvalidTypeDeclarationTyped)) {
                    e.setHeader("Malformed type declaration: splats cannot be used in types");
                }
                result.type = core::Types::untypedUntracked();
                return;
            }

            if (s.fun != core::Names::squareBrackets()) {
                if (auto e = ctx.beginError(s.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
                }
                result.type = core::Types::untypedUntracked();
                return;
            }

            InlinedVector<unique_ptr<core::TypeAndOrigins>, 2> holders;
            InlinedVector<const core::TypeAndOrigins *, 2> targs;
            InlinedVector<core::LocOffsets, 2> argLocs;
            const auto argSize = s.numPosArgs() + (2 * s.numKwArgs()) + (s.hasKwSplat() ? 1 : 0);
            targs.reserve(argSize);
            argLocs.reserve(argSize);
            holders.reserve(argSize);

            for (auto &arg : s.posArgs()) {
                auto ty = makeTypeAndOrigins(ctx, arg.loc(),
                                             core::make_type<core::MetaType>(getResultTypeWithSelfTypeParams(
                                                 ctx, arg, sigBeingParsed, args.withoutSelfType())));
                holders.emplace_back(move(ty));
                targs.emplace_back(holders.back().get());
                argLocs.emplace_back(arg.loc());
            }

            const auto numKwArgs = s.numKwArgs();
            for (auto i = 0; i < numKwArgs; ++i) {
                auto &kw = s.getKwKey(i);
                auto &val = s.getKwValue(i);

                // Fill these in with a dummy type. We don't want to parse this as type syntax
                // because we already know it's garbage.
                // But we still want to record some sort of arg (for the loc specifically) so
                // that the calls.cc intrinsic can craft an autocorrect.
                auto kwty = makeTypeAndOrigins(ctx, kw.loc(), core::Types::untypedUntracked());
                auto valty = makeTypeAndOrigins(ctx, val.loc(), core::Types::untypedUntracked());

                holders.emplace_back(move(kwty));
                targs.emplace_back(holders.back().get());
                argLocs.emplace_back(kw.loc());

                holders.emplace_back(move(valty));
                targs.emplace_back(holders.back().get());
                argLocs.emplace_back(val.loc());
            }

            if (auto *splat = s.kwSplat()) {
                auto ty = makeTypeAndOrigins(ctx, splat->loc(), core::Types::untypedUntracked());
                holders.emplace_back(move(ty));
                targs.emplace_back(holders.back().get());
                argLocs.emplace_back(splat->loc());
            }

            core::SymbolRef corrected;
            if (recvi->symbol == core::Symbols::Array()) {
                corrected = core::Symbols::T_Array();
            } else if (recvi->symbol == core::Symbols::Hash()) {
                corrected = core::Symbols::T_Hash();
            } else if (recvi->symbol == core::Symbols::Enumerable()) {
                corrected = core::Symbols::T_Enumerable();
            } else if (recvi->symbol == core::Symbols::Enumerator()) {
                corrected = core::Symbols::T_Enumerator();
            } else if (recvi->symbol == core::Symbols::Range()) {
                corrected = core::Symbols::T_Range();
            } else if (recvi->symbol == core::Symbols::Set()) {
                corrected = core::Symbols::T_Set();
            }
            if (corrected.exists()) {
                if (auto e = ctx.beginError(s.loc, core::errors::Resolver::BadStdlibGeneric)) {
                    e.setHeader("Use `{}`, not `{}` to declare a typed `{}`", corrected.show(ctx) + "[...]",
                                recvi->symbol.show(ctx) + "[...]", recvi->symbol.show(ctx));
                    e.addErrorNote(
                        "`{}` will raise at runtime because this generic was defined in the standard library",
                        recvi->symbol.show(ctx) + "[...]");
                    e.replaceWith(fmt::format("Change `{}` to `{}`", recvi->symbol.show(ctx), corrected.show(ctx)),
                                  core::Loc(ctx.file, recvi->loc), "{}", corrected.show(ctx));
                }
                result.type = core::Types::untypedUntracked();
                return;
            } else {
                corrected = recvi->symbol;
            }
            corrected = corrected.dealias(ctx);

            if (!corrected.isClassOrModule()) {
                if (auto e = ctx.beginError(s.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Expected a class or module");
                }
                result.type = core::Types::untypedUntracked();
                return;
            }

            auto correctedSingleton = corrected.asClassOrModuleRef().data(ctx)->lookupSingletonClass(ctx);
            ENFORCE_NO_TIMER(correctedSingleton.exists());
            auto ctype = core::make_type<core::ClassType>(correctedSingleton);
            auto ctypeAndOrigins = core::TypeAndOrigins{ctype, {core::Loc(ctx.file, s.loc)}};
            // In `dispatchArgs` this is ordinarily used to specify the origin tag for
            // uninitialized variables. Inside of a signature we shouldn't need this:
            auto originForUninitialized = core::Loc::none();
            core::CallLocs locs{
                ctx.file, s.loc, recvi->loc, s.loc.copyWithZeroLength(), argLocs,
            };
            auto suppressErrors = false;
            core::DispatchArgs dispatchArgs{core::Names::squareBrackets(),
                                            locs,
                                            s.numPosArgs(),
                                            targs,
                                            ctype,
                                            ctypeAndOrigins,
                                            ctype,
                                            nullptr,
                                            originForUninitialized,
                                            s.flags.isPrivateOk,
                                            suppressErrors};
            auto out = core::Types::dispatchCallWithoutBlock(ctx, ctype, dispatchArgs);

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
                result.type = core::make_type<core::UnresolvedAppliedType>(correctedSingleton, move(targPtrs));
                return;
            }
            if (auto *mt = core::cast_type<core::MetaType>(out)) {
                result.type = mt->wrapped;
                return;
            }

            if (auto e = ctx.beginError(s.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
            }
            result.type = core::Types::untypedUntracked();
        },
        [&](const ast::Local &slf) {
            if (expr.isSelfReference()) {
                result.type = ctxOwnerData->selfType(ctx);
            } else {
                if (auto e = ctx.beginError(slf.loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Unsupported type syntax");
                }
                result.type = core::Types::untypedUntracked();
            }
        },
        [&](const ast::Literal &lit) {
            auto underlying = core::isa_type<core::LiteralType>(lit.value) ? lit.value.underlying(ctx) : lit.value;
            if (auto e = ctx.beginError(lit.loc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Unsupported literal in type syntax", lit.value.show(ctx));
                e.replaceWith("Replace with underlying type", core::Loc(ctx.file, lit.loc), "{}", underlying.show(ctx));
            }
            result.type = underlying;
        },
        [&](const ast::ExpressionPtr &e) {
            if (auto e = ctx.beginError(expr.loc(), core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Unsupported type syntax");
            }
            result.type = core::Types::untypedUntracked();
        });
    ENFORCE(result.type != nullptr);
    result.type.sanityCheck(ctx);
    return result;
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
