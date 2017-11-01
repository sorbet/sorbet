#include "Context.h"
#include "Hashing.h"
#include "common/common.h"
#include <algorithm>
#include <sstream>
#include <string>

using namespace std;

namespace ruby_typer {
namespace ast {

SymbolRef GlobalState::synthesizeClass(UTF8Desc name) {
    NameRef nameId = enterNameUTF8(name);

    // This can't use enterClass since there is a chicken and egg problem.
    // These will be added to defn_root().members later.
    SymbolRef symRef = SymbolRef(symbols.size());
    symbols.emplace_back();
    Symbol &info = symRef.info(*this, true); // allowing noSymbol is needed because this enters noSymbol.
    info.name = nameId;
    info.owner = defn_root();
    info.flags = 0;
    info.setClass();

    symRef.info(*this, true).setCompleted();
    if (symRef._id > GlobalState::defn_root()._id) {
        GlobalState::defn_root().info(*this, true).members.push_back(make_pair(nameId, symRef));
    }
    return symRef;
}

static const char *init = "initialize";
static UTF8Desc init_DESC{(char *)init, (int)strlen(init)};

static const char *andAnd = "&&";
static UTF8Desc andAnd_DESC{(char *)andAnd, (int)strlen(andAnd)};

static const char *orOr = "||";
static UTF8Desc orOr_DESC{(char *)orOr, (int)strlen(orOr)};

static const char *to_s = "to_s";
static UTF8Desc to_s_DESC{(char *)to_s, (int)strlen(to_s)};

static const char *concat = "concat";
static UTF8Desc concat_DESC{(char *)concat, (int)strlen(concat)};

static const char *call = "call";
static UTF8Desc call_DESC{(char *)call, (int)strlen(call)};

static const char *bang = "!";
static UTF8Desc bang_DESC{(char *)bang, (int)strlen(bang)};

static const char *squareBrackets = "[]";
static UTF8Desc squareBrackets_DESC{(char *)squareBrackets, (int)strlen(squareBrackets)};

static const char *squareBracketsEq = "[]=";
static UTF8Desc squareBracketsEq_DESC{(char *)squareBracketsEq, (int)strlen(squareBracketsEq)};

static const char *unaryPlus = "+@";
static UTF8Desc unaryPlus_DESC{(char *)unaryPlus, (int)strlen(unaryPlus)};

static const char *unaryMinus = "-@";
static UTF8Desc unaryMinus_DESC{(char *)unaryMinus, (int)strlen(unaryMinus)};

static const char *star = "*";
static UTF8Desc star_DESC{(char *)star, (int)strlen(star)};

static const char *starStar = "**";
static UTF8Desc starStar_DESC{(char *)starStar, (int)strlen(starStar)};

static const char *whileTemp = "whileTmp";
static UTF8Desc whileTemp_DESC{(char *)whileTemp, (int)std::strlen(whileTemp)};

static const char *ifTemp = "ifTmp";
static UTF8Desc ifTemp_DESC{(char *)ifTemp, (int)std::strlen(ifTemp)};

static const char *returnTemp = "returnTmp";
static UTF8Desc retunTemp_DESC{(char *)returnTemp, (int)std::strlen(returnTemp)};

static const char *statTemp = "statTmp";
static UTF8Desc statTemp_DESC{(char *)statTemp, (int)std::strlen(statTemp)};

static const char *assignTemp = "assignTmp";
static UTF8Desc assignTemp_DESC{(char *)assignTemp, (int)std::strlen(assignTemp)};

static const char *returnMethodTemp = "<ret>";
static UTF8Desc returnMethodTemp_DESC{(char *)returnMethodTemp, (int)std::strlen(returnMethodTemp)};

static const char *selfMethodTemp = "<self>";
static UTF8Desc selfMethodTemp_DESC{(char *)selfMethodTemp, (int)std::strlen(selfMethodTemp)};

static const char *blockReturnTemp = "<blockret>";
static UTF8Desc blockReturnTemp_DESC{(char *)blockReturnTemp, (int)std::strlen(blockReturnTemp)};

static const char *no_symbol_str = "<none>";
static UTF8Desc no_symbol_DESC{(char *)no_symbol_str, (int)strlen(no_symbol_str)};

static const char *top_str = "<top>";
static UTF8Desc top_DESC{(char *)top_str, (int)strlen(top_str)};

static const char *bottom_str = "<bottom>";
static UTF8Desc bottom_DESC{(char *)bottom_str, (int)strlen(bottom_str)};

static const char *dynamic_str = "<dynamic>";
static UTF8Desc dynamic_DESC{(char *)dynamic_str, (int)strlen(dynamic_str)};

static const char *root_str = "<root>";
static UTF8Desc root_DESC{(char *)root_str, (int)strlen(root_str)};

static const char *nil_str = "nil";
static UTF8Desc nil_DESC{(char *)nil_str, (int)strlen(nil_str)};

static const char *todo_str = "<todo sym>";
static UTF8Desc todo_DESC{(char *)todo_str, (int)strlen(todo_str)};

static const char *todo_ivar_str = "<todo ivar sym>";
static UTF8Desc todo_ivar_DESC{(char *)todo_ivar_str, (int)strlen(todo_ivar_str)};

static const char *todo_cvar_str = "<todo cvar sym>";
static UTF8Desc todo_cvar_DESC{(char *)todo_cvar_str, (int)strlen(todo_cvar_str)};

static const char *todo_gvar_str = "<todo gvar sym>";
static UTF8Desc todo_gvar_DESC{(char *)todo_gvar_str, (int)strlen(todo_gvar_str)};

static const char *todo_lvar_str = "<todo lvar sym>";
static UTF8Desc todo_lvar_DESC{(char *)todo_lvar_str, (int)strlen(todo_lvar_str)};

static const char *object_str = "Object";
static UTF8Desc object_DESC{(char *)object_str, (int)std::strlen(object_str)};

static const char *junk_str = "<<JUNK>>";
static UTF8Desc junk_DESC{(char *)junk_str, (int)std::strlen(junk_str)};

static const char *always_str = "<always>";
static UTF8Desc always_DESC{(char *)always_str, (int)std::strlen(always_str)};

static const char *never_str = "<never>";
static UTF8Desc never_DESC{(char *)never_str, (int)std::strlen(never_str)};

static const char *block_call_str = "<block-call>";
static UTF8Desc block_call_DESC{(char *)block_call_str, (int)std::strlen(block_call_str)};

static const char *include = "include";
static UTF8Desc include_DESC{(char *)include, (int)strlen(include)};

static const char *currentFile = "__FILE__";
static UTF8Desc currentFile_DESC{(char *)currentFile, (int)strlen(currentFile)};

static const char *string_str = "String";
static UTF8Desc string_DESC{(char *)string_str, (int)strlen(string_str)};

static const char *integer_str = "Integer";
static UTF8Desc integer_DESC{(char *)integer_str, (int)strlen(integer_str)};

static const char *float_str = "Float";
static UTF8Desc float_DESC{(char *)float_str, (int)strlen(float_str)};

static const char *symbol_str = "Symbol";
static UTF8Desc symbol_DESC{(char *)symbol_str, (int)strlen(symbol_str)};

static const char *array_str = "Array";
static UTF8Desc array_DESC{(char *)array_str, (int)strlen(array_str)};

static const char *hash_str = "Hash";
static UTF8Desc hash_DESC{(char *)hash_str, (int)strlen(hash_str)};

static const char *trueClass_str = "TrueClass";
static UTF8Desc trueClass_DESC{(char *)trueClass_str, (int)strlen(trueClass_str)};

static const char *falseClass_str = "FalseClass";
static UTF8Desc falseClass_DESC{(char *)falseClass_str, (int)strlen(falseClass_str)};

static const char *nilClass_str = "NilClass";
static UTF8Desc nilClass_DESC{(char *)nilClass_str, (int)strlen(nilClass_str)};

static const char *merge = "merge";
static UTF8Desc merge_DESC{(char *)merge, (int)strlen(merge)};

static const char *standardMethod = "standard_method";
static UTF8Desc standardMethod_DESC{(char *)standardMethod, (int)strlen(standardMethod)};

static const char *returns = "returns";
static UTF8Desc returns_DESC{(char *)returns, (int)strlen(returns)};

static const char *any = "any";
static UTF8Desc any_DESC{(char *)any, (int)strlen(any)};

static const char *all = "all";
static UTF8Desc all_DESC{(char *)all, (int)strlen(all)};

static const char *nilable = "nilable";
static UTF8Desc nilable_DESC{(char *)nilable, (int)strlen(nilable)};

static const char *opus = "Opus";
static UTF8Desc opus_DESC{(char *)opus, (int)strlen(opus)};

static const char *types = "Types";
static UTF8Desc types_DESC{(char *)types, (int)strlen(types)};

GlobalState::GlobalState(spdlog::logger &logger) : logger(logger), errors(*this) {
    unsigned int max_name_count = 262144;   // 6MB
    unsigned int max_symbol_count = 524288; // 32MB

    names.reserve(max_name_count);
    symbols.reserve(max_symbol_count);
    int names_by_hash_size = 2 * max_name_count;
    names_by_hash.resize(names_by_hash_size);
    DEBUG_ONLY(Error::check((names_by_hash_size & (names_by_hash_size - 1)) == 0));

    names.emplace_back(); // first name is used in hashes to indicate empty cell
    // Second name is always <init>, see Symbol::isConstructor
    NameRef init_id = enterNameUTF8(init_DESC);
    NameRef andAnd_id = enterNameUTF8(andAnd_DESC);
    NameRef orOr_id = enterNameUTF8(orOr_DESC);
    NameRef to_s_id = enterNameUTF8(to_s_DESC);
    NameRef concat_id = enterNameUTF8(concat_DESC);
    NameRef call_id = enterNameUTF8(call_DESC);
    NameRef bang_id = enterNameUTF8(bang_DESC);
    NameRef squareBrackets_id = enterNameUTF8(squareBrackets_DESC);
    NameRef squareBracketsEq_id = enterNameUTF8(squareBracketsEq_DESC);
    NameRef unaryPlus_id = enterNameUTF8(unaryPlus_DESC);
    NameRef unaryMinus_id = enterNameUTF8(unaryMinus_DESC);
    NameRef star_id = enterNameUTF8(star_DESC);
    NameRef starStar_id = enterNameUTF8(starStar_DESC);
    NameRef whileTemp_id = enterNameUTF8(whileTemp_DESC);
    NameRef ifTemp_id = enterNameUTF8(ifTemp_DESC);
    NameRef returnTemp_id = enterNameUTF8(retunTemp_DESC);
    NameRef statTemp_id = enterNameUTF8(statTemp_DESC);
    NameRef assignTemp_id = enterNameUTF8(assignTemp_DESC);
    NameRef returnMethodTemp_id = enterNameUTF8(returnMethodTemp_DESC);
    NameRef blockReturnTemp_id = enterNameUTF8(blockReturnTemp_DESC);
    NameRef include_id = enterNameUTF8(include_DESC);
    NameRef currentFile_id = enterNameUTF8(currentFile_DESC);
    NameRef merge_id = enterNameUTF8(merge_DESC);
    NameRef selfMethodTemp_id = enterNameUTF8(selfMethodTemp_DESC);
    NameRef standardMethod_id = enterNameUTF8(standardMethod_DESC);
    NameRef returns_id = enterNameUTF8(returns_DESC);
    NameRef all_id = enterNameUTF8(all_DESC);
    NameRef any_id = enterNameUTF8(any_DESC);
    NameRef nilable_id = enterNameUTF8(nilable_DESC);

    DEBUG_ONLY(Error::check(init_id == Names::initialize()));
    DEBUG_ONLY(Error::check(andAnd_id == Names::andAnd()));
    DEBUG_ONLY(Error::check(orOr_id == Names::orOr()));
    DEBUG_ONLY(Error::check(to_s_id == Names::to_s()));
    DEBUG_ONLY(Error::check(concat_id == Names::concat()));
    DEBUG_ONLY(Error::check(call_id == Names::call()));
    DEBUG_ONLY(Error::check(bang_id == Names::bang()));
    DEBUG_ONLY(Error::check(squareBrackets_id == Names::squareBrackets()));
    DEBUG_ONLY(Error::check(squareBracketsEq_id == Names::squareBracketsEq()));
    DEBUG_ONLY(Error::check(unaryPlus_id == Names::unaryPlus()));
    DEBUG_ONLY(Error::check(unaryMinus_id == Names::unaryMinus()));
    DEBUG_ONLY(Error::check(star_id == Names::star()));
    DEBUG_ONLY(Error::check(starStar_id == Names::starStar()));
    DEBUG_ONLY(Error::check(whileTemp_id == Names::whileTemp()));
    DEBUG_ONLY(Error::check(ifTemp_id == Names::ifTemp()));
    DEBUG_ONLY(Error::check(returnTemp_id == Names::returnTemp()));
    DEBUG_ONLY(Error::check(statTemp_id == Names::statTemp()));
    DEBUG_ONLY(Error::check(assignTemp_id == Names::assignTemp()));
    DEBUG_ONLY(Error::check(returnMethodTemp_id == Names::returnMethodTemp()));
    DEBUG_ONLY(Error::check(blockReturnTemp_id == Names::blockReturnTemp()));
    DEBUG_ONLY(Error::check(include_id == Names::include()));
    DEBUG_ONLY(Error::check(currentFile_id == Names::currentFile()));
    DEBUG_ONLY(Error::check(merge_id == Names::merge()));
    DEBUG_ONLY(Error::check(selfMethodTemp_id == Names::selfMethodTemp()));
    DEBUG_ONLY(Error::check(standardMethod_id == Names::standardMethod()));
    DEBUG_ONLY(Error::check(returns_id == Names::returns()));
    DEBUG_ONLY(Error::check(all_id == Names::all()));
    DEBUG_ONLY(Error::check(any_id == Names::any()));
    DEBUG_ONLY(Error::check(nilable_id == Names::nilable()));

    SymbolRef no_symbol_id = synthesizeClass(no_symbol_DESC);
    SymbolRef top_id = synthesizeClass(top_DESC); // BasicObject
    SymbolRef bottom_id = synthesizeClass(bottom_DESC);
    SymbolRef root_id = synthesizeClass(root_DESC);
    SymbolRef nil_id = synthesizeClass(nil_DESC);
    SymbolRef todo_id = synthesizeClass(todo_DESC);
    SymbolRef lvar_id = synthesizeClass(todo_lvar_DESC);
    SymbolRef ivar_id = synthesizeClass(todo_ivar_DESC);
    SymbolRef gvar_id = synthesizeClass(todo_gvar_DESC);
    SymbolRef cvar_id = synthesizeClass(todo_cvar_DESC);
    SymbolRef object_id = synthesizeClass(object_DESC);
    SymbolRef junk_id = synthesizeClass(junk_DESC);
    SymbolRef always_id = synthesizeClass(always_DESC);
    SymbolRef never_id = synthesizeClass(never_DESC);
    SymbolRef block_call_id = synthesizeClass(block_call_DESC);
    SymbolRef integer_id = synthesizeClass(integer_DESC);
    SymbolRef float_id = synthesizeClass(float_DESC);
    SymbolRef string_id = synthesizeClass(string_DESC);
    SymbolRef symbol_id = synthesizeClass(symbol_DESC);
    SymbolRef array_id = synthesizeClass(array_DESC);
    SymbolRef hash_id = synthesizeClass(hash_DESC);
    SymbolRef trueClass_id = synthesizeClass(trueClass_DESC);
    SymbolRef falseClass_id = synthesizeClass(falseClass_DESC);
    SymbolRef nilClass_id = synthesizeClass(nilClass_DESC);
    SymbolRef dynamic_id = synthesizeClass(dynamic_DESC);
    SymbolRef opus_id = synthesizeClass(opus_DESC);
    SymbolRef opus_types_id = enterClassSymbol(opus_id, enterNameUTF8(types_DESC));

    Error::check(no_symbol_id == noSymbol());
    Error::check(top_id == defn_top());
    Error::check(bottom_id == defn_bottom());
    Error::check(root_id == defn_root());
    Error::check(nil_id == defn_nil());
    Error::check(todo_id == defn_todo());
    Error::check(lvar_id == defn_lvar_todo());
    Error::check(ivar_id == defn_ivar_todo());
    Error::check(gvar_id == defn_gvar_todo());
    Error::check(cvar_id == defn_cvar_todo());
    Error::check(object_id == defn_object());
    Error::check(junk_id == defn_junk());
    Error::check(always_id == defn_cfg_always());
    Error::check(never_id == defn_cfg_never());
    Error::check(block_call_id == defn_cfg_block_call());
    Error::check(integer_id == defn_Integer());
    Error::check(float_id == defn_Float());
    Error::check(string_id == defn_String());
    Error::check(symbol_id == defn_Symbol());
    Error::check(array_id == defn_Array());
    Error::check(hash_id == defn_Hash());
    Error::check(trueClass_id == defn_TrueClass());
    Error::check(falseClass_id == defn_FalseClass());
    Error::check(nilClass_id == defn_NilClass());
    Error::check(dynamic_id == defn_dynamic());
    Error::check(opus_id == defn_Opus());
    Error::check(opus_types_id == defn_Opus_Types());

    /* 0: <none>
     * 1: <top>
     * 2: <bottom>
     * 3: <root>;
     * 4: nil;
     * 5: <todo>
     * 6: <todo lvar>
     * 7: <todo ivar>
     * 8: <todo gvar>
     * 9: <todo cvar>
     * 10: Object;
     * 11: <<JUNK>>;
     * 12: <always>
     * 13: <never>
     * 14: <block-call>
     * 15: Integer
     * 16: Float
     * 17: String
     * 18: Symbol
     * 19: Array
     * 20: Hash
     * 21: TrueClass
     * 22: FalseClass
     * 23: NilClass
     * 24: Opus
     * 25: Opus::Types
     */
    Error::check(symbols.size() == defn_last_synthetic_sym()._id + 1);

    // Add them back in since synthesizeClass couldn't
    for (Symbol &info : symbols) {
        if (info.owner != defn_root()) {
            defn_root().info(*this).members.push_back(make_pair(info.name, info.owner));
        }
    }
}

GlobalState::~GlobalState() {}

constexpr decltype(GlobalState::STRINGS_PAGE_SIZE) GlobalState::STRINGS_PAGE_SIZE;

SymbolRef GlobalState::enterClassSymbol(SymbolRef owner, NameRef name) {
    // TODO Unify this with enterSymbol
    DEBUG_ONLY(Error::check(owner.exists()));
    Symbol &ownerScope = owner.info(*this, true);
    for (auto &member : ownerScope.members) {
        if (member.first == name) {
            return member.second;
        }
    }

    bool reallocate = symbols.size() == symbols.capacity();

    SymbolRef ret = SymbolRef(symbols.size());
    symbols.emplace_back();
    Symbol &info = ret.info(*this, true);
    info.name = name;
    info.flags = 0;
    info.owner = owner;
    info.setClass();

    if (!reallocate) {
        ownerScope.members.push_back(make_pair(name, ret));
    } else {
        owner.info(*this, true).members.push_back(make_pair(name, ret));
    }
    return ret;
}

SymbolRef GlobalState::enterSymbol(SymbolRef owner, NameRef name, bool isMethod) {
    DEBUG_ONLY(Error::check(owner.exists()));
    Error::check(name.exists());
    Symbol &ownerScope = owner.info(*this, true);
    auto from = ownerScope.members.begin();
    auto to = ownerScope.members.end();
    while (from != to) {
        auto &el = *from;
        if (el.first == name) {
            return from->second;
        }
        from++;
    }

    bool reallocate = symbols.size() == symbols.capacity();

    SymbolRef ret = SymbolRef(symbols.size());
    symbols.emplace_back();
    Symbol &info = ret.info(*this, true);
    info.name = name;
    info.flags = 0;
    info.owner = owner;
    if (isMethod)
        info.setMethod();
    else
        info.setField();

    if (!reallocate)
        ownerScope.members.push_back(make_pair(name, ret));
    else
        owner.info(*this, true).members.push_back(make_pair(name, ret));
    return ret;
}

NameRef GlobalState::enterNameUTF8(UTF8Desc nm) {
    const auto hs = _hash(nm);
    unsigned int hashTableSize = names_by_hash.size();
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probe_count = 1;

    while (names_by_hash[bucketId].second) {
        auto &bucket = names_by_hash[bucketId];
        if (bucket.first == hs) {
            auto name_id = bucket.second;
            auto &nm2 = names[name_id];
            if (nm2.kind == NameKind::UTF8 && nm2.raw.utf8 == nm)
                return name_id;
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

    Error::check(nm.to < GlobalState::STRINGS_PAGE_SIZE);
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
    names[idx].kind = NameKind::UTF8;
    names[idx].raw.utf8.from = from;
    names[idx].raw.utf8.to = nm.to;
    strings_last_page_used += nm.to;

    return idx;
}

void moveNames(pair<unsigned int, unsigned int> *from, pair<unsigned int, unsigned int> *to, unsigned int szFrom,
               unsigned int szTo) {
    // printf("\nResizing name hash table from %u to %u\n", szFrom, szTo);
    DEBUG_ONLY(Error::check((szTo & (szTo - 1)) == 0));
    DEBUG_ONLY(Error::check((szFrom & (szFrom - 1)) == 0));
    unsigned int mask = szTo - 1;
    for (unsigned int orig = 0; orig < szFrom; orig++) {
        if (from[orig].second) {
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
    Error::check(names.size() == names.capacity());
    Error::check(names.capacity() * 2 == names_by_hash.capacity());
    Error::check(names_by_hash.size() == names_by_hash.capacity());

    names.reserve(names.size() * 2);
    vector<pair<unsigned int, unsigned int>> new_names_by_hash(names_by_hash.capacity() * 2);
    moveNames(names_by_hash.data(), new_names_by_hash.data(), names_by_hash.capacity(), new_names_by_hash.capacity());
    names_by_hash.swap(new_names_by_hash);
}

NameRef GlobalState::freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original) {
    u2 num = freshNameId++;
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
                nm2.unique.original == original)
                return bucket.second;
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
    files.emplace_back();
    auto &file = files.back();
    file.path_ = path.toString();
    file.source_ = source.toString();
    return FileRef(idx);
}

SymbolRef GlobalState::newTemporary(UniqueNameKind kind, NameRef name, SymbolRef owner) {
    NameRef tempName = this->freshNameUnique(kind, name);
    return this->enterSymbol(owner, tempName, false);
}

unsigned int GlobalState::symbolsUsed() {
    return symbols.size();
}

unsigned int GlobalState::namesUsed() {
    return names.size();
}

std::string GlobalState::toString() {
    std::vector<std::string> children;
    for (auto element : defn_root().info(*this).members) {
        if (true || !element.second.isSynthetic()) {
            children.push_back(element.second.toString(*this));
        }
    }
    std::sort(children.begin(), children.end());
    std::ostringstream os;
    for (auto child : children) {
        os << child;
    }
    return os.str();
}

} // namespace ast
} // namespace ruby_typer
