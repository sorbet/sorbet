#include "GlobalState.h"
#include "Types.h"

using namespace std;

namespace ruby_typer {
namespace core {

namespace {
const char *top_str = "<top>";
UTF8Desc top_DESC{(char *)top_str, (int)strlen(top_str)};

const char *bottom_str = "<bottom>";
UTF8Desc bottom_DESC{(char *)bottom_str, (int)strlen(bottom_str)};

const char *untyped_str = "untyped";
UTF8Desc untyped_DESC{(char *)untyped_str, (int)strlen(untyped_str)};

const char *root_str = "<root>";
UTF8Desc root_DESC{(char *)root_str, (int)strlen(root_str)};

const char *nil_str = "nil";
UTF8Desc nil_DESC{(char *)nil_str, (int)strlen(nil_str)};

const char *object_str = "Object";
UTF8Desc object_DESC{(char *)object_str, (int)strlen(object_str)};

const char *junk_str = "<<JUNK>>";
UTF8Desc junk_DESC{(char *)junk_str, (int)strlen(junk_str)};

const char *string_str = "String";
UTF8Desc string_DESC{(char *)string_str, (int)strlen(string_str)};

const char *integer_str = "Integer";
UTF8Desc integer_DESC{(char *)integer_str, (int)strlen(integer_str)};

const char *float_str = "Float";
UTF8Desc float_DESC{(char *)float_str, (int)strlen(float_str)};

const char *symbol_str = "Symbol";
UTF8Desc symbol_DESC{(char *)symbol_str, (int)strlen(symbol_str)};

const char *array_str = "Array";
UTF8Desc array_DESC{(char *)array_str, (int)strlen(array_str)};

const char *hash_str = "Hash";
UTF8Desc hash_DESC{(char *)hash_str, (int)strlen(hash_str)};

const char *trueClass_str = "TrueClass";
UTF8Desc trueClass_DESC{(char *)trueClass_str, (int)strlen(trueClass_str)};

const char *falseClass_str = "FalseClass";
UTF8Desc falseClass_DESC{(char *)falseClass_str, (int)strlen(falseClass_str)};

const char *nilClass_str = "NilClass";
UTF8Desc nilClass_DESC{(char *)nilClass_str, (int)strlen(nilClass_str)};

const char *class_str = "Class";
UTF8Desc class_DESC{(char *)class_str, (int)strlen(class_str)};

const char *merge = "merge";
UTF8Desc merge_DESC{(char *)merge, (int)strlen(merge)};

const char *todo_str = "<todo sym>";
UTF8Desc todo_DESC{(char *)todo_str, (int)strlen(todo_str)};

const char *no_symbol_str = "<none>";
UTF8Desc no_symbol_DESC{(char *)no_symbol_str, (int)strlen(no_symbol_str)};

const char *opus = "Opus";
UTF8Desc opus_DESC{(char *)opus, (int)strlen(opus)};

const char *types = "Types";
UTF8Desc types_DESC{(char *)types, (int)strlen(types)};

const char *basicObject = "BasicObject";
UTF8Desc basicObject_DESC{(char *)basicObject, (int)strlen(basicObject)};

const char *kernel = "Kernel";
UTF8Desc kernel_DESC{(char *)kernel, (int)strlen(kernel)};

const char *range = "Range";
UTF8Desc range_DESC{(char *)range, (int)strlen(range)};

const char *regexp = "Regexp";
UTF8Desc regexp_DESC{(char *)regexp, (int)strlen(regexp)};

// A magic non user-creatable class with methods to keep state between passes
const char *magic = "<Magic>";
UTF8Desc magic_DESC{(char *)magic, (int)strlen(magic)};

// This fills in all the way up to MAX_SYNTHETIC_SYMBOLS
const char *reserved = "<<RESERVED>>";
UTF8Desc reserved_DESC{(char *)reserved, (int)strlen(reserved)};
} // namespace

SymbolRef GlobalState::synthesizeClass(UTF8Desc name, SymbolRef superclass) {
    NameRef nameId = enterNameConstant(name);

    // This can't use enterClass since there is a chicken and egg problem.
    // These will be added to defn_root().members later.
    SymbolRef symRef = SymbolRef(symbols.size());
    symbols.emplace_back();
    Symbol &info = symRef.info(*this, true); // allowing noSymbol is needed because this enters noSymbol.
    info.name = nameId;
    info.owner = defn_root();
    info.superClass = superclass;
    info.flags = 0;
    info.setClass();

    if (symRef._id > GlobalState::defn_root()._id) {
        GlobalState::defn_root().info(*this, true).members.push_back(make_pair(nameId, symRef));
    }
    return symRef;
}

GlobalState::GlobalState(spdlog::logger &logger) : logger(logger), errors(*this) {
    unsigned int max_name_count = 262144;   // 6MB
    unsigned int max_symbol_count = 524288; // 32MB

    names.reserve(max_name_count);
    symbols.reserve(max_symbol_count);
    int names_by_hash_size = 2 * max_name_count;
    names_by_hash.resize(names_by_hash_size);
    DEBUG_ONLY(Error::check((names_by_hash_size & (names_by_hash_size - 1)) == 0));

    names.emplace_back(); // first name is used in hashes to indicate empty cell
    names[0].kind = NameKind::UTF8;
    names[0].raw.utf8.from = nullptr;
    names[0].raw.utf8.to = 0;
    Names::registerNames(*this);

    SymbolRef no_symbol_id = synthesizeClass(no_symbol_DESC, 0);
    SymbolRef top_id = synthesizeClass(top_DESC, 0);
    SymbolRef bottom_id = synthesizeClass(bottom_DESC, 0);
    SymbolRef root_id = synthesizeClass(root_DESC, 0);
    SymbolRef nil_id = synthesizeClass(nil_DESC);
    SymbolRef todo_id = synthesizeClass(todo_DESC, 0);
    SymbolRef object_id = synthesizeClass(object_DESC, core::GlobalState::defn_BasicObject());
    SymbolRef junk_id = synthesizeClass(junk_DESC, 0);
    SymbolRef integer_id = synthesizeClass(integer_DESC);
    SymbolRef float_id = synthesizeClass(float_DESC);
    SymbolRef string_id = synthesizeClass(string_DESC);
    SymbolRef symbol_id = synthesizeClass(symbol_DESC);
    SymbolRef array_id = synthesizeClass(array_DESC);
    SymbolRef hash_id = synthesizeClass(hash_DESC);
    SymbolRef trueClass_id = synthesizeClass(trueClass_DESC);
    SymbolRef falseClass_id = synthesizeClass(falseClass_DESC);
    SymbolRef nilClass_id = synthesizeClass(nilClass_DESC);
    SymbolRef untyped_id = synthesizeClass(untyped_DESC, 0);
    SymbolRef opus_id = synthesizeClass(opus_DESC, 0);
    SymbolRef opus_types_id = enterClassSymbol(Loc::none(0), opus_id, enterNameConstant(types_DESC));
    SymbolRef class_id = synthesizeClass(class_DESC, 0);
    SymbolRef basicObject_id = synthesizeClass(basicObject_DESC, 0);
    SymbolRef kernel_id = synthesizeClass(kernel_DESC, 0);
    SymbolRef range_id = synthesizeClass(range_DESC);
    SymbolRef regexp_id = synthesizeClass(regexp_DESC);
    SymbolRef magic_id = synthesizeClass(magic_DESC);

    Error::check(no_symbol_id == noSymbol());
    Error::check(top_id == defn_top());
    Error::check(bottom_id == defn_bottom());
    Error::check(root_id == defn_root());
    Error::check(nil_id == defn_nil());
    Error::check(todo_id == defn_todo());
    Error::check(object_id == defn_Object());
    Error::check(junk_id == defn_junk());
    Error::check(integer_id == defn_Integer());
    Error::check(float_id == defn_Float());
    Error::check(string_id == defn_String());
    Error::check(symbol_id == defn_Symbol());
    Error::check(array_id == defn_Array());
    Error::check(hash_id == defn_Hash());
    Error::check(trueClass_id == defn_TrueClass());
    Error::check(falseClass_id == defn_FalseClass());
    Error::check(nilClass_id == defn_NilClass());
    Error::check(untyped_id == defn_untyped());
    Error::check(opus_id == defn_Opus());
    Error::check(opus_types_id == defn_Opus_Types());
    Error::check(class_id == defn_Class());
    Error::check(basicObject_id == defn_BasicObject());
    Error::check(kernel_id == defn_Kernel());
    Error::check(range_id == defn_Range());
    Error::check(regexp_id == defn_Regexp());
    Error::check(magic_id == defn_Magic());

    // Synthesize nil = NilClass()
    defn_nil().info(*this).resultType = make_unique<ClassType>(defn_NilClass());

    // <Magic> has a special Type
    defn_Magic().info(*this).resultType = make_shared<MagicType>();

    // Synthesize <Magic>#build_hash(*vs : Object) => Hash
    SymbolRef method = enterMethodSymbol(Loc::none(0), defn_Magic(), Names::buildHash());
    SymbolRef arg = enterMethodArgumentSymbol(Loc::none(0), method, Names::arg0());
    arg.info(*this).setRepeated();
    arg.info(*this).resultType = make_unique<ClassType>(defn_Object());
    method.info(*this).arguments().push_back(arg);
    method.info(*this).resultType = make_unique<ClassType>(defn_Hash());

    // Synthesize <Magic>#build_array(*vs : Object) => Array
    method = enterMethodSymbol(Loc::none(0), defn_Magic(), Names::buildArray());
    arg = enterMethodArgumentSymbol(Loc::none(0), method, Names::arg0());
    arg.info(*this).setRepeated();
    arg.info(*this).resultType = make_unique<ClassType>(defn_Object());
    method.info(*this).arguments().push_back(arg);
    method.info(*this).resultType = make_unique<ClassType>(defn_Array());

    // Synthesize <Magic>#<splat>(a: Array) => Untyped
    method = enterMethodSymbol(Loc::none(0), defn_Magic(), Names::splat());
    arg = enterMethodArgumentSymbol(Loc::none(0), method, Names::arg0());
    arg.info(*this).resultType = make_unique<ClassType>(defn_Array());
    method.info(*this).arguments().push_back(arg);
    method.info(*this).resultType = make_unique<ClassType>(defn_untyped());

    while (symbols.size() < GlobalState::MAX_SYNTHETIC_SYMBOLS) {
        synthesizeClass(reserved_DESC);
    }

    Error::check(symbols.size() == defn_last_synthetic_sym()._id + 1);
}

GlobalState::~GlobalState() = default;

constexpr decltype(GlobalState::STRINGS_PAGE_SIZE) GlobalState::STRINGS_PAGE_SIZE;

SymbolRef GlobalState::enterSymbol(Loc loc, SymbolRef owner, NameRef name, u4 flags) {
    DEBUG_ONLY(Error::check(owner.exists()));
    Error::check(name.exists());
    Symbol &ownerScope = owner.info(*this, true);
    auto from = ownerScope.members.begin();
    auto to = ownerScope.members.end();
    while (from != to) {
        auto &el = *from;
        if (el.first == name) {
            Error::check((from->second.info(*this).flags & flags) == flags);
            return from->second;
        }
        from++;
    }

    bool reallocate = symbols.size() == symbols.capacity();

    SymbolRef ret = SymbolRef(symbols.size());
    symbols.emplace_back();
    Symbol &info = ret.info(*this, true);
    info.name = name;
    info.flags = flags;
    info.owner = owner;
    info.definitionLoc = loc;

    if (!reallocate) {
        ownerScope.members.push_back(make_pair(name, ret));
    } else {
        owner.info(*this, true).members.push_back(make_pair(name, ret));
    }
    return ret;
}

SymbolRef GlobalState::enterClassSymbol(Loc loc, SymbolRef owner, NameRef name) {
    return enterSymbol(loc, owner, name, Symbol::Flags::CLASS);
}

SymbolRef GlobalState::enterMethodSymbol(Loc loc, SymbolRef owner, NameRef name) {
    return enterSymbol(loc, owner, name, Symbol::Flags::METHOD);
}

SymbolRef GlobalState::enterFieldSymbol(Loc loc, SymbolRef owner, NameRef name) {
    return enterSymbol(loc, owner, name, Symbol::Flags::FIELD);
}

SymbolRef GlobalState::enterStaticFieldSymbol(Loc loc, SymbolRef owner, NameRef name) {
    return enterSymbol(loc, owner, name, Symbol::Flags::STATIC_FIELD);
}

SymbolRef GlobalState::enterMethodArgumentSymbol(Loc loc, SymbolRef owner, NameRef name) {
    return enterSymbol(loc, owner, name, Symbol::Flags::METHOD_ARGUMENT);
}

LocalVariable GlobalState::enterLocalSymbol(SymbolRef owner, NameRef name) {
    // THIS IS NOT TRUE. Top level code is still a thing
    // Error::check(owner.info(*this).isMethod());
    if (owner.info(*this).isBlockSymbol(*this) && !name.isBlockClashSafe(*this)) {
        name = freshNameUnique(UniqueNameKind::NestedScope, name, owner._id);
    }
    LocalVariable r(name);
    return r;
}

UTF8Desc GlobalState::enterString(UTF8Desc nm) {
    char *from = nullptr;
    if (nm.to > GlobalState::STRINGS_PAGE_SIZE) {
        strings.push_back(make_unique<vector<char>>(nm.to));
        from = strings.back()->data();
        if (strings.size() > 1) {
            swap(*(strings.end() - 1), *(strings.end() - 2));
        }
    } else {
        if (strings_last_page_used + nm.to > GlobalState::STRINGS_PAGE_SIZE) {
            strings.push_back(make_unique<vector<char>>(GlobalState::STRINGS_PAGE_SIZE));
            // printf("Wasted %i space\n", STRINGS_PAGE_SIZE - strings_last_page_used);
            strings_last_page_used = 0;
        }
        from = strings.back()->data() + strings_last_page_used;
    }

    memcpy(from, nm.from, nm.to);
    strings_last_page_used += nm.to;
    UTF8Desc res(from, nm.to);
    return res;
}

NameRef GlobalState::enterNameUTF8(UTF8Desc nm) {
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
                return name_id;
            }
        }
        bucketId = (bucketId + probe_count) & mask;
        probe_count++;
    }
    DEBUG_ONLY(if (probe_count == hashTableSize) { Error::raise("Full table?"); });

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

    return idx;
}

NameRef GlobalState::enterNameConstant(NameRef original) {
    Error::check(original.exists());
    Error::check(original.name(*this).kind == UTF8);

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
                return bucket.second;
            }
        }
        bucketId = (bucketId + probe_count) & mask;
        probe_count++;
    }
    if (probe_count == hashTableSize) {
        Error::raise("Full table?");
    }

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
    return idx;
}

NameRef GlobalState::enterNameConstant(UTF8Desc original) {
    return enterNameConstant(enterNameUTF8(original));
}

void moveNames(pair<unsigned int, unsigned int> *from, pair<unsigned int, unsigned int> *to, unsigned int szFrom,
               unsigned int szTo) {
    // printf("\nResizing name hash table from %u to %u\n", szFrom, szTo);
    DEBUG_ONLY(Error::check((szTo & (szTo - 1)) == 0));
    DEBUG_ONLY(Error::check((szFrom & (szFrom - 1)) == 0));
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
    DEBUG_ONLY(sanityCheck());
    Error::check(names.size() == names.capacity());

    names.reserve(names.size() * 2);
    vector<pair<unsigned int, unsigned int>> new_names_by_hash(names_by_hash.capacity() * 2);
    moveNames(names_by_hash.data(), new_names_by_hash.data(), names_by_hash.capacity(), new_names_by_hash.capacity());
    names_by_hash.swap(new_names_by_hash);
}

NameRef GlobalState::freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num) {
    if (num == 0) {
        num = freshNameId++;
    }
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
                return bucket.second;
            }
        }
        bucketId = (bucketId + probe_count) & mask;
        probe_count++;
    }
    if (probe_count == hashTableSize) {
        Error::raise("Full table?");
    }

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
    return idx;
}

FileRef GlobalState::enterFile(UTF8Desc path, UTF8Desc source) {
    auto idx = files.size();
    std::string sigil("# @typed");
    File::Type source_type = File::Untyped;
    if (search(source.from, source.from + source.to, sigil.begin(), sigil.end()) != source.from + source.to) {
        source_type = File::Typed;
    }

    files.emplace_back(path.toString(), source.toString(), source_type);
    return FileRef(idx);
}

LocalVariable GlobalState::newTemporary(UniqueNameKind kind, NameRef name, SymbolRef owner) {
    Symbol &info = owner.info(*this);
    Error::check(info.isMethod());
    int id = ++(info.uniqueCounter);
    NameRef tempName = this->freshNameUnique(kind, name, id);

    return this->enterLocalSymbol(owner, tempName);
}

unsigned int GlobalState::symbolsUsed() {
    return symbols.size();
}

unsigned int GlobalState::filesUsed() {
    return files.size();
}

unsigned int GlobalState::namesUsed() {
    return names.size();
}

string GlobalState::toString(bool showHidden) {
    vector<string> children;
    for (auto element : defn_root().info(*this).members) {
        if (showHidden || !element.second.isHiddenFromPrinting(*this)) {
            children.push_back(element.second.toString(*this, 0, showHidden));
        }
    }
    sort(children.begin(), children.end());
    ostringstream os;
    for (auto child : children) {
        os << child;
    }
    return os.str();
}

void GlobalState::sanityCheck() const {
    Error::check((names_by_hash.size() & (names_by_hash.size() - 1)) == 0);
    Error::check(names.capacity() * 2 == names_by_hash.capacity());
    Error::check(names_by_hash.size() == names_by_hash.capacity());
    for (auto &ent : names_by_hash) {
        if (ent.second == 0) {
            continue;
        }
        const Name &nm = names[ent.second];
        Error::check(ent.first == nm.hash(*this));
    }
}

} // namespace core
} // namespace ruby_typer
