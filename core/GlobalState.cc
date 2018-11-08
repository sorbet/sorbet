#include "GlobalState.h"

#include "core/Error.h"
#include "core/Names.h"
#include "core/Names_gen.h"
#include "core/Types.h"
#include "core/Unfreeze.h"
#include "core/errors/errors.h"
#include <utility>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "core/errors/infer.h"

template class std::vector<std::pair<unsigned int, unsigned int>>;
template class std::shared_ptr<sorbet::core::GlobalState>;
template class std::unique_ptr<sorbet::core::GlobalState>;

using namespace std;

namespace sorbet::core {

namespace {
constexpr string_view top_str = "<any>"sv;
constexpr string_view bottom_str = "T.noreturn"sv;
constexpr string_view untyped_str = "T.untyped"sv;
constexpr string_view root_str = "<root>"sv;
constexpr string_view object_str = "Object"sv;
constexpr string_view string_str = "String"sv;
constexpr string_view integer_str = "Integer"sv;
constexpr string_view float_str = "Float"sv;
constexpr string_view symbol_str = "Symbol"sv;
constexpr string_view array_str = "Array"sv;
constexpr string_view hash_str = "Hash"sv;
constexpr string_view proc_str = "Proc"sv;
constexpr string_view trueClass_str = "TrueClass"sv;
constexpr string_view falseClass_str = "FalseClass"sv;
constexpr string_view nilClass_str = "NilClass"sv;
constexpr string_view class_str = "Class"sv;
constexpr string_view module_str = "Module"sv;
constexpr string_view todo_str = "<todo sym>"sv;
constexpr string_view no_symbol_str = "<none>"sv;
constexpr string_view opus_str = "Opus"sv;
constexpr string_view T_str = "T"sv;
constexpr string_view basicObject_str = "BasicObject"sv;
constexpr string_view kernel_str = "Kernel"sv;
constexpr string_view range_str = "Range"sv;
constexpr string_view regexp_str = "Regexp"sv;
constexpr string_view standardError_str = "StandardError"sv;
constexpr string_view complex_str = "Complex"sv;
constexpr string_view rational_str = "Rational"sv;
// A magic non user-creatable class with methods to keep state between passes
constexpr string_view magic_str = "<Magic>"sv;
constexpr string_view enumerable_str = "Enumerable"sv;
constexpr string_view set_str = "Set"sv;
constexpr string_view struct_str = "Struct"sv;
constexpr string_view file_str = "File"sv;
constexpr string_view ruby_typer_str = "RubyTyper"sv;
constexpr string_view stubClass_str = "StubClass"sv;
constexpr string_view stubAncestor_str = "StubAncestor"sv;
constexpr string_view configatron_str = "Configatron"sv;
constexpr string_view store_str = "Store"sv;
constexpr string_view root_store_str = "RootStore"sv;
constexpr string_view sinatra_str = "Sinatra"sv;
constexpr string_view base_str = "Base"sv;
constexpr string_view void_str = "Void"sv;
constexpr string_view typeAliasTemp_str = "<TypeAlias>"sv;
constexpr string_view chalk_str = "Chalk"sv;
constexpr string_view tools_str = "Tools"sv;
constexpr string_view accessible_str = "Accessible"sv;
constexpr string_view generic_str = "Generic"sv;
constexpr string_view tuple_str = "Tuple"sv;
constexpr string_view shape_str = "Shape"sv;
constexpr string_view subclasses_str = "SUBCLASSES"sv;
constexpr string_view sorbet_str = "Sorbet"sv;
constexpr string_view return_type_inference_str = "ReturnTypeInference"sv;
constexpr string_view inferred_return_type_str = "INFERRED_RETURN_TYPE"sv;
constexpr string_view inferred_argument_type_str = "INFERRED_ARGUMENT_TYPE"sv;
constexpr string_view implicit_module_superclass_str = "ImplicitModuleSuperclass"sv;
constexpr string_view guessed_type_type_parameter_holder_str = "guessed_type_type_parameter_holder"sv;
constexpr string_view private_str = "Private"sv;
constexpr string_view builder_str = "Builder"sv;
constexpr string_view helpers_str = "Helpers"sv;
constexpr string_view garbage_type_str = "GarbageType"sv;

// This fills in all the way up to MAX_SYNTHETIC_SYMBOLS
constexpr string_view reserved_str = "<<RESERVED>>"sv;
} // namespace

SymbolRef GlobalState::synthesizeClass(string_view name, u4 superclass, bool isModule) {
    NameRef nameId = enterNameConstant(name);

    // This can't use enterClass since there is a chicken and egg problem.
    // These will be added to Symbols::root().members later.
    SymbolRef symRef = SymbolRef(this, symbols.size());
    symbols.emplace_back();
    SymbolData data = symRef.data(*this, true); // allowing noSymbol is needed because this enters noSymbol.
    data->name = nameId;
    data->owner = Symbols::root();
    data->superClass = SymbolRef(this, superclass);
    data->flags = 0;
    data->setClass();
    data->setIsModule(isModule);

    if (symRef._id > Symbols::root()._id) {
        Symbols::root().data(*this, true)->members[nameId] = symRef;
    }
    return symRef;
}

atomic<int> globalStateIdCounter(1);
const int Symbols::MAX_PROC_ARITY;

GlobalState::GlobalState(const shared_ptr<ErrorQueue> &errorQueue)
    : globalStateId(globalStateIdCounter.fetch_add(1)), errorQueue(errorQueue) {
    // Empirically determined to be the smallest powers of two larger than the
    // values required by the payload
    unsigned int max_name_count = 8192;
    unsigned int max_symbol_count = 16384;

    names.reserve(max_name_count);
    symbols.reserve(max_symbol_count);
    int names_by_hash_size = 2 * max_name_count;
    names_by_hash.resize(names_by_hash_size);
    ENFORCE((names_by_hash_size & (names_by_hash_size - 1)) == 0, "names_by_hash_size is not a power of 2");
}

void GlobalState::initEmpty() {
    UnfreezeFileTable fileTableAccess(*this);
    UnfreezeNameTable nameTableAccess(*this);
    UnfreezeSymbolTable symTableAccess(*this);
    names.emplace_back(); // first name is used in hashes to indicate empty cell
    names[0].kind = NameKind::UTF8;
    names[0].raw.utf8 = string_view();
    Names::registerNames(*this);

    SymbolRef id;
    id = synthesizeClass(no_symbol_str, 0);
    ENFORCE(id == Symbols::noSymbol());
    id = synthesizeClass(top_str, 0);
    ENFORCE(id == Symbols::top());
    id = synthesizeClass(bottom_str, 0);
    ENFORCE(id == Symbols::bottom());
    id = synthesizeClass(root_str, 0);
    ENFORCE(id == Symbols::root());
    id = synthesizeClass(todo_str, 0);
    ENFORCE(id == Symbols::todo());
    id = synthesizeClass(object_str, Symbols::BasicObject()._id);
    ENFORCE(id == Symbols::Object());
    id = synthesizeClass(integer_str);
    ENFORCE(id == Symbols::Integer());
    id = synthesizeClass(float_str);
    ENFORCE(id == Symbols::Float());
    id = synthesizeClass(string_str);
    ENFORCE(id == Symbols::String());
    id = synthesizeClass(symbol_str);
    ENFORCE(id == Symbols::Symbol());
    id = synthesizeClass(array_str);
    ENFORCE(id == Symbols::Array());
    id = synthesizeClass(hash_str);
    ENFORCE(id == Symbols::Hash());
    id = synthesizeClass(trueClass_str);
    ENFORCE(id == Symbols::TrueClass());
    id = synthesizeClass(falseClass_str);
    ENFORCE(id == Symbols::FalseClass());
    id = synthesizeClass(nilClass_str);
    ENFORCE(id == Symbols::NilClass());
    id = synthesizeClass(untyped_str, 0);
    ENFORCE(id == Symbols::untyped());
    id = synthesizeClass(opus_str, 0, true);
    ENFORCE(id == Symbols::Opus());
    id = synthesizeClass(T_str, Symbols::todo()._id, true);
    ENFORCE(id == Symbols::T());
    id = synthesizeClass(class_str, 0);
    ENFORCE(id == Symbols::Class());
    id = synthesizeClass(basicObject_str, 0);
    ENFORCE(id == Symbols::BasicObject());
    id = synthesizeClass(kernel_str, 0, true);
    ENFORCE(id == Symbols::Kernel());
    id = synthesizeClass(range_str);
    ENFORCE(id == Symbols::Range());
    id = synthesizeClass(regexp_str);
    ENFORCE(id == Symbols::Regexp());
    id = synthesizeClass(magic_str);
    ENFORCE(id == Symbols::Magic());
    id = synthesizeClass(module_str);
    ENFORCE(id == Symbols::Module());
    id = synthesizeClass(standardError_str);
    ENFORCE(id == Symbols::StandardError());
    id = synthesizeClass(complex_str);
    ENFORCE(id == Symbols::Complex());
    id = synthesizeClass(rational_str);
    ENFORCE(id == Symbols::Rational());
    id = enterClassSymbol(Loc::none(), Symbols::T(), enterNameConstant(array_str));
    ENFORCE(id == Symbols::T_Array());
    id = enterClassSymbol(Loc::none(), Symbols::T(), enterNameConstant(hash_str));
    ENFORCE(id == Symbols::T_Hash());
    id = enterClassSymbol(Loc::none(), Symbols::T(), enterNameConstant(proc_str));
    ENFORCE(id == Symbols::T_Proc());
    id = synthesizeClass(proc_str);
    ENFORCE(id == Symbols::Proc());
    id = synthesizeClass(enumerable_str, 0, true);
    ENFORCE(id == Symbols::Enumerable());
    id = synthesizeClass(set_str);
    ENFORCE(id == Symbols::Set());
    id = synthesizeClass(struct_str);
    ENFORCE(id == Symbols::Struct());
    id = synthesizeClass(file_str);
    ENFORCE(id == Symbols::File());
    id = synthesizeClass(ruby_typer_str, 0, true);
    ENFORCE(id == Symbols::RubyTyper());
    id = enterClassSymbol(Loc::none(), Symbols::RubyTyper(), enterNameConstant(stubClass_str));
    ENFORCE(id == Symbols::StubClass());
    id = enterClassSymbol(Loc::none(), Symbols::RubyTyper(), enterNameConstant(stubAncestor_str));
    ENFORCE(id == Symbols::StubAncestor());
    id = enterClassSymbol(Loc::none(), Symbols::T(), enterNameConstant(enumerable_str));
    ENFORCE(id == Symbols::T_Enumerable());
    id = enterClassSymbol(Loc::none(), Symbols::T(), enterNameConstant(range_str));
    ENFORCE(id == Symbols::T_Range());
    id = enterClassSymbol(Loc::none(), Symbols::T(), enterNameConstant(set_str));
    ENFORCE(id == Symbols::T_Set());
    id = synthesizeClass(configatron_str);
    ENFORCE(id == Symbols::Configatron());
    id = enterClassSymbol(Loc::none(), Symbols::Configatron(), enterNameConstant(store_str));
    ENFORCE(id == Symbols::Configatron_Store());
    id = enterClassSymbol(Loc::none(), Symbols::Configatron(), enterNameConstant(root_store_str));
    ENFORCE(id == Symbols::Configatron_RootStore());
    id = synthesizeClass(sinatra_str, 0, true);
    ENFORCE(id == Symbols::Sinatra());
    id = enterClassSymbol(Loc::none(), Symbols::Sinatra(), enterNameConstant(base_str));
    ENFORCE(id == Symbols::SinatraBase());
    id = enterClassSymbol(Loc::none(), Symbols::RubyTyper(), enterNameConstant(void_str));
    ENFORCE(id == Symbols::void_());
    id = synthesizeClass(typeAliasTemp_str, 0);
    ENFORCE(id == Symbols::typeAliasTemp());
    id = synthesizeClass(chalk_str, 0, true);
    ENFORCE(id == Symbols::Chalk());
    id = enterClassSymbol(Loc::none(), Symbols::Chalk(), enterNameConstant(tools_str));
    ENFORCE(id == Symbols::Chalk_Tools());
    id = enterClassSymbol(Loc::none(), Symbols::Chalk_Tools(), enterNameConstant(accessible_str));
    ENFORCE(id == Symbols::Chalk_Tools_Accessible());
    id = enterClassSymbol(Loc::none(), Symbols::T(), enterNameConstant(generic_str));
    ENFORCE(id == Symbols::T_Generic());
    id = enterClassSymbol(Loc::none(), Symbols::RubyTyper(), enterNameConstant(tuple_str));
    ENFORCE(id == Symbols::Tuple());
    id = enterClassSymbol(Loc::none(), Symbols::RubyTyper(), enterNameConstant(shape_str));
    ENFORCE(id == Symbols::Shape());
    id = enterClassSymbol(Loc::none(), Symbols::RubyTyper(), enterNameConstant(subclasses_str));
    ENFORCE(id == Symbols::Subclasses());
    id = synthesizeClass(sorbet_str);
    ENFORCE(id == Symbols::Sorbet());
    id = enterClassSymbol(Loc::none(), Symbols::RubyTyper(), enterNameConstant(implicit_module_superclass_str));
    ENFORCE(id == Symbols::RubyTyper_ImplicitModuleSuperClass());
    id = enterClassSymbol(Loc::none(), Symbols::RubyTyper(), enterNameConstant(return_type_inference_str));
    ENFORCE(id == Symbols::RubyTyper_ReturnTypeInference());
    id = enterMethodSymbol(Loc::none(), Symbols::RubyTyper(), enterNameUTF8(guessed_type_type_parameter_holder_str));
    ENFORCE(id == Symbols::RubyTyper_ReturnTypeInference_guessed_type_type_parameter_holder());
    id = enterTypeArgument(
        Loc::none(), Symbols::RubyTyper_ReturnTypeInference_guessed_type_type_parameter_holder(),
        freshNameUnique(core::UniqueNameKind::TypeVarName, enterNameUTF8(inferred_return_type_str), 1),
        core::Variance::ContraVariant);
    id.data(*this)->resultType = make_shared<core::TypeVar>(id);
    ENFORCE(id == Symbols::RubyTyper_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_contravariant());
    id = enterTypeArgument(
        Loc::none(), Symbols::RubyTyper_ReturnTypeInference_guessed_type_type_parameter_holder(),
        freshNameUnique(core::UniqueNameKind::TypeVarName, enterNameUTF8(inferred_argument_type_str), 1),
        core::Variance::CoVariant);
    id.data(*this)->resultType = make_shared<core::TypeVar>(id);
    ENFORCE(id == Symbols::RubyTyper_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_covariant());
    id = enterClassSymbol(Loc::none(), Symbols::Sorbet(), enterNameConstant(private_str));
    ENFORCE(id == Symbols::Sorbet_Private());
    id = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private(), enterNameConstant(builder_str));
    ENFORCE(id == Symbols::Sorbet_Private_Builder());
    id = enterClassSymbol(Loc::none(), Symbols::T(), enterNameConstant(helpers_str));
    ENFORCE(id == Symbols::T_Helpers());

    // Root members
    Symbols::root().data(*this, true)->members[enterNameConstant(no_symbol_str)] = Symbols::noSymbol();
    Symbols::root().data(*this, true)->members[enterNameConstant(top_str)] = Symbols::top();
    Symbols::root().data(*this, true)->members[enterNameConstant(bottom_str)] = Symbols::bottom();
    Context ctx(*this, Symbols::root());

    // Synthesize untyped = T.untyped
    Symbols::untyped().data(*this)->resultType = Types::untyped(*this, Symbols::untyped());

    // <Magic> has its own type
    Symbols::Magic().data(*this)->resultType = make_shared<ClassType>(Symbols::Magic());

    // Synthesize <Magic>#build_hash(*vs : T.untyped) => Hash
    SymbolRef method = enterMethodSymbol(Loc::none(), Symbols::Magic(), Names::buildHash());
    SymbolRef arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
    arg.data(*this)->setRepeated();
    arg.data(*this)->resultType = Types::untyped(*this, arg);
    method.data(*this)->arguments().emplace_back(arg);
    method.data(*this)->resultType = Types::hashOfUntyped();

    // Synthesize <Magic>#build_array(*vs : T.untyped) => Array
    method = enterMethodSymbol(Loc::none(), Symbols::Magic(), Names::buildArray());
    arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
    arg.data(*this)->setRepeated();
    arg.data(*this)->resultType = Types::untyped(*this, arg);
    method.data(*this)->arguments().emplace_back(arg);
    method.data(*this)->resultType = Types::arrayOfUntyped();

    // Synthesize <Magic>#<splat>(a: Array) => Untyped
    method = enterMethodSymbol(Loc::none(), Symbols::Magic(), Names::splat());
    arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
    arg.data(*this)->resultType = Types::arrayOfUntyped();
    method.data(*this)->arguments().emplace_back(arg);
    method.data(*this)->resultType = Types::untyped(*this, method);

    // Synthesize <Magic>#<defined>(arg0: Object) => Boolean
    method = enterMethodSymbol(Loc::none(), Symbols::Magic(), Names::defined_p());
    arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
    arg.data(*this)->resultType = Types::Object();
    method.data(*this)->arguments().emplace_back(arg);
    method.data(*this)->resultType = Types::any(ctx, Types::nilClass(), Types::String());

    // Synthesize <Magic>#<expandSplat>(arg0: T.untyped, arg1: Integer, arg2: Integer) => T.untyped
    method = enterMethodSymbol(Loc::none(), Symbols::Magic(), Names::expandSplat());
    arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
    arg.data(*this)->resultType = Types::untyped(*this, method);
    method.data(*this)->arguments().emplace_back(arg);
    arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg1());
    arg.data(*this)->resultType = Types::Integer();
    method.data(*this)->arguments().emplace_back(arg);
    arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg2());
    arg.data(*this)->resultType = Types::Integer();
    method.data(*this)->arguments().emplace_back(arg);
    method.data(*this)->resultType = Types::untyped(*this, method);

    // Synthesize <Magic>#<call-with-splat>(args: *T.untyped) => T.untyped
    method = enterMethodSymbol(Loc::none(), Symbols::Magic(), Names::callWithSplat());
    arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
    arg.data(*this)->resultType = Types::untyped(*this, method);
    arg.data(*this)->setRepeated();
    method.data(*this)->arguments().emplace_back(arg);
    method.data(*this)->resultType = Types::untyped(*this, method);

    // Some of these are Modules
    Symbols::T().data(*this)->setIsModule(true);
    Symbols::StubAncestor().data(*this)->setIsModule(true);

    // Some of these are Classes
    Symbols::SinatraBase().data(*this)->setIsModule(false);
    Symbols::SinatraBase().data(*this)->superClass = Symbols::Object();

    // Synthesize GarbageType
    id = enterStaticFieldSymbol(Loc::none(), Symbols::Sorbet(), enterNameConstant(garbage_type_str));
    id.data(*this)->resultType = make_shared<core::AliasType>(Symbols::untyped());

    int reservedCount = 0;

    // Set the correct resultTypes for all synthesized classes
    // Does it in two passes since the singletonClass will go in the Symbols::root() members which will invalidate the
    // iterator
    vector<SymbolRef> needSingletons;
    for (auto &sym : symbols) {
        auto ref = sym.ref(*this);
        if (ref.exists() && sym.isClass()) {
            needSingletons.emplace_back(ref);
        }
    }
    for (auto sym : needSingletons) {
        sym.data(*this)->singletonClass(*this);
    }

    ENFORCE(symbols.size() < Symbols::Proc0()._id);
    while (symbols.size() < Symbols::Proc0()._id) {
        string res = absl::StrCat(reserved_str, reservedCount);
        synthesizeClass(res);
        reservedCount++;
    }

    for (int arity = 0; arity <= Symbols::MAX_PROC_ARITY; ++arity) {
        auto id = synthesizeClass(absl::StrCat("Proc", arity), Symbols::Proc()._id);
        ENFORCE(id == Symbols::Proc(arity), "Proc creation failed for arity: ", arity, " got: ", id._id,
                " expected: ", Symbols::Proc(arity)._id);
        id.data(*this)->singletonClass(*this);
    }

    ENFORCE(symbols.size() == Symbols::last_synthetic_sym()._id + 1,
            "Too many synthetic symbols? have: ", symbols.size(), " expected: ", Symbols::last_synthetic_sym()._id + 1);

    installIntrinsics();

    // First file is used to indicate absence of a file
    files.emplace_back();
    freezeNameTable();
    freezeSymbolTable();
    freezeFileTable();
    sanityCheck();
}

void GlobalState::installIntrinsics() {
    for (auto &entry : intrinsicMethods) {
        auto symbol = entry.symbol;
        if (entry.singleton) {
            symbol = symbol.data(*this)->singletonClass(*this);
        }
        SymbolRef method = enterMethodSymbol(Loc::none(), symbol, entry.method);
        method.data(*this)->intrinsic = entry.impl;
    }
}

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
u4 nextPowerOfTwo(u4 v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

void GlobalState::reserveMemory(u4 kb) {
    u8 allocated = (sizeof(Name) + sizeof(decltype(names_by_hash)::value_type)) * names.capacity() +
                   sizeof(Symbol) * symbols.capacity();
    u8 want = 1024 * kb;
    if (allocated > want) {
        return;
    }
    u4 scale = nextPowerOfTwo(want / allocated);
    symbols.reserve(symbols.capacity() * scale);
    expandNames(scale);
    sanityCheck();

    allocated = (sizeof(Name) + sizeof(decltype(names_by_hash)::value_type)) * names.capacity() +
                sizeof(Symbol) * symbols.capacity();

    trace(absl::StrCat("Reserved ", allocated / 1024, "KiB of memory. symbols=", symbols.capacity(),
                       " names=", names.capacity()));
}

constexpr decltype(GlobalState::STRINGS_PAGE_SIZE) GlobalState::STRINGS_PAGE_SIZE;

SymbolRef GlobalState::enterSymbol(Loc loc, SymbolRef owner, NameRef name, u4 flags) {
    ENFORCE(owner.exists(), "entering symbol in to non-existing owner");
    ENFORCE(name.exists(), "entering symbol with non-existing name");
    SymbolData ownerScope = owner.data(*this, true);
    histogramInc("symbol_enter_by_name", ownerScope->members.size());

    auto &store = ownerScope->members[name];
    if (store.exists()) {
        ENFORCE((store.data(*this)->flags & flags) == flags, "existing symbol has wrong flags");
        counterInc("symbols.hit");
        return store;
    }

    ENFORCE(!symbolTableFrozen);

    SymbolRef ret = SymbolRef(this, symbols.size());
    store = ret; // DO NOT MOVE this assignment down. emplace_back on symbol invalidates `store`
    symbols.emplace_back();
    SymbolData data = ret.data(*this, true);
    data->name = name;
    data->flags = flags;
    data->owner = owner;
    data->addLoc(*this, loc);
    if (data->isBlockSymbol(*this)) {
        categoryCounterInc("symbols", "block");
    } else if (data->isClass()) {
        categoryCounterInc("symbols", "class");
    } else if (data->isMethod()) {
        categoryCounterInc("symbols", "method");
    } else if (data->isField()) {
        categoryCounterInc("symbols", "field");
    } else if (data->isStaticField()) {
        categoryCounterInc("symbols", "static_field");
    } else if (data->isMethodArgument()) {
        categoryCounterInc("symbols", "argument");
    }

    wasModified_ = true;
    return ret;
}

SymbolRef GlobalState::enterClassSymbol(Loc loc, SymbolRef owner, NameRef name) {
    ENFORCE(!owner.exists() || // used when entering entirely syntehtic classes
            owner.data(*this)->isClass());
    ENFORCE(name.data(*this)->isClassName(*this));
    return enterSymbol(loc, owner, name, Symbol::Flags::CLASS);
}

SymbolRef GlobalState::enterTypeMember(Loc loc, SymbolRef owner, NameRef name, Variance variance) {
    u4 flags;
    ENFORCE(owner.data(*this)->isClass());
    if (variance == Variance::Invariant) {
        flags = Symbol::Flags::TYPE_INVARIANT;
    } else if (variance == Variance::CoVariant) {
        flags = Symbol::Flags::TYPE_COVARIANT;
    } else if (variance == Variance::ContraVariant) {
        flags = Symbol::Flags::TYPE_CONTRAVARIANT;
    } else {
        Exception::notImplemented();
    }

    flags = flags | Symbol::Flags::TYPE_MEMBER;
    SymbolRef result = enterSymbol(loc, owner, name, flags);
    auto &members = owner.data(*this)->typeMembers();
    if (!absl::c_linear_search(members, result)) {
        members.emplace_back(result);
    }
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
        Exception::notImplemented();
    }

    flags = flags | Symbol::Flags::TYPE_ARGUMENT;
    SymbolRef result = enterSymbol(loc, owner, name, flags);
    owner.data(*this)->typeArguments().emplace_back(result);
    return result;
}

SymbolRef GlobalState::enterMethodSymbol(Loc loc, SymbolRef owner, NameRef name) {
    bool isBlock =
        name.data(*this)->kind == NameKind::UNIQUE && name.data(*this)->unique.original == Names::blockTemp();
    ENFORCE(isBlock || owner.data(*this)->isClass(), "entering method symbol into not-a-class");
    return enterSymbol(loc, owner, name, Symbol::Flags::METHOD);
}

SymbolRef GlobalState::enterNewMethodOverload(Loc loc, SymbolRef original, u2 num) {
    NameRef name = freshNameUnique(UniqueNameKind::Overload, original.data(*this)->name, num);
    SymbolRef res = enterMethodSymbol(loc, original.data(*this)->owner, name);
    res.data(*this)->arguments().reserve(original.data(*this)->arguments().size());
    for (auto &arg : original.data(*this)->arguments()) {
        Loc loc = arg.data(*this)->loc();
        NameRef nm = arg.data(*this)->name;
        SymbolRef newArg = enterMethodArgumentSymbol(loc, res, nm);
        newArg.data(*this)->flags = arg.data(*this)->flags;
        res.data(*this)->arguments().emplace_back(newArg);
    }
    return res;
}

SymbolRef GlobalState::enterFieldSymbol(Loc loc, SymbolRef owner, NameRef name) {
    ENFORCE(owner.data(*this)->isClass(), "entering field symbol into not-a-class");
    return enterSymbol(loc, owner, name, Symbol::Flags::FIELD);
}

SymbolRef GlobalState::enterStaticFieldSymbol(Loc loc, SymbolRef owner, NameRef name) {
    // ENFORCE(owner.data(*this)->isClass()); // TODO: enable. This is violated by proto
    return enterSymbol(loc, owner, name, Symbol::Flags::STATIC_FIELD);
}

SymbolRef GlobalState::enterMethodArgumentSymbol(Loc loc, SymbolRef owner, NameRef name) {
    ENFORCE(owner.data(*this)->isMethod(), "entering method argument symbol into not-a-method");
    return enterSymbol(loc, owner, name, Symbol::Flags::METHOD_ARGUMENT);
}

string_view GlobalState::enterString(string_view nm) {
    char *from = nullptr;
    if (nm.size() > GlobalState::STRINGS_PAGE_SIZE) {
        auto &inserted = strings.emplace_back(make_unique<vector<char>>(nm.size()));
        from = inserted->data();
        if (strings.size() > 1) {
            // last page wasn't full, keep it in the end
            swap(*(strings.end() - 1), *(strings.end() - 2));
        }
    } else {
        if (strings_last_page_used + nm.size() > GlobalState::STRINGS_PAGE_SIZE) {
            strings.emplace_back(make_unique<vector<char>>(GlobalState::STRINGS_PAGE_SIZE));
            // printf("Wasted %i space\n", STRINGS_PAGE_SIZE - strings_last_page_used);
            strings_last_page_used = 0;
        }
        from = strings.back()->data() + strings_last_page_used;
    }

    counterInc("strings");
    memcpy(from, nm.data(), nm.size());
    strings_last_page_used += nm.size();
    return string_view(from, nm.size());
}

NameRef GlobalState::enterNameUTF8(string_view nm) {
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
            } else {
                counterInc("names.hash_collision.utf8");
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
    ENFORCE(names[idx].hash(*this) == hs);
    categoryCounterInc("names", "utf8");

    wasModified_ = true;
    return NameRef(*this, idx);
}

NameRef GlobalState::enterNameConstant(NameRef original) {
    ENFORCE(original.exists(), "making a constant name over non-exiting name");
    ENFORCE(original.data(*this)->kind == UTF8 ||
                (original.data(*this)->kind == UNIQUE &&
                 original.data(*this)->unique.uniqueNameKind == UniqueNameKind::ResolverMissingClass),
            "making a constant name over wrong name kind");

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
            } else {
                counterInc("names.hash_collision.constant");
            }
        }
        bucketId = (bucketId + probe_count) & mask;
        probe_count++;
    }
    if (probe_count == hashTableSize) {
        Exception::raise("Full table?");
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
    ENFORCE(names[idx].hash(*this) == hs);
    wasModified_ = true;
    categoryCounterInc("names", "constant");
    return NameRef(*this, idx);
}

NameRef GlobalState::enterNameConstant(string_view original) {
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

void GlobalState::expandNames(int growBy) {
    sanityCheck();

    names.reserve(names.capacity() * growBy);
    vector<pair<unsigned int, unsigned int>> new_names_by_hash(names_by_hash.capacity() * growBy);
    moveNames(names_by_hash.data(), new_names_by_hash.data(), names_by_hash.size(), new_names_by_hash.capacity());
    names_by_hash.swap(new_names_by_hash);
}

NameRef GlobalState::getNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num) const {
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
            } else {
                counterInc("names.hash_collision.unique");
            }
        }
        bucketId = (bucketId + probe_count) & mask;
        probe_count++;
    }
    Exception::raise("should never happen");
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
            } else {
                counterInc("names.hash_collision.unique");
            }
        }
        bucketId = (bucketId + probe_count) & mask;
        probe_count++;
    }
    if (probe_count == hashTableSize) {
        Exception::raise("Full table?");
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
    ENFORCE(names[idx].hash(*this) == hs);
    wasModified_ = true;
    categoryCounterInc("names", "unique");
    return NameRef(*this, idx);
}

FileRef GlobalState::enterFile(const shared_ptr<File> &file) {
    ENFORCE(!fileTableFrozen);

    DEBUG_ONLY(for (auto &f
                    : this->files) {
        if (f) {
            if (f->path() == file->path()) {
                Exception::raise("should never happen");
            }
        }
    })

    files.emplace_back(file);
    auto ret = FileRef(filesUsed() - 1);
    fileRefByPath[string(file->path())] = ret;
    return ret;
}

FileRef GlobalState::enterFile(string_view path, string_view source) {
    return GlobalState::enterFile(
        make_shared<File>(string(path.begin(), path.end()), string(source.begin(), source.end()), File::Type::Normal));
}

FileRef GlobalState::enterFileAt(string_view path, string_view source, FileRef id) {
    if (this->files[id.id()] && this->files[id.id()]->sourceType != File::Type::TombStone) {
        Exception::raise("should never happen");
    }

    auto ret = GlobalState::enterNewFileAt(
        make_shared<File>(string(path.begin(), path.end()), string(source.begin(), source.end()), File::Type::Normal),
        id);
    ENFORCE(ret == id);
    return ret;
}

FileRef GlobalState::enterNewFileAt(const shared_ptr<File> &file, FileRef id) {
    ENFORCE(!fileTableFrozen);
    ENFORCE(id.id() < this->files.size());
    ENFORCE(this->files[id.id()]->sourceType == File::Type::TombStone);
    ENFORCE(this->files[id.id()]->path() == file->path());

    // was a tombstone before.
    this->files[id.id()] = file;
    FileRef result(id);
    return result;
}

FileRef GlobalState::reserveFileRef(string path) {
    return GlobalState::enterFile(make_shared<File>(move(path), "", File::Type::TombStone));
}

void GlobalState::mangleRenameSymbol(SymbolRef what, NameRef origName, UniqueNameKind kind) {
    auto &owner = what.data(*this)->owner;
    auto &members = owner.data(*this)->members;
    auto fnd = members.find(origName);
    ENFORCE(fnd != members.end());
    auto oldSym = fnd->second;
    u2 collisionCount = 1;
    NameRef name;
    do {
        name = freshNameUnique(kind, origName, collisionCount++);
    } while (owner.data(*this)->findMember(*this, name).exists());
    members.erase(fnd);
    members[name] = oldSym;
    oldSym.data(*this)->name = name;
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
    ENFORCE(names.capacity() * 2 == names_by_hash.capacity(), "name table and hash name table sizes out of sync",
            " names.capacity=", names.capacity(), " names_by_hash.capacity=", names_by_hash.capacity());
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

unique_ptr<GlobalState> GlobalState::deepCopy(bool keepId) const {
    this->sanityCheck();
    auto result = make_unique<GlobalState>(this->errorQueue);
    result->silenceErrors = this->silenceErrors;
    result->suggestGarbageType = this->suggestGarbageType;

    if (keepId) {
        result->globalStateId = this->globalStateId;
    }
    result->parentGlobalStateId = this->globalStateId;
    result->lastNameKnownByParentGlobalState = namesUsed();

    result->strings = this->strings;
    result->strings_last_page_used = STRINGS_PAGE_SIZE;
    result->files = this->files;
    result->fileRefByPath = this->fileRefByPath;
    result->lspInfoQueryLoc = this->lspInfoQueryLoc;
    result->lspQuerySymbol = this->lspQuerySymbol;

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

void GlobalState::addAnnotation(Loc loc, string str, u4 blockId, AnnotationPos pos) const {
    unique_lock<mutex> lk(annotations_mtx);
    annotations.emplace_back(Annotation{loc, move(str), pos, blockId});
}

string GlobalState::showAnnotatedSource(FileRef file) const {
    unique_lock<mutex> lk(annotations_mtx);
    if (annotations.empty()) {
        return "";
    }

    // Sort the locs backwards
    auto compare = [](const Annotation &left, const Annotation &right) {
        if (left.loc.file() != right.loc.file()) {
            return left.loc.file().id() > right.loc.file().id();
        }

        auto a = left.pos == GlobalState::AnnotationPos::BEFORE ? left.loc.beginPos() : left.loc.endPos();
        auto b = right.pos == GlobalState::AnnotationPos::BEFORE ? right.loc.beginPos() : right.loc.endPos();

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
        return left.blockId > right.blockId;
    };
    auto sorted = annotations;
    fast_sort(sorted, compare);

    auto source = file.data(*this).source();
    string outline(source.begin(), source.end());
    for (auto annotation : sorted) {
        if (annotation.loc.file() != file) {
            continue;
        }
        fmt::memory_buffer buf;

        auto pos = annotation.loc.position(*this);
        vector<string> lines = absl::StrSplit(annotation.str, '\n');
        while (!lines.empty() && lines.back().empty()) {
            lines.pop_back();
        }
        if (!lines.empty()) {
            fmt::format_to(buf, "\n");
            for (auto line : lines) {
                string spaces(pos.first.column - 1, ' ');
                fmt::format_to(buf, "{}#{}{}\n", spaces, line.empty() ? "" : " ", line);
            }
        }
        auto out = to_string(buf);
        out = out.substr(0, out.size() - 1); // Remove the last newline that the buf always has

        size_t start_of_line;
        switch (annotation.pos) {
            case GlobalState::AnnotationPos::BEFORE:
                start_of_line = annotation.loc.beginPos();
                start_of_line = outline.find_last_of('\n', start_of_line);
                if (start_of_line == string::npos) {
                    start_of_line = 0;
                }
                break;
            case GlobalState::AnnotationPos::AFTER:
                start_of_line = annotation.loc.endPos();
                start_of_line = outline.find_first_of('\n', start_of_line);
                if (start_of_line == string::npos) {
                    start_of_line = outline.end() - outline.begin();
                }
                break;
        }
        outline = absl::StrCat(outline.substr(0, start_of_line), out, outline.substr(start_of_line));
    }
    return outline;
}

int GlobalState::totalErrors() const {
    return errorQueue->nonSilencedErrorCount.load();
}

void GlobalState::_error(unique_ptr<Error> error) const {
    if (error->isCritical()) {
        errorQueue->hadCritical = true;
    }
    auto loc = error->loc;
    if (loc.file().exists()) {
        loc.file().data(*this).hadErrors_ = true;
    }

    errorQueue->pushError(*this, move(error));
}

bool GlobalState::hadCriticalError() const {
    return errorQueue->hadCritical;
}

ErrorBuilder GlobalState::beginError(Loc loc, ErrorClass what) const {
    bool reportForFile = shouldReportErrorOn(loc, what);
    if (reportForFile) {
        histogramAdd("error", what.code, 1);
    }
    bool report = (what == errors::Internal::InternalError) || (reportForFile && !this->silenceErrors);
    return ErrorBuilder(*this, report, loc, what);
}

void GlobalState::suppressErrorClass(ErrorClass err) {
    suppressed_error_classes.insert(err);
}

bool GlobalState::shouldReportErrorOn(Loc loc, ErrorClass what) const {
    StrictLevel level = StrictLevel::Strong;
    if (loc.file().exists()) {
        level = loc.file().data(*this).strict;
    }
    if (what.code == errors::Internal::InternalError.code) {
        return true;
    }
    if (suppressed_error_classes.count(what) != 0) {
        return false;
    }

    if (level > StrictLevel::Strong) {
        // Custom rules
        if (level == StrictLevel::Autogenerated) {
            level = StrictLevel::Strict;
            if (what == errors::Resolver::StubConstant || what == errors::Infer::UntypedMethod) {
                return false;
            }
        }
    }
    ENFORCE(level <= StrictLevel::Strong);

    return level >= what.minLevel;
}

bool GlobalState::wasModified() const {
    return wasModified_;
}

void GlobalState::trace(std::string_view msg) const {
    errorQueue->tracer.trace(msg);
}

void GlobalState::markAsPayload() {
    bool seenEmpty = false;
    for (auto &f : files) {
        if (!seenEmpty) {
            ENFORCE(!f);
            seenEmpty = true;
            continue;
        }
        f->sourceType = File::Type::Payload;
    }
}

unique_ptr<GlobalState> GlobalState::replaceFile(unique_ptr<GlobalState> inWhat, FileRef whatFile,
                                                 const shared_ptr<File> &withWhat) {
    ENFORCE(whatFile.id() < inWhat->filesUsed());
    ENFORCE(whatFile.data(*inWhat, true).path() == withWhat->path());
    inWhat->files[whatFile.id()] = withWhat;
    return inWhat;
}

FileRef GlobalState::findFileByPath(string_view path) {
    auto fnd = fileRefByPath.find(string(path));
    if (fnd != fileRefByPath.end()) {
        return fnd->second;
    }
    return FileRef();
}

unique_ptr<GlobalState> GlobalState::markFileAsTombStone(unique_ptr<GlobalState> what, FileRef fref) {
    ENFORCE(fref.id() < what->filesUsed());
    what->files[fref.id()]->sourceType = File::Type::TombStone;
    return what;
}

unsigned int GlobalState::hash() const {
    constexpr bool DEBUG_HASHING_TAIL = false;
    unsigned int result = 0;
    int counter = 0;
    for (const auto &sym : this->symbols) {
        counter++;
        if (!sym.ignoreInHashing(*this)) {
            result = mix(result, sym.hash(*this));
        }
        if (DEBUG_HASHING_TAIL && counter > symbolsUsed() - 15) {
            errorQueue->logger.info("Hashing symbols: {}, {}", result, sym.name.show(*this));
        }
    }

    return (result == GlobalState::HASH_STATE_INVALID) ? GlobalState::HASH_STATE_INVALID + 1 : result;
}

vector<shared_ptr<File>> GlobalState::getFiles() const {
    return files;
}

SymbolRef GlobalState::staticInitForFile(FileRef file) {
    auto nm = freshNameUnique(core::UniqueNameKind::Namer, core::Names::staticInit(), file.id());
    return enterMethodSymbol(core::Loc::none(file), core::Symbols::root(), nm);
}

} // namespace sorbet::core
