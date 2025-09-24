#ifndef TEST_HELPERS_PACKAGES_H
#define TEST_HELPERS_PACKAGES_H

#include <string>
#include <vector>

#include "ast/Trees.h"

namespace sorbet::core {
class GlobalState;
struct AutocorrectSuggestion;
class FileRef;
} // namespace sorbet::core

namespace sorbet::core::packages {
class PackageInfo;
}

namespace sorbet::test {

class PackageTextBuilder {
    std::string name;
    std::string strictDeps;
    std::string layer;
    std::vector<std::string> imports;
    std::vector<std::string> testImports;
    bool preludePackage;

public:
    PackageTextBuilder &withName(std::string name) {
        this->name = std::move(name);
        return *this;
    }

    PackageTextBuilder &withStrictDeps(std::string strictDeps) {
        this->strictDeps = std::move(strictDeps);
        return *this;
    }

    PackageTextBuilder &withLayer(std::string layer) {
        this->layer = std::move(layer);
        return *this;
    }

    PackageTextBuilder &withImports(std::vector<std::string> imports) {
        this->imports = std::move(imports);
        return *this;
    }

    PackageTextBuilder &withTestImports(std::vector<std::string> testImports) {
        this->testImports = std::move(testImports);
        return *this;
    }

    PackageTextBuilder &withPreludePackage() {
        this->preludePackage = true;
        return *this;
    }

    std::string build();
};

// Helpers for writing tests about the package DB.
class PackageHelpers {
public:
    static const std::vector<std::string> NO_LAYERS;
    static const std::vector<std::string> LAYERS_LIB_APP;
    static const std::vector<std::string> LAYERS_UTIL_LIB_APP;

    // Make a __package.rb source with the supplied eatures. A package name, and the `strict_deps` and `layer`
    // attributes are required, while imports and test_imports are optional.
    static std::string makePackageRB(std::string name, std::string strictDeps, std::string layer,
                                     std::vector<std::string> imports = {}, std::vector<std::string> testImports = {});

    // Construct a GlobalState that has the packager enabled, and optionally sets up the supplied layers.
    static void makeDefaultPackagerGlobalState(core::GlobalState &gs,
                                               const std::vector<std::string> &packagerLayers = NO_LAYERS);

    // Enter the sources into the package DB. The vector takes filename/source pairs as arguments, where each file name
    // must be `__package.rb`, but the directory name can vary.
    static std::vector<ast::ParsedFile> enterPackages(core::GlobalState &gs,
                                                      std::vector<std::pair<std::string, std::string>> packageSources);

    // Fetch the package info for this file.
    static const core::packages::PackageInfo &packageInfoFor(const core::GlobalState &gs, core::FileRef file);
};

} // namespace sorbet::test

#endif
