#include "GlobalState.h"
#include "ErrorQueue.h"
#include "Types.h"
#include "core/Names/core.h"
#include "core/errors/errors.h"
#include <regex>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"

template class std::vector<std::pair<unsigned int, unsigned int>>;
template class std::shared_ptr<ruby_typer::core::GlobalState>;

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
const char *ruby_typer_str = "RubyTyper";
const char *stub_str = "StubClass";
// This fills in all the way up to MAX_SYNTHETIC_SYMBOLS
const char *reserved_str = "<<RESERVED>>";
} // namespace

SymbolRef GlobalState::synthesizeClass(absl::string_view name, u4 superclass, bool isModule) {
    NameRef nameId = enterNameConstant(name);

    // This can't use enterClass since there is a chicken and egg problem.
    // These will be added to Symbols::root().members later.
    SymbolRef symRef = SymbolRef(this, symbols.size());
    symbols.emplace_back();
    Symbol &data = symRef.data(*this, true); // allowing noSymbol is needed because this enters noSymbol.
    data.name = nameId;
    data.owner = Symbols::root();
    data.superClass = SymbolRef(this, superclass);
    data.flags = 0;
    data.setClass();
    data.setIsModule(isModule);

    if (symRef._id > Symbols::root()._id) {
        Symbols::root().data(*this, true).members.push_back(make_pair(nameId, symRef));
    }
    return symRef;
}

int globalStateIdCounter = 1;
const int Symbols::MAX_PROC_ARITY;

GlobalState::GlobalState(std::shared_ptr<ErrorQueue> errorQueue)
    : globalStateId(globalStateIdCounter++), errorQueue(errorQueue) {
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
    Symbols::root().data(*this, true).members.push_back(make_pair(enterNameConstant(no_symbol_str), no_symbol_id));
    Symbols::root().data(*this, true).members.push_back(make_pair(enterNameConstant(top_str), top_id));
    Symbols::root().data(*this, true).members.push_back(make_pair(enterNameConstant(bottom_str), bottom_id));
    SymbolRef nil_id = synthesizeClass(nil_str);
    SymbolRef todo_id = synthesizeClass(todo_str, 0);
    SymbolRef object_id = synthesizeClass(object_str, core::Symbols::BasicObject()._id);
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

    SymbolRef T_id = synthesizeClass(T_str, core::Symbols::todo()._id, true);
    T_id.data(*this).setIsModule(true);

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
    SymbolRef T_Array_id = enterClassSymbol(Loc::none(), Symbols::T(), enterNameConstant(array_str));
    SymbolRef T_Hash_id = enterClassSymbol(Loc::none(), Symbols::T(), enterNameConstant(hash_str));
    SymbolRef T_Proc_id = enterClassSymbol(Loc::none(), Symbols::T(), enterNameConstant(proc_str));
    SymbolRef proc_id = synthesizeClass(proc_str);
    SymbolRef T_any_id = enterMethodSymbol(Loc::none(), Symbols::T(), Names::any());
    SymbolRef T_all_id = enterMethodSymbol(Loc::none(), Symbols::T(), Names::all());
    SymbolRef T_untyped_id = enterMethodSymbol(Loc::none(), Symbols::T(), Names::untyped());
    SymbolRef T_nilable_id = enterMethodSymbol(Loc::none(), Symbols::T(), Names::nilable());
    SymbolRef enumerable_id = synthesizeClass(enumerable_str, 0, true);
    SymbolRef set_id = synthesizeClass(set_str);
    SymbolRef struct_id = synthesizeClass(struct_str);
    SymbolRef file_id = synthesizeClass(file_str);
    SymbolRef ruby_typer_id = synthesizeClass(ruby_typer_str, 0, true);
    SymbolRef stub_id = enterClassSymbol(Loc::none(), ruby_typer_id, enterNameConstant(stub_str));
    SymbolRef T_Enumerable_id = enterClassSymbol(Loc::none(), Symbols::T(), enterNameConstant(enumerable_str));

    ENFORCE(no_symbol_id == Symbols::noSymbol(), "no symbol creation failed");
    ENFORCE(top_id == Symbols::top(), "top symbol creation failed");
    ENFORCE(bottom_id == Symbols::bottom(), "bottom symbol creation failed");
    ENFORCE(root_id == Symbols::root(), "root symbol creation failed");
    ENFORCE(nil_id == Symbols::nil(), "nil symbol creation failed");
    ENFORCE(todo_id == Symbols::todo(), "todo symbol creation failed");
    ENFORCE(object_id == Symbols::Object(), "object symbol creation failed");
    ENFORCE(junk_id == Symbols::junk(), "junk symbol creation failed");
    ENFORCE(integer_id == Symbols::Integer(), "Integer symbol creation failed");
    ENFORCE(float_id == Symbols::Float(), "Float symbol creation failed");
    ENFORCE(string_id == Symbols::String(), "String symbol creation failed");
    ENFORCE(symbol_id == Symbols::Symbol(), "Symbol symbol creation failed");
    ENFORCE(array_id == Symbols::Array(), "Array symbol creation failed");
    ENFORCE(hash_id == Symbols::Hash(), "Hash symbol creation failed");
    ENFORCE(trueClass_id == Symbols::TrueClass(), "TrueClass symbol creation failed");
    ENFORCE(falseClass_id == Symbols::FalseClass(), "FalseClass symbol creation failed");
    ENFORCE(nilClass_id == Symbols::NilClass(), "NilClass symbol creation failed");
    ENFORCE(untyped_id == Symbols::untyped(), "untyped symbol creation failed");
    ENFORCE(opus_id == Symbols::Opus(), "Opus symbol creation failed");
    ENFORCE(T_id == Symbols::T(), "T symbol creation failed");
    ENFORCE(class_id == Symbols::Class(), "Class symbol creation failed");
    ENFORCE(basicObject_id == Symbols::BasicObject(), "BasicObject symbol creation failed");
    ENFORCE(kernel_id == Symbols::Kernel(), "Kernel symbol creation failed");
    ENFORCE(range_id == Symbols::Range(), "Range symbol creation failed");
    ENFORCE(regexp_id == Symbols::Regexp(), "Regexp symbol creation failed");
    ENFORCE(magic_id == Symbols::Magic(), "Magic symbol creation failed");
    ENFORCE(module_id == Symbols::Module(), "Module symbol creation failed");
    ENFORCE(standardError_id == Symbols::StandardError(), "StandardError symbol creation failed");
    ENFORCE(complex_id == Symbols::Complex(), "Complex symbol creation failed");
    ENFORCE(rational_id == Symbols::Rational(), "Rational symbol creation failed");
    ENFORCE(T_Array_id = Symbols::T_Array(), "T::Array symbol creation failed");
    ENFORCE(T_Hash_id = Symbols::T_Hash(), "T::Hash symbol creation failed");
    ENFORCE(T_Proc_id = Symbols::T_Proc(), "T::Proc symbol creation failed");
    ENFORCE(proc_id = Symbols::Proc(), "Proc symbol creation failed");
    ENFORCE(T_any_id = Symbols::T_any());
    ENFORCE(T_all_id = Symbols::T_all());
    ENFORCE(T_untyped_id = Symbols::T_untyped());
    ENFORCE(T_nilable_id = Symbols::T_nilable());
    ENFORCE(enumerable_id = Symbols::Enumerable());
    ENFORCE(set_id = Symbols::Set());
    ENFORCE(struct_id = Symbols::Struct());
    ENFORCE(file_id = Symbols::File());
    ENFORCE(ruby_typer_id = Symbols::RubyTyper());
    ENFORCE(stub_id = Symbols::StubClass());
    ENFORCE(T_Enumerable_id = Symbols::T_Enumerable());

    // Synthesize nil = NilClass()
    Symbols::nil().data(*this).resultType = core::Types::nil();

    // Synthesize untyped = dynamic()
    Symbols::untyped().data(*this).resultType = core::Types::dynamic();

    // <Magic> has a special Type
    Symbols::Magic().data(*this).resultType = make_shared<MagicType>();

    // Synthesize <Magic>#build_hash(*vs : Object) => Hash
    SymbolRef method = enterMethodSymbol(Loc::none(), Symbols::Magic(), Names::buildHash());
    SymbolRef arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
    arg.data(*this).setRepeated();
    arg.data(*this).resultType = core::Types::Object();
    method.data(*this).arguments().push_back(arg);
    method.data(*this).resultType = core::Types::hashOfUntyped();

    // Synthesize <Magic>#build_array(*vs : Object) => Array
    method = enterMethodSymbol(Loc::none(), Symbols::Magic(), Names::buildArray());
    arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
    arg.data(*this).setRepeated();
    arg.data(*this).resultType = core::Types::Object();
    method.data(*this).arguments().push_back(arg);
    method.data(*this).resultType = core::Types::arrayOfUntyped();

    // Synthesize <Magic>#<splat>(a: Array) => Untyped
    method = enterMethodSymbol(Loc::none(), Symbols::Magic(), Names::splat());
    arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
    arg.data(*this).resultType = core::Types::arrayOfUntyped();
    method.data(*this).arguments().push_back(arg);
    method.data(*this).resultType = core::Types::dynamic();

    // TODO(pay-server) Synthesize ::M = ::Opus::DB::Model
    //
    // This is a hack to handle that specific alias in pay-server; More-general
    // handling will require substantial additional sophistication in the
    // namer+resolver.

    SymbolRef db = enterClassSymbol(Loc::none(), Symbols::Opus(), enterNameConstant(DB_str));
    db.data(*this).setIsModule(true);

    SymbolRef model = enterClassSymbol(Loc::none(), db, enterNameConstant(model_str));
    model.data(*this).setIsModule(true);

    SymbolRef m = enterStaticFieldSymbol(Loc::none(), Symbols::root(), enterNameConstant(m_str));
    m.data(*this).resultType = make_unique<AliasType>(model);

    int reservedCount = 0;

    // Set the correct resultTypes for all synthesized classes
    // Does it in two passes since the singletonClass will go in the Symbols::root() members which will invalidate the
    // iterator
    vector<SymbolRef> needsResultType;
    for (auto &data : symbols) {
        auto ref = data.ref(*this);
        if (ref._id < Symbols::root()._id) {
            // These aren't real classes and won't have singleton classes
            continue;
        }
        if (data.isClass() && !data.resultType) {
            needsResultType.emplace_back(data.ref(*this));
        }
    }
    for (auto sym : needsResultType) {
        Symbol &data = sym.data(*this);
        data.resultType = make_unique<core::ClassType>(data.singletonClass(*this));
    }

    while (symbols.size() < Symbols::MAX_SYNTHETIC_SYMBOLS - Symbols::MAX_PROC_ARITY - 1) {
        std::string res(reserved_str);
        res = res + to_string(reservedCount);
        synthesizeClass(res);
        reservedCount++;
    }

    for (int arity = 0; arity <= Symbols::MAX_PROC_ARITY; ++arity) {
        auto id = synthesizeClass(absl::StrCat("Proc", arity), Symbols::Proc()._id);
        ENFORCE(id == Symbols::Proc(arity), "Proc creation failed for arity: ", arity);
    }

    // First file is used to indicate absence of a file
    files.emplace_back();

    freezeNameTable();
    freezeSymbolTable();
    freezeFileTable();
    ENFORCE(symbols.size() == Symbols::last_synthetic_sym()._id + 1, "Too many synthetic symbols?");
    sanityCheck();
}

GlobalState::~GlobalState(){};

constexpr decltype(GlobalState::STRINGS_PAGE_SIZE) GlobalState::STRINGS_PAGE_SIZE;

SymbolRef GlobalState::enterSymbol(Loc loc, SymbolRef owner, NameRef name, u4 flags) {
    ENFORCE(owner.exists(), "entering symbol in to non-existing owner");
    ENFORCE(name.exists(), "entering symbol with non-existing name");
    Symbol &ownerScope = owner.data(*this, true);
    auto from = ownerScope.members.begin();
    auto to = ownerScope.members.end();
    histogramInc("symbol_enter_by_name", ownerScope.members.size());

    while (from != to) {
        auto &el = *from;
        if (el.first == name) {
            ENFORCE((from->second.data(*this).flags & flags) == flags, "existing symbol has wrong flags");
            counterInc("symbols.hit");
            return from->second;
        }
        from++;
    }
    ENFORCE(!symbolTableFrozen);

    bool reallocate = symbols.size() == symbols.capacity();

    SymbolRef ret = SymbolRef(this, symbols.size());
    symbols.emplace_back();
    Symbol &data = ret.data(*this, true);
    data.name = name;
    data.flags = flags;
    data.owner = owner;
    data.definitionLoc = loc;
    if (data.isBlockSymbol(*this)) {
        categoryCounterInc("symbols", "block");
    } else if (data.isClass()) {
        categoryCounterInc("symbols", "class");
    } else if (data.isMethod()) {
        categoryCounterInc("symbols", "method");
    } else if (data.isField()) {
        categoryCounterInc("symbols", "field");
    } else if (data.isStaticField()) {
        categoryCounterInc("symbols", "static_field");
    } else if (data.isMethodArgument()) {
        categoryCounterInc("symbols", "argument");
    }

    if (!reallocate) {
        ownerScope.members.push_back(make_pair(name, ret));
    } else {
        owner.data(*this, true).members.push_back(make_pair(name, ret));
    }
    return ret;
}

SymbolRef GlobalState::enterClassSymbol(Loc loc, SymbolRef owner, NameRef name) {
    ENFORCE(name.data(*this).isClassName(*this));
    return enterSymbol(loc, owner, name, Symbol::Flags::CLASS);
}

SymbolRef GlobalState::enterTypeMember(Loc loc, SymbolRef owner, NameRef name, Variance variance) {
    u4 flags;
    ENFORCE(owner.data(*this).isClass());
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
    owner.data(*this).typeMembers().emplace_back(result);
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
    owner.data(*this).typeMembers().emplace_back(result);
    return result;
}

SymbolRef GlobalState::enterMethodSymbol(Loc loc, SymbolRef owner, NameRef name) {
    bool isBlock = name.data(*this).kind == NameKind::UNIQUE && name.data(*this).unique.original == Names::blockTemp();
    ENFORCE(isBlock || owner.data(*this).isClass(), "entering method symbol into not-a-class");
    return enterSymbol(loc, owner, name, Symbol::Flags::METHOD);
}

SymbolRef GlobalState::enterNewMethodOverload(Loc loc, SymbolRef original, u2 num) {
    NameRef name = freshNameUnique(UniqueNameKind::Overload, original.data(*this).name, num);
    SymbolRef res = enterMethodSymbol(loc, original.data(*this).owner, name);
    res.data(*this).argumentsOrMixins.reserve(original.data(*this).argumentsOrMixins.size());
    for (auto &arg : original.data(*this).argumentsOrMixins) {
        Loc loc = arg.data(*this).definitionLoc;
        NameRef nm = arg.data(*this).name;
        SymbolRef newArg = enterMethodArgumentSymbol(loc, res, nm);
        newArg.data(*this).flags = arg.data(*this).flags;
        res.data(*this).argumentsOrMixins.push_back(newArg);
    }
    return res;
}

SymbolRef GlobalState::enterFieldSymbol(Loc loc, SymbolRef owner, NameRef name) {
    ENFORCE(owner.data(*this).isClass(), "entering field symbol into not-a-class");
    return enterSymbol(loc, owner, name, Symbol::Flags::FIELD);
}

SymbolRef GlobalState::enterStaticFieldSymbol(Loc loc, SymbolRef owner, NameRef name) {
    return enterSymbol(loc, owner, name, Symbol::Flags::STATIC_FIELD);
}

SymbolRef GlobalState::enterMethodArgumentSymbol(Loc loc, SymbolRef owner, NameRef name) {
    ENFORCE(owner.data(*this).isMethod(), "entering method argument symbol into not-a-method");
    return enterSymbol(loc, owner, name, Symbol::Flags::METHOD_ARGUMENT);
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
    ENFORCE(original.data(*this).kind == UTF8, "making a constant name over wrong name kind");

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
    return FileRef(idx);
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
        core::FileRef result(id);
        return result;
    }
}

void GlobalState::mangleRenameSymbol(SymbolRef what, NameRef origName, UniqueNameKind kind) {
    auto &owner = what.data(*this).owner;
    auto &members = owner.data(*this).members;
    for (auto it = members.begin(); it != members.end(); ++it) {
        if (it->first == origName) {
            int collisionCount = 1;
            core::NameRef name;
            do {
                name = freshNameUnique(kind, origName, collisionCount++);
            } while (owner.data(*this).findMember(*this, name).exists());
            it->first = name;
            it->second.data(*this).name = it->first;
            break;
        }
    }
}

LocalVariable GlobalState::newTemporary(NameRef name, SymbolRef owner) {
    Symbol &data = owner.data(*this);
    ENFORCE(data.isMethod(), "entering temporary outside of a method");
    int id = ++(data.uniqueCounter);

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
    return Symbols::root().toString(*this, 0, showHidden);
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

std::unique_ptr<GlobalState> GlobalState::deepCopy(bool keepId) const {
    this->sanityCheck();
    auto result = make_unique<GlobalState>(this->errorQueue);

    if (keepId) {
        result->globalStateId = this->globalStateId;
    }

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

    auto source = file.data(*this).source();
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
    return errorQueue->queue.enqueuedEstimate();
}

void GlobalState::_error(unique_ptr<BasicError> error) const {
    File::Type source_type = File::Typed;
    if (error->loc.file.exists()) {
        source_type = error->loc.file.data(*this).source_type;
    }

    switch (error->what.code) {
        case errors::Internal::InternalError.code:
            error->isCritical = true;
            errorQueue->hadCritical = true;
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

    histogramAdd("error", error->what.code, 1);
    ErrorQueueMessage msg;
    msg.kind = ErrorQueueMessage::Kind::Error;
    msg.whatFile = error->loc.file;
    msg.text = error->toString(*this);
    msg.error = move(error);
    errorQueue->queue.push(move(msg), 1);
}

bool GlobalState::hadCriticalError() const {
    return errorQueue->hadCritical;
}

void GlobalState::flushErrors() {
    this->errorQueue->flushErrors();
}

} // namespace core
} // namespace ruby_typer
