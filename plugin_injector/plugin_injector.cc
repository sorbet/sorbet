#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
// ^^^ violate our poisons so they go first
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "compiler/IRHelpers/IRHelpers.h"
#include "compiler/LLVMIREmitter/LLVMIREmitter.h"
#include "compiler/ObjectFileEmitter/ObjectFileEmitter.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include <cxxopts.hpp>
#include <optional>
using namespace std;
namespace sorbet::pipeline::semantic_extension {
namespace {
string funcName2moduleName(string sourceName) {
    // TODO: make it reversible
    absl::c_replace(sourceName, '.', '_');
    absl::c_replace(sourceName, '<', '_');
    absl::c_replace(sourceName, '>', '_');
    absl::c_replace(sourceName, '-', '_');
    return sourceName;
}
} // namespace

class LLVMSemanticExtension : public SemanticExtension {
    optional<string> irOutputDir;

public:
    LLVMSemanticExtension(optional<string> irOutputDir) {
        this->irOutputDir = move(irOutputDir);
    }
    virtual void typecheck(const core::GlobalState &gs, cfg::CFG &cfg,
                           std::unique_ptr<ast::MethodDef> &md) const override {
        if (!irOutputDir.has_value()) {
            return;
        }

        llvm::LLVMContext lctx;
        string functionName = cfg.symbol.data(gs)->toStringFullName(gs);
        unique_ptr<llvm::Module> module = sorbet::compiler::IRHelpers::readDefaultModule(functionName.data(), lctx);
        sorbet::compiler::LLVMIREmitter::run(gs, lctx, cfg, md, functionName, module.get());
        string fileName = funcName2moduleName(functionName);
        sorbet::compiler::ObjectFileEmitter::run(gs, lctx, move(module), cfg.symbol, irOutputDir.value(), fileName);
    };
    virtual std::vector<std::unique_ptr<ast::Expression>> replaceDSL(core::GlobalState &, ast::Send *) const override {
        return {};
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
