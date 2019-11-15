#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
// ^^^ violate our poisons so they go first
#include "absl/synchronization/mutex.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "compiler/DefinitionRewriter/DefinitionRewriter.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IRHelpers/IRHelpers.h"
#include "compiler/LLVMIREmitter/LLVMIREmitter.h"
#include "compiler/ObjectFileEmitter/ObjectFileEmitter.h"
#include "core/ErrorQueue.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include <cxxopts.hpp>
#include <optional>

using namespace std;
namespace sorbet::pipeline::semantic_extension {
namespace {
string objectFileName(const core::GlobalState &gs, const core::FileRef &f) {
    string sourceFile(f.data(gs).path());
    absl::c_replace(sourceFile, '/', '_');
    absl::c_replace(sourceFile, '.', '_');
    sourceFile.replace(sourceFile.find("_rb"), 3, ".rb");
    return sourceFile;
}
} // namespace

class ThreadState {
public:
    llvm::LLVMContext lctx;
    unique_ptr<llvm::Module> combinedModule;
    core::FileRef file;
    bool aborted = false;
};

class LLVMSemanticExtension : public SemanticExtension {
    optional<string> irOutputDir;
    mutable struct {
        UnorderedMap<std::thread::id, shared_ptr<ThreadState>> states;
        absl::Mutex mtx;
    } mutableState;

    shared_ptr<ThreadState> getThreadState() const {
        {
            absl::ReaderMutexLock lock(&mutableState.mtx);
            if (mutableState.states.contains(std::this_thread::get_id())) {
                return mutableState.states.at(std::this_thread::get_id());
            }
        }
        {
            absl::WriterMutexLock lock(&mutableState.mtx);
            return mutableState.states[std::this_thread::get_id()] = make_shared<ThreadState>();
        }
    }

public:
    LLVMSemanticExtension(optional<string> irOutputDir) {
        this->irOutputDir = move(irOutputDir);
    }

    virtual void finishTypecheckFile(const core::GlobalState &gs, const core::FileRef &f) const override {
        if (!irOutputDir.has_value()) {
            return;
        }
        auto threadState = getThreadState();
        llvm::LLVMContext &lctx = threadState->lctx;
        unique_ptr<llvm::Module> module = move(threadState->combinedModule);
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
        if (f.data(gs).minErrorLevel() >= core::StrictLevel::True) {
            string fileName = objectFileName(gs, f);
            sorbet::compiler::ObjectFileEmitter::run(lctx, move(module), irOutputDir.value(), fileName);
        }
        ENFORCE(threadState->combinedModule == nullptr);
        threadState->file = core::FileRef();
    };

    virtual void typecheck(const core::GlobalState &gs, cfg::CFG &cfg,
                           std::unique_ptr<ast::MethodDef> &md) const override {
        if (!irOutputDir.has_value()) {
            return;
        }
        auto threadState = getThreadState();
        if (threadState->aborted) {
            return;
        }
        llvm::LLVMContext &lctx = threadState->lctx;
        string functionName = cfg.symbol.data(gs)->toStringFullName(gs);
        unique_ptr<llvm::Module> &module = threadState->combinedModule;
        // TODO: Figure out why this isn't true
        // ENFORCE(absl::c_find(cfg.symbol.data(gs)->locs(), md->loc) != cfg.symbol.data(gs)->locs().end(),
        // md->loc.toString(gs));
        ENFORCE(md->loc.file().exists());
        if (!module) {
            module = sorbet::compiler::IRHelpers::readDefaultModule(functionName.data(), lctx);
            threadState->file = md->loc.file();
        } else {
            ENFORCE(threadState->file == md->loc.file());
        }
        ENFORCE(threadState->file.exists());
        compiler::CompilerState state(gs, lctx, module.get());
        try {
            sorbet::compiler::LLVMIREmitter::run(state, cfg, md, functionName);
            string fileName = objectFileName(gs, cfg.symbol.data(gs)->loc().file());
            sorbet::compiler::LLVMIREmitter::buildInitFor(state, cfg.symbol, fileName);
        } catch (sorbet::compiler::AbortCompilation &) {
            threadState->aborted = true;
        } catch (...) {
            // cleanup
            module = nullptr;
            threadState->file = core::FileRef();
            throw;
        }
    };

    virtual void run(core::MutableContext &ctx, ast::ClassDef *klass) const override {
        if (!irOutputDir.has_value()) {
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

        sorbet::compiler::DefinitionRewriter::run(ctx, klass);
    };

    virtual ~LLVMSemanticExtension(){};
    virtual std::unique_ptr<SemanticExtension> deepCopy(const core::GlobalState &from, core::GlobalState &to) override {
        return make_unique<LLVMSemanticExtension>(this->irOutputDir);
    };
    virtual void merge(const core::GlobalState &from, core::GlobalState &to, core::GlobalSubstitution &subst) override {
    }
};

class LLVMSemanticExtensionProvider : public SemanticExtensionProvider {
public:
    virtual void injectOptions(cxxopts::Options &optsBuilder) const override {
        optsBuilder.add_options("lvm")("llvm-ir-folder", "Output LLVM IR to directory", cxxopts::value<string>());
    };
    virtual std::unique_ptr<SemanticExtension> readOptions(cxxopts::ParseResult &providedOptions) const override {
        optional<string> irOutputDir;
        if (providedOptions.count("llvm-ir-folder") > 0) {
            irOutputDir = providedOptions["llvm-ir-folder"].as<string>();
        }
        return make_unique<LLVMSemanticExtension>(irOutputDir);
    };
    virtual ~LLVMSemanticExtensionProvider(){};
};

vector<SemanticExtensionProvider *> SemanticExtensionProvider::getProviders() {
    static LLVMSemanticExtensionProvider provider;
    return {&provider};
}
} // namespace sorbet::pipeline::semantic_extension
