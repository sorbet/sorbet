// These violate our poisons so have to happen first
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "absl/strings/str_split.h"
#include "absl/synchronization/mutex.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/FileOps.h"
#include "compiler/Core/AbortCompilation.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IREmitter/IREmitter.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/Payload/PayloadLoader.h"
#include "compiler/Names/Names.h"
#include "compiler/ObjectFileEmitter/ObjectFileEmitter.h"
#include "core/ErrorQueue.h"
#include "main/options/options.h"
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
    return sourceFile;
}

void ensureOutputDir(const string_view irOutputDir, string_view fileName) {
    string path(irOutputDir);

    auto finalSlashPos = fileName.rfind('/');
    if (finalSlashPos == string_view::npos) {
        return;
    }

    // Trim the filename so that we only iterate directory parts below
    fileName.remove_suffix(fileName.size() - finalSlashPos);

    for (auto part : absl::StrSplit(fileName, '/')) {
        absl::StrAppend(&path, "/", part);
        FileOps::ensureDir(path);
    }
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

    unique_ptr<llvm::DIBuilder> debugInfo;
    llvm::DICompileUnit *compileUnit = nullptr;

    // The basic-block that holds the initialization of string constants.
    llvm::BasicBlock *allocRubyIdsEntry;

    // The basic-block that holds the initialization of static-init constants.
    llvm::BasicBlock *initializeStaticInitNamesEntry;

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
        return isCompiledTrue(gs, f);
    }

    bool isCompiledTrue(const core::GlobalState &gs, const core::FileRef &f) const {
        // TODO parse this the same way as `typed:`
        return f.data(gs).source().find("# compiled: true\n") != string_view::npos;
    }

public:
    LLVMSemanticExtension(optional<string> irOutputDir, bool forceCompiled) {
        this->irOutputDir = move(irOutputDir);
        this->forceCompiled = forceCompiled;
    }

    virtual void run(core::MutableContext &ctx, ast::ClassDef *klass) const override {
        compiler::Names::init(ctx);
    };

    virtual void typecheck(const core::GlobalState &gs, cfg::CFG &cfg, ast::MethodDef &md) const override {
        if (!shouldCompile(gs, cfg.file)) {
            return;
        }

        // Don't emit bodies for abstract methods.
        if (md.symbol.data(gs)->isAbstract()) {
            return;
        }

        if (md.symbol.data(gs)->name == core::Names::staticInit()) {
            auto attachedClass = md.symbol.data(gs)->owner.data(gs)->attachedClass(gs);
            if (attachedClass.exists() && attachedClass.data(gs)->name.data(gs)->isTEnumName(gs)) {
                return;
            }
        }

        auto threadState = getTypecheckThreadState();
        if (threadState->aborted) {
            return;
        }
        llvm::LLVMContext &lctx = threadState->lctx;
        unique_ptr<llvm::Module> &module = threadState->combinedModule;
        unique_ptr<llvm::DIBuilder> &debug = threadState->debugInfo;
        llvm::DICompileUnit *&compUnit = threadState->compileUnit;
        // TODO: Figure out why this isn't true
        // ENFORCE(absl::c_find(cfg.symbol.data(gs)->locs(), md->loc) != cfg.symbol.data(gs)->locs().end(),
        // loc.toString(gs));
        ENFORCE(cfg.file.exists());
        if (!module) {
            ENFORCE(threadState->globalConstructorsEntry == nullptr);
            ENFORCE(debug == nullptr);
            ENFORCE(compUnit == nullptr);
            module = compiler::PayloadLoader::readDefaultModule(lctx);

            module->addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);

            if (llvm::Triple(llvm::sys::getProcessTriple()).isOSDarwin()) {
                // osx only supports dwarf2
                module->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 2);
            }

            debug = llvm::make_unique<llvm::DIBuilder>(*module);

            // NOTE: we use C here because our generated functions follow its abi
            auto language = llvm::dwarf::DW_LANG_C;
            auto filename = cfg.file.data(gs).path();
            auto isOptimized = false;
            auto runtimeVersion = 0;
            compUnit = debug->createCompileUnit(
                language, debug->createFile(llvm::StringRef(filename.data(), filename.size()), "."), "Sorbet LLVM",
                isOptimized, "", runtimeVersion);

            threadState->file = cfg.file;

            {
                auto linkageType = llvm::Function::InternalLinkage;
                auto argTys = std::vector<llvm::Type *>{llvm::Type::getInt64Ty(lctx)};
                auto varArgs = false;
                auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(lctx), argTys, varArgs);
                auto globalConstructors = llvm::Function::Create(ft, linkageType, "sorbet_globalConstructors", *module);
                threadState->allocRubyIdsEntry = llvm::BasicBlock::Create(lctx, "allocRubyIds", globalConstructors);
                threadState->initializeStaticInitNamesEntry =
                    llvm::BasicBlock::Create(lctx, "initializeStaticInitNames", globalConstructors);
                threadState->globalConstructorsEntry =
                    llvm::BasicBlock::Create(lctx, "globalConstructors", globalConstructors);
            }

        } else {
            ENFORCE(threadState->file == cfg.file);
            ENFORCE(threadState->globalConstructorsEntry != nullptr);
        }
        ENFORCE(threadState->file.exists());
        compiler::CompilerState state(gs, lctx, module.get(), debug.get(), compUnit, threadState->file,
                                      threadState->allocRubyIdsEntry, threadState->initializeStaticInitNamesEntry,
                                      threadState->globalConstructorsEntry);
        try {
            compiler::IREmitter::run(state, cfg, md);
            string fileName = objectFileName(gs, cfg.file);
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

        {
            llvm::IRBuilder<> builder(lctx);

            builder.SetInsertPoint(threadState->allocRubyIdsEntry);
            builder.CreateBr(threadState->initializeStaticInitNamesEntry);

            builder.SetInsertPoint(threadState->initializeStaticInitNamesEntry);
            builder.CreateBr(threadState->globalConstructorsEntry);

            builder.SetInsertPoint(threadState->globalConstructorsEntry);
            builder.CreateRetVoid();
        }

        unique_ptr<llvm::Module> module = move(threadState->combinedModule);
        unique_ptr<llvm::DIBuilder> debug = move(threadState->debugInfo);
        llvm::DICompileUnit *compileUnit = threadState->compileUnit;
        threadState->compileUnit = nullptr;
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

        debug->finalize();

        compiler::CompilerState cs(gs, lctx, module.get(), debug.get(), compileUnit, f, nullptr, nullptr, nullptr);
        if (f.data(gs).minErrorLevel() >= core::StrictLevel::True) {
            if (f.data(gs).source().find("frozen_string_literal: true"sv) == string_view::npos) {
                cs.failCompilation(core::Loc(f, 0, 0), "Compiled files need to have '# frozen_string_literal: true'");
            }
            string fileName = objectFileName(gs, f);
            ensureOutputDir(irOutputDir.value(), fileName);
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
        if (providedOptions["version"].as<bool>()) {
            fmt::print("Sorbet compiler {}\n", sorbet_full_version_string);
            throw realmain::options::EarlyReturnWithCode(0);
        }

        optional<string> irOutputDir;
        bool forceCompiled = false;
        if (providedOptions.count("llvm-ir-folder") > 0) {
            auto outputDir = providedOptions["llvm-ir-folder"].as<string>();

            if (!FileOps::dirExists(outputDir)) {
                fmt::print("Missing output directory {}\n", outputDir);
                throw realmain::options::EarlyReturnWithCode(1);
            }

            irOutputDir = outputDir;
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
