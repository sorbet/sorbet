#include "GlobalState.h"
#include "Types.h"
#include "core/Names/core.h"
#include "core/errors/errors.h"
#include <regex>

#include "absl/strings/str_split.h"

template class std::vector<std::pair<unsigned int, unsigned int>>;

using namespace std;

namespace ruby_typer {
namespace core {

namespace {
const char *top_str = "<top>";

const char *bottom_str = "<bottom>";

const char *untyped_str = "untyped";

const char *root_str = "<root>";

const char *nil_str = "nil";

const char *object_str = "Object";

const char *junk_str = "<<JUNK>>";

const char *string_str = "String";

const char *integer_str = "Integer";

const char *float_str = "Float";

const char *symbol_str = "Symbol";

const char *array_str = "Array";

const char *hash_str = "Hash";

const char *proc_str = "Proc";

const char *trueClass_str = "TrueClass";

const char *falseClass_str = "FalseClass";

const char *nilClass_str = "NilClass";

const char *class_str = "Class";

const char *module_str = "Module";

const char *todo_str = "<todo sym>";

const char *no_symbol_str = "<none>";

const char *opus_str = "Opus";

const char *T_str = "T";

const char *basicObject_str = "BasicObject";

const char *kernel_str = "Kernel";

const char *range_str = "Range";

const char *regexp_str = "Regexp";

const char *standardError_str = "StandardError";

const char *complex_str = "Complex";

const char *rational_str = "Rational";

// A magic non user-creatable class with methods to keep state between passes
const char *magic_str = "<Magic>";

const char *DB_str = "DB";

const char *model_str = "Model";

const char *m_str = "M";

const char *enumerable_str = "Enumerable";

const char *set_str = "Set";

const char *struct_str = "Struct";

const char *file_str = "File";

// This fills in all the way up to MAX_SYNTHETIC_SYMBOLS
const char *reserved_str = "<<RESERVED>>";
} // namespace

SymbolRef GlobalState::synthesizeClass(absl::string_view name, u4 superclass, bool isModule) {
    NameRef nameId = enterNameConstant(name);

    // This can't use enterClass since there is a chicken and egg problem.
    // These will be added to defn_root().members later.
    SymbolRef symRef = SymbolRef(this, symbols.size());
    symbols.emplace_back();
    Symbol &info = symRef.info(*this, true); // allowing noSymbol is needed because this enters noSymbol.
    info.name = nameId;
    info.owner = defn_root();
    info.superClass = SymbolRef(this, superclass);
    info.flags = 0;
    info.setClass();
    info.setIsModule(isModule);

    if (symRef._id > GlobalState::defn_root()._id) {
        GlobalState::defn_root().info(*this, true).members.push_back(make_pair(nameId, symRef));
    }
    return symRef;
}

int globalStateIdCounter = 1;

GlobalState::GlobalState(spdlog::logger &logger) : logger(logger), globalStateId(globalStateIdCounter++) {
    unsigned int max_name_count = 262144;   // 6MB
    unsigned int max_symbol_count = 524288; // 32MB

    names.reserve(max_name_count);
    symbols.reserve(max_symbol_count);
    int names_by_hash_size = 2 * max_name_count;
    names_by_hash.resize(names_by_hash_size);
    ENFORCE((names_by_hash_size & (names_by_hash_size - 1)) == 0, "names_by_hash_size is not a power of 2");
}

void GlobalState::initEmpty() {
    names.emplace_back(); // first name is used in hashes to indicate empty cell
    names[0].kind = NameKind::UTF8;
    names[0].raw.utf8 = absl::string_view();
    Names::registerNames(*this);

    SymbolRef no_symbol_id = synthesizeClass(no_symbol_str, 0);
    SymbolRef top_id = synthesizeClass(top_str, 0);
    SymbolRef bottom_id = synthesizeClass(bottom_str, 0);
    SymbolRef root_id = synthesizeClass(root_str, 0);
    GlobalState::defn_root()
        .info(*this, true)
        .members.push_back(make_pair(enterNameConstant(no_symbol_str), no_symbol_id));
    GlobalState::defn_root().info(*this, true).members.push_back(make_pair(enterNameConstant(top_str), top_id));
    GlobalState::defn_root().info(*this, true).members.push_back(make_pair(enterNameConstant(bottom_str), bottom_id));
    SymbolRef nil_id = synthesizeClass(nil_str);
    SymbolRef todo_id = synthesizeClass(todo_str, 0);
    SymbolRef object_id = synthesizeClass(object_str, core::GlobalState::defn_BasicObject()._id);
    SymbolRef junk_id = synthesizeClass(junk_str, 0);
    SymbolRef integer_id = synthesizeClass(integer_str);
    SymbolRef float_id = synthesizeClass(float_str);
    SymbolRef string_id = synthesizeClass(string_str);
    SymbolRef symbol_id = synthesizeClass(symbol_str);
    SymbolRef array_id = synthesizeClass(array_str);
    SymbolRef hash_id = synthesizeClass(hash_str);
    SymbolRef trueClass_id = synthesizeClass(trueClass_str);
    SymbolRef falseClass_id = synthesizeClass(falseClass_str);
    SymbolRef nilClass_id = synthesizeClass(nilClass_str);
    SymbolRef untyped_id = synthesizeClass(untyped_str, 0);
    SymbolRef opus_id = synthesizeClass(opus_str, 0, true);

    SymbolRef T_id = synthesizeClass(T_str, core::GlobalState::defn_todo()._id, true);
    T_id.info(*this).setIsModule(true);

    SymbolRef class_id = synthesizeClass(class_str, 0);
    SymbolRef basicObject_id = synthesizeClass(basicObject_str, 0);
    SymbolRef kernel_id = synthesizeClass(kernel_str, 0, true);
    SymbolRef range_id = synthesizeClass(range_str);
    SymbolRef regexp_id = synthesizeClass(regexp_str);
    SymbolRef magic_id = synthesizeClass(magic_str);
    SymbolRef module_id = synthesizeClass(module_str);
    SymbolRef standardError_id = synthesizeClass(standardError_str);
    SymbolRef complex_id = synthesizeClass(complex_str);
    SymbolRef rational_id = synthesizeClass(rational_str);
    SymbolRef T_Array_id = enterClassSymbol(Loc::none(), defn_T(), enterNameConstant(array_str));
    SymbolRef T_Hash_id = enterClassSymbol(Loc::none(), defn_T(), enterNameConstant(hash_str));
    SymbolRef T_Proc_id = enterClassSymbol(Loc::none(), defn_T(), enterNameConstant(proc_str));
    SymbolRef proc_id = synthesizeClass(proc_str);
    SymbolRef T_any_id = enterMethodSymbol(Loc::none(), defn_T(), Names::any());
    SymbolRef T_all_id = enterMethodSymbol(Loc::none(), defn_T(), Names::all());
    SymbolRef T_untyped_id = enterMethodSymbol(Loc::none(), defn_T(), Names::untyped());
    SymbolRef T_nilable_id = enterMethodSymbol(Loc::none(), defn_T(), Names::nilable());
    SymbolRef enumerable_id = synthesizeClass(enumerable_str, 0, true);
    SymbolRef set_id = synthesizeClass(set_str);
    SymbolRef struct_id = synthesizeClass(struct_str);
    SymbolRef file_id = synthesizeClass(file_str);

    ENFORCE(no_symbol_id == noSymbol(), "no symbol creation failed");
    ENFORCE(top_id == defn_top(), "top symbol creation failed");
    ENFORCE(bottom_id == defn_bottom(), "bottom symbol creation failed");
    ENFORCE(root_id == defn_root(), "root symbol creation failed");
    ENFORCE(nil_id == defn_nil(), "nil symbol creation failed");
    ENFORCE(todo_id == defn_todo(), "todo symbol creation failed");
    ENFORCE(object_id == defn_Object(), "object symbol creation failed");
    ENFORCE(junk_id == defn_junk(), "junk symbol creation failed");
    ENFORCE(integer_id == defn_Integer(), "Integer symbol creation failed");
    ENFORCE(float_id == defn_Float(), "Float symbol creation failed");
    ENFORCE(string_id == defn_String(), "String symbol creation failed");
    ENFORCE(symbol_id == defn_Symbol(), "Symbol symbol creation failed");
    ENFORCE(array_id == defn_Array(), "Array symbol creation failed");
    ENFORCE(hash_id == defn_Hash(), "Hash symbol creation failed");
    ENFORCE(trueClass_id == defn_TrueClass(), "TrueClass symbol creation failed");
    ENFORCE(falseClass_id == defn_FalseClass(), "FalseClass symbol creation failed");
    ENFORCE(nilClass_id == defn_NilClass(), "NilClass symbol creation failed");
    ENFORCE(untyped_id == defn_untyped(), "untyped symbol creation failed");
    ENFORCE(opus_id == defn_Opus(), "Opus symbol creation failed");
    ENFORCE(T_id == defn_T(), "T symbol creation failed");
    ENFORCE(class_id == defn_Class(), "Class symbol creation failed");
    ENFORCE(basicObject_id == defn_BasicObject(), "BasicObject symbol creation failed");
    ENFORCE(kernel_id == defn_Kernel(), "Kernel symbol creation failed");
    ENFORCE(range_id == defn_Range(), "Range symbol creation failed");
    ENFORCE(regexp_id == defn_Regexp(), "Regexp symbol creation failed");
    ENFORCE(magic_id == defn_Magic(), "Magic symbol creation failed");
    ENFORCE(module_id == defn_Module(), "Module symbol creation failed");
    ENFORCE(standardError_id == defn_StandardError(), "StandardError symbol creation failed");
    ENFORCE(complex_id == defn_Complex(), "Complex symbol creation failed");
    ENFORCE(rational_id == defn_Rational(), "Rational symbol creation failed");
    ENFORCE(T_Array_id = defn_T_Array(), "T::Array symbol creation failed");
    ENFORCE(T_Hash_id = defn_T_Hash(), "T::Hash symbol creation failed");
    ENFORCE(T_Proc_id = defn_T_Proc(), "T::Proc symbol creation failed");
    ENFORCE(proc_id = defn_Proc(), "Proc symbol creation failed");
    ENFORCE(T_any_id = defn_T_any());
    ENFORCE(T_all_id = defn_T_all());
    ENFORCE(T_untyped_id = defn_T_untyped());
    ENFORCE(T_nilable_id = defn_T_nilable());
    ENFORCE(enumerable_id = defn_Enumerable());
    ENFORCE(set_id = defn_Set());
    ENFORCE(struct_id = defn_Struct());
    ENFORCE(file_id = defn_File());

    // Synthesize nil = NilClass()
    defn_nil().info(*this).resultType = core::Types::nil();

    // Synthesize untyped = dynamic()
    defn_untyped().info(*this).resultType = core::Types::dynamic();

    // <Magic> has a special Type
    defn_Magic().info(*this).resultType = make_shared<MagicType>();

    // Synthesize <Magic>#build_hash(*vs : Object) => Hash
    SymbolRef method = enterMethodSymbol(Loc::none(), defn_Magic(), Names::buildHash());
    SymbolRef arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
    arg.info(*this).setRepeated();
    arg.info(*this).resultType = core::Types::Object();
    method.info(*this).arguments().push_back(arg);
    method.info(*this).resultType = core::Types::hashOfUntyped();

    // Synthesize <Magic>#build_array(*vs : Object) => Array
    method = enterMethodSymbol(Loc::none(), defn_Magic(), Names::buildArray());
    arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
    arg.info(*this).setRepeated();
    arg.info(*this).resultType = core::Types::Object();
    method.info(*this).arguments().push_back(arg);
    method.info(*this).resultType = core::Types::arrayOfUntyped();

    // Synthesize <Magic>#<splat>(a: Array) => Untyped
    method = enterMethodSymbol(Loc::none(), defn_Magic(), Names::splat());
    arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
    arg.info(*this).resultType = core::Types::arrayOfUntyped();
    method.info(*this).arguments().push_back(arg);
    method.info(*this).resultType = core::Types::dynamic();

    // TODO(pay-server) Synthesize ::M = ::Opus::DB::Model
    //
    // This is a hack to handle that specific alias in pay-server; More-general
    // handling will require substantial additional sophistication in the
    // namer+resolver.

    SymbolRef db = enterClassSymbol(Loc::none(), defn_Opus(), enterNameConstant(DB_str));
    db.info(*this).setIsModule(true);

    SymbolRef model = enterClassSymbol(Loc::none(), db, enterNameConstant(model_str));
    model.info(*this).setIsModule(true);

    SymbolRef m = enterStaticFieldSymbol(Loc::none(), defn_root(), enterNameConstant(m_str));
    m.info(*this).resultType = make_unique<AliasType>(model);

    // Allow T::Array[Int] to work as an an alias for Array[Int]
    T_Array_id.info(*this).resultType = make_shared<ClassType>(T_Array_id);
    // same for hash.
    T_Hash_id.info(*this).resultType = make_shared<ClassType>(T_Hash_id);

    int reservedCount = 0;

    // Set the correct resultTypes for all synthesized classes
    // Does it in two passes since the singletonClass will go in the defn_root() members which will invalidate the
    // iterator
    vector<SymbolRef> needsResultType;
    for (auto &info : symbols) {
        auto ref = info.ref(*this);
        if (ref._id < defn_root()._id) {
            // These aren't real classes and won't have singleton classes
            continue;
        }
        if (info.isClass() && !info.resultType) {
            needsResultType.emplace_back(info.ref(*this));
        }
    }
    for (auto sym : needsResultType) {
        Symbol &info = sym.info(*this);
        info.resultType = make_unique<core::ClassType>(info.singletonClass(*this));
    }

    while (symbols.size() < GlobalState::MAX_SYNTHETIC_SYMBOLS) {
        std::string res(reserved_str);
        res = res + to_string(reservedCount);
        synthesizeClass(res);
        reservedCount++;
    }

    freezeNameTable();
    freezeSymbolTable();
    freezeFileTable();
    ENFORCE(symbols.size() == defn_last_synthetic_sym()._id + 1, "Too many synthetic symbols?");
    sanityCheck();
}

GlobalState::~GlobalState() {
    ENFORCE(errors.buffer.empty());
};

constexpr decltype(GlobalState::STRINGS_PAGE_SIZE) GlobalState::STRINGS_PAGE_SIZE;

SymbolRef GlobalState::enterSymbol(Loc loc, SymbolRef owner, NameRef name, u4 flags) {
    ENFORCE(owner.exists(), "entering symbol in to non-existing owner");
    ENFORCE(name.exists(), "entering symbol with non-existing name");
    Symbol &ownerScope = owner.info(*this, true);
    auto from = ownerScope.members.begin();
    auto to = ownerScope.members.end();
    histogramInc("symbol_enter_by_name", ownerScope.members.size());

    while (from != to) {
        auto &el = *from;
        if (el.first == name) {
            ENFORCE((from->second.info(*this).flags & flags) == flags, "existing symbol has wrong flags");
            counterInc("symbols.hit");
            return from->second;
        }
        from++;
    }
    ENFORCE(!symbolTableFrozen);

    bool reallocate = symbols.size() == symbols.capacity();

    SymbolRef ret = SymbolRef(this, symbols.size());
    symbols.emplace_back();
    Symbol &info = ret.info(*this, true);
    info.name = name;
    info.flags = flags;
    info.owner = owner;
    info.definitionLoc = loc;
    if (info.isBlockSymbol(*this)) {
        categoryCounterInc("symbols", "block");
    } else if (info.isClass()) {
        categoryCounterInc("symbols", "class");
    } else if (info.isMethod()) {
        categoryCounterInc("symbols", "method");
    } else if (info.isField()) {
        categoryCounterInc("symbols", "field");
    } else if (info.isStaticField()) {
        categoryCounterInc("symbols", "static_field");
    } else if (info.isMethodArgument()) {
        categoryCounterInc("symbols", "argument");
    }

    if (!reallocate) {
        ownerScope.members.push_back(make_pair(name, ret));
    } else {
        owner.info(*this, true).members.push_back(make_pair(name, ret));
    }
    return ret;
}

SymbolRef GlobalState::enterClassSymbol(Loc loc, SymbolRef owner, NameRef name) {
    ENFORCE(name.name(*this).isClassName(*this));
    return enterSymbol(loc, owner, name, Symbol::Flags::CLASS);
}

SymbolRef GlobalState::enterTypeMember(Loc loc, SymbolRef owner, NameRef name, Variance variance) {
    u4 flags;
    ENFORCE(owner.info(*this).isClass());
    if (variance == Variance::Invariant) {
        flags = Symbol::Flags::TYPE_INVARIANT;
    } else if (variance == Variance::CoVariant) {
        flags = Symbol::Flags::TYPE_COVARIANT;
    } else if (variance == Variance::ContraVariant) {
        flags = Symbol::Flags::TYPE_CONTRAVARIANT;
    } else {
        Error::notImplemented();
    }

    flags = flags | Symbol::Flags::TYPE_MEMBER;
    SymbolRef result = enterSymbol(loc, owner, name, flags);
    owner.info(*this).typeMembers().emplace_back(result);
    return result;
}

SymbolRef GlobalState::enterTypeArgument(Loc loc, SymbolRef owner, NameRef name, Variance variance) {
    u4 flags;
    if (variance == Variance::Invariant) {
        flags = Symbol::Flags::TYPE_INVARIANT;
    } else if (variance == Variance::CoVariant) {
        flags = Symbol::Flags::TYPE_COVARIANT;
    } else if (variance == Variance::ContraVariant) {
        flags = Symbol::Flags::TYPE_CONTRAVARIANT;
    } else {
        Error::notImplemented();
    }

    flags = flags | Symbol::Flags::TYPE_ARGUMENT;
    SymbolRef result = enterSymbol(loc, owner, name, flags);
    owner.info(*this).typeMembers().emplace_back(result);
    return result;
}

SymbolRef GlobalState::enterMethodSymbol(Loc loc, SymbolRef owner, NameRef name) {
    bool isBlock = name.name(*this).kind == NameKind::UNIQUE && name.name(*this).unique.original == Names::blockTemp();
    ENFORCE(isBlock || owner.info(*this).isClass(), "entering method symbol into not-a-class");
    return enterSymbol(loc, owner, name, Symbol::Flags::METHOD);
}

SymbolRef GlobalState::enterNewMethodOverload(Loc loc, SymbolRef original, u2 num) {
    NameRef name = freshNameUnique(UniqueNameKind::Overload, original.info(*this).name, num);
    SymbolRef res = enterMethodSymbol(loc, original.info(*this).owner, name);
    res.info(*this).argumentsOrMixins.reserve(original.info(*this).argumentsOrMixins.size());
    for (auto &arg : original.info(*this).argumentsOrMixins) {
        Loc loc = arg.info(*this).definitionLoc;
        NameRef nm = arg.info(*this).name;
        SymbolRef newArg = enterMethodArgumentSymbol(loc, res, nm);
        newArg.info(*this).flags = arg.info(*this).flags;
        res.info(*this).argumentsOrMixins.push_back(newArg);
    }
    return res;
}

SymbolRef GlobalState::enterFieldSymbol(Loc loc, SymbolRef owner, NameRef name) {
    ENFORCE(owner.info(*this).isClass(), "entering field symbol into not-a-class");
    return enterSymbol(loc, owner, name, Symbol::Flags::FIELD);
}

SymbolRef GlobalState::enterStaticFieldSymbol(Loc loc, SymbolRef owner, NameRef name) {
    return enterSymbol(loc, owner, name, Symbol::Flags::STATIC_FIELD);
}

SymbolRef GlobalState::enterMethodArgumentSymbol(Loc loc, SymbolRef owner, NameRef name) {
    ENFORCE(owner.info(*this).isMethod(), "entering method argument symbol into not-a-method");
    return enterSymbol(loc, owner, name, Symbol::Flags::METHOD_ARGUMENT);
}

LocalVariable GlobalState::enterLocalSymbol(SymbolRef owner, NameRef name) const {
    // THIS IS NOT TRUE. Top level code is still a thing
    // ENFORCE(owner.info(*this).isMethod());
    categoryCounterInc("symbols", "local");
    if (owner.info(*this).isBlockSymbol(*this) && !name.isBlockClashSafe(*this)) {
        // Reproducibly create a name that depends on the owners scope.
        // This is used to distinguish between name "foo" defined in a block
        // and name "foo" defined by outer function.
        // We can't use block nesting level, otherwise there may be crosstalk
        // currently we use lower bits of owner id
        // this is fine until method has more than 64k blocks + nested methods + nested classes
        constexpr u2 rangeSize = (256 * 256 - 1);
        LocalVariable ret(name, 1 + owner._id % rangeSize);
        ENFORCE(owner._id - owner.info(*this).enclosingMethod(*this)._id < rangeSize);
        // check that did not overflow
        return ret;
    }
    LocalVariable r(name, 0);
    return r;
}

absl::string_view GlobalState::enterString(absl::string_view nm) {
    char *from = nullptr;
    if (nm.size() > GlobalState::STRINGS_PAGE_SIZE) {
        strings.push_back(make_unique<vector<char>>(nm.size()));
        from = strings.back()->data();
        if (strings.size() > 1) {
            swap(*(strings.end() - 1), *(strings.end() - 2));
        }
    } else {
        if (strings_last_page_used + nm.size() > GlobalState::STRINGS_PAGE_SIZE) {
            strings.push_back(make_unique<vector<char>>(GlobalState::STRINGS_PAGE_SIZE));
            // printf("Wasted %i space\n", STRINGS_PAGE_SIZE - strings_last_page_used);
            strings_last_page_used = 0;
        }
        from = strings.back()->data() + strings_last_page_used;
    }

    counterInc("strings");
    memcpy(from, nm.data(), nm.size());
    strings_last_page_used += nm.size();
    return absl::string_view(from, nm.size());
}

NameRef GlobalState::enterNameUTF8(absl::string_view nm) {
    const auto hs = _hash(nm);
    unsigned int hashTableSize = names_by_hash.size();
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probe_count = 1;

    while (names_by_hash[bucketId].second != 0u) {
        auto &bucket = names_by_hash[bucketId];
        if (bucket.first == hs) {
            auto name_id = bucket.second;
            auto &nm2 = names[name_id];
            if (nm2.kind == NameKind::UTF8 && nm2.raw.utf8 == nm) {
                counterInc("names.utf8.hit");
                return nm2.ref(*this);
            }
        }
        bucketId = (bucketId + probe_count) & mask;
        probe_count++;
    }
    ENFORCE(!nameTableFrozen);

    ENFORCE(probe_count != hashTableSize, "Full table?");

    if (names.size() == names.capacity()) {
        expandNames();
        hashTableSize = names_by_hash.size();
        mask = hashTableSize - 1;
        bucketId = hs & mask; // look for place in the new size
        probe_count = 1;
        while (names_by_hash[bucketId].second != 0) {
            bucketId = (bucketId + probe_count) & mask;
            probe_count++;
        }
    }

    auto idx = names.size();
    auto &bucket = names_by_hash[bucketId];
    bucket.first = hs;
    bucket.second = idx;
    names.emplace_back();

    names[idx].kind = NameKind::UTF8;
    names[idx].raw.utf8 = enterString(nm);
    categoryCounterInc("names", "utf8");

    return NameRef(*this, idx);
}

NameRef GlobalState::enterNameConstant(NameRef original) {
    ENFORCE(original.exists(), "making a constant name over non-exiting name");
    ENFORCE(original.name(*this).kind == UTF8, "making a constant name over wrong name kind");

    const auto hs = _hash_mix_constant(CONSTANT, original.id());
    unsigned int hashTableSize = names_by_hash.size();
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probe_count = 1;

    while (names_by_hash[bucketId].second != 0 && probe_count < hashTableSize) {
        auto &bucket = names_by_hash[bucketId];
        if (bucket.first == hs) {
            auto &nm2 = names[bucket.second];
            if (nm2.kind == CONSTANT && nm2.cnst.original == original) {
                counterInc("names.constant.hit");
                return nm2.ref(*this);
            }
        }
        bucketId = (bucketId + probe_count) & mask;
        probe_count++;
    }
    if (probe_count == hashTableSize) {
        Error::raise("Full table?");
    }
    ENFORCE(!nameTableFrozen);

    if (names.size() == names.capacity()) {
        expandNames();
        hashTableSize = names_by_hash.size();
        mask = hashTableSize - 1;

        bucketId = hs & mask; // look for place in the new size
        probe_count = 1;
        while (names_by_hash[bucketId].second != 0) {
            bucketId = (bucketId + probe_count) & mask;
            probe_count++;
        }
    }

    auto &bucket = names_by_hash[bucketId];
    bucket.first = hs;
    bucket.second = names.size();

    auto idx = names.size();
    names.emplace_back();

    names[idx].kind = CONSTANT;
    names[idx].cnst.original = original;
    categoryCounterInc("names", "constant");
    return NameRef(*this, idx);
}

NameRef GlobalState::enterNameConstant(absl::string_view original) {
    return enterNameConstant(enterNameUTF8(original));
}

void moveNames(pair<unsigned int, unsigned int> *from, pair<unsigned int, unsigned int> *to, unsigned int szFrom,
               unsigned int szTo) {
    // printf("\nResizing name hash table from %u to %u\n", szFrom, szTo);
    ENFORCE((szTo & (szTo - 1)) == 0, "name hash table size corruption");
    ENFORCE((szFrom & (szFrom - 1)) == 0, "name hash table size corruption");
    unsigned int mask = szTo - 1;
    for (unsigned int orig = 0; orig < szFrom; orig++) {
        if (from[orig].second != 0u) {
            auto hs = from[orig].first;
            unsigned int probe = 1;
            auto bucketId = hs & mask;
            while (to[bucketId].second != 0) {
                bucketId = (bucketId + probe) & mask;
                probe++;
            }
            to[bucketId] = from[orig];
        }
    }
}

void GlobalState::expandNames() {
    sanityCheck();
    ENFORCE(names.size() == names.capacity(), "names have wrong capacity");

    names.reserve(names.size() * 2);
    vector<pair<unsigned int, unsigned int>> new_names_by_hash(names_by_hash.capacity() * 2);
    moveNames(names_by_hash.data(), new_names_by_hash.data(), names_by_hash.capacity(), new_names_by_hash.capacity());
    names_by_hash.swap(new_names_by_hash);
}

NameRef GlobalState::freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num) {
    ENFORCE(num > 0, "num == 0, name overflow");
    const auto hs = _hash_mix_unique((u2)uniqueNameKind, UNIQUE, num, original.id());
    unsigned int hashTableSize = names_by_hash.size();
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probe_count = 1;

    while (names_by_hash[bucketId].second != 0 && probe_count < hashTableSize) {
        auto &bucket = names_by_hash[bucketId];
        if (bucket.first == hs) {
            auto &nm2 = names[bucket.second];
            if (nm2.kind == UNIQUE && nm2.unique.uniqueNameKind == uniqueNameKind && nm2.unique.num == num &&
                nm2.unique.original == original) {
                counterInc("names.unique.hit");
                return nm2.ref(*this);
            }
        }
        bucketId = (bucketId + probe_count) & mask;
        probe_count++;
    }
    if (probe_count == hashTableSize) {
        Error::raise("Full table?");
    }
    ENFORCE(!nameTableFrozen);

    if (names.size() == names.capacity()) {
        expandNames();
        hashTableSize = names_by_hash.size();
        mask = hashTableSize - 1;

        bucketId = hs & mask; // look for place in the new size
        probe_count = 1;
        while (names_by_hash[bucketId].second != 0) {
            bucketId = (bucketId + probe_count) & mask;
            probe_count++;
        }
    }

    auto &bucket = names_by_hash[bucketId];
    bucket.first = hs;
    bucket.second = names.size();

    auto idx = names.size();
    names.emplace_back();

    names[idx].kind = UNIQUE;
    names[idx].unique.num = num;
    names[idx].unique.uniqueNameKind = uniqueNameKind;
    names[idx].unique.original = original;
    categoryCounterInc("names", "unique");
    return NameRef(*this, idx);
}

bool fileIsTyped(absl::string_view source) {
    static regex sigil("^\\s*#\\s*@typed\\s*$");
    size_t off = 0;
    // std::regex appears to be ludicrously slow, to the point where running
    // this regex is as slow as running the entirety of the remainder of our
    // pipeline.
    //
    // Help it out by manually scanning for the `@typed` literal, and then
    // running the regex only over the single line.
    while (true) {
        off = source.find("@typed", off);
        if (off == absl::string_view::npos) {
            return false;
        }

        size_t line_start = source.rfind('\n', off);
        if (line_start == absl::string_view::npos) {
            line_start = 0;
        }
        size_t line_end = source.find('\n', off);
        if (line_end == absl::string_view::npos) {
            line_end = source.size();
        }
        if (regex_search(source.data() + line_start, source.data() + line_end, sigil)) {
            return true;
        }
        off = line_end;
    }
}

FileRef GlobalState::enterFile(std::shared_ptr<File> file) {
    ENFORCE(!fileTableFrozen);
    auto idx = files.size();
    files.emplace_back(file);
    return FileRef(*this, idx);
}

FileRef GlobalState::enterFile(absl::string_view path, absl::string_view source) {
    File::Type source_type = File::Untyped;
    if (fileIsTyped(source)) {
        source_type = File::Typed;
    }

    return GlobalState::enterFile(
        std::make_shared<File>(string(path.begin(), path.end()), string(source.begin(), source.end()), source_type));
}

FileRef GlobalState::enterFileAt(absl::string_view path, absl::string_view source, int id) {
    File::Type source_type = File::Untyped;
    if (fileIsTyped(source)) {
        source_type = File::Typed;
    }

    return GlobalState::enterFileAt(
        std::make_shared<File>(string(path.begin(), path.end()), string(source.begin(), source.end()), source_type),
        id);
}

FileRef GlobalState::enterFileAt(std::shared_ptr<File> file, int id) {
    ENFORCE(id >= this->files.size() || this->files[id]->source_type == File::Type::TombStone);
    if (id >= this->files.size()) {
        while (id > this->files.size()) {
            auto file = std::make_shared<File>("", "", File::Type::TombStone);
            this->enterFile(move(file));
        }

        core::FileRef result = this->enterFile(file);
        ENFORCE(result.id() == id);
        return result;
    } else {
        // was a tombstone before.
        this->files[id] = file;
        core::FileRef result(*this, id);
        return result;
    }
}

LocalVariable GlobalState::newTemporary(NameRef name, SymbolRef owner) {
    Symbol &info = owner.info(*this);
    ENFORCE(info.isMethod(), "entering temporary outside of a method");
    int id = ++(info.uniqueCounter);

    LocalVariable ret(name, id);
    return ret;
}

unsigned int GlobalState::symbolsUsed() const {
    return symbols.size();
}

unsigned int GlobalState::filesUsed() const {
    return files.size();
}

unsigned int GlobalState::namesUsed() const {
    return names.size();
}

string GlobalState::toString(bool showHidden) const {
    return defn_root().toString(*this, 0, showHidden);
}

void GlobalState::sanityCheck() const {
    if (!debug_mode) {
        return;
    }
    ENFORCE(!names.empty(), "empty name table size");
    ENFORCE(!strings.empty(), "empty string table size");
    ENFORCE(!names_by_hash.empty(), "empty name hash table size");
    ENFORCE((names_by_hash.size() & (names_by_hash.size() - 1)) == 0, "name hash table size is not a power of two");
    ENFORCE(names.capacity() * 2 == names_by_hash.capacity(), "name table and hash name table sizes out of sync");
    ENFORCE(names_by_hash.size() == names_by_hash.capacity(), "hash name table not at full capacity");
    int i = -1;
    for (auto &nm : names) {
        i++;
        if (i != 0) {
            nm.sanityCheck(*this);
        }
    }

    i = -1;
    for (auto &sym : symbols) {
        i++;
        if (i != 0) {
            sym.sanityCheck(*this);
        }
    }
    for (auto &ent : names_by_hash) {
        if (ent.second == 0) {
            continue;
        }
        const Name &nm = names[ent.second];
        ENFORCE(ent.first == nm.hash(*this), "name hash table corruption");
    }
}

bool GlobalState::freezeNameTable() {
    bool old = this->nameTableFrozen;
    this->nameTableFrozen = true;
    return old;
}

bool GlobalState::freezeFileTable() {
    bool old = this->fileTableFrozen;
    this->fileTableFrozen = true;
    return old;
}

bool GlobalState::freezeSymbolTable() {
    bool old = this->symbolTableFrozen;
    this->symbolTableFrozen = true;
    return old;
}

bool GlobalState::unfreezeNameTable() {
    bool old = this->nameTableFrozen;
    this->nameTableFrozen = false;
    return old;
}

bool GlobalState::unfreezeFileTable() {
    bool old = this->fileTableFrozen;
    this->fileTableFrozen = false;
    return old;
}

bool GlobalState::unfreezeSymbolTable() {
    bool old = this->symbolTableFrozen;
    this->symbolTableFrozen = false;
    return old;
}

std::unique_ptr<GlobalState> GlobalState::deepCopy() const {
    this->sanityCheck();
    auto result = make_unique<GlobalState>(this->logger);

    result->strings = this->strings;
    result->strings_last_page_used = STRINGS_PAGE_SIZE;
    result->files = this->files;

    result->names.reserve(this->names.capacity());
    for (auto &nm : this->names) {
        result->names.emplace_back(nm.deepCopy(*result));
    }

    result->names_by_hash.reserve(this->names_by_hash.size());
    result->names_by_hash = this->names_by_hash;

    result->symbols.reserve(this->symbols.size());
    for (auto &sym : this->symbols) {
        result->symbols.emplace_back(sym.deepCopy(*result));
    }
    result->sanityCheck();
    return result;
}

string GlobalState::showAnnotatedSource(FileRef file) {
    if (annotations.empty()) {
        return "";
    }

    // Sort the locs backwards
    auto compare = [](Annotation left, Annotation right) {
        if (left.loc.file != right.loc.file) {
            return left.loc.file.id() > right.loc.file.id();
        }

        auto a = left.pos == GlobalState::AnnotationPos::BEFORE ? left.loc.begin_pos : left.loc.end_pos;
        auto b = right.pos == GlobalState::AnnotationPos::BEFORE ? right.loc.begin_pos : right.loc.end_pos;

        if (a != b) {
            return a > b;
        }
        if (left.pos != right.pos) {
            if (left.pos == GlobalState::AnnotationPos::BEFORE && right.pos == GlobalState::AnnotationPos::AFTER) {
                return false;
            }
            if (left.pos == GlobalState::AnnotationPos::AFTER && right.pos == GlobalState::AnnotationPos::BEFORE) {
                return true;
            }
        }
        return false;
    };
    sort(annotations.begin(), annotations.end(), compare);

    auto source = file.file(*this).source();
    string outline(source.begin(), source.end());
    for (auto annotation : annotations) {
        if (annotation.loc.file != file) {
            continue;
        }
        stringstream buf;

        auto pos = annotation.loc.position(*this);
        std::vector<std::string> lines = absl::StrSplit(annotation.str, "\n");
        while (!lines.empty() && lines.back().empty()) {
            lines.pop_back();
        }
        if (!lines.empty()) {
            buf << endl;
            for (auto line : lines) {
                for (int p = 1; p < pos.first.column; p++) {
                    buf << " ";
                }
                if (line.empty()) {
                    // Avoid the trailing space
                    buf << "#";
                } else {
                    buf << "# " << line;
                }
                buf << endl;
            }
        }
        auto out = buf.str();
        out = out.substr(0, out.size() - 1); // Remove the last newline that the buf always has

        size_t start_of_line;
        switch (annotation.pos) {
            case GlobalState::AnnotationPos::BEFORE:
                start_of_line = annotation.loc.begin_pos;
                start_of_line = outline.find_last_of('\n', start_of_line);
                if (start_of_line == std::string::npos) {
                    start_of_line = 0;
                }
                break;
            case GlobalState::AnnotationPos::AFTER:
                start_of_line = annotation.loc.end_pos;
                start_of_line = outline.find_first_of('\n', start_of_line);
                if (start_of_line == std::string::npos) {
                    start_of_line = outline.end() - outline.begin();
                }
                break;
        }
        outline = outline.substr(0, start_of_line) + out + outline.substr(start_of_line);
    }
    return outline;
}

int GlobalState::totalErrors() const {
    int sum = 0;
    for (const auto &e : this->errors.histogram) {
        sum += e.second;
    }
    return sum;
}

void GlobalState::_error(unique_ptr<BasicError> error) const {
    File::Type source_type = File::Typed;
    if (error->loc.file.exists()) {
        source_type = error->loc.file.file(*this).source_type;
    }

    switch (error->what.code) {
        case errors::Internal::InternalError.code:
            error->isCritical = true;
            this->errors.hadCritical = true;
            break;
        case errors::Parser::ParserError.code:
        case errors::Resolver::InvalidMethodSignature.code:
        case errors::Resolver::InvalidTypeDeclaration.code:
        case errors::Resolver::InvalidDeclareVariables.code:
        case errors::Resolver::DuplicateVariableDeclaration.code:
            // These are always shown, even for untyped source
            break;

        default:
            if (source_type == File::Untyped) {
                return;
            }
    }

    auto f = find_if(this->errors.histogram.begin(), this->errors.histogram.end(),
                     [&](auto &el) -> bool { return el.first == error->what.code; });
    if (f != this->errors.histogram.end()) {
        (*f).second++;
    } else {
        this->errors.histogram.push_back(std::make_pair((int)error->what.code, 1));
    }
    this->errors.buffer.emplace_back(move(error));
}

void GlobalState::flushErrors() {
    if (this->errors.buffer.empty()) {
        return;
    }
    stringstream critical;
    stringstream nonCritical;
    for (auto &error : this->errors.buffer) {
        auto &out = error->isCritical ? critical : nonCritical;
        if (out.tellp() != 0) {
            out << '\n';
        }
        out << error->toString(*this);
    }

    if (critical.tellp() != 0) {
        this->logger.log(spdlog::level::critical, "{}", critical.str());
    }
    if (nonCritical.tellp() != 0) {
        this->logger.log(spdlog::level::err, "{}", nonCritical.str());
    }
    this->errors.buffer.clear();
}

std::vector<std::unique_ptr<BasicError>> GlobalState::drainErrors() {
    std::vector<std::unique_ptr<BasicError>> res;
    swap(res, this->errors.buffer);
    return res;
}

} // namespace core
} // namespace ruby_typer
