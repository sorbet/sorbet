// These violate our poisons so have to happen first
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType
#include "llvm/IR/IRBuilder.h"

#include "Payload.h"
#include "absl/base/casts.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/sort.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/IREmitter/CFGHelpers.h"
#include "compiler/IREmitter/IREmitterContext.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/MethodCallContext.h"
#include "compiler/Names/Names.h"

using namespace std;
namespace sorbet::compiler {

namespace {

optional<string_view> isSymbol(const core::GlobalState &gs, const cfg::InstructionPtr &insn) {
    auto *liti = cfg::cast_instruction<cfg::Literal>(insn);
    if (liti == nullptr) {
        return std::nullopt;
    }

    if (!core::isa_type<core::LiteralType>(liti->value)) {
        return std::nullopt;
    }

    const auto &lit = core::cast_type_nonnull<core::LiteralType>(liti->value);
    if (lit.literalKind != core::LiteralType::LiteralTypeKind::Symbol) {
        return std::nullopt;
    }

    return lit.asName(gs).shortName(gs);
}

struct AliasesAndKeywords {
    UnorderedMap<cfg::LocalRef, Alias> aliases;
    UnorderedMap<cfg::LocalRef, string_view> symbols;
};

// Iterate over all instructions in the CFG, populating the alias map.
AliasesAndKeywords setupAliasesAndKeywords(CompilerState &cs, const cfg::CFG &cfg) {
    AliasesAndKeywords res;

    for (auto &bb : cfg.basicBlocks) {
        for (auto &bind : bb->exprs) {
            if (auto *i = cfg::cast_instruction<cfg::Alias>(bind.value)) {
                ENFORCE(res.aliases.find(bind.bind.variable) == res.aliases.end(),
                        "Overwriting an entry in the aliases map");

                if (i->what == core::Symbols::Magic_undeclaredFieldStub()) {
                    // When `i->what` is undeclaredFieldStub, `i->name` is populated
                    auto name = i->name.shortName(cs);
                    if (name.size() > 2 && name[0] == '@' && name[1] == '@') {
                        res.aliases[bind.bind.variable] = Alias::forClassField(i->name);
                    } else if (name.size() > 1 && name[0] == '@') {
                        res.aliases[bind.bind.variable] = Alias::forInstanceField(i->name);
                    } else if (name.size() > 1 && name[0] == '$') {
                        res.aliases[bind.bind.variable] = Alias::forGlobalField(i->name);
                    } else {
                        ENFORCE(stoi(string(name)) > 0, "'" + string(name) + "' is not a valid global name");
                        res.aliases[bind.bind.variable] = Alias::forGlobalField(i->name);
                    }
                } else {
                    // It's currently impossible in Sorbet to declare a global field with a T.let
                    // (they will all be Magic_undeclaredFieldStub)
                    auto name = i->what.data(cs)->name;
                    auto shortName = name.shortName(cs);
                    ENFORCE(!(shortName.size() > 0 && shortName[0] == '$'));

                    if (i->what.data(cs)->isField()) {
                        res.aliases[bind.bind.variable] = Alias::forInstanceField(name);
                    } else if (i->what.data(cs)->isStaticField()) {
                        if (shortName.size() > 2 && shortName[0] == '@' && shortName[1] == '@') {
                            res.aliases[bind.bind.variable] = Alias::forClassField(name);
                        } else {
                            res.aliases[bind.bind.variable] = Alias::forConstant(i->what);
                        }
                    } else {
                        res.aliases[bind.bind.variable] = Alias::forConstant(i->what);
                    }
                }
            } else if (auto sym = isSymbol(cs, bind.value)) {
                res.symbols[bind.bind.variable] = sym.value();
            }
        }
    }

    return res;
}

// Iterate over all instructions in the CFG, determining which ruby blocks use `break`
vector<bool> blocksThatUseBreak(CompilerState &cs, const cfg::CFG &cfg) {
    vector<bool> res(cfg.maxRubyBlockId + 1, false);

    for (auto &bb : cfg.basicBlocks) {
        for (auto &bind : bb->exprs) {
            if (bind.bind.variable.data(cfg)._name == core::Names::blockBreak()) {
                res[bb->rubyBlockId] = true;
                break;
            }
        }
    }

    return res;
}

std::tuple<UnorderedMap<cfg::LocalRef, llvm::AllocaInst *>, UnorderedMap<int, llvm::AllocaInst *>>
setupLocalVariables(CompilerState &cs, cfg::CFG &cfg, const UnorderedMap<cfg::LocalRef, int> &variablesPrivateToBlocks,
                    const IREmitterContext &irctx) {
    UnorderedMap<cfg::LocalRef, llvm::AllocaInst *> llvmVariables;
    llvm::IRBuilder<> builder(cs);
    {
        // nill out block local variables.
        auto valueType = cs.getValueType();
        vector<pair<cfg::LocalRef, int>> variablesPrivateToBlocksSorted;

        for (const auto &entry : variablesPrivateToBlocks) {
            variablesPrivateToBlocksSorted.emplace_back(entry);
        }
        fast_sort(variablesPrivateToBlocksSorted,
                  [](const auto &left, const auto &right) -> bool { return left.first.id() < right.first.id(); });
        for (const auto &entry : variablesPrivateToBlocksSorted) {
            auto var = entry.first;
            auto svName = var.data(cfg)._name.shortName(cs);
            builder.SetInsertPoint(irctx.functionInitializersByFunction[entry.second]);
            auto alloca = llvmVariables[var] =
                builder.CreateAlloca(valueType, nullptr, llvm::StringRef(svName.data(), svName.length()));
            auto nilValueRaw = Payload::rubyNil(cs, builder);
            Payload::boxRawValue(cs, builder, alloca, nilValueRaw);
        }
    }

    // reserve self in all basic blocks.  We rely on LLVM to delete any allocas
    // we wind up not actually needing.
    UnorderedMap<int, llvm::AllocaInst *> selfVariables;
    for (int i = 0; i <= cfg.maxRubyBlockId; ++i) {
        builder.SetInsertPoint(irctx.functionInitializersByFunction[i]);
        auto var = cfg::LocalRef::selfVariable();
        auto nameStr = var.toString(cs, cfg);
        selfVariables[i] =
            builder.CreateAlloca(cs.getValueType(), nullptr, llvm::StringRef(nameStr.data(), nameStr.length()));
    }

    {
        // reserve the magical return value
        builder.SetInsertPoint(irctx.functionInitializersByFunction[0]);
        auto name = Names::returnValue(cs);
        auto var = cfg.enterLocal(core::LocalVariable{name, 1});
        auto nameStr = name.toString(cs);
        llvmVariables[var] =
            builder.CreateAlloca(cs.getValueType(), nullptr, llvm::StringRef(nameStr.data(), nameStr.length()));
    }

    return {std::move(llvmVariables), std::move(selfVariables)};
}

struct CapturedVariables {
    // LocalRefs that are only referenced from a single block.  Maps from variables
    // to the block in which they are referenced.
    UnorderedMap<cfg::LocalRef, int> privateVariables;

    // LocalRefs referenced from multiple blocks.  Maps from variables to their index
    // in the local variable area on the Ruby stack.
    UnorderedMap<cfg::LocalRef, int> escapedVariableIndexes;

    // Whether the ruby method uses a block argument.
    bool usesBlockArgs;
};

// Bundle up a bunch of state used for capture tracking to simplify the interface in findCaptures below.
class TrackCaptures final {
public:
    UnorderedMap<cfg::LocalRef, optional<int>> privateUsages;
    UnorderedMap<cfg::LocalRef, int> escapedIndexes;
    int escapedIndexCounter = 0;
    bool usesBlockArg = false;
    cfg::LocalRef blkArg = cfg::LocalRef::noVariable();

    void trackBlockUsage(cfg::BasicBlock *bb, cfg::LocalRef lv) {
        if (lv == cfg::LocalRef::selfVariable()) {
            return;
        }
        usesBlockArg = usesBlockArg || lv == blkArg;
        auto fnd = privateUsages.find(lv);
        if (fnd != privateUsages.end()) {
            auto &store = fnd->second;
            if (store && store.value() != bb->rubyBlockId) {
                store = nullopt;
                escapedIndexes[lv] = escapedIndexCounter;
                escapedIndexCounter += 1;
            }
        } else {
            privateUsages[lv] = bb->rubyBlockId;
        }
    }

    CapturedVariables finalize() {
        // privateUsages entries that have nullopt values are only interesting to the
        // capture analysis process, so remove them
        UnorderedMap<cfg::LocalRef, int> realPrivateUsages;
        for (auto &entry : privateUsages) {
            if (!entry.second.has_value()) {
                continue;
            }

            realPrivateUsages[entry.first] = entry.second.value();
        }

        return CapturedVariables{std::move(realPrivateUsages), std::move(escapedIndexes), usesBlockArg};
    }
};

/* if local variable is only used in block X, it maps the local variable to X, otherwise, it maps local variable to a
 * negative number */
CapturedVariables findCaptures(CompilerState &cs, const ast::MethodDef &mdef, cfg::CFG &cfg,
                               const vector<int> &exceptionHandlingBlockHeaders) {
    TrackCaptures usage;

    int argId = -1;
    for (auto &arg : mdef.args) {
        argId += 1;
        ast::Local const *local = nullptr;
        if (auto *opt = ast::cast_tree<ast::OptionalArg>(arg)) {
            local = ast::cast_tree<ast::Local>(opt->expr);
        } else {
            local = ast::cast_tree<ast::Local>(arg);
        }
        ENFORCE(local);
        auto localRef = cfg.enterLocal(local->localVariable);
        usage.trackBlockUsage(cfg.entry(), localRef);
        if (cfg.symbol.data(cs)->arguments()[argId].flags.isBlock) {
            usage.blkArg = localRef;
        }
    }

    for (auto &bb : cfg.basicBlocks) {
        for (cfg::Binding &bind : bb->exprs) {
            usage.trackBlockUsage(bb.get(), bind.bind.variable);
            typecase(
                bind.value, [&](cfg::Ident &i) { usage.trackBlockUsage(bb.get(), i.what); },
                [&](cfg::Alias &i) { /* nothing */
                },
                [&](cfg::SolveConstraint &i) { /* nothing*/ },
                [&](cfg::Send &i) {
                    for (auto &arg : i.args) {
                        usage.trackBlockUsage(bb.get(), arg.variable);
                    }
                    usage.trackBlockUsage(bb.get(), i.recv.variable);
                },
                [&](cfg::GetCurrentException &i) {
                    // if the current block is an exception header, record a usage of the variable in the else block
                    // (the body block of the exception handling) to force it to escape.
                    if (exceptionHandlingBlockHeaders[bb->id] != 0) {
                        usage.trackBlockUsage(bb->bexit.elseb, bind.bind.variable);
                    }
                },
                [&](cfg::Return &i) { usage.trackBlockUsage(bb.get(), i.what.variable); },
                [&](cfg::BlockReturn &i) { usage.trackBlockUsage(bb.get(), i.what.variable); },
                [&](cfg::LoadSelf &i) { /*nothing*/ /*todo: how does instance exec pass self?*/ },
                [&](cfg::Literal &i) { /* nothing*/ }, [&](cfg::ArgPresent &i) { /*nothing*/ },
                [&](cfg::LoadArg &i) { /*nothing*/ }, [&](cfg::LoadYieldParams &i) { /*nothing*/ },
                [&](cfg::YieldParamPresent &i) { /* nothing */ }, [&](cfg::YieldLoadArg &i) { /* nothing */ },
                [&](cfg::Cast &i) { usage.trackBlockUsage(bb.get(), i.value.variable); },
                [&](cfg::TAbsurd &i) { usage.trackBlockUsage(bb.get(), i.what.variable); });
        }

        // no need to track the condition variable if the jump is unconditional
        if (bb->bexit.thenb != bb->bexit.elseb) {
            usage.trackBlockUsage(bb.get(), bb->bexit.cond.variable);
        }
    }

    return usage.finalize();
}

int getMaxSendArgCount(cfg::CFG &cfg) {
    int maxSendArgCount = 0;
    for (auto &bb : cfg.basicBlocks) {
        for (cfg::Binding &bind : bb->exprs) {
            if (auto snd = cfg::cast_instruction<cfg::Send>(bind.value)) {
                int numPosArgs = snd->numPosArgs;
                int numKwArgs = snd->args.size() - numPosArgs;

                // add one for the receiver when pushing args on the ruby stack
                int numArgs = 1 + numPosArgs;

                if (numKwArgs % 2 == 1) {
                    // Odd keyword args indicate a keyword splat, and in that case we merge all keyword args into a
                    // single hash as the VM doesn't support mixed kwarg/kwsplat sends.
                    numArgs += 1;
                } else {
                    // Otherwise the keyword args indicate the number of symbol/value pairs, and since we push only the
                    // values on the stack the send arg count is increased by the number of keyword args / 2.
                    numArgs += numKwArgs / 2;
                }

                // For backwards compatibility with the fillSendArgArray method of argument passing, the allocated array
                // must be large enough to hold all of the inlined keyword arguments when initializing a hash. This
                // comes up in cases like `super` that will forward all kwargs as a single hash.
                int buildHashArgs = numKwArgs & ~0x1;

                maxSendArgCount = std::max({maxSendArgCount, numArgs, buildHashArgs});
            }
        }
    }
    return maxSendArgCount;
}

// TODO
llvm::DISubroutineType *getDebugFunctionType(CompilerState &cs, llvm::Function *func) {
    vector<llvm::Metadata *> eltTys;

    auto *valueTy = cs.debug->createBasicType("VALUE", 64, llvm::dwarf::DW_ATE_signed);
    eltTys.push_back(valueTy);

    // NOTE: the return type is always the first element in the array
    return cs.debug->createSubroutineType(cs.debug->getOrCreateTypeArray(eltTys));
}

llvm::DISubprogram *getDebugScope(CompilerState &cs, cfg::CFG &cfg, llvm::DIScope *parent, llvm::Function *func,
                                  int rubyBlockId) {
    auto debugFile = cs.debug->createFile(cs.compileUnit->getFilename(), cs.compileUnit->getDirectory());
    auto loc = cfg.symbol.data(cs)->loc();

    auto owner = cfg.symbol.data(cs)->owner;
    std::string diName(owner.data(cs)->name.shortName(cs));

    if (owner.data(cs)->isSingletonClass(cs)) {
        diName += ".";
    } else {
        diName += "#";
    }

    diName += cfg.symbol.data(cs)->name.shortName(cs);

    // Line number 0 indicates that the compiler knows the entity came from this
    // particular file, but doesn't have precise location tracking for it.  This
    // can happen with e.g. synthesized packages.
    auto lineNo = loc.exists() ? loc.position(cs).first.line : 0;

    return cs.debug->createFunction(parent, diName, func->getName(), debugFile, lineNo, getDebugFunctionType(cs, func),
                                    lineNo, llvm::DINode::FlagPrototyped, llvm::DISubprogram::SPFlagDefinition);
}

void getRubyBlocks2FunctionsMapping(CompilerState &cs, cfg::CFG &cfg, llvm::Function *func,
                                    const vector<FunctionType> &blockTypes, vector<llvm::Function *> &funcs,
                                    vector<llvm::DISubprogram *> &scopes) {
    auto *bt = cs.getRubyBlockFFIType();
    auto *et = cs.getRubyExceptionFFIType();
    for (int i = 0; i <= cfg.maxRubyBlockId; i++) {
        switch (blockTypes[i]) {
            case FunctionType::Method:
            case FunctionType::StaticInitFile:
            case FunctionType::StaticInitModule:
                funcs[i] = func;
                break;

            case FunctionType::Block: {
                auto *fp =
                    llvm::Function::Create(bt, llvm::Function::InternalLinkage,
                                           llvm::Twine{func->getName()} + "$block_" + llvm::Twine(i), *cs.module);
                // setup argument names
                fp->arg_begin()->setName("firstYieldArgRaw");
                (fp->arg_begin() + 1)->setName("localsOffset");
                (fp->arg_begin() + 2)->setName("argc");
                (fp->arg_begin() + 3)->setName("argArray");
                (fp->arg_begin() + 4)->setName("blockArg");
                funcs[i] = fp;
                break;
            }

            // NOTE: explicitly treating Unused functions like Exception functions, as they'll be collected by llvm
            // anyway.
            case FunctionType::ExceptionBegin:
            case FunctionType::Rescue:
            case FunctionType::Ensure:
            case FunctionType::Unused: {
                auto *fp =
                    llvm::Function::Create(et, llvm::Function::InternalLinkage,
                                           llvm::Twine{func->getName()} + "$block_" + llvm::Twine(i), *cs.module);

                // argument names
                fp->arg_begin()->setName("pc");
                (fp->arg_begin() + 1)->setName("localsOffset");
                (fp->arg_begin() + 2)->setName("cfp");

                funcs[i] = fp;
                break;
            }
        }

        auto *parent = i == 0 ? static_cast<llvm::DIScope *>(cs.compileUnit) : scopes[0];
        auto *scope = getDebugScope(cs, cfg, parent, funcs[i], i);
        scopes[i] = scope;
        funcs[i]->setSubprogram(scope);

        ENFORCE(scope->describes(funcs[i]));
    }
};

// Returns the mapping of ruby block id to function type, as well as the mapping from basic block to exception handling
// body block id.
void determineBlockTypes(CompilerState &cs, cfg::CFG &cfg, vector<FunctionType> &blockTypes, vector<int> &blockParents,
                         vector<int> &exceptionHandlingBlockHeaders, vector<int> &basicBlockJumpOverrides) {
    // ruby block 0 is always the top-level of the method being compiled
    if (IREmitterHelpers::isClassStaticInit(cs, cfg.symbol)) {
        // When ruby runs the `Init_` function to initialize the whole object for this function it pushes a c frame for
        // that function on the ruby stack, and we update that frame with the iseq that we make for tracking line
        // numbers.  However when we run our static-init methods for classes and modules we call the c functions
        // directly, and want to avoid mutating the ruby stack frame. Thus those functions get marked as StaticInit,
        // while the file-level static-init gets marked as Method.
        //
        // https://github.com/ruby/ruby/blob/a9a48e6a741f048766a2a287592098c4f6c7b7c7/load.c#L1033-L1034
        blockTypes[0] = FunctionType::StaticInitModule;
    } else if (IREmitterHelpers::isFileStaticInit(cs, cfg.symbol)) {
        blockTypes[0] = FunctionType::StaticInitFile;
    } else {
        blockTypes[0] = FunctionType::Method;
    }

    for (auto &b : cfg.basicBlocks) {
        if (b->bexit.cond.variable == cfg::LocalRef::blockCall()) {
            blockTypes[b->rubyBlockId] = FunctionType::Block;

            // the else branch always points back to the original owning rubyBlockId of the block call
            blockParents[b->rubyBlockId] = b->bexit.elseb->rubyBlockId;

        } else if (b->bexit.cond.variable.data(cfg)._name == core::Names::exceptionValue()) {
            auto *bodyBlock = b->bexit.elseb;
            auto *handlersBlock = b->bexit.thenb;

            // the relative block ids of blocks that are involved in the translation of an exception handling block.
            auto bodyBlockId = bodyBlock->rubyBlockId;
            auto handlersBlockId = bodyBlockId + cfg::CFG::HANDLERS_BLOCK_OFFSET;
            auto ensureBlockId = bodyBlockId + cfg::CFG::ENSURE_BLOCK_OFFSET;
            auto elseBlockId = bodyBlockId + cfg::CFG::ELSE_BLOCK_OFFSET;

            // `b` is the exception handling header block if the two branches from it have the sequential ids we would
            // expect for the handler and body blocks. The reason we bail out here if this isn't the case is because
            // there are other blocks within the translation that will also jump based on the value of the same
            // exception value variable.
            if (handlersBlock->rubyBlockId != handlersBlockId) {
                continue;
            }

            auto *elseBlock = CFGHelpers::findRegionEntry(cfg, elseBlockId);
            auto *ensureBlock = CFGHelpers::findRegionEntry(cfg, ensureBlockId);

            // The way the CFG is constructed ensures that there will always be an else block, an ensure block, or both
            // present.
            ENFORCE(elseBlock || ensureBlock);

            {
                // Find the exit block for exception handling so that we can redirect the header to it.
                auto *exit = ensureBlock == nullptr ? elseBlock : ensureBlock;
                auto exits = CFGHelpers::findRegionExits(cfg, b->rubyBlockId, exit->rubyBlockId);

                // The ensure block should only ever jump to the code that follows the begin/end block.
                ENFORCE(exits.size() <= 1);

                if (exits.empty()) {
                    // When control flow terminates in the block that ends exception handling, else or ensure, that
                    // block will transition to the dead block. As `findRegionExits` will ignore the dead block to
                    // simplify the common case of looking for reachable exits, the exits vector being empty indicates
                    // that a return is present in the exception handling exit, and that the transition will never
                    // happen. In this case we can explicitly jump from the exception handling entry block directly to
                    // the dead block.
                    basicBlockJumpOverrides[handlersBlock->id] = cfg.deadBlock()->id;
                    basicBlockJumpOverrides[bodyBlock->id] = cfg.deadBlock()->id;
                } else {
                    // Have the entry block jump over all of the exception handling machinery.
                    basicBlockJumpOverrides[handlersBlock->id] = exits.front()->id;
                    basicBlockJumpOverrides[bodyBlock->id] = exits.front()->id;
                }
            }

            exceptionHandlingBlockHeaders[b->id] = bodyBlockId;

            blockTypes[bodyBlockId] = FunctionType::ExceptionBegin;
            blockTypes[handlersBlockId] = FunctionType::Rescue;

            if (elseBlock) {
                blockTypes[elseBlockId] = FunctionType::ExceptionBegin;
            }

            if (ensureBlock) {
                blockTypes[ensureBlockId] = FunctionType::Ensure;
            }

            // All exception handling blocks are children of `b`, as far as ruby iseq allocation is concerned.
            blockParents[bodyBlockId] = b->rubyBlockId;
            blockParents[handlersBlockId] = b->rubyBlockId;
            blockParents[elseBlockId] = b->rubyBlockId;
            blockParents[ensureBlockId] = b->rubyBlockId;
        }
    }

    return;
}

bool returnAcrossBlockIsPresent(CompilerState &cs, cfg::CFG &cfg, const vector<int> &blockNestingLevels) {
    for (auto &bb : cfg.basicBlocks) {
        for (auto &bind : bb->exprs) {
            if (cfg::isa_instruction<cfg::Return>(bind.value)) {
                // This will be non-zero if there was a block in any of our parent blocks.
                if (blockNestingLevels[bb->rubyBlockId] != 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Returns the number of scopes that must be traversed to get back out out to the
// top-level method frame and the number of blocks that must be traversed to get
// back out to the top-level method frame.  The latter is important for providing
// accurate location information for block iseqs.
tuple<int, int> getBlockNesting(vector<int> &blockParents, vector<FunctionType> &blockTypes, int rubyBlockId) {
    auto level = 0;
    auto blockLevel = 0;

    while (true) {
        switch (blockTypes[rubyBlockId]) {
            case FunctionType::Method:
            case FunctionType::StaticInitFile:
            case FunctionType::StaticInitModule:
                return {level, blockLevel};

            case FunctionType::Block:
                ++blockLevel;
                [[fallthrough]];
            case FunctionType::Rescue:
            case FunctionType::Ensure:
                // Increment the level, as we're crossing through a non-method stack frame to get back to our parent.
                ++level;
                rubyBlockId = blockParents[rubyBlockId];
                break;

            case FunctionType::ExceptionBegin:
            case FunctionType::Unused:
                // ExceptionBegin is considered to be part of the containing frame, so there's no block present here,
                // and unused functions will never be called, so it's fine for them to have garbage values here.
                rubyBlockId = blockParents[rubyBlockId];
                break;
        }
    }
}

tuple<vector<int>, vector<int>> getBlockLevels(vector<int> &blockParents, vector<FunctionType> &blockTypes) {
    vector<int> levels(blockTypes.size(), 0);
    vector<int> blockNesting(blockTypes.size(), 0);

    for (auto i = 0; i < blockTypes.size(); ++i) {
        auto [level, blockLevel] = getBlockNesting(blockParents, blockTypes, i);
        levels[i] = level;
        blockNesting[i] = blockLevel;
    }

    return {move(levels), move(blockNesting)};
}

string locationNameFor(CompilerState &cs, core::MethodRef symbol) {
    if (IREmitterHelpers::isClassStaticInit(cs, symbol)) {
        auto enclosingClassRef = symbol.enclosingClass(cs);
        ENFORCE(enclosingClassRef.exists());
        enclosingClassRef = enclosingClassRef.data(cs)->attachedClass(cs);
        ENFORCE(enclosingClassRef.exists());
        const auto &enclosingClass = enclosingClassRef.data(cs);
        return fmt::format("<{}:{}>", enclosingClass->isClassOrModuleClass() ? "class"sv : "module"sv,
                           enclosingClassRef.show(cs));
    } else if (IREmitterHelpers::isFileStaticInit(cs, symbol)) {
        return string("<top (required)>"sv);
    } else {
        return string(symbol.data(cs)->name.shortName(cs));
    }
}

// Given a Ruby block, finds the block id of the nearest _proper_ ancestor of that block that allocates an iseq.
int getNearestIseqAllocatorBlock(const vector<int> &blockParents, const vector<FunctionType> &blockTypes,
                                 int rubyBlockId) {
    do {
        rubyBlockId = blockParents[rubyBlockId];
    } while (rubyBlockId > 0 && blockTypes[rubyBlockId] == FunctionType::ExceptionBegin);

    return rubyBlockId;
}

// Block names in Ruby are built recursively as the file is parsed.  We aren't guaranteed
// to process blocks in depth-first order and we don't want to constantly recompute
// parent names.  So we build the names for everything up front.
vector<optional<string>> getBlockLocationNames(CompilerState &cs, cfg::CFG &cfg, const vector<int> &blockLevels,
                                               const vector<int> &blockNestingLevels, const vector<int> &blockParents,
                                               const vector<FunctionType> &blockTypes) {
    struct BlockInfo {
        int rubyBlockId;
        // blockLevels[this->rubyBlockId];
        int level;
    };

    ENFORCE(blockLevels.size() == blockParents.size());
    ENFORCE(blockLevels.size() == blockTypes.size());

    vector<optional<string>> blockLocationNames(blockLevels.size());
    // Sort blocks by their depth so that we can process things in a breadth-first order.
    vector<BlockInfo> blocksByDepth;
    blocksByDepth.reserve(blockLevels.size());
    for (int i = 0; i <= cfg.maxRubyBlockId; ++i) {
        blocksByDepth.emplace_back(BlockInfo{i, blockLevels[i]});
    }

    fast_sort(blocksByDepth, [](const auto &left, const auto &right) -> bool { return left.level < right.level; });

    const string topLevelLocation = locationNameFor(cs, cfg.symbol);

    for (const auto &info : blocksByDepth) {
        optional<string> &iseqName = blockLocationNames[info.rubyBlockId];
        const auto blockType = blockTypes[info.rubyBlockId];
        switch (blockType) {
            case FunctionType::Method:
            case FunctionType::StaticInitFile:
            case FunctionType::StaticInitModule:
                iseqName.emplace(topLevelLocation);
                break;

            case FunctionType::Block: {
                int blockLevel = blockNestingLevels[info.rubyBlockId];
                if (blockLevel == 1) {
                    iseqName.emplace(fmt::format("block in {}", topLevelLocation));
                } else {
                    iseqName.emplace(fmt::format("block ({} levels) in {}", blockLevel, topLevelLocation));
                }
                break;
            }

            case FunctionType::Rescue:
            case FunctionType::Ensure: {
                int parent = getNearestIseqAllocatorBlock(blockParents, blockTypes, info.rubyBlockId);
                const string *parentLocation;
                if (parent == 0) {
                    parentLocation = &topLevelLocation;
                } else {
                    const auto &parentName = blockLocationNames[parent];
                    ENFORCE(blockLevels[parent] < blockLevels[info.rubyBlockId]);
                    ENFORCE(parentName.has_value());
                    parentLocation = &*parentName;
                }
                iseqName.emplace(
                    fmt::format("{} in {}", blockType == FunctionType::Rescue ? "rescue" : "ensure", *parentLocation));
                break;
            }

            case FunctionType::ExceptionBegin:
            case FunctionType::Unused:
                // These types do not have iseqs allocated for them and therefore have no name.
                break;
        }
    }

    return blockLocationNames;
}

void collectRubyBlockArgs(const cfg::CFG &cfg, const cfg::BasicBlock *b, vector<cfg::LocalRef> &blockArgs) {
    bool insideThenBlock = false;

    while (true) {
        for (auto &expr : b->exprs) {
            auto loadArg = cfg::cast_instruction<cfg::YieldLoadArg>(expr.value);
            if (loadArg == nullptr) {
                continue;
            }
            blockArgs[loadArg->argId] = expr.bind.variable;
        }

        // When the exit condition for this block is constructed through `argPresent`, continue down the `then` branch
        // to collect more block arg definitions.
        if (b->bexit.cond.variable.data(cfg)._name == core::Names::argPresent()) {
            insideThenBlock = true;
            b = b->bexit.thenb;
            continue;
        }

        // The blocks emitted when optional arguments are present end in an unconditional jump to join with the path
        // that would populate the arg with the default value.
        if (insideThenBlock) {
            insideThenBlock = false;
            b = b->bexit.thenb;
            continue;
        }

        // We've reaced the end of argument handling entry blocks in the ruby block CFG.
        return;
    }
}

// Given a block's argument list, compute the minimum and maximum arguments that will be present at runtime.
BlockArity computeBlockArity(const vector<core::ArgInfo::ArgFlags> &argFlags) {
    BlockArity blockArity;

    bool seenKwarg = false;
    bool seenKwopt = false;
    bool seenSplat = false;
    for (auto &arg : argFlags) {
        if (arg.isBlock || arg.isShadow) {
            break;
        } else if (arg.isKeyword) {
            // Defaulted and repeated kwargs will both cause the max arg count to go to -1 when no required
            // kwargs are present.
            if (arg.isDefault || arg.isRepeated) {
                seenKwopt = true;
            } else {
                seenKwarg = true;
            }
        } else {
            if (!arg.isDefault && !arg.isRepeated) {
                blockArity.min += 1;
            }

            blockArity.max += 1;

            seenSplat = seenSplat || arg.isRepeated;
        }
    }

    if (seenKwarg) {
        blockArity.min += 1;
        blockArity.max += 1;
    } else if (seenKwopt) {
        blockArity.max = -1;
    }

    if (seenSplat) {
        blockArity.max = -1;
    }

    return blockArity;
}

} // namespace

IREmitterContext IREmitterContext::getSorbetBlocks2LLVMBlockMapping(CompilerState &cs, cfg::CFG &cfg,
                                                                    const ast::MethodDef &md,
                                                                    llvm::Function *mainFunc) {
    vector<int> basicBlockJumpOverrides(cfg.maxBasicBlockId);
    vector<int> basicBlockRubyBlockId(cfg.maxBasicBlockId, 0);
    llvm::IRBuilder<> builder(cs);
    {
        for (int i = 0; i < cfg.maxBasicBlockId; i++) {
            basicBlockJumpOverrides[i] = i;
        }

        for (auto &bb : cfg.basicBlocks) {
            basicBlockRubyBlockId[bb->id] = bb->rubyBlockId;
        }
    }

    vector<FunctionType> blockTypes(cfg.maxRubyBlockId + 1, FunctionType::Unused);
    vector<int> blockParents(cfg.maxRubyBlockId + 1, 0);
    vector<int> exceptionHandlingBlockHeaders(cfg.maxBasicBlockId + 1, 0);
    determineBlockTypes(cs, cfg, blockTypes, blockParents, exceptionHandlingBlockHeaders, basicBlockJumpOverrides);

    vector<llvm::Function *> rubyBlock2Function(cfg.maxRubyBlockId + 1, nullptr);
    vector<llvm::DISubprogram *> blockScopes(cfg.maxRubyBlockId + 1, nullptr);
    getRubyBlocks2FunctionsMapping(cs, cfg, mainFunc, blockTypes, rubyBlock2Function, blockScopes);

    auto [blockLevels, blockNestingLevels] = getBlockLevels(blockParents, blockTypes);
    auto blockLocationNames = getBlockLocationNames(cs, cfg, blockLevels, blockNestingLevels, blockParents, blockTypes);

    auto [aliases, symbols] = setupAliasesAndKeywords(cs, cfg);
    const int maxSendArgCount = getMaxSendArgCount(cfg);
    auto [variablesPrivateToBlocks, escapedVariableIndices, usesBlockArgs] =
        findCaptures(cs, md, cfg, exceptionHandlingBlockHeaders);
    vector<llvm::BasicBlock *> functionInitializersByFunction;
    vector<llvm::BasicBlock *> argumentSetupBlocksByFunction;
    vector<llvm::BasicBlock *> userEntryBlockByFunction(rubyBlock2Function.size());
    vector<llvm::AllocaInst *> sendArgArrayByBlock;
    vector<llvm::AllocaInst *> lineNumberPtrsByFunction;
    bool hasReturnAcrossBlock = false;
    UnorderedMap<int, llvm::AllocaInst *> blockControlFramePtrs;

    int i = 0;
    auto lineNumberPtrType = llvm::PointerType::getUnqual(llvm::Type::getInt64PtrTy(cs));
    auto *controlFrameStructType = llvm::StructType::getTypeByName(cs, "struct.rb_control_frame_struct");
    ENFORCE(controlFrameStructType != nullptr);
    auto *controlFramePtrType = controlFrameStructType->getPointerTo();
    for (auto &fun : rubyBlock2Function) {
        auto inits = functionInitializersByFunction.emplace_back(llvm::BasicBlock::Create(
            cs, "functionEntryInitializers",
            fun)); // we will build a link for this block later, after we finish building expressions into it
        builder.SetInsertPoint(inits);
        auto sendArgArray = builder.CreateAlloca(llvm::ArrayType::get(llvm::Type::getInt64Ty(cs), maxSendArgCount),
                                                 nullptr, "callArgs");
        sendArgArrayByBlock.emplace_back(sendArgArray);
        auto lineNumberPtr = builder.CreateAlloca(lineNumberPtrType, nullptr, "lineCountStore");
        lineNumberPtrsByFunction.emplace_back(lineNumberPtr);
        // We cache cfp for ordinary ruby blocks; methods and exception begin/else
        // blocks receive cfp as an argument.  We currently consider caching not
        // worth it for ensure/rescue blocks.  They are, after all, exceptional,
        // and probably don't contain enough code to make it worth caching the
        // CFP.
        if (blockTypes[i] == FunctionType::Block) {
            auto *controlFramePtr = builder.CreateAlloca(controlFramePtrType, nullptr, "controlFrameStore");
            blockControlFramePtrs[i] = controlFramePtr;
        }
        argumentSetupBlocksByFunction.emplace_back(llvm::BasicBlock::Create(cs, "argumentSetup", fun));
        if (i == 0) {
            // If return statements are present inside blocks, we need to codegen
            // differently when finalizing the function.
            //
            // TODO(aprocter): I think this is a little bit more conservative than it needs to be, because it will push
            // a tag even if the a return-from-block comes from a lambda, which is not actually necessary.
            if (returnAcrossBlockIsPresent(cs, cfg, blockNestingLevels)) {
                hasReturnAcrossBlock = true;
            }
        }
        i++;
    }

    vector<llvm::BasicBlock *> blockExits(cfg.maxRubyBlockId + 1);
    for (auto rubyBlockId = 0; rubyBlockId <= cfg.maxRubyBlockId; ++rubyBlockId) {
        blockExits[rubyBlockId] =
            llvm::BasicBlock::Create(cs, llvm::Twine("blockExit"), rubyBlock2Function[rubyBlockId]);
    }

    vector<llvm::BasicBlock *> deadBlocks(cfg.maxRubyBlockId + 1);
    vector<llvm::BasicBlock *> llvmBlocks(cfg.maxBasicBlockId + 1);
    for (auto &b : cfg.basicBlocks) {
        if (b.get() == cfg.entry()) {
            llvmBlocks[b->id] = userEntryBlockByFunction[0] =
                llvm::BasicBlock::Create(cs, "userEntry", rubyBlock2Function[0]);
        } else if (b.get() == cfg.deadBlock()) {
            for (auto rubyBlockId = 0; rubyBlockId <= cfg.maxRubyBlockId; ++rubyBlockId) {
                deadBlocks[rubyBlockId] =
                    llvm::BasicBlock::Create(cs, llvm::Twine("dead"), rubyBlock2Function[rubyBlockId]);
            }

            llvmBlocks[b->id] = deadBlocks[0];
        } else {
            llvmBlocks[b->id] = llvm::BasicBlock::Create(cs, llvm::Twine("BB") + llvm::Twine(b->id),
                                                         rubyBlock2Function[b->rubyBlockId]);
        }
    }
    vector<shared_ptr<core::SendAndBlockLink>> blockLinks(rubyBlock2Function.size());
    vector<vector<cfg::LocalRef>> rubyBlockArgs(rubyBlock2Function.size());

    {
        // fill in data about args for main function
        for (auto &treeArg : md.args) {
            auto *a = ast::MK::arg2Local(treeArg);
            rubyBlockArgs[0].emplace_back(cfg.enterLocal(a->localVariable));
        }
    }

    vector<vector<cfg::LocalRef>> argPresentVariables(cfg.maxRubyBlockId + 1);
    vector<BlockArity> rubyBlockArity(cfg.maxRubyBlockId + 1);

    // The method arguments are initialized here, while the block arguments are initialized when the blockCall header is
    // encountered in the loop below.
    int numArgs = md.symbol.data(cs)->arguments().size();
    argPresentVariables[0].resize(numArgs, cfg::LocalRef::noVariable());

    for (auto &b : cfg.basicBlocks) {
        if (b->bexit.cond.variable == cfg::LocalRef::blockCall()) {
            userEntryBlockByFunction[b->rubyBlockId] = llvmBlocks[b->bexit.thenb->id];
            basicBlockJumpOverrides[b->id] = b->bexit.elseb->id;
            auto backId = -1;
            for (auto bid = 0; bid < b->backEdges.size(); bid++) {
                if (b->backEdges[bid]->rubyBlockId < b->rubyBlockId) {
                    backId = bid;
                    break;
                };
            }
            ENFORCE(backId >= 0);

            cfg::InstructionPtr *expected = nullptr;
            for (auto i = b->backEdges[backId]->exprs.rbegin(); i != b->backEdges[backId]->exprs.rend(); ++i) {
                if (i->bind.variable.data(cfg)._name == core::Names::blockPreCallTemp()) {
                    expected = &i->value;
                    break;
                }
            }
            ENFORCE(expected);

            auto expectedSend = cfg::cast_instruction<cfg::Send>(*expected);
            ENFORCE(expectedSend);
            ENFORCE(expectedSend->link);
            blockLinks[b->rubyBlockId] = expectedSend->link;

            auto &argFlags = expectedSend->link->argFlags;
            auto numBlockArgs = argFlags.size();
            auto &blockArgs = rubyBlockArgs[b->rubyBlockId];
            blockArgs.resize(numBlockArgs, cfg::LocalRef::noVariable());
            collectRubyBlockArgs(cfg, b->bexit.thenb, blockArgs);

            auto &argsPresent = argPresentVariables[b->bexit.thenb->rubyBlockId];
            argsPresent.resize(numBlockArgs, cfg::LocalRef::noVariable());

            rubyBlockArity[b->rubyBlockId] = computeBlockArity(argFlags);

        } else if (b->bexit.cond.variable.data(cfg)._name == core::Names::exceptionValue()) {
            if (exceptionHandlingBlockHeaders[b->id] == 0) {
                continue;
            }

            auto *bodyBlock = b->bexit.elseb;
            auto *handlersBlock = b->bexit.thenb;

            // the relative block ids of blocks that are involved in the translation of an exception handling block.
            auto bodyBlockId = bodyBlock->rubyBlockId;
            auto handlersBlockId = bodyBlockId + cfg::CFG::HANDLERS_BLOCK_OFFSET;
            auto ensureBlockId = bodyBlockId + cfg::CFG::ENSURE_BLOCK_OFFSET;
            auto elseBlockId = bodyBlockId + cfg::CFG::ELSE_BLOCK_OFFSET;

            userEntryBlockByFunction[bodyBlockId] = llvmBlocks[bodyBlock->id];
            userEntryBlockByFunction[handlersBlockId] = llvmBlocks[handlersBlock->id];

            if (auto *elseBlock = CFGHelpers::findRegionEntry(cfg, elseBlockId)) {
                userEntryBlockByFunction[elseBlockId] = llvmBlocks[elseBlock->id];
            }

            if (auto *ensureBlock = CFGHelpers::findRegionEntry(cfg, ensureBlockId)) {
                userEntryBlockByFunction[ensureBlockId] = llvmBlocks[ensureBlock->id];
            }
        } else if (b->bexit.cond.variable.data(cfg)._name == core::Names::argPresent()) {
            // the ArgPresent instruction is always the last one generated in the block
            int argId = -1;
            auto &bind = b->exprs.back();
            typecase(
                bind.value,
                [&](cfg::ArgPresent &i) {
                    ENFORCE(bind.bind.variable == b->bexit.cond.variable);
                    argId = i.argId;
                },
                [&](cfg::YieldParamPresent &i) {
                    ENFORCE(bind.bind.variable == b->bexit.cond.variable);
                    argId = i.argId;
                });
            ENFORCE(argId >= 0, "Missing an index for argPresent condition variable");

            argPresentVariables[b->rubyBlockId][argId] = b->bexit.cond.variable;
        }
    }

    auto blockUsesBreak = blocksThatUseBreak(cs, cfg);
    llvm::BasicBlock *postProcessBlock = llvm::BasicBlock::Create(cs, "postProcess", mainFunc);

    IREmitterContext approximation{
        cfg,
        aliases,
        symbols,
        functionInitializersByFunction,
        argumentSetupBlocksByFunction,
        userEntryBlockByFunction,
        llvmBlocks,
        move(basicBlockJumpOverrides),
        move(basicBlockRubyBlockId),
        maxSendArgCount,
        move(sendArgArrayByBlock),
        std::move(escapedVariableIndices),
        std::move(argPresentVariables),
        {},
        {},
        postProcessBlock,
        move(blockLinks),
        move(rubyBlockArgs),
        move(rubyBlock2Function),
        move(blockTypes),
        move(blockParents),
        move(blockLevels),
        move(lineNumberPtrsByFunction),
        std::move(blockControlFramePtrs),
        usesBlockArgs,
        move(exceptionHandlingBlockHeaders),
        move(deadBlocks),
        move(blockExits),
        move(blockScopes),
        move(blockUsesBreak),
        hasReturnAcrossBlock,
        move(blockLocationNames),
        move(rubyBlockArity),
    };

    auto [llvmVariables, selfVariables] = setupLocalVariables(cs, cfg, variablesPrivateToBlocks, approximation);
    approximation.llvmVariables = std::move(llvmVariables);
    approximation.selfVariables = std::move(selfVariables);

    return approximation;
}

} // namespace sorbet::compiler
