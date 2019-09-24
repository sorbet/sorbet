#ifndef SORBET_PIPELINE_SEMANTIC_EXTENSION
#define SORBET_PIPELINE_SEMANTIC_EXTENSION
#include <memory> // unique_ptr
#include <vector>

namespace cxxopts {
class ParseResult;
class Options;
} // namespace cxxopts
namespace sorbet {
namespace core {
class GlobalState;
class GlobalSubstitution;
} // namespace core

namespace ast {
class Send;
class MethodDef;
class Expression;
} // namespace ast

namespace cfg {
class CFG;
}

namespace pipeline::semantic_extension {
class SemanticExtension {
public:
    virtual void typecheck(const core::GlobalState &, cfg::CFG &, std::unique_ptr<ast::MethodDef> &) const = 0;
    virtual std::vector<std::unique_ptr<ast::Expression>> replaceDSL(core::GlobalState &, ast::Send *) const = 0;
    virtual ~SemanticExtension() = default;
    virtual std::unique_ptr<SemanticExtension> deepCopy(const core::GlobalState &from, core::GlobalState &to) = 0;
    virtual void merge(const core::GlobalState &from, core::GlobalState &to, core::GlobalSubstitution &subst) = 0;
};

class SemanticExtensionProvider {
public:
    virtual void injectOptions(cxxopts::Options &) const = 0;
    virtual std::unique_ptr<SemanticExtension> readOptions(cxxopts::ParseResult &) const = 0;
    static std::vector<SemanticExtensionProvider *> getProviders();
    virtual ~SemanticExtensionProvider() = default;
};
} // namespace pipeline::semantic_extension
} // namespace sorbet

#endif
