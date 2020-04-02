// These violate our poisons so have to happen first
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "absl/synchronization/mutex.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "compiler/Core/AbortCompilation.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IREmitter/IREmitter.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/Payload/PayloadLoader.h"
#include "compiler/ObjectFileEmitter/ObjectFileEmitter.h"
#include "compiler/Rewriters/DefinitionRewriter.h"
#include "compiler/Rewriters/SigRewriter.h"
#include "core/ErrorQueue.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include <cxxopts.hpp>
#include <optional>

using namespace std;
namespace sorbet::pipeline::semantic_extension {
namespace {
string objectFileName(const core::GlobalState &gs, const core::FileRef &f) {
    string sourceFile(f.data(gs).path());
    if (sourceFile[0] == '.' && sourceFile[1] == '/') {
        sourceFile = sourceFile.substr(2);
    }
    absl::c_replace(sourceFile, '/', '_');
    absl::c_replace(sourceFile, '.', '_');
    sourceFile.replace(sourceFile.find("_rb"), 3, ".rb");
    return sourceFile;
}
} // namespace

// Sorbet's pipeline is architected such that one thread is typechecking one file at a time.
// This struct allows us to store state local to one typechecking thread.
class TypecheckThreadState {
public:
    // Creating an LLVMContext is somewhat expensive, so we don't want to create more than one per thread.
    llvm::LLVMContext lctx;

    // The file this thread is currently typechecking
    core::FileRef file;

    // The output of IREmitter for a file.
    //
    // "Combined" because all the methods in this file get compiled and accumulated into this Module,
    // but each method is typechecked (and thus compiled) individually.
    unique_ptr<llvm::Module> combinedModule;

    // The function that holds calls to global constructors
    //
    // This works as a replacement to llvm.global_ctors so that we can delay initialization until after
    // our sorbet_ruby version check.
    llvm::BasicBlock *globalConstructorsEntry;

    bool aborted = false;
};

class LLVMSemanticExtension : public SemanticExtension {
    optional<string> irOutputDir;
    bool forceCompiled;
    mutable struct {
        UnorderedMap<std::thread::id, shared_ptr<TypecheckThreadState>> states;
        absl::Mutex mtx;
    } mutableState;

    shared_ptr<TypecheckThreadState> getTypecheckThreadState() const {
        {
            absl::ReaderMutexLock lock(&mutableState.mtx);
            if (mutableState.states.contains(std::this_thread::get_id())) {
                return mutableState.states.at(std::this_thread::get_id());
            }
        }
        {
            absl::WriterMutexLock lock(&mutableState.mtx);
            return mutableState.states[std::this_thread::get_id()] = make_shared<TypecheckThreadState>();
        }
    }

    bool shouldCompile(const core::GlobalState &gs, const core::FileRef &f) const {
        if (!irOutputDir.has_value()) {
            return false;
        }
        if (forceCompiled) {
            return true;
        }
        // TODO parse this the same way as `typed:`
        return f.data(gs).source().find("# compiled: true\n") != string_view::npos;
    }

public:
    LLVMSemanticExtension(optional<string> irOutputDir, bool forceCompiled) {
        this->irOutputDir = move(irOutputDir);
        this->forceCompiled = forceCompiled;
    }

    virtual void run(core::MutableContext &ctx, ast::ClassDef *klass) const override {
        if (!shouldCompile(ctx, klass->loc.file())) {
            return;
        }
        if (klass->loc.file().data(ctx).strictLevel < core::StrictLevel::True) {
            if (auto e = ctx.state.beginError(klass->loc, core::errors::Compiler::Untyped)) {
                e.setHeader("File must be `typed: true` or higher to be compiled");
            }
        }
        if (!ast::isa_tree<ast::EmptyTree>(klass->name.get())) {
            return;
        }

        compiler::DefinitionRewriter::run(ctx, klass);
        compiler::SigRewriter::run(ctx, klass);
    };

    virtual void typecheck(const core::GlobalState &gs, cfg::CFG &cfg,
                           std::unique_ptr<ast::MethodDef> &md) const override {
        auto loc = md->declLoc;
        if (!shouldCompile(gs, loc.file())) {
            return;
        }
        auto threadState = getTypecheckThreadState();
        if (threadState->aborted) {
            return;
        }
        llvm::LLVMContext &lctx = threadState->lctx;
        unique_ptr<llvm::Module> &module = threadState->combinedModule;
        // TODO: Figure out why this isn't true
        // ENFORCE(absl::c_find(cfg.symbol.data(gs)->locs(), md->loc) != cfg.symbol.data(gs)->locs().end(),
        // loc.toString(gs));
        ENFORCE(loc.file().exists());
        if (!module) {
            ENFORCE(threadState->globalConstructorsEntry == nullptr);
            module = compiler::PayloadLoader::readDefaultModule(lctx);
            threadState->file = loc.file();

            auto linkageType = llvm::Function::InternalLinkage;
            auto noArgs = std::vector<llvm::Type *>(0, llvm::Type::getVoidTy(lctx));
            auto varArgs = false;
            auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(lctx), noArgs, varArgs);
            auto globalConstructors = llvm::Function::Create(ft, linkageType, "sorbet_globalConstructors", *module);
            threadState->globalConstructorsEntry = llvm::BasicBlock::Create(lctx, "entry", globalConstructors);
        } else {
            ENFORCE(threadState->file == loc.file());
            ENFORCE(threadState->globalConstructorsEntry != nullptr);
        }
        ENFORCE(threadState->file.exists());
        compiler::CompilerState state(gs, lctx, module.get(), threadState->globalConstructorsEntry);
        try {
            compiler::IREmitter::run(state, cfg, md);
            string fileName = objectFileName(gs, loc.file());
            compiler::IREmitter::buildInitFor(state, cfg.symbol, fileName);
        } catch (sorbet::compiler::AbortCompilation &) {
            threadState->aborted = true;
        } catch (...) {
            // cleanup
            module = nullptr;
            threadState->file = core::FileRef();
            throw;
        }
    };

    virtual void finishTypecheckFile(const core::GlobalState &gs, const core::FileRef &f) const override {
        if (!shouldCompile(gs, f)) {
            return;
        }
        auto threadState = getTypecheckThreadState();
        llvm::LLVMContext &lctx = threadState->lctx;

        llvm::IRBuilder<> builder(lctx);
        builder.SetInsertPoint(threadState->globalConstructorsEntry);
        builder.CreateRetVoid();

        unique_ptr<llvm::Module> module = move(threadState->combinedModule);
        threadState->globalConstructorsEntry = nullptr;

        if (!module) {
            ENFORCE(!threadState->file.exists());
            return;
        }
        if (threadState->aborted) {
            threadState->file = core::FileRef();
            return;
        }

        ENFORCE(threadState->file.exists());
        ENFORCE(f == threadState->file);

        ENFORCE(threadState->combinedModule == nullptr);
        ENFORCE(threadState->globalConstructorsEntry == nullptr);
        threadState->file = core::FileRef();

        compiler::CompilerState cs(gs, lctx, module.get(), nullptr);
        if (f.data(gs).minErrorLevel() >= core::StrictLevel::True) {
            if (f.data(gs).source().find("frozen_string_literal: true"sv) == string_view::npos) {
                cs.failCompilation(core::Loc(f, 0, 0), "Compiled files need to have '# frozen_string_literal: true'");
            }
            string fileName = objectFileName(gs, f);
            compiler::ObjectFileEmitter::run(gs.tracer(), lctx, move(module), irOutputDir.value(), fileName);
        }
    };

    virtual void finishTypecheck(const core::GlobalState &gs) const override {}

    virtual ~LLVMSemanticExtension(){};
    virtual std::unique_ptr<SemanticExtension> deepCopy(const core::GlobalState &from, core::GlobalState &to) override {
        return make_unique<LLVMSemanticExtension>(this->irOutputDir, this->forceCompiled);
    };
    virtual void merge(const core::GlobalState &from, core::GlobalState &to, core::GlobalSubstitution &subst) override {
    }
};

class LLVMSemanticExtensionProvider : public SemanticExtensionProvider {
public:
    virtual void injectOptions(cxxopts::Options &optsBuilder) const override {
        optsBuilder.add_options("compiler")("llvm-ir-folder", "Output LLVM IR to directory", cxxopts::value<string>());
        optsBuilder.add_options("compiler")("force-compiled", "Force all files to this compiled level",
                                            cxxopts::value<bool>());
    };
    virtual std::unique_ptr<SemanticExtension> readOptions(cxxopts::ParseResult &providedOptions) const override {
        optional<string> irOutputDir;
        bool forceCompiled = false;
        if (providedOptions.count("llvm-ir-folder") > 0) {
            irOutputDir = providedOptions["llvm-ir-folder"].as<string>();
        }
        if (providedOptions.count("force-compiled") > 0) {
            forceCompiled = providedOptions["force-compiled"].as<bool>();
        }
        return make_unique<LLVMSemanticExtension>(irOutputDir, forceCompiled);
    };
    virtual std::unique_ptr<SemanticExtension> defaultInstance() const override {
        auto forceCompile = false;
        optional<string> irOutputDir;
        return make_unique<LLVMSemanticExtension>(irOutputDir, forceCompile);
    }
    virtual ~LLVMSemanticExtensionProvider(){};
};

vector<SemanticExtensionProvider *> SemanticExtensionProvider::getProviders() {
    static LLVMSemanticExtensionProvider provider;
    return {&provider};
}
} // namespace sorbet::pipeline::semantic_extension
