#include "Context.h"
#include "Hashing.h"
#include "common/common.h"
#include <algorithm>
#include <sstream>
#include <string>

using namespace std;

namespace ruby_typer {
namespace core {

SymbolRef GlobalState::synthesizeClass(UTF8Desc name) {
    NameRef nameId = enterNameConstant(name);

    // This can't use enterClass since there is a chicken and egg problem.
    // These will be added to defn_root().members later.
    SymbolRef symRef = SymbolRef(symbols.size());
    symbols.emplace_back();
    Symbol &info = symRef.info(*this, true); // allowing noSymbol is needed because this enters noSymbol.
    info.name = nameId;
    info.owner = defn_root();
    info.flags = 0;
    info.setClass();

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
static UTF8Desc whileTemp_DESC{(char *)whileTemp, (int)strlen(whileTemp)};

static const char *ifTemp = "ifTmp";
static UTF8Desc ifTemp_DESC{(char *)ifTemp, (int)strlen(ifTemp)};

static const char *returnTemp = "returnTmp";
static UTF8Desc retunTemp_DESC{(char *)returnTemp, (int)strlen(returnTemp)};

static const char *statTemp = "statTmp";
static UTF8Desc statTemp_DESC{(char *)statTemp, (int)strlen(statTemp)};

static const char *assignTemp = "assignTmp";
static UTF8Desc assignTemp_DESC{(char *)assignTemp, (int)strlen(assignTemp)};

static const char *returnMethodTemp = "<ret>";
static UTF8Desc returnMethodTemp_DESC{(char *)returnMethodTemp, (int)strlen(returnMethodTemp)};

static const char *selfMethodTemp = "<self>";
static UTF8Desc selfMethodTemp_DESC{(char *)selfMethodTemp, (int)strlen(selfMethodTemp)};

static const char *singletonClass = "<singleton class>";
static UTF8Desc singletonClass_DESC{(char *)singletonClass, (int)strlen(singletonClass)};

static const char *attachedClass = "<attached class>";
static UTF8Desc attachedClass_DESC{(char *)attachedClass, (int)strlen(attachedClass)};

static const char *blockReturnTemp = "<blockret>";
static UTF8Desc blockReturnTemp_DESC{(char *)blockReturnTemp, (int)strlen(blockReturnTemp)};

static const char *blockTemp = "<block>";
static UTF8Desc blockTemp_DESC{(char *)blockTemp, (int)strlen(blockTemp)};

static const char *no_symbol_str = "<none>";
static UTF8Desc no_symbol_DESC{(char *)no_symbol_str, (int)strlen(no_symbol_str)};

static const char *top_str = "<top>";
static UTF8Desc top_DESC{(char *)top_str, (int)strlen(top_str)};

static const char *bottom_str = "<bottom>";
static UTF8Desc bottom_DESC{(char *)bottom_str, (int)strlen(bottom_str)};

static const char *untyped_str = "untyped";
static UTF8Desc untyped_DESC{(char *)untyped_str, (int)strlen(untyped_str)};

static const char *root_str = "<root>";
static UTF8Desc root_DESC{(char *)root_str, (int)strlen(root_str)};

static const char *nil_str = "nil";
static UTF8Desc nil_DESC{(char *)nil_str, (int)strlen(nil_str)};

static const char *new_str = "new";
static UTF8Desc new_DESC{(char *)new_str, (int)strlen(new_str)};

static const char *destructureArg_str = "<destructure>";
static UTF8Desc destructureArg_DESC{(char *)destructureArg_str, (int)strlen(destructureArg_str)};

static const char *ampersand_str = "&";
static UTF8Desc ampersand_DESC{(char *)ampersand_str, (int)strlen(ampersand_str)};

static const char *lambda_str = "lambda";
static UTF8Desc lambda_DESC{(char *)lambda_str, (int)strlen(lambda_str)};

static const char *nil_p_str = "nil?";
static UTF8Desc nil_p_DESC{(char *)nil_p_str, (int)strlen(nil_p_str)};

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
static UTF8Desc object_DESC{(char *)object_str, (int)strlen(object_str)};

static const char *junk_str = "<<JUNK>>";
static UTF8Desc junk_DESC{(char *)junk_str, (int)strlen(junk_str)};

static const char *block_call_str = "<block-call>";
static UTF8Desc block_call_DESC{(char *)block_call_str, (int)strlen(block_call_str)};

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

static const char *class_str = "Class";
static UTF8Desc class_DESC{(char *)class_str, (int)strlen(class_str)};

static const char *merge = "merge";
static UTF8Desc merge_DESC{(char *)merge, (int)strlen(merge)};

static const char *standardMethod = "standard_method";
static UTF8Desc standardMethod_DESC{(char *)standardMethod, (int)strlen(standardMethod)};

static const char *declareVariables = "declare_variables";
static UTF8Desc declareVariables_DESC{(char *)declareVariables, (int)strlen(declareVariables)};

static const char *returns = "returns";
static UTF8Desc returns_DESC{(char *)returns, (int)strlen(returns)};

static const char *any = "any";
static UTF8Desc any_DESC{(char *)any, (int)strlen(any)};

static const char *all = "all";
static UTF8Desc all_DESC{(char *)all, (int)strlen(all)};

static const char *super = "super";
static UTF8Desc super_DESC{(char *)super, (int)strlen(super)};

static const char *empty = "";
static UTF8Desc empty_DESC{(char *)empty, (int)strlen(empty)};

static const char *tripleEq = "===";
static UTF8Desc tripleEq_DESC{(char *)tripleEq, (int)strlen(tripleEq)};

static const char *nilable = "nilable";
static UTF8Desc nilable_DESC{(char *)nilable, (int)strlen(nilable)};

static const char *opus = "Opus";
static UTF8Desc opus_DESC{(char *)opus, (int)strlen(opus)};

static const char *types = "Types";
static UTF8Desc types_DESC{(char *)types, (int)strlen(types)};

static const char *basicObject = "BasicObject";
static UTF8Desc basicObject_DESC{(char *)basicObject, (int)strlen(basicObject)};

static const char *kernel = "Kernel";
static UTF8Desc kernel_DESC{(char *)kernel, (int)strlen(kernel)};

static const char *reserved = "<<RESERVED>>";
static UTF8Desc reserved_DESC{(char *)reserved, (int)strlen(reserved)};

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
    NameRef singletonClass_id = enterNameUTF8(singletonClass_DESC);
    NameRef attachedClass_id = enterNameUTF8(attachedClass_DESC);
    NameRef blockTemp_id = enterNameUTF8(blockTemp_DESC);
    NameRef declareVariables_id = enterNameUTF8(declareVariables_DESC);
    NameRef new_id = enterNameUTF8(new_DESC);
    NameRef destructureArg_id = enterNameUTF8(destructureArg_DESC);
    NameRef ampersand_id = enterNameUTF8(ampersand_DESC);
    NameRef block_call_id = enterNameUTF8(block_call_DESC);
    NameRef lambda_id = enterNameUTF8(lambda_DESC);
    NameRef nil_p_id = enterNameUTF8(nil_p_DESC);
    NameRef super_id = enterNameUTF8(super_DESC);
    NameRef empty_id = enterNameUTF8(empty_DESC);
    NameRef tripleEq_id = enterNameUTF8(tripleEq_DESC);

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
    DEBUG_ONLY(Error::check(singletonClass_id == Names::singletonClass()));
    DEBUG_ONLY(Error::check(attachedClass_id == Names::attachedClass()));
    DEBUG_ONLY(Error::check(blockTemp_id == Names::blockTemp()));
    DEBUG_ONLY(Error::check(declareVariables_id == Names::declareVariables()));
    DEBUG_ONLY(Error::check(new_id == Names::new_()));
    DEBUG_ONLY(Error::check(destructureArg_id == Names::destructureArg()));
    DEBUG_ONLY(Error::check(ampersand_id == Names::ampersand()));
    DEBUG_ONLY(Error::check(block_call_id == Names::blockCall()));
    DEBUG_ONLY(Error::check(lambda_id == Names::lambda()));
    DEBUG_ONLY(Error::check(nil_p_id == Names::nil_p()));
    DEBUG_ONLY(Error::check(super_id == Names::super()));
    DEBUG_ONLY(Error::check(empty_id == Names::empty()));
    DEBUG_ONLY(Error::check(tripleEq_id == Names::tripleEq()));

    SymbolRef no_symbol_id = synthesizeClass(no_symbol_DESC);
    SymbolRef top_id = synthesizeClass(top_DESC); // BasicObject
    SymbolRef bottom_id = synthesizeClass(bottom_DESC);
    SymbolRef root_id = synthesizeClass(root_DESC);
    SymbolRef nil_id = synthesizeClass(nil_DESC);
    SymbolRef todo_id = synthesizeClass(todo_DESC);
    SymbolRef object_id = synthesizeClass(object_DESC);
    SymbolRef junk_id = synthesizeClass(junk_DESC);
    SymbolRef integer_id = synthesizeClass(integer_DESC);
    SymbolRef float_id = synthesizeClass(float_DESC);
    SymbolRef string_id = synthesizeClass(string_DESC);
    SymbolRef symbol_id = synthesizeClass(symbol_DESC);
    SymbolRef array_id = synthesizeClass(array_DESC);
    SymbolRef hash_id = synthesizeClass(hash_DESC);
    SymbolRef trueClass_id = synthesizeClass(trueClass_DESC);
    SymbolRef falseClass_id = synthesizeClass(falseClass_DESC);
    SymbolRef nilClass_id = synthesizeClass(nilClass_DESC);
    SymbolRef untyped_id = synthesizeClass(untyped_DESC);
    SymbolRef opus_id = synthesizeClass(opus_DESC);
    SymbolRef opus_types_id = enterClassSymbol(Loc::none(0), opus_id, enterNameConstant(types_DESC));
    SymbolRef class_id = synthesizeClass(class_DESC);
    SymbolRef basicObject_id = synthesizeClass(basicObject_DESC);
    SymbolRef kernel_id = synthesizeClass(kernel_DESC);

    Error::check(no_symbol_id == noSymbol());
    Error::check(top_id == defn_top());
    Error::check(bottom_id == defn_bottom());
    Error::check(root_id == defn_root());
    Error::check(nil_id == defn_nil());
    Error::check(todo_id == defn_todo());
    Error::check(object_id == defn_object());
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
    Error::check(basicObject_id == defn_Basic_Object());
    Error::check(kernel_id == defn_Kernel());

    while (symbols.size() < GlobalState::MAX_SYNTHETIC_SYMBOLS) {
        synthesizeClass(reserved_DESC);
    }

    Error::check(symbols.size() == defn_last_synthetic_sym()._id + 1);
}

GlobalState::~GlobalState() {}

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

    if (!reallocate)
        ownerScope.members.push_back(make_pair(name, ret));
    else
        owner.info(*this, true).members.push_back(make_pair(name, ret));
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
            if (nm2.kind == CONSTANT && nm2.cnst.original == original)
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

LocalVariable GlobalState::newTemporary(UniqueNameKind kind, NameRef name, SymbolRef owner) {
    Symbol &info = owner.info(*this);
    Error::check(info.isMethod());
    int id = info.uniqueCounter++;
    NameRef tempName = this->freshNameUnique(kind, name, id);

    return this->enterLocalSymbol(owner, tempName);
}

unsigned int GlobalState::symbolsUsed() {
    return symbols.size();
}

unsigned int GlobalState::namesUsed() {
    return names.size();
}

string GlobalState::toString() {
    vector<string> children;
    for (auto element : defn_root().info(*this).members) {
        if (!element.second.isHiddenFromPrinting()) {
            children.push_back(element.second.toString(*this));
        }
    }
    sort(children.begin(), children.end());
    ostringstream os;
    for (auto child : children) {
        os << child;
    }
    return os.str();
}

SymbolRef Context::selfClass() {
    Symbol &info = this->owner.info(this->state);
    if (info.isClass())
        return info.singletonClass(this->state);
    return this->contextClass();
}

SymbolRef Context::enclosingMethod() {
    SymbolRef owner = this->owner;
    while (owner != GlobalState::defn_root() && !owner.info(this->state, false).isMethod()) {
        Error::check(owner.exists());
        owner = owner.info(this->state).owner;
    }
    if (owner == GlobalState::defn_root())
        owner._id = 0;
    return owner;
}

SymbolRef Context::contextClass() {
    SymbolRef owner = this->owner;
    while (!owner.info(this->state, false).isClass()) {
        Error::check(owner.exists());
        owner = owner.info(this->state).owner;
    }
    return owner;
}

} // namespace core
} // namespace ruby_typer
