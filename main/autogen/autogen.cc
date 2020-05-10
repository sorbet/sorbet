// has to go first because it violates our poisons
#include "msgpack.hpp"

#include "absl/strings/str_split.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/FileOps.h"
#include "common/formatting.h"
#include "common/typecase.h"
#include "core/Names.h"
#include "main/autogen/autogen.h"
#include "main/autogen/autoloader.h"

#include "CRC.h"

using namespace std;
namespace sorbet::autogen {

const Definition &DefinitionRef::data(const ParsedFile &pf) const {
    return pf.defs[_id];
}

const Reference &ReferenceRef::data(const ParsedFile &pf) const {
    return pf.refs[_id];
}

class AutogenWalk {
    vector<Definition> defs;
    vector<Reference> refs;
    vector<core::NameRef> requires;
    vector<DefinitionRef> nesting;

    enum class ScopeType { Class, Block };
    vector<ast::Send *> ignoring;
    vector<ScopeType> scopeTypes;

    UnorderedMap<void *, ReferenceRef> refMap;

    vector<core::NameRef> symbolName(core::Context ctx, core::SymbolRef sym) {
        vector<core::NameRef> out;
        while (sym.exists() && sym != core::Symbols::root()) {
            out.emplace_back(sym.data(ctx)->name);
            sym = sym.data(ctx)->owner;
        }
        reverse(out.begin(), out.end());
        return out;
    }

    vector<core::NameRef> constantName(core::Context ctx, ast::ConstantLit *cnst) {
        vector<core::NameRef> out;
        while (cnst != nullptr && cnst->original != nullptr) {
            auto &original = ast::ref_tree<ast::UnresolvedConstantLit>(cnst->original);
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

    ast::TreePtr preTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        auto &original = ast::ref_tree<ast::ClassDef>(tree);

        if (!ast::isa_tree<ast::ConstantLit>(original.name)) {
            return tree;
        }
        scopeTypes.emplace_back(ScopeType::Class);

        // cerr << "preTransformClassDef(" << original->toString(ctx) << ")\n";

        auto &def = defs.emplace_back();
        def.id = defs.size() - 1;
        if (original.kind == ast::ClassDef::Kind::Class) {
            def.type = Definition::Type::Class;
        } else {
            def.type = Definition::Type::Module;
        }
        def.is_empty =
            absl::c_all_of(original.rhs, [](auto &tree) { return sorbet::ast::BehaviorHelpers::checkEmptyDeep(tree); });
        def.defines_behavior = sorbet::ast::BehaviorHelpers::checkClassDefinesBehavior(tree);

        // TODO: ref.parent_of, def.parent_ref
        // TODO: expression_range
        original.name = ast::TreeMap::apply(ctx, *this, move(original.name));
        auto it = refMap.find(original.name.get());
        ENFORCE(it != refMap.end());
        def.defining_ref = it->second;
        refs[it->second.id()].is_defining_ref = true;
        refs[it->second.id()].definitionLoc = core::Loc(ctx.file, original.loc);

        auto ait = original.ancestors.begin();
        if (original.kind == ast::ClassDef::Kind::Class && !original.ancestors.empty()) {
            // Handle the superclass at outer scope
            *ait = ast::TreeMap::apply(ctx, *this, move(*ait));
            ++ait;
        }
        // Then push a scope
        nesting.emplace_back(def.id);

        for (; ait != original.ancestors.end(); ++ait) {
            *ait = ast::TreeMap::apply(ctx, *this, move(*ait));
        }
        for (auto &ancst : original.singletonAncestors) {
            ancst = ast::TreeMap::apply(ctx, *this, move(ancst));
        }

        for (auto &ancst : original.ancestors) {
            auto *cnst = ast::cast_tree<ast::ConstantLit>(ancst);
            if (cnst == nullptr || cnst->original == nullptr) {
                // Don't include synthetic ancestors
                continue;
            }

            auto it = refMap.find(ancst.get());
            if (it == refMap.end()) {
                continue;
            }
            if (original.kind == ast::ClassDef::Kind::Class && &ancst == &original.ancestors.front()) {
                // superclass
                def.parent_ref = it->second;
            }
            refs[it->second.id()].parent_of = def.id;
        }

        return tree;
    }

    ast::TreePtr postTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        auto &original = ast::ref_tree<ast::ClassDef>(tree);

        if (!ast::isa_tree<ast::ConstantLit>(original.name)) {
            return tree;
        }

        nesting.pop_back();
        scopeTypes.pop_back();

        return tree;
    }

    ast::TreePtr preTransformBlock(core::Context ctx, ast::TreePtr block) {
        scopeTypes.emplace_back(ScopeType::Block);
        return block;
    }

    ast::TreePtr postTransformBlock(core::Context ctx, ast::TreePtr block) {
        scopeTypes.pop_back();
        return block;
    }

    bool isCBaseConstant(ast::ConstantLit *cnst) {
        while (cnst != nullptr && cnst->original != nullptr) {
            auto &original = ast::ref_tree<ast::UnresolvedConstantLit>(cnst->original);
            cnst = ast::cast_tree<ast::ConstantLit>(original.scope);
        }
        if (cnst && cnst->symbol == core::Symbols::root()) {
            return true;
        }
        return false;
    }

    ast::TreePtr postTransformConstantLit(core::Context ctx, ast::TreePtr tree) {
        auto *original = ast::cast_tree<ast::ConstantLit>(tree);

        if (!ignoring.empty()) {
            return tree;
        }
        if (original->original == nullptr) {
            return tree;
        }

        auto &ref = refs.emplace_back();
        ref.id = refs.size() - 1;
        if (isCBaseConstant(original)) {
            ref.scope = nesting.front();
        } else {
            ref.nesting = nesting;
            reverse(ref.nesting.begin(), ref.nesting.end());
            ref.nesting.pop_back();
            ref.scope = nesting.back();
        }
        ref.loc = core::Loc(ctx.file, original->loc);

        // This will get overridden if this loc is_defining_ref at the point
        // where we set that flag.
        ref.definitionLoc = core::Loc(ctx.file, original->loc);
        ref.name = constantName(ctx, original);
        auto sym = original->symbol;
        if (!sym.data(ctx)->isClassOrModule() || sym != core::Symbols::StubModule()) {
            ref.resolved = symbolName(ctx, sym);
        }
        ref.is_resolved_statically = true;
        ref.is_defining_ref = false;
        // if we're already in the scope of the class (which will be the newest-created one) then we're looking at the
        // `ancestors` or `singletonAncestors` values. Otherwise, (at least for the parent relationships we care about)
        // we're looking at the first `class Child < Parent` relationship, so we change `is_subclassing` to true.
        if (!defs.empty() && !nesting.empty() && defs.back().id._id != nesting.back()._id) {
            ref.parentKind = ClassKind::Class;
        }
        refMap[tree.get()] = ref.id;
        return tree;
    }

    ast::TreePtr postTransformAssign(core::Context ctx, ast::TreePtr tree) {
        auto &original = ast::ref_tree<ast::Assign>(tree);

        auto *lhs = ast::cast_tree<ast::ConstantLit>(original.lhs);
        if (lhs == nullptr || lhs->original == nullptr) {
            return tree;
        }

        auto &def = defs.emplace_back();
        def.id = defs.size() - 1;
        auto *rhs = ast::cast_tree<ast::ConstantLit>(original.rhs);
        if (rhs && rhs->symbol.exists() && !rhs->symbol.data(ctx)->isTypeAlias()) {
            def.type = Definition::Type::Alias;
            ENFORCE(refMap.count(original.rhs.get()));
            def.aliased_ref = refMap[original.rhs.get()];
        } else {
            def.type = Definition::Type::Casgn;
        }
        ENFORCE(refMap.count(original.lhs.get()));
        auto &ref = refs[refMap[original.lhs.get()].id()];
        def.defining_ref = ref.id;
        ref.is_defining_ref = true;
        ref.definitionLoc = core::Loc(ctx.file, original.loc);

        def.defines_behavior = true;
        def.is_empty = false;

        return tree;
    }

    ast::TreePtr preTransformSend(core::Context ctx, ast::TreePtr tree) {
        auto *original = ast::cast_tree<ast::Send>(tree);

        bool inBlock = !scopeTypes.empty() && scopeTypes.back() == ScopeType::Block;
        // Ignore keepForIde nodes. Also ignore include/extend sends iff they are directly at the
        // class/module level. These cases are handled in `preTransformClassDef`. Do not ignore in
        // block scope so that we a ref to the included module is still rendered.
        if (original->fun == core::Names::keepForIde() ||
            (!inBlock && original->recv->isSelfReference() &&
             (original->fun == core::Names::include() || original->fun == core::Names::extend()))) {
            ignoring.emplace_back(original);
        }
        if (original->flags.isPrivateOk && original->fun == core::Names::require() && original->args.size() == 1) {
            auto *lit = ast::cast_tree<ast::Literal>(original->args.front());
            if (lit && lit->isString(ctx)) {
                requires.emplace_back(lit->asString(ctx));
            }
        }
        return tree;
    }
    ast::TreePtr postTransformSend(core::Context ctx, ast::TreePtr tree) {
        auto *original = ast::cast_tree<ast::Send>(tree);
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

ParsedFile Autogen::generate(core::Context ctx, ast::ParsedFile tree) {
    AutogenWalk walk;
    tree.tree = ast::TreeMap::apply(ctx, walk, move(tree.tree));
    auto pf = walk.parsedFile();
    pf.path = string(tree.file.data(ctx).path());
    auto src = tree.file.data(ctx).source();
    pf.cksum = CRC::Calculate(src.data(), src.size(), CRC::CRC_32());
    pf.tree = move(tree);
    return pf;
}

vector<core::NameRef> ParsedFile::showFullName(const core::GlobalState &gs, DefinitionRef id) const {
    auto &def = id.data(*this);
    if (!def.defining_ref.exists()) {
        return {};
    }
    auto &ref = def.defining_ref.data(*this);
    auto scope = showFullName(gs, ref.scope);
    scope.insert(scope.end(), ref.name.begin(), ref.name.end());
    return scope;
}

string ParsedFile::toString(const core::GlobalState &gs) const {
    fmt::memory_buffer out;
    auto nameToString = [&](const auto &nm) -> string { return nm.data(gs)->show(gs); };

    fmt::format_to(out,
                   "# ParsedFile: {}\n"
                   "requires: [{}]\n"
                   "## defs:\n",
                   path, fmt::map_join(requires, ", ", nameToString));

    for (auto &def : defs) {
        string_view type;
        switch (def.type) {
            case Definition::Type::Module:
                type = "module"sv;
                break;
            case Definition::Type::Class:
                type = "class"sv;
                break;
            case Definition::Type::Casgn:
                type = "casgn"sv;
                break;
            case Definition::Type::Alias:
                type = "alias"sv;
                break;
        }

        fmt::format_to(out,
                       "[def id={}]\n"
                       " type={}\n"
                       " defines_behavior={}\n"
                       " is_empty={}\n",
                       def.id.id(), type, (int)def.defines_behavior, (int)def.is_empty);

        if (def.defining_ref.exists()) {
            auto &ref = def.defining_ref.data(*this);
            fmt::format_to(out, " defining_ref=[{}]\n", fmt::map_join(ref.name, " ", nameToString));
        }
        if (def.parent_ref.exists()) {
            auto &ref = def.parent_ref.data(*this);
            fmt::format_to(out, " parent_ref=[{}]\n", fmt::map_join(ref.name, " ", nameToString));
        }
        if (def.aliased_ref.exists()) {
            auto &ref = def.aliased_ref.data(*this);
            fmt::format_to(out, " aliased_ref=[{}]\n", fmt::map_join(ref.name, " ", nameToString));
        }
    }
    fmt::format_to(out, "## refs:\n");
    for (auto &ref : refs) {
        vector<string> nestingStrings;
        for (auto &scope : ref.nesting) {
            auto fullScopeName = showFullName(gs, scope);
            nestingStrings.emplace_back(fmt::format("[{}]", fmt::map_join(fullScopeName, " ", nameToString)));
        }

        auto refFullName = showFullName(gs, ref.scope);
        fmt::format_to(out,
                       "[ref id={}]\n"
                       " scope=[{}]\n"
                       " name=[{}]\n"
                       " nesting=[{}]\n"
                       " resolved=[{}]\n"
                       " loc={}\n"
                       " is_defining_ref={}\n",

                       ref.id.id(), fmt::map_join(refFullName, " ", nameToString),
                       fmt::map_join(ref.name, " ", nameToString), fmt::join(nestingStrings, " "),
                       fmt::map_join(ref.resolved, " ", nameToString), ref.loc.filePosToString(gs),
                       (int)ref.is_defining_ref);

        if (ref.parent_of.exists()) {
            auto parentOfFullName = showFullName(gs, ref.parent_of);
            fmt::format_to(out, " parent_of=[{}]\n", fmt::map_join(parentOfFullName, " ", nameToString));
        }
    }
    return to_string(out);
}

class MsgpackWriter {
private:
    void packName(core::NameRef nm) {
        u4 id;
        auto it = symbolIds.find(nm);
        if (it == symbolIds.end()) {
            id = symbols.size();
            symbols.emplace_back(nm);
            symbolIds[nm] = id;
        } else {
            id = it->second;
        }
        packer.pack_uint32(id);
    }

    void packNames(vector<core::NameRef> &names) {
        packer.pack_array(names.size());
        for (auto nm : names) {
            packName(nm);
        }
    }

    void packString(string_view str) {
        packer.pack_str(str.size());
        packer.pack_str_body(str.data(), str.size());
    }

    void packString(msgpack::packer<msgpack::sbuffer> &packer, string_view str) {
        packer.pack_str(str.size());
        packer.pack_str_body(str.data(), str.size());
    }

    void packBool(bool b) {
        if (b) {
            packer.pack_true();
        } else {
            packer.pack_false();
        }
    }

    void packReferenceRef(ReferenceRef ref) {
        if (!ref.exists()) {
            packer.pack_nil();
        } else {
            packer.pack_uint16(ref.id());
        }
    }

    void packDefinitionnRef(DefinitionRef ref) {
        if (!ref.exists()) {
            packer.pack_nil();
        } else {
            packer.pack_uint16(ref.id());
        }
    }

    void packRange(u4 begin, u4 end) {
        packer.pack_uint64(((u8)begin << 32) | end);
    }

    void packDefinition(core::Context ctx, ParsedFile &pf, Definition &def) {
        packer.pack_array(def_attrs[version].size());

        // raw_full_name
        auto raw_full_name = pf.showFullName(ctx, def.id);
        packNames(raw_full_name);

        // type
        packer.pack_uint8(static_cast<u8>(def.type));

        // defines_behavior
        packBool(def.defines_behavior);

        // is_empty
        packBool(def.is_empty);

        // parent_ref
        packReferenceRef(def.parent_ref);

        // aliased_ref
        packReferenceRef(def.aliased_ref);

        // defining_ref
        packReferenceRef(def.defining_ref);
    }

    void packReference(core::Context ctx, ParsedFile &pf, Reference &ref) {
        packer.pack_array(ref_attrs[version].size());

        // scope
        packDefinitionnRef(ref.scope.id());

        // name
        packNames(ref.name);

        // nesting
        packer.pack_array(ref.nesting.size());
        for (auto &scope : ref.nesting) {
            packDefinitionnRef(scope.id());
        }

        // expression_range
        auto expression_range = ref.definitionLoc.position(ctx);
        packRange(expression_range.first.line, expression_range.second.line);
        // expression_pos_range
        packRange(ref.loc.beginPos(), ref.loc.endPos());

        // resolved
        if (ref.resolved.empty()) {
            packer.pack_nil();
        } else {
            packNames(ref.resolved);
        }

        // is_defining_ref
        packBool(ref.is_defining_ref);

        // parent_of
        packDefinitionnRef(ref.parent_of);
    }

    static int assert_valid_version(int version) {
        if (version < MIN_VERSION || version > MAX_VERSION) {
            Exception::raise("msgpack version {} not in available range [{}, {}]", version, MIN_VERSION, MAX_VERSION);
        }
        return version;
    }

public:
    constexpr static int MIN_VERSION = 2;
    constexpr static int MAX_VERSION = 2;

    // symbols[0..3] are reserved for the Type aliases
    MsgpackWriter(int version)
        : version(assert_valid_version(version)), ref_attrs(ref_attr_map.at(version)),
          def_attrs(def_attr_map.at(version)), packer(payload), symbols(4) {}

    string pack(core::Context ctx, ParsedFile &pf) {
        packer.pack_array(6);

        packer.pack_true(); // did_resolution
        packString(pf.path);
        packer.pack_uint32(pf.cksum);

        // requires
        packer.pack_array(pf.requires.size());
        for (auto nm : pf.requires) {
            packString(nm.data(ctx)->show(ctx));
        }

        packer.pack_array(pf.defs.size());
        for (auto &def : pf.defs) {
            packDefinition(ctx, pf, def);
        }
        packer.pack_array(pf.refs.size());
        for (auto &ref : pf.refs) {
            packReference(ctx, pf, ref);
        }

        msgpack::sbuffer out;
        msgpack::packer<msgpack::sbuffer> header(out);
        header.pack_map(5);

        packString(header, "symbols");
        int i = -1;
        header.pack_array(symbols.size());
        for (auto sym : symbols) {
            ++i;
            string str;
            switch ((Definition::Type)i) {
                case Definition::Type::Module:
                    str = "module";
                    break;
                case Definition::Type::Class:
                    str = "class";
                    break;
                case Definition::Type::Casgn:
                    str = "casgn";
                    break;
                case Definition::Type::Alias:
                    str = "alias";
                    break;
                default:
                    str = sym.data(ctx)->show(ctx);
            }
            packString(header, str);
        }

        packString(header, "ref_count");
        header.pack_uint32(pf.refs.size());
        packString(header, "def_count");
        header.pack_uint32(pf.defs.size());

        packString(header, "ref_attrs");
        header.pack_array(ref_attrs.size());
        for (auto attr : ref_attrs) {
            packString(header, attr);
        }

        packString(header, "def_attrs");
        header.pack_array(def_attrs.size());
        for (auto attr : def_attrs) {
            packString(header, attr);
        }
        out.write(payload.data(), payload.size());
        return string(out.data(), out.size());
    }

private:
    int version;
    const vector<string> &ref_attrs;
    const vector<string> &def_attrs;
    msgpack::sbuffer payload;
    msgpack::packer<msgpack::sbuffer> packer;

    vector<core::NameRef> symbols;
    UnorderedMap<core::NameRef, u4> symbolIds;

    static const map<int, vector<string>> ref_attr_map;
    static const map<int, vector<string>> def_attr_map;
};

const map<int, vector<string>> MsgpackWriter::ref_attr_map{
    {
        2,
        {
            "scope",
            "name",
            "nesting",
            "expression_range",
            "expression_pos_range",
            "resolved",
            "is_defining_ref",
            "parent_of",
        },
    },
};

const map<int, vector<string>> MsgpackWriter::def_attr_map{
    {
        2,
        {
            "raw_full_name",
            "type",
            "defines_behavior",
            "is_empty",
            "parent_ref",
            "aliased_ref",
            "defining_ref",
        },
    },
};

string ParsedFile::toMsgpack(core::Context ctx, int version) {
    MsgpackWriter write(version);
    return write.pack(ctx, *this);
}

vector<string> ParsedFile::listAllClasses(core::Context ctx) {
    vector<string> out;

    for (auto &def : defs) {
        if (def.type != Definition::Type::Class) {
            continue;
        }
        vector<core::NameRef> names = showFullName(ctx, def.id);
        out.emplace_back(fmt::format("{}", fmt::map_join(names, "::", [&ctx](const core::NameRef &nm) -> string_view {
                                         return nm.data(ctx)->shortName(ctx);
                                     })));
    }

    return out;
}

} // namespace sorbet::autogen
