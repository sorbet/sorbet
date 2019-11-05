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

BasicBlockMap LLVMIREmitterHelpers::getSorbetBlocks2LLVMBlockMapping(
    CompilerState &cs, cfg::CFG &cfg, unique_ptr<ast::MethodDef> &md, vector<llvm::Function *> rubyBlocks2Functions,
    UnorderedMap<core::LocalVariable, int> &&escapedVariableIndices, int maxSendArgCount,
    const UnorderedMap<core::LocalVariable, optional<int>> &variablesPrivateToBlocks,
    UnorderedMap<core::LocalVariable, Alias> &aliases) {
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
                                       {}};
    approximation.llvmVariables =
        setupLocalVariables(cs, cfg, rubyBlocks2Functions, variablesPrivateToBlocks, approximation, aliases);

    return approximation;
}

vector<llvm::Function *> LLVMIREmitterHelpers::getRubyBlocks2FunctionsMapping(CompilerState &cs, cfg::CFG &cfg,
                                                                              llvm::Function *func) {
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
} // namespace sorbet::compiler
