#include "main/autogen/packages.h"
#include "ast/Trees.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"

#include "common/formatting.h"

using namespace std;
namespace sorbet::autogen {

class PackageWalk {
    vector<core::NameRef> package;
    vector<QualifiedName> imports;
    vector<QualifiedName> exports;

    // Convert a constant literal into a fully qualified name
    vector<core::NameRef> constantName(core::Context ctx, ast::ConstantLit *cnst) {
        vector<core::NameRef> out;
        while (cnst != nullptr && cnst->original != nullptr) {
            auto &original = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(cnst->original);
            out.emplace_back(original.cnst);
            cnst = ast::cast_tree<ast::ConstantLit>(original.scope);
        }
        reverse(out.begin(), out.end());
        return out;
    }

public:
    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (classDef.symbol == core::Symbols::root() || classDef.ancestors.size() != 1 ||
            classDef.kind != ast::ClassDef::Kind::Class) {
            return tree;
        }
        auto cnst = ast::cast_tree<ast::ConstantLit>(classDef.name);
        if (!cnst) {
            return tree;
        }

        package = constantName(ctx, cnst);
        return tree;
    }

    ast::ExpressionPtr postTransformSend(core::Context ctx, ast::ExpressionPtr tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);
        // we're not going to report errors about ill-formed things here: those errors should get reported elsewhere,
        // and instead we'll bail if things don't look like we expect
        if (send.numPosArgs() != 1) {
            return tree;
        }
        if (send.fun != core::Names::export_() && send.fun != core::Names::import()) {
            return tree;
        }

        auto cnst = ast::cast_tree<ast::ConstantLit>(send.getPosArg(0));
        if (!cnst) {
            return tree;
        }

        auto name = QualifiedName::fromFullName(constantName(ctx, cnst));
        if (send.fun == core::Names::export_()) {
            exports.emplace_back(move(name));
        } else if (send.fun == core::Names::import()) {
            imports.emplace_back(move(name));
        }

        return tree;
    }

    Package getPackage() {
        Package pkg;
        pkg.package = move(package);
        pkg.imports = move(imports);
        pkg.exports = move(exports);
        return pkg;
    }
};

Package Packages::extractPackage(core::Context ctx, ast::ParsedFile tree) {
    PackageWalk walk;
    tree.tree = ast::TreeMap::apply(ctx, walk, move(tree.tree));
    auto package = walk.getPackage();
    package.tree = move(tree);
    return package;
}

} // namespace sorbet::autogen
