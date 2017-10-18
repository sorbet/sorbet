#include "Context.h"
#include "Hashing.h"
#include "common/common.h"
#include <algorithm>
#include <sstream>
#include <string>

using namespace std;

namespace ruby_typer {
namespace ast {

SymbolRef ContextBase::synthesizeClass(UTF8Desc name) {
    auto nameId = enterNameUTF8(name);

    // This can't use enterClass since there is a chicken and egg problem.
    // These will be added to defn_root().members later.
    auto symRef = SymbolRef(symbols.size());
    symbols.emplace_back();
    auto &info = symRef.info(*this, true); // allowing noSymbol is needed because this enters noSymbol.
    info.name = nameId;
    info.owner = defn_root();
    info.flags = 0;
    info.setClass();

    symRef.info(*this, true).setCompleted();
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

static const char *whileTemp = "whileTemp";
static UTF8Desc whileTemp_DESC{(char *)whileTemp, (int)std::strlen(whileTemp)};

static const char *ifTemp = "ifTemp";
static UTF8Desc ifTemp_DESC{(char *)ifTemp, (int)std::strlen(ifTemp)};

static const char *returnTemp = "returnTemp";
static UTF8Desc retunTemp_DESC{(char *)returnTemp, (int)std::strlen(returnTemp)};

static const char *statTemp = "statTemp";
static UTF8Desc statTemp_DESC{(char *)statTemp, (int)std::strlen(statTemp)};

static const char *assignTemp = "statTemp";
static UTF8Desc assignTemp_DESC{(char *)assignTemp, (int)std::strlen(assignTemp)};

static const char *no_symbol_str = "<none>";
static UTF8Desc no_symbol_DESC{(char *)no_symbol_str, (int)strlen(no_symbol_str)};

static const char *top_str = "<top>";
static UTF8Desc top_DESC{(char *)top_str, (int)strlen(top_str)};

static const char *bottom_str = "<bottom>";
static UTF8Desc bottom_DESC{(char *)bottom_str, (int)strlen(bottom_str)};

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

ContextBase::ContextBase(spdlog::logger &logger) : logger(logger), errors(*this) {
    unsigned int max_name_count = 262144;   // 6MB
    unsigned int max_symbol_count = 524288; // 32MB

    names.reserve(max_name_count);
    symbols.reserve(max_symbol_count);
    int names_by_hash_size = 2 * max_name_count;
    names_by_hash.resize(names_by_hash_size);
    DEBUG_ONLY(Error::check((names_by_hash_size & (names_by_hash_size - 1)) == 0));

    names.emplace_back(); // first name is used in hashes to indicate empty cell
    // Second name is always <init>, see SymbolInfo::isConstructor
    auto init_id = enterNameUTF8(init_DESC);
    auto andAnd_id = enterNameUTF8(andAnd_DESC);
    auto orOr_id = enterNameUTF8(orOr_DESC);
    auto to_s_id = enterNameUTF8(to_s_DESC);
    auto concat_id = enterNameUTF8(concat_DESC);
    auto call_id = enterNameUTF8(call_DESC);
    auto bang_id = enterNameUTF8(bang_DESC);
    auto squareBrackets_id = enterNameUTF8(squareBrackets_DESC);
    auto squareBracketsEq_id = enterNameUTF8(squareBracketsEq_DESC);
    auto unaryPlus_id = enterNameUTF8(unaryPlus_DESC);
    auto unaryMinus_id = enterNameUTF8(unaryMinus_DESC);
    auto star_id = enterNameUTF8(star_DESC);
    auto starStar_id = enterNameUTF8(starStar_DESC);
    auto whileTemp_id = enterNameUTF8(whileTemp_DESC);
    auto ifTemp_id = enterNameUTF8(ifTemp_DESC);
    auto returnTemp_id = enterNameUTF8(retunTemp_DESC);
    auto statTemp_id = enterNameUTF8(statTemp_DESC);
    auto assignTemp_id = enterNameUTF8(assignTemp_DESC);

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
     */
    Error::check(symbols.size() == defn_last_synthetic_sym()._id + 1);

    // Add them back in since synthesizeClass couldn't
    for (SymbolInfo info : symbols) {
        if (info.owner != defn_root()) {
            defn_root().info(*this).members.push_back(make_pair(info.name, info.owner));
        }
    }
}

ContextBase::~ContextBase() {}

constexpr decltype(ContextBase::STRINGS_PAGE_SIZE) ContextBase::STRINGS_PAGE_SIZE;

SymbolRef ContextBase::enterClassSymbol(SymbolRef owner, NameRef name) {
    // TODO Unify this with enterSymbol
    DEBUG_ONLY(Error::check(owner.exists()));
    auto &ownerScope = owner.info(*this, true);
    for (auto &member : ownerScope.members) {
        if (member.first == name) {
            return member.second;
        }
    }

    bool reallocate = symbols.size() == symbols.capacity();

    auto ret = SymbolRef(symbols.size());
    symbols.emplace_back();
    auto &info = ret.info(*this, true);
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

SymbolRef ContextBase::enterSymbol(SymbolRef owner, NameRef name, SymbolRef result, vector<SymbolRef> &args,
                                   bool isMethod) {
    DEBUG_ONLY(Error::check(owner.exists()));
    auto &ownerScope = owner.info(*this, true);
    auto from = ownerScope.members.begin();
    auto to = ownerScope.members.end();
    while (from != to) {
        auto &el = *from;
        if (el.first == name) {
            auto &otherInfo = el.second.info(*this, true);
            if (otherInfo.result() == result && otherInfo.arguments() == args)
                return from->second;
        }
        from++;
    }

    bool reallocate = symbols.size() == symbols.capacity();

    auto ret = SymbolRef(symbols.size());
    symbols.emplace_back();
    auto &info = ret.info(*this, true);
    info.name = name;
    info.flags = 0;

    info.owner = owner;
    info.resultOrParentOrLoader = result;
    if (isMethod)
        info.setMethod();
    else
        info.setField();

    info.argumentsOrMixins.swap(args);
    if (!reallocate)
        ownerScope.members.push_back(make_pair(name, ret));
    else
        owner.info(*this, true).members.push_back(make_pair(name, ret));
    return ret;
}

NameRef ContextBase::enterNameUTF8(UTF8Desc nm) {
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

    Error::check(nm.to < ContextBase::STRINGS_PAGE_SIZE);
    char *from = nullptr;
    if (nm.to > ContextBase::STRINGS_PAGE_SIZE) {
        strings.push_back(make_unique<vector<char>>(nm.to));
        from = strings.back()->data();
        if (strings.size() > 1) {
            swap(*(strings.end() - 1), *(strings.end() - 2));
        }
    } else {

        if (strings_last_page_used + nm.to > ContextBase::STRINGS_PAGE_SIZE) {
            strings.push_back(make_unique<vector<char>>(ContextBase::STRINGS_PAGE_SIZE));
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

void ContextBase::expandNames() {
    Error::check(names.size() == names.capacity());
    Error::check(names.capacity() * 2 == names_by_hash.capacity());
    Error::check(names_by_hash.size() == names_by_hash.capacity());

    names.reserve(names.size() * 2);
    vector<pair<unsigned int, unsigned int>> new_names_by_hash(names_by_hash.capacity() * 2);
    moveNames(names_by_hash.data(), new_names_by_hash.data(), names_by_hash.capacity(), new_names_by_hash.capacity());
    names_by_hash.swap(new_names_by_hash);
}

NameRef ContextBase::freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original) {
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

FileRef ContextBase::enterFile(UTF8Desc path, UTF8Desc source) {
    auto idx = files.size();
    files.emplace_back();
    auto &file = files.back();
    file.path_ = path.toString();
    file.source_ = source.toString();
    return FileRef(idx);
}

SymbolRef ContextBase::newTemporary(UniqueNameKind kind, NameRef name, SymbolRef owner) {
    auto tempName = this->freshNameUnique(kind, name);
    vector<SymbolRef> empty;
    return this->enterSymbol(owner, tempName, SymbolRef(), empty, false);
}

unsigned int ContextBase::symbolsUsed() {
    return symbols.size();
}

unsigned int ContextBase::namesUsed() {
    return names.size();
}

std::string ContextBase::toString() {
    std::vector<std::string> children;
    for (auto element : defn_root().info(*this).members) {
        children.push_back(element.second.toString(*this));
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
