#include "name_locals.h"
#include "ast/treemap/treemap.h"
#include "common/typecase.h"
#include "core/core.h"
#include "core/errors/namer.h"

using namespace std;

namespace sorbet::name_locals {

class LocalNameInserter {
    friend class NameLocals;

    struct ParsedArg {
        core::NameRef name;
        core::Loc loc;
        unique_ptr<ast::Expression> default_;
        bool keyword = false;
        bool block = false;
        bool repeated = false;
        bool shadow = false;
    };

    ParsedArg parseArg(core::MutableContext ctx, unique_ptr<ast::Reference> arg) {
        ParsedArg parsedArg;

        typecase(
            arg.get(),
            [&](ast::UnresolvedIdent *nm) {
                parsedArg.name = nm->name;
                parsedArg.loc = nm->loc;
            },
            [&](ast::RestArg *rest) {
                parsedArg = parseArg(ctx, move(rest->expr));
                parsedArg.repeated = true;
            },
            [&](ast::KeywordArg *kw) {
                parsedArg = parseArg(ctx, move(kw->expr));
                parsedArg.keyword = true;
            },
            [&](ast::OptionalArg *opt) {
                parsedArg = parseArg(ctx, move(opt->expr));
                parsedArg.default_ = move(opt->default_);
            },
            [&](ast::BlockArg *blk) {
                parsedArg = parseArg(ctx, move(blk->expr));
                parsedArg.block = true;
            },
            [&](ast::ShadowArg *shadow) {
                parsedArg = parseArg(ctx, move(shadow->expr));
                parsedArg.shadow = true;
            },
            [&](ast::Local *local) {
                // Namer replaces args with locals, so to make namer idempotent,
                // we need to be able to handle Locals here.
                parsedArg.name = local->localVariable._name;
                parsedArg.loc = local->loc;
            });
        return parsedArg;
    }

    pair<core::SymbolRef, unique_ptr<ast::Expression>> arg2Symbol(core::MutableContext ctx, int pos,
                                                                  ParsedArg parsedArg) {
        if (pos < ctx.owner.data(ctx)->arguments().size()) {
            // TODO: check that flags match;
            core::SymbolRef sym = ctx.owner.data(ctx)->arguments()[pos];
            core::LocalVariable localVariable = enterLocal(ctx, parsedArg.name);
            auto localExpr = make_unique<ast::Local>(parsedArg.loc, localVariable);
            return make_pair(sym, move(localExpr));
        }
        core::NameRef name;
        if (parsedArg.keyword) {
            name = parsedArg.name;
        } else if (parsedArg.block) {
            name = core::Names::blkArg();
        } else {
            name = ctx.state.freshNameUnique(core::UniqueNameKind::PositionalArg, core::Names::arg(), pos + 1);
        }
        core::SymbolRef sym = ctx.state.enterMethodArgumentSymbol(parsedArg.loc, ctx.owner, name);
        core::LocalVariable localVariable = enterLocal(ctx, parsedArg.name);
        unique_ptr<ast::Reference> localExpr = make_unique<ast::Local>(parsedArg.loc, localVariable);

        core::SymbolData data = sym.data(ctx);
        if (parsedArg.default_) {
            data->setOptional();
            localExpr = make_unique<ast::OptionalArg>(parsedArg.loc, move(localExpr), move(parsedArg.default_));
        }
        if (parsedArg.keyword) {
            data->setKeyword();
        }
        if (parsedArg.block) {
            data->setBlockArgument();
        }
        if (parsedArg.repeated) {
            data->setRepeated();
        }
        return make_pair(sym, move(localExpr));
    }

    struct LocalFrame {
        UnorderedMap<core::NameRef, core::LocalVariable> locals;
        vector<core::LocalVariable> args;
        u4 localId;
        std::optional<u4> oldBlockCounter;
    };

    LocalFrame &enterBlock() {
        scopeStack.emplace_back().localId = blockCounter;
        ++blockCounter;
        return scopeStack.back();
    }

    LocalFrame &enterClassOrMethod() {
        scopeStack.emplace_back().localId = 0;
        scopeStack.back().oldBlockCounter = blockCounter;
        blockCounter = 1;
        return scopeStack.back();
    }

    void exitScope() {
        auto &oldScopeCounter = scopeStack.back().oldBlockCounter;
        if (oldScopeCounter) {
            blockCounter = *oldScopeCounter;
        }
        scopeStack.pop_back();
    }

    vector<LocalFrame> scopeStack;
    // The purpose of this counter is to ensure that every block within a method/class has a unique scope id.
    // For example, a possible assignment of ids is the following:
    //
    // [].map { # $0 }
    // class A
    //   [].each { # $0 }
    //   [].map { # $1 }
    // end
    // [].each { # $1 }
    // def foo
    //   [].each { # $0 }
    //   [].map { # $1 }
    // end
    // [].each { # $2 }
    u4 blockCounter;

    core::LocalVariable enterLocal(core::MutableContext ctx, core::NameRef name) {
        if (!ctx.owner.data(ctx)->isBlockSymbol(ctx)) {
            return core::LocalVariable(name, 0);
        }
        return core::LocalVariable(name, scopeStack.back().localId);
    }

    // Allow stub symbols created to hold intrinsics to be filled in
    // with real types from code
    bool isIntrinsic(core::Context ctx, core::SymbolRef sym) {
        auto data = sym.data(ctx);
        return data->intrinsic != nullptr && data->resultType == nullptr;
    }

    ast::MethodDef::ARGS_store fillInArgs(core::MutableContext ctx, vector<ParsedArg> parsedArgs) {
        ast::MethodDef::ARGS_store args;
        bool inShadows = false;
        bool intrinsic = isIntrinsic(ctx, ctx.owner);
        bool swapArgs = intrinsic && (ctx.owner.data(ctx)->arguments().size() == 1);
        core::SymbolRef swappedArg;
        if (swapArgs) {
            // When we're filling in an intrinsic method, we want to overwrite the block arg that used
            // to exist with the block arg that we got from desugaring the method def in the RBI files.
            ENFORCE(ctx.owner.data(ctx)->arguments()[0].data(ctx)->isBlockArgument());
            swappedArg = ctx.owner.data(ctx)->arguments()[0];
            ctx.owner.data(ctx)->arguments().clear();
        }

        int i = -1;
        for (auto &arg : parsedArgs) {
            i++;
            auto name = arg.name;
            auto localVariable = enterLocal(ctx, name);

            if (arg.shadow) {
                inShadows = true;
                auto localExpr = make_unique<ast::Local>(arg.loc, localVariable);
                args.emplace_back(move(localExpr));
            } else {
                ENFORCE(!inShadows, "shadow argument followed by non-shadow argument!");
                if (swapArgs && arg.block) {
                    // see commnent on if (swapArgs) above
                    ctx.owner.data(ctx)->arguments().emplace_back(swappedArg);
                }
                auto pair = arg2Symbol(ctx, i, move(arg));
                core::SymbolRef sym = pair.first;
                args.emplace_back(move(pair.second));
                ENFORCE(i < ctx.owner.data(ctx)->arguments().size());
                ENFORCE(swapArgs || (ctx.owner.data(ctx)->arguments()[i] == sym));
            }

            scopeStack.back().locals[name] = localVariable;
            scopeStack.back().args.emplace_back(localVariable);
        }

        return args;
    }

    core::SymbolRef methodOwner(core::MutableContext ctx) {
        core::SymbolRef owner = ctx.owner.data(ctx)->enclosingClass(ctx);
        if (owner == core::Symbols::root()) {
            // Root methods end up going on object
            owner = core::Symbols::Object();
        }
        return owner;
    }

    bool paramsMatch(core::MutableContext ctx, core::Loc loc, const vector<ParsedArg> &parsedArgs) {
        auto sym = ctx.owner.data(ctx)->dealias(ctx);
        if (sym.data(ctx)->arguments().size() != parsedArgs.size()) {
            if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                if (sym != ctx.owner) {
                    // TODO(jez) Subtracting 1 because of the block arg we added everywhere.
                    // Eventually we should be more principled about how we report this.
                    e.setHeader(
                        "Method alias `{}` redefined without matching argument count. Expected: `{}`, got: `{}`",
                        ctx.owner.data(ctx)->show(ctx), sym.data(ctx)->arguments().size() - 1, parsedArgs.size() - 1);
                    e.addErrorLine(ctx.owner.data(ctx)->loc(), "Previous alias definition");
                    e.addErrorLine(sym.data(ctx)->loc(), "Dealiased definition");
                } else {
                    // TODO(jez) Subtracting 1 because of the block arg we added everywhere.
                    // Eventually we should be more principled about how we report this.
                    e.setHeader("Method `{}` redefined without matching argument count. Expected: `{}`, got: `{}`",
                                sym.data(ctx)->show(ctx), sym.data(ctx)->arguments().size() - 1, parsedArgs.size() - 1);
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
            }
            return false;
        }
        for (int i = 0; i < parsedArgs.size(); i++) {
            auto &methodArg = parsedArgs[i];
            auto symArg = sym.data(ctx)->arguments()[i].data(ctx);

            if (symArg->isKeyword() != methodArg.keyword) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader(
                        "Method `{}` redefined with mismatched argument attribute `{}`. Expected: `{}`, got: `{}`",
                        sym.data(ctx)->show(ctx), "isKeyword", symArg->isKeyword(), methodArg.keyword);
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
                return false;
            }
            if (symArg->isBlockArgument() != methodArg.block) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader(
                        "Method `{}` redefined with mismatched argument attribute `{}`. Expected: `{}`, got: `{}`",
                        sym.data(ctx)->show(ctx), "isBlock", symArg->isBlockArgument(), methodArg.block);
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
                return false;
            }
            if (symArg->isRepeated() != methodArg.repeated) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader(
                        "Method `{}` redefined with mismatched argument attribute `{}`. Expected: `{}`, got: `{}`",
                        sym.data(ctx)->show(ctx), "isRepeated", symArg->isRepeated(), methodArg.repeated);
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
                return false;
            }
            if (symArg->isKeyword() && symArg->name != methodArg.name) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader("Method `{}` redefined with mismatched argument name. Expected: `{}`, got: `{}`",
                                sym.data(ctx)->show(ctx), symArg->name.show(ctx), methodArg.name.show(ctx));
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
                return false;
            }
        }

        return true;
    }

public:
    unique_ptr<ast::ClassDef> preTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> klass) {
        enterClassOrMethod();
        return klass;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> klass) {
        exitScope();
        return klass;
    }

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> method) {
        enterClassOrMethod();

        core::SymbolRef owner = methodOwner(ctx);

        if (method->isSelf()) {
            if (owner.data(ctx)->isClass()) {
                owner = owner.data(ctx)->singletonClass(ctx);
            }
        }
        ENFORCE(owner.data(ctx)->isClass());

        vector<ParsedArg> parsedArgs;
        for (auto &arg : method->args) {
            auto *refExp = ast::cast_tree<ast::Reference>(arg.get());
            if (!refExp) {
                Exception::raise("Must be a reference!");
            }
            unique_ptr<ast::Reference> refExpImpl(refExp);
            arg.release();
            parsedArgs.emplace_back(parseArg(ctx, move(refExpImpl)));
        }

        auto sym = owner.data(ctx)->findMemberNoDealias(ctx, method->name);
        if (sym.exists()) {
            if (method->declLoc == sym.data(ctx)->loc()) {
                // TODO remove if the paramsMatch is perfect
                // Reparsing the same file
                method->symbol = sym;
                method->args = fillInArgs(ctx.withOwner(method->symbol), move(parsedArgs));
                return method;
            }
            if (isIntrinsic(ctx, sym) || paramsMatch(ctx.withOwner(sym), method->declLoc, parsedArgs)) {
                sym.data(ctx)->addLoc(ctx, method->declLoc);
            } else {
                ctx.state.mangleRenameSymbol(sym, method->name);
            }
        }
        method->symbol = ctx.state.enterMethodSymbol(method->declLoc, owner, method->name);
        method->args = fillInArgs(ctx.withOwner(method->symbol), move(parsedArgs));
        method->symbol.data(ctx)->addLoc(ctx, method->declLoc);
        if (method->isDSLSynthesized()) {
            method->symbol.data(ctx)->setDSLSynthesized();
        }
        return method;
    }

    unique_ptr<ast::MethodDef> postTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> method) {
        ENFORCE(method->args.size() == method->symbol.data(ctx)->arguments().size());
        exitScope();
        ENFORCE(method->args.size() == method->symbol.data(ctx)->arguments().size(), "{}: {} != {}",
                method->name.showRaw(ctx), method->args.size(), method->symbol.data(ctx)->arguments().size());
        // Not all information is unfortunately available in the symbol. Original argument names aren't.
        // method->args.clear();
        return method;
    }

    unique_ptr<ast::Expression> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> original) {
        if (original->args.size() == 1 && ast::isa_tree<ast::ZSuperArgs>(original->args[0].get())) {
            original->args.clear();
            core::SymbolRef method = ctx.owner.data(ctx)->enclosingMethod(ctx);
            if (method.data(ctx)->isMethod()) {
                for (auto arg : scopeStack.back().args) {
                    original->args.emplace_back(make_unique<ast::Local>(original->loc, arg));
                }
            } else {
                if (auto e = ctx.state.beginError(original->loc, core::errors::Namer::SelfOutsideClass)) {
                    e.setHeader("`{}` outside of method", "super");
                }
            }
        }

        return original;
    }

    unique_ptr<ast::Block> preTransformBlock(core::MutableContext ctx, unique_ptr<ast::Block> blk) {
        auto outerArgs = scopeStack.back().args;
        auto &frame = enterBlock();
        frame.args = std::move(outerArgs);
        auto &parent = *(scopeStack.end() - 2);

        // We inherit our parent's locals
        for (auto &binding : parent.locals) {
            frame.locals.insert(binding);
        }

        // If any of our arguments shadow our parent, fillInArgs will overwrite
        // them in `frame.locals`
        vector<ParsedArg> parsedArgs;
        for (auto &arg : blk->args) {
            auto *refExp = ast::cast_tree<ast::Reference>(arg.get());
            if (!refExp) {
                Exception::raise("Must be a reference!");
            }
            unique_ptr<ast::Reference> refExpImpl(refExp);
            arg.release();
            parsedArgs.emplace_back(parseArg(ctx, move(refExpImpl)));
        }
        blk->args = fillInArgs(ctx.withOwner(blk->symbol), move(parsedArgs));

        return blk;
    }

    unique_ptr<ast::Block> postTransformBlock(core::MutableContext ctx, unique_ptr<ast::Block> blk) {
        exitScope();
        return blk;
    }

    unique_ptr<ast::Expression> postTransformUnresolvedIdent(core::MutableContext ctx,
                                                             unique_ptr<ast::UnresolvedIdent> nm) {
        if (nm->kind == ast::UnresolvedIdent::Local) {
            auto &frame = scopeStack.back();
            core::LocalVariable &cur = frame.locals[nm->name];
            if (!cur.exists()) {
                cur = enterLocal(ctx, nm->name);
                frame.locals[nm->name] = cur;
            }
            return make_unique<ast::Local>(nm->loc, cur);
        } else {
            return nm;
        }
    }

private:
    LocalNameInserter() : blockCounter(0) {
        enterBlock();
    }
};

unique_ptr<ast::Expression> NameLocals::run(core::MutableContext ctx, unique_ptr<ast::Expression> tree) {
    LocalNameInserter localNameInserter;
    return ast::TreeMap::apply(ctx, localNameInserter, move(tree));
}

} // namespace sorbet::name_locals
