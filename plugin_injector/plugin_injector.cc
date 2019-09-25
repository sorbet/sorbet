#include "ast/ast.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include <cxxopts.hpp>
#include <optional>
using namespace std;
namespace sorbet::pipeline::semantic_extension {

class LLVMSemanticExtension : public SemanticExtension {
    optional<string> irOutputDir;

public:
    LLVMSemanticExtension(optional<string> irOutputDir) {
        this->irOutputDir = move(irOutputDir);
    }
    virtual void typecheck(const core::GlobalState &, cfg::CFG &, std::unique_ptr<ast::MethodDef> &) const override{};
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
