#include "compiler/IRHelpers/IRHelpers.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType
#include "llvm/IR/IRBuilder.h"

// needef for LLVMIREmitterHelpers
#include "core/core.h"
// ^^^ violate our poisons
#include "LLVMIREmitterHelpers.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/sort.h"

using namespace std;
namespace sorbet::compiler {

UnorderedMap<core::LocalVariable, llvm::AllocaInst *>
setupLocalVariables(CompilerState &cs, cfg::CFG &cfg, vector<llvm::Function *> &rubyBlocks2Functions,
                    const UnorderedMap<core::LocalVariable, optional<int>> &variablesPrivateToBlocks,
                    const BasicBlockMap &blockMap, UnorderedMap<core::LocalVariable, Alias> &aliases) {
    UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables;
    llvm::IRBuilder<> builder(cs);
    {
        // nill out block local variables.
        auto valueType = cs.getValueType();
        vector<pair<core::LocalVariable, optional<int>>> variablesPrivateToBlocksSorted;

        for (const auto &entry : variablesPrivateToBlocks) {
            variablesPrivateToBlocksSorted.emplace_back(entry);
        }
        fast_sort(variablesPrivateToBlocksSorted,
                  [](const auto &left, const auto &right) -> bool { return left.first < right.first; });
        for (const auto &entry : variablesPrivateToBlocksSorted) {
            auto var = entry.first;
            if (entry.second == std::nullopt) {
                continue;
            }
            auto svName = var._name.data(cs)->shortName(cs);
            builder.SetInsertPoint(blockMap.functionInitializersByFunction[entry.second.value()]);
            auto alloca = llvmVariables[var] =
                builder.CreateAlloca(valueType, nullptr, llvm::StringRef(svName.data(), svName.length()));
            auto nilValueRaw = cs.getRubyNilRaw(builder);
            cs.boxRawValue(builder, alloca, nilValueRaw);
        }
    }

    {
        // nill out closure variables

        builder.SetInsertPoint(blockMap.functionInitializersByFunction[0]);
        auto escapedVariablesCount = blockMap.escapedVariableIndeces.size();
        for (auto i = 0; i < escapedVariablesCount; i++) {
            auto store =
                builder.CreateCall(cs.module->getFunction("sorbet_getClosureElem"),
                                   {blockMap.escapedClosure[0], llvm::ConstantInt::get(cs, llvm::APInt(32, i))});
            builder.CreateStore(cs.getRubyNilRaw(builder), store);
        }
    }
    return llvmVariables;
}

void trackBlockUsage(CompilerState &cs, cfg::CFG &cfg, core::LocalVariable lv, cfg::BasicBlock *bb,
                     UnorderedMap<core::LocalVariable, optional<int>> &privateUsages,
                     UnorderedMap<core::LocalVariable, int> &escapedIndexes, int &escapedIndexCounter

) {
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

/* if local variable is only used in block X, it maps the local variable to X, otherwise, it maps local variable to a
 * negative number */
pair<UnorderedMap<core::LocalVariable, optional<int>>, UnorderedMap<core::LocalVariable, int>>
findCaptures(CompilerState &cs, unique_ptr<ast::MethodDef> &mdef, cfg::CFG &cfg) {
    UnorderedMap<core::LocalVariable, optional<int>> ret;
    UnorderedMap<core::LocalVariable, int> escapedVariableIndexes;
    int idx = 0;
    for (auto &arg : mdef->args) {
        ast::Expression *maybeLocal = arg.get();
        if (auto *opt = ast::cast_tree<ast::OptionalArg>(arg.get())) {
            maybeLocal = opt->expr.get();
        }
        auto local = ast::cast_tree<ast::Local>(maybeLocal);
        ENFORCE(local);
        trackBlockUsage(cs, cfg, local->localVariable, cfg.entry(), ret, escapedVariableIndexes, idx);
    }

    for (auto &bb : cfg.basicBlocks) {
        for (cfg::Binding &bind : bb->exprs) {
            trackBlockUsage(cs, cfg, bind.bind.variable, bb.get(), ret, escapedVariableIndexes, idx);
            typecase(
                bind.value.get(),
                [&](cfg::Ident *i) { trackBlockUsage(cs, cfg, i->what, bb.get(), ret, escapedVariableIndexes, idx); },
                [&](cfg::Alias *i) { /* nothing */
                },
                [&](cfg::SolveConstraint *i) { /* nothing*/ },
                [&](cfg::Send *i) {
                    for (auto &arg : i->args) {
                        trackBlockUsage(cs, cfg, arg.variable, bb.get(), ret, escapedVariableIndexes, idx);
                    }
                    trackBlockUsage(cs, cfg, i->recv.variable, bb.get(), ret, escapedVariableIndexes, idx);
                },
                [&](cfg::Return *i) {
                    trackBlockUsage(cs, cfg, i->what.variable, bb.get(), ret, escapedVariableIndexes, idx);
                },
                [&](cfg::BlockReturn *i) {
                    trackBlockUsage(cs, cfg, i->what.variable, bb.get(), ret, escapedVariableIndexes, idx);
                },
                [&](cfg::LoadSelf *i) { /*nothing*/ /*todo: how does instance exec pass self?*/ },
                [&](cfg::Literal *i) { /* nothing*/ }, [&](cfg::Unanalyzable *i) { /*nothing*/ },
                [&](cfg::LoadArg *i) { /*nothing*/ }, [&](cfg::LoadYieldParams *i) { /*nothing*/ },
                [&](cfg::Cast *i) {
                    trackBlockUsage(cs, cfg, i->value.variable, bb.get(), ret, escapedVariableIndexes, idx);
                },
                [&](cfg::TAbsurd *i) {
                    trackBlockUsage(cs, cfg, i->what.variable, bb.get(), ret, escapedVariableIndexes, idx);
                });
        }
    }
    return {std::move(ret), std::move(escapedVariableIndexes)};
}

int getMaxSendArgCount(cfg::CFG &cfg) {
    int maxSendArgCount = 0;
    for (auto &bb : cfg.basicBlocks) {
        for (cfg::Binding &bind : bb->exprs) {
            if (auto snd = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
                if (maxSendArgCount < snd->args.size()) {
                    maxSendArgCount = snd->args.size();
                }
            }
        }
    }
    return maxSendArgCount;
}

vector<llvm::Function *> getRubyBlocks2FunctionsMapping(CompilerState &cs, cfg::CFG &cfg, llvm::Function *func) {
    vector<llvm::Function *> res;
    res.emplace_back(func);
    llvm::Type *args[] = {
        llvm::Type::getInt64Ty(cs),    // first yielded argument(first argument is both here and in argArray
        llvm::Type::getInt64Ty(cs),    // data
        llvm::Type::getInt32Ty(cs),    // arg count
        llvm::Type::getInt64PtrTy(cs), // argArray
        llvm::Type::getInt64Ty(cs),    // blockArg
    };
    auto ft = llvm::FunctionType::get(llvm::Type::getInt64Ty(cs), args, false /*not varargs*/);

    for (int i = 1; i <= cfg.maxRubyBlockId; i++) {
        auto fp = llvm::Function::Create(ft, llvm::Function::InternalLinkage,
                                         llvm::Twine{func->getName()} + "$block_" + llvm::Twine(i), *cs.module);
        {
            // setup argument names
            // setup function argument names
            fp->arg_begin()->setName("firstYieldArgRaw");
            (fp->arg_begin() + 1)->setName("captures");
            (fp->arg_begin() + 2)->setName("argc");
            (fp->arg_begin() + 3)->setName("argArray");
            (fp->arg_begin() + 4)->setName("blockArg");
        }
        res.emplace_back(fp);
    }
    return res;
};

BasicBlockMap LLVMIREmitterHelpers::getSorbetBlocks2LLVMBlockMapping(CompilerState &cs, cfg::CFG &cfg,
                                                                     unique_ptr<ast::MethodDef> &md,
                                                                     UnorderedMap<core::LocalVariable, Alias> &aliases,
                                                                     llvm::Function *mainFunc) {
    vector<llvm::Function *> rubyBlocks2Functions = getRubyBlocks2FunctionsMapping(cs, cfg, mainFunc);
    const int maxSendArgCount = getMaxSendArgCount(cfg);
    auto [variablesPrivateToBlocks, escapedVariableIndices] = findCaptures(cs, md, cfg);
    vector<llvm::BasicBlock *> functionInitializersByFunction;
    vector<llvm::BasicBlock *> argumentSetupBlocksByFunction;
    vector<llvm::BasicBlock *> userEntryBlockByFunction(rubyBlocks2Functions.size());
    vector<llvm::AllocaInst *> sendArgArrays;
    vector<llvm::Value *> escapedClosure;
    vector<int> basicBlockJumpOverrides(cfg.maxBasicBlockId);
    llvm::IRBuilder<> builder(cs);
    {
        for (int i = 0; i < cfg.maxBasicBlockId; i++) {
            basicBlockJumpOverrides[i] = i;
        }
    }
    int i = 0;
    for (auto &fun : rubyBlocks2Functions) {
        auto inits = functionInitializersByFunction.emplace_back(llvm::BasicBlock::Create(
            cs, "functionEntryInitializers",
            fun)); // we will build a link for this block later, after we finish building expressions into it
        builder.SetInsertPoint(inits);
        auto sendArgArray = builder.CreateAlloca(llvm::ArrayType::get(llvm::Type::getInt64Ty(cs), maxSendArgCount),
                                                 nullptr, "callArgs");
        llvm::Value *localClosure = nullptr;
        if (i == 0) {
            if (!escapedVariableIndices.empty())
                localClosure =
                    builder.CreateCall(cs.module->getFunction("sorbet_allocClosureAsValue"),
                                       {llvm::ConstantInt::get(cs, llvm::APInt(32, escapedVariableIndices.size()))});
            else {
                localClosure = cs.getRubyNilRaw(builder);
            }
        } else {
            localClosure = fun->arg_begin() + 1;
        }
        escapedClosure.emplace_back(localClosure);
        sendArgArrays.emplace_back(sendArgArray);
        argumentSetupBlocksByFunction.emplace_back(llvm::BasicBlock::Create(cs, "argumentSetup", fun));
        i++;
    }

    llvm::BasicBlock *sigVerificationBlock = llvm::BasicBlock::Create(cs, "checkSig", rubyBlocks2Functions[0]);

    vector<llvm::BasicBlock *> llvmBlocks(cfg.maxBasicBlockId);
    for (auto &b : cfg.basicBlocks) {
        if (b.get() == cfg.entry()) {
            llvmBlocks[b->id] = userEntryBlockByFunction[0] =
                llvm::BasicBlock::Create(cs, "userEntry", rubyBlocks2Functions[0]);
        } else {
            llvmBlocks[b->id] = llvm::BasicBlock::Create(cs, llvm::Twine("BB") + llvm::Twine(b->id),
                                                         rubyBlocks2Functions[b->rubyBlockId]);
        }
    }
    vector<shared_ptr<core::SendAndBlockLink>> blockLinks(rubyBlocks2Functions.size());
    vector<vector<core::LocalVariable>> rubyBlockArgs(rubyBlocks2Functions.size());

    {
        // fill in data about args for main function
        for (auto &treeArg : md->args) {
            auto *a = ast::MK::arg2Local(treeArg.get());
            rubyBlockArgs[0].emplace_back(a->localVariable);
        }
    }

    for (auto &b : cfg.basicBlocks) {
        if (b->bexit.cond.variable == core::LocalVariable::blockCall()) {
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
            auto &expectedSendBind = b->backEdges[backId]->exprs[b->backEdges[backId]->exprs.size() - 2];
            auto expectedSend = cfg::cast_instruction<cfg::Send>(expectedSendBind.value.get());
            ENFORCE(expectedSend);
            ENFORCE(expectedSend->link);
            blockLinks[b->rubyBlockId] = expectedSend->link;

            rubyBlockArgs[b->rubyBlockId].resize(expectedSend->link->argFlags.size(),
                                                 core::LocalVariable::noVariable());
            for (auto &maybeCallOnLoadYieldArg : b->bexit.thenb->exprs) {
                auto maybeCast = cfg::cast_instruction<cfg::Send>(maybeCallOnLoadYieldArg.value.get());
                if (maybeCast == nullptr || maybeCast->recv.variable._name != core::Names::blkArg() ||
                    maybeCast->fun != core::Names::squareBrackets() || maybeCast->args.size() != 1) {
                    continue;
                }
                auto litType = core::cast_type<core::LiteralType>(maybeCast->args[0].type.get());
                if (litType == nullptr) {
                    continue;
                }
                rubyBlockArgs[b->rubyBlockId][litType->value] = maybeCallOnLoadYieldArg.bind.variable;
            }
        }
    }

    auto approximation = BasicBlockMap{cfg.symbol,
                                       functionInitializersByFunction,
                                       argumentSetupBlocksByFunction,
                                       userEntryBlockByFunction,
                                       llvmBlocks,
                                       move(basicBlockJumpOverrides),
                                       move(sendArgArrays),
                                       escapedClosure,
                                       std::move(escapedVariableIndices),
                                       sigVerificationBlock,
                                       move(blockLinks),
                                       move(rubyBlockArgs),
                                       {},
                                       rubyBlocks2Functions};
    approximation.llvmVariables =
        setupLocalVariables(cs, cfg, rubyBlocks2Functions, variablesPrivateToBlocks, approximation, aliases);

    return approximation;
}

} // namespace sorbet::compiler
