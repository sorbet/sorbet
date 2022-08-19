// These violate our poisons so have to happen first
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Host.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "absl/cleanup/cleanup.h"
#include "absl/strings/str_split.h"
#include "absl/synchronization/mutex.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/FileOps.h"
#include "common/typecase.h"
#include "compiler/Core/AbortCompilation.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/Core/FailCompilation.h"
#include "compiler/Core/OptimizerException.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IREmitter/IREmitter.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/Payload/PayloadLoader.h"
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

void ensureOutputDir(const string_view outputDir, string_view fileName) {
    string path(outputDir);

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
    llvm::BasicBlock *allocRubyIdsEntry = nullptr;

    compiler::StringTable stringTable;

    compiler::IDTable idTable;

    compiler::RubyStringTable rubyStringTable;

    // The function that holds calls to global constructors
    //
    // This works as a replacement to llvm.global_ctors so that we can delay initialization until after
    // our sorbet_ruby version check.
    llvm::BasicBlock *globalConstructorsEntry = nullptr;

    bool aborted = false;

    const unique_ptr<const llvm::Module> codegenPayload;

    TypecheckThreadState() : codegenPayload(compiler::PayloadLoader::readDefaultModule(lctx)) {}
};

class LLVMSemanticExtension : public SemanticExtension {
    optional<string> compiledOutputDir;
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
        if (!compiledOutputDir.has_value()) {
            return false;
        }
        if (forceCompiled) {
            return true;
        }
        // TODO parse this the same way as `typed:`
        return isCompiledTrue(gs, f);
    }

    bool isCompiledTrue(const core::GlobalState &gs, const core::FileRef &f) const {
        return f.data(gs).compiledLevel == core::CompiledLevel::True;
    }

    // There are a certain class of method calls that sorbet generates for auxiliary
    // information for IDEs that do not have meaning at runtime.  These calls are all
    // of the form `foo(bar(baz))`, i.e. straight line code with no variables.
    // We take advantage of this special knowledge to do a simple form of dead code
    // elimination.
    void deleteDoNothingSends(cfg::CFG &cfg) const {
        for (auto &block : cfg.basicBlocks) {
            UnorderedSet<cfg::LocalRef> refsToDelete;
            for (auto i = block->exprs.rbegin(), e = block->exprs.rend(); i != e; ++i) {
                auto &binding = *i;
                if (auto *send = cfg::cast_instruction<cfg::Send>(binding.value)) {
                    switch (send->fun.rawId()) {
                        case core::Names::keepForIde().rawId():
                            // TODO: figure out why we can't delete this.
                            // case core::Names::keepForCfg().rawId():
                            refsToDelete.emplace(binding.bind.variable);
                            break;
                        default:
                            if (!refsToDelete.contains(binding.bind.variable)) {
                                continue;
                            }
                            break;
                    }

                    // We're binding a ref that is unneeded, so anything that this
                    // instruction requires must be unneeded as well.
                    for (auto &arg : send->args) {
                        refsToDelete.emplace(arg.variable);
                    }
                    refsToDelete.emplace(send->recv.variable);
                } else if (auto *read = cfg::cast_instruction<cfg::KeepAlive>(binding.value)) {
                    refsToDelete.emplace(read->what);
                }
            }

            auto e = std::remove_if(block->exprs.begin(), block->exprs.end(),
                                    [&](auto &binding) { return refsToDelete.contains(binding.bind.variable); });
            block->exprs.erase(e, block->exprs.end());
        }
    }

public:
    LLVMSemanticExtension(optional<string> compiledOutputDir, optional<string> irOutputDir, bool forceCompiled) {
        this->compiledOutputDir = move(compiledOutputDir);
        this->irOutputDir = move(irOutputDir);
        this->forceCompiled = forceCompiled;
    }

    virtual void run(core::MutableContext &ctx, ast::ClassDef *klass) const override{};

    virtual void typecheck(const core::GlobalState &gs, core::FileRef file, cfg::CFG &cfg,
                           ast::MethodDef &md) const override {
        if (!shouldCompile(gs, file)) {
            return;
        }

        // This method will be handled as a VM_METHOD_TYPE_IVAR method by the
        // standard VM mechanisms, so we don't need to generate code for it.
        if (md.flags.isAttrReader && !md.symbol.data(gs)->flags.isFinal) {
            return;
        }

        if (md.symbol.data(gs)->name == core::Names::staticInit()) {
            auto attachedClass = md.symbol.data(gs)->owner.data(gs)->attachedClass(gs);
            if (attachedClass.exists() && attachedClass.data(gs)->name.isTEnumName(gs)) {
                return;
            }
        }

        auto threadState = getTypecheckThreadState();
        if (threadState->aborted) {
            return;
        }

        deleteDoNothingSends(cfg);

        llvm::LLVMContext &lctx = threadState->lctx;
        unique_ptr<llvm::Module> &module = threadState->combinedModule;
        unique_ptr<llvm::DIBuilder> &debug = threadState->debugInfo;
        llvm::DICompileUnit *&compUnit = threadState->compileUnit;
        // TODO: Figure out why this isn't true
        // ENFORCE(absl::c_find(cfg.symbol.data(gs)->locs(), md->loc) != cfg.symbol.data(gs)->locs().end(),
        // loc.toString(gs));
        ENFORCE(file.exists());
        if (!module) {
            ENFORCE(threadState->globalConstructorsEntry == nullptr);
            ENFORCE(debug == nullptr);
            ENFORCE(compUnit == nullptr);
            module = llvm::CloneModule(*threadState->codegenPayload);

            module->addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
            module->addModuleFlag(llvm::Module::Override, "cf-protection-return", 1);
            module->addModuleFlag(llvm::Module::Override, "cf-protection-branch", 1);

            if (llvm::Triple(llvm::sys::getProcessTriple()).isOSDarwin()) {
                // osx only supports dwarf2
                module->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 2);
            }

            debug = std::make_unique<llvm::DIBuilder>(*module);

            // NOTE: we use C here because our generated functions follow its abi
            auto language = llvm::dwarf::DW_LANG_C;
            auto filename = file.data(gs).path();
            auto isOptimized = false;
            auto runtimeVersion = 0;
            compUnit = debug->createCompileUnit(
                language, debug->createFile(llvm::StringRef(filename.data(), filename.size()), "."), "Sorbet LLVM",
                isOptimized, "", runtimeVersion);

            threadState->file = file;
            threadState->stringTable.clear();
            threadState->idTable.clear();
            threadState->rubyStringTable.clear();

            {
                auto linkageType = llvm::Function::InternalLinkage;
                auto argTys = std::vector<llvm::Type *>{llvm::Type::getInt64Ty(lctx)};
                auto varArgs = false;
                auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(lctx), argTys, varArgs);
                auto globalConstructors = llvm::Function::Create(ft, linkageType, "sorbet_globalConstructors", *module);
                threadState->allocRubyIdsEntry = llvm::BasicBlock::Create(lctx, "allocRubyIds", globalConstructors);
                threadState->globalConstructorsEntry =
                    llvm::BasicBlock::Create(lctx, "globalConstructors", globalConstructors);
            }

        } else {
            ENFORCE(threadState->file == file);
            ENFORCE(threadState->globalConstructorsEntry != nullptr);
        }
        ENFORCE(threadState->file.exists());
        compiler::CompilerState state(gs, lctx, module.get(), debug.get(), compUnit, threadState->file,
                                      threadState->allocRubyIdsEntry, threadState->globalConstructorsEntry,
                                      threadState->stringTable, threadState->idTable, threadState->rubyStringTable);
        absl::Cleanup dropInternalState = [&] {
            threadState->aborted = true;
            module = nullptr;
            threadState->file = core::FileRef();
        };
        try {
            compiler::IREmitter::run(state, cfg, md);
            string fileName = objectFileName(gs, file);
            compiler::IREmitter::buildInitFor(state, cfg.symbol, fileName);
            std::move(dropInternalState).Cancel();
        } catch (sorbet::compiler::AbortCompilation &) {
            threadState->aborted = true;
            std::move(dropInternalState).Cancel();
        } catch (sorbet::compiler::OptimizerException &oe) {
            threadState->aborted = true;
            std::move(dropInternalState).Cancel();
            // This exception is thrown from within an optimizer pass, where GlobalState
            // is not available, so we need to emit an error here, where we do have
            // access to GlobalState.
            if (auto e = gs.beginError(core::Loc(file, 0, 0), core::errors::Compiler::OptimizerFailure)) {
                e.setHeader("{}", oe.what());
            }
        }
    };

    virtual void finishTypecheckFile(const core::GlobalState &gs, const core::FileRef &f) const override {
        if (!shouldCompile(gs, f)) {
            return;
        }

        if (f.data(gs).minErrorLevel() >= core::StrictLevel::True) {
            if (f.data(gs).source().find("frozen_string_literal: true"sv) == string_view::npos) {
                compiler::failCompilation(gs, core::Loc(f, 0, 0),
                                          "Compiled files need to have '# frozen_string_literal: true'");
            }
        } else {
            compiler::failCompilation(gs, core::Loc(f, 0, 0),
                                      "Compiled files must be at least '# typed: true' or above");
        }

        auto threadState = getTypecheckThreadState();
        llvm::LLVMContext &lctx = threadState->lctx;

        unique_ptr<llvm::Module> module = move(threadState->combinedModule);
        unique_ptr<llvm::DIBuilder> debug = move(threadState->debugInfo);
        threadState->compileUnit = nullptr;

        // It is possible, though unusual, to never have typecheck() called.
        if (!module) {
            ENFORCE(!threadState->file.exists());
            return;
        }
        if (threadState->aborted) {
            threadState->file = core::FileRef();
            return;
        }

        {
            llvm::IRBuilder<> builder(lctx);

            threadState->stringTable.defineGlobalVariables(lctx, *module);

            builder.SetInsertPoint(threadState->allocRubyIdsEntry);
            threadState->idTable.defineGlobalVariables(lctx, *module, builder);
            threadState->rubyStringTable.defineGlobalVariables(lctx, *module, builder);
            builder.CreateBr(threadState->globalConstructorsEntry);

            builder.SetInsertPoint(threadState->globalConstructorsEntry);
            builder.CreateRetVoid();
        }

        threadState->globalConstructorsEntry = nullptr;

        ENFORCE(threadState->file.exists());
        ENFORCE(f == threadState->file);

        ENFORCE(threadState->combinedModule == nullptr);
        ENFORCE(threadState->globalConstructorsEntry == nullptr);
        threadState->file = core::FileRef();

        debug->finalize();

        string fileName = objectFileName(gs, f);
        ensureOutputDir(compiledOutputDir.value(), fileName);
        if (irOutputDir.has_value()) {
            ensureOutputDir(irOutputDir.value(), fileName);
        }
        if (!compiler::ObjectFileEmitter::run(gs.tracer(), lctx, move(module), compiledOutputDir.value(), irOutputDir,
                                              fileName)) {
            compiler::failCompilation(gs, core::Loc(f, 0, 0), "Object file emitter failed");
        }
    };

    virtual void finishTypecheck(const core::GlobalState &gs) const override {}

    virtual ~LLVMSemanticExtension(){};
    virtual std::unique_ptr<SemanticExtension> deepCopy(const core::GlobalState &from, core::GlobalState &to) override {
        return make_unique<LLVMSemanticExtension>(this->compiledOutputDir, this->irOutputDir, this->forceCompiled);
    };
    virtual void merge(const core::GlobalState &from, core::GlobalState &to, core::NameSubstitution &subst) override {}
};

class LLVMSemanticExtensionProvider : public SemanticExtensionProvider {
public:
    virtual void injectOptions(cxxopts::Options &optsBuilder) const override {
        optsBuilder.add_options("compiler")(
            "compiled-out-dir", "Output compiled code (*.rb.so or *.rb.bundle) to directory, which must already exist",
            cxxopts::value<string>());
        optsBuilder.add_options("compiler")("llvm-ir-dir", "Output LLVM IR to directory, which must already exist",
                                            cxxopts::value<string>());
        optsBuilder.add_options("compiler")("force-compiled", "Force all files to this compiled level",
                                            cxxopts::value<bool>());
    };
    virtual std::unique_ptr<SemanticExtension> readOptions(cxxopts::ParseResult &providedOptions) const override {
        if (providedOptions["version"].as<bool>()) {
            fmt::print("Sorbet compiler {}\n", sorbet_full_version_string);
            throw EarlyReturnWithCode(0);
        }

        optional<string> compiledOutputDir;
        optional<string> irOutputDir;
        bool forceCompiled = false;
        if (providedOptions.count("compiled-out-dir") > 0) {
            auto outputDir = providedOptions["compiled-out-dir"].as<string>();

            if (!FileOps::dirExists(outputDir)) {
                fmt::print("Missing output directory {}\n", outputDir);
                throw EarlyReturnWithCode(1);
            }

            compiledOutputDir = outputDir;
        }
        if (providedOptions.count("llvm-ir-dir") > 0) {
            auto outputDir = providedOptions["llvm-ir-dir"].as<string>();

            if (!FileOps::dirExists(outputDir)) {
                fmt::print("Missing output directory {}\n", outputDir);
                throw EarlyReturnWithCode(1);
            }

            irOutputDir = outputDir;
        }
        if (providedOptions.count("force-compiled") > 0) {
            forceCompiled = providedOptions["force-compiled"].as<bool>();
        }

        return make_unique<LLVMSemanticExtension>(compiledOutputDir, irOutputDir, forceCompiled);
    };
    virtual std::unique_ptr<SemanticExtension> defaultInstance() const override {
        optional<string> compiledOutputDir;
        optional<string> irOutputDir;
        auto forceCompile = false;
        return make_unique<LLVMSemanticExtension>(compiledOutputDir, irOutputDir, forceCompile);
    }
    virtual ~LLVMSemanticExtensionProvider(){};
};

vector<SemanticExtensionProvider *> SemanticExtensionProvider::getProviders() {
    static LLVMSemanticExtensionProvider provider;
    return {&provider};
}
} // namespace sorbet::pipeline::semantic_extension
