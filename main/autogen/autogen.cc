#include "main/autogen/autogen.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/formatting.h"
#include "main/autogen/autoloader.h"
#include "main/autogen/crc.h"

using namespace std;
namespace sorbet::autogen {

// The `AutogenWalk` class converts the internal Sorbet representation (of symbols and constants and everything) into a
// simplified set of `ParsedFile`s which contain a set of `Definition`s and a set of `Reference`s.
class AutogenWalk {
    vector<Definition> defs;
    vector<Reference> refs;
    vector<core::NameRef>
    requires;
    vector<DefinitionRef> nesting;

    enum class ScopeType { Class, Block };
    vector<ast::Send *> ignoring;
    vector<ScopeType> scopeTypes;

    UnorderedMap<void *, ReferenceRef> refMap;

    // Convert a symbol name into a fully qualified name
    vector<core::NameRef> symbolName(core::Context ctx, core::SymbolRef sym) {
        vector<core::NameRef> out;
        while (sym.exists() && sym != core::Symbols::root()) {
            out.emplace_back(sym.data(ctx)->name);
            sym = sym.data(ctx)->owner;
        }
        reverse(out.begin(), out.end());
        return out;
    }

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
    AutogenWalk() {
        auto &def = defs.emplace_back();
        def.id = 0;
        def.type = Definition::Type::Module;
        def.defines_behavior = false;
        def.is_empty = false;
        nesting.emplace_back(def.id);
    }

    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::ClassDef>(tree);

        if (!ast::isa_tree<ast::ConstantLit>(original.name)) {
            return tree;
        }
        scopeTypes.emplace_back(ScopeType::Class);

        if (original.symbol.data(ctx)->owner == core::Symbols::PackageRegistry()) {
            // this is a package, so do not enter a definition for it
            return tree;
        }

        // create a new `Definition`
        auto &def = defs.emplace_back();
        def.id = defs.size() - 1;

        // mark it as being either a `Class` or a `Module`
        if (original.kind == ast::ClassDef::Kind::Class) {
            def.type = Definition::Type::Class;
        } else {
            def.type = Definition::Type::Module;
        }

        // is it (recursively) empty?
        def.is_empty =
            absl::c_all_of(original.rhs, [](auto &tree) { return sorbet::ast::BehaviorHelpers::checkEmptyDeep(tree); });
        // does it define behavior?
        def.defines_behavior = sorbet::ast::BehaviorHelpers::checkClassDefinesBehavior(tree);

        // TODO: ref.parent_of, def.parent_ref
        // TODO: expression_range

        // we're 'pre-traversing' the constant literal here (instead of waiting for the walk to get to it naturally)
        // which means that we'll have entered in a `Reference` for it already.
        original.name = ast::TreeMap::apply(ctx, *this, move(original.name));
        // ...find the reference we just created for it
        auto it = refMap.find(original.name.get());
        ENFORCE(it != refMap.end());
        // ...so we can use that reference as the 'defining reference'
        def.defining_ref = it->second;
        // update that reference with the relevant metadata so we know 1. it's the defining ref and 2. it encompasses
        // the entire class, not just the constant name
        refs[it->second.id()].is_defining_ref = true;
        refs[it->second.id()].definitionLoc = original.loc;

        auto ait = original.ancestors.begin();
        // if this is a class, then the first ancestor is the parent class
        if (original.kind == ast::ClassDef::Kind::Class && !original.ancestors.empty()) {
            // we need to do name resolution for that class "outside" of the class body, so handle this before we've
            // modified the current scoping
            *ait = ast::TreeMap::apply(ctx, *this, move(*ait));
            ++ait;
        }

        // The rest of the ancestors are all references inside the class body (i.e. uses of `include` or `extend`) so
        // add the current class to the scoping
        nesting.emplace_back(def.id);

        // ...and then run the treemap over all the includes and extends
        for (; ait != original.ancestors.end(); ++ait) {
            *ait = ast::TreeMap::apply(ctx, *this, move(*ait));
        }
        for (auto &ancst : original.singletonAncestors) {
            ancst = ast::TreeMap::apply(ctx, *this, move(ancst));
        }

        // and now that we've processed all the ancestors, we should have created references for them all, so traverse
        // them once again...
        for (auto &ancst : original.ancestors) {
            auto *cnst = ast::cast_tree<ast::ConstantLit>(ancst);
            if (cnst == nullptr || cnst->original == nullptr) {
                // ignore them if they're not statically-known ancestors (i.e. not constants)
                continue;
            }

            // ...find the references
            auto it = refMap.find(ancst.get());
            if (it == refMap.end()) {
                continue;
            }
            // if it's the parent class, then we can set the parent_ref of the `Definition`
            if (original.kind == ast::ClassDef::Kind::Class && &ancst == &original.ancestors.front()) {
                // superclass
                def.parent_ref = it->second;
            }
            // otherwise, make sure we know the ref is the parent of this `Definition`
            refs[it->second.id()].parent_of = def.id;
        }

        return tree;
    }

    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::ClassDef>(tree);

        if (!ast::isa_tree<ast::ConstantLit>(original.name)) {
            // if the name of the class wasn't a constant, then we didn't run the `preTransformClassDef` step anyway, so
            // just skip it
            return tree;
        }

        // remove the stuff added to handle the class scope here
        nesting.pop_back();
        scopeTypes.pop_back();

        return tree;
    }

    ast::ExpressionPtr preTransformBlock(core::Context ctx, ast::ExpressionPtr block) {
        scopeTypes.emplace_back(ScopeType::Block);
        return block;
    }

    ast::ExpressionPtr postTransformBlock(core::Context ctx, ast::ExpressionPtr block) {
        scopeTypes.pop_back();
        return block;
    }

    // `true` if the constant is fully qualified and can be traced back to the root scope, `false` otherwise
    bool isCBaseConstant(ast::ConstantLit *cnst) {
        while (cnst != nullptr && cnst->original != nullptr) {
            auto &original = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(cnst->original);
            cnst = ast::cast_tree<ast::ConstantLit>(original.scope);
        }
        if (cnst && cnst->symbol == core::Symbols::root()) {
            return true;
        }
        return false;
    }

    ast::ExpressionPtr postTransformConstantLit(core::Context ctx, ast::ExpressionPtr tree) {
        auto *original = ast::cast_tree<ast::ConstantLit>(tree);

        if (!ignoring.empty()) {
            // this is either a constant in a `keepForIde` node (in which case we don't care) or it was an `include` or
            // an `extend` which already got handled in `preTransformClassDef` (in which case don't handle it again)
            return tree;
        }
        if (original->original == nullptr) {
            return tree;
        }

        // Create a new `Reference`
        auto &ref = refs.emplace_back();
        ref.id = refs.size() - 1;

        // if it's a constant we can resolve from the root...
        if (isCBaseConstant(original)) {
            // then its scope is easy
            ref.scope = nesting.front();
        } else {
            // otherwise we need to figure out how it's nested in the current scope and mark that
            ref.nesting = nesting;
            reverse(ref.nesting.begin(), ref.nesting.end());
            ref.nesting.pop_back();
            ref.scope = nesting.back();
        }
        ref.loc = original->loc;

        // the reference location is the location of constant, but this might get updated if the reference corresponds
        // to the definition of the constant, because in that case we'll later on extend the location to cover the whole
        // class or assignment
        ref.definitionLoc = original->loc;
        ref.name = QualifiedName::fromFullName(constantName(ctx, original));
        auto sym = original->symbol;
        if (!sym.isClassOrModule() || sym != core::Symbols::StubModule()) {
            ref.resolved = QualifiedName::fromFullName(symbolName(ctx, sym));
        }
        ref.is_resolved_statically = true;
        ref.is_defining_ref = false;
        // if we're already in the scope of the class (which will be the newest-created one) then we're looking at the
        // `ancestors` or `singletonAncestors` values. Otherwise, (at least for the parent relationships we care about)
        // we're looking at the first `class Child < Parent` relationship, so we change `is_subclassing` to true.
        if (!defs.empty() && !nesting.empty() && defs.back().id._id != nesting.back()._id) {
            ref.parentKind = ClassKind::Class;
        }
        // now, add it to the refmap
        refMap[tree.get()] = ref.id;
        return tree;
    }

    ast::ExpressionPtr postTransformAssign(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::Assign>(tree);

        // autogen only cares about constant assignments/definitions, so bail otherwise
        auto *lhs = ast::cast_tree<ast::ConstantLit>(original.lhs);
        if (lhs == nullptr || lhs->original == nullptr) {
            return tree;
        }

        // Create the Definition for it
        auto &def = defs.emplace_back();
        def.id = defs.size() - 1;

        // if the RHS is _also_ a constant, then this is an alias
        auto *rhs = ast::cast_tree<ast::ConstantLit>(original.rhs);
        if (rhs && rhs->symbol.exists() && !rhs->symbol.data(ctx)->isTypeAlias()) {
            def.type = Definition::Type::Alias;
            // since this is a `post`- method, then we've already created a `Reference` for the constant on the
            // RHS. Mark this `Definition` as an alias for it.
            ENFORCE(refMap.count(original.rhs.get()));
            def.aliased_ref = refMap[original.rhs.get()];
        } else {
            // if the RHS _isn't_ just a constant literal, then this is a constant definition.
            def.type = Definition::Type::Casgn;
        }

        // We also should already have a `Reference` for the name of this constant (because this is running after the
        // pre-traversal of the constant) so find that
        ENFORCE(refMap.count(original.lhs.get()));
        auto &ref = refs[refMap[original.lhs.get()].id()];
        // ...and mark that this is the defining ref for that one
        def.defining_ref = ref.id;
        ref.is_defining_ref = true;
        ref.definitionLoc = original.loc;

        // Constant definitions always count as non-empty behavior-defining definitions
        def.defines_behavior = true;
        def.is_empty = false;

        return tree;
    }

    ast::ExpressionPtr preTransformSend(core::Context ctx, ast::ExpressionPtr tree) {
        auto *original = ast::cast_tree<ast::Send>(tree);

        bool inBlock = !scopeTypes.empty() && scopeTypes.back() == ScopeType::Block;
        // Ignore keepForIde nodes. Also ignore include/extend sends iff they are directly at the
        // class/module level. These cases are handled in `preTransformClassDef`. Do not ignore in
        // block scope so that we a ref to the included module is still rendered.
        if (original->fun == core::Names::keepForIde() ||
            (!inBlock && original->recv.isSelfReference() &&
             (original->fun == core::Names::include() || original->fun == core::Names::extend()))) {
            ignoring.emplace_back(original);
        }
        // This means it's a `require`; mark it as such
        if (original->flags.isPrivateOk && original->fun == core::Names::require() && original->args.size() == 1) {
            auto *lit = ast::cast_tree<ast::Literal>(original->args.front());
            if (lit && lit->isString(ctx)) {
                requires.emplace_back(lit->asString(ctx));
            }
        }
        return tree;
    }

    ast::ExpressionPtr postTransformSend(core::Context ctx, ast::ExpressionPtr tree) {
        auto *original = ast::cast_tree<ast::Send>(tree);
        // if this send was something we were ignoring (i.e. a `keepForIde` or an `include` or `require`) then pop this
        if (!ignoring.empty() && ignoring.back() == original) {
            ignoring.pop_back();
        }
        return tree;
    }

    ParsedFile parsedFile() {
        ENFORCE(scopeTypes.empty());

        ParsedFile out;
        out.refs = move(refs);
        out.defs = move(defs);
        out.requires = move(requires);
        return out;
    }
};

// Convert a Sorbet `ParsedFile` into an Autogen `ParsedFile` by walking it as above and also recording the checksum of
// the current file
ParsedFile Autogen::generate(core::Context ctx, ast::ParsedFile tree, const CRCBuilder &crcBuilder) {
    AutogenWalk walk;
    tree.tree = ast::TreeMap::apply(ctx, walk, move(tree.tree));
    auto pf = walk.parsedFile();
    pf.path = string(tree.file.data(ctx).path());
    auto src = tree.file.data(ctx).source();
    pf.cksum = crcBuilder.crc32(src);
    pf.tree = move(tree);
    return pf;
}

} // namespace sorbet::autogen
