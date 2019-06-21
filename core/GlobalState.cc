#include "GlobalState.h"

#include "common/Timer.h"
#include "core/Error.h"
#include "core/Hashing.h"
#include "core/NameHash.h"
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

SymbolRef GlobalState::synthesizeClass(NameRef nameId, u4 superclass, bool isModule) {
    // This can't use enterClass since there is a chicken and egg problem.
    // These will be added to Symbols::root().members later.
    SymbolRef symRef = SymbolRef(this, symbols.size());
    symbols.emplace_back();
    SymbolData data = symRef.dataAllowingNone(*this); // allowing noSymbol is needed because this enters noSymbol.
    data->name = nameId;
    data->owner = Symbols::root();
    data->flags = 0;
    data->setClass();
    data->setIsModule(isModule);
    data->setSuperClass(SymbolRef(this, superclass));

    if (symRef._id > Symbols::root()._id) {
        Symbols::root().dataAllowingNone(*this)->members()[nameId] = symRef;
    }
    return symRef;
}

atomic<int> globalStateIdCounter(1);
const int Symbols::MAX_PROC_ARITY;

GlobalState::GlobalState(shared_ptr<ErrorQueue> errorQueue)
    : globalStateId(globalStateIdCounter.fetch_add(1)), errorQueue(std::move(errorQueue)),
      lspQuery(lsp::Query::noQuery()) {
    // Empirically determined to be the smallest powers of two larger than the
    // values required by the payload
    unsigned int maxNameCount = 8192;
    unsigned int maxSymbolCount = 16384;

    names.reserve(maxNameCount);
    symbols.reserve(maxSymbolCount);
    int namesByHashSize = 2 * maxNameCount;
    namesByHash.resize(namesByHashSize);
    ENFORCE((namesByHashSize & (namesByHashSize - 1)) == 0, "namesByHashSize is not a power of 2");
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
    id = synthesizeClass(core::Names::Constants::NoSymbol(), 0);
    ENFORCE(id == Symbols::noSymbol());
    id = synthesizeClass(core::Names::Constants::Top(), 0);
    ENFORCE(id == Symbols::top());
    id = synthesizeClass(core::Names::Constants::Bottom(), 0);
    ENFORCE(id == Symbols::bottom());
    id = synthesizeClass(core::Names::Constants::Root(), 0);
    ENFORCE(id == Symbols::root());
    id = core::Symbols::root().data(*this)->singletonClass(*this);
    ENFORCE(id == Symbols::rootSingleton());
    id = synthesizeClass(core::Names::Constants::Todo(), 0);
    ENFORCE(id == Symbols::todo());
    id = synthesizeClass(core::Names::Constants::Object(), Symbols::BasicObject()._id);
    ENFORCE(id == Symbols::Object());
    id = synthesizeClass(core::Names::Constants::Integer());
    ENFORCE(id == Symbols::Integer());
    id = synthesizeClass(core::Names::Constants::Float());
    ENFORCE(id == Symbols::Float());
    id = synthesizeClass(core::Names::Constants::String());
    ENFORCE(id == Symbols::String());
    id = synthesizeClass(core::Names::Constants::Symbol());
    ENFORCE(id == Symbols::Symbol());
    id = synthesizeClass(core::Names::Constants::Array());
    ENFORCE(id == Symbols::Array());
    id = synthesizeClass(core::Names::Constants::Hash());
    ENFORCE(id == Symbols::Hash());
    id = synthesizeClass(core::Names::Constants::TrueClass());
    ENFORCE(id == Symbols::TrueClass());
    id = synthesizeClass(core::Names::Constants::FalseClass());
    ENFORCE(id == Symbols::FalseClass());
    id = synthesizeClass(core::Names::Constants::NilClass());
    ENFORCE(id == Symbols::NilClass());
    id = synthesizeClass(core::Names::Constants::Untyped(), 0);
    ENFORCE(id == Symbols::untyped());
    id = synthesizeClass(core::Names::Constants::Opus(), 0, true);
    ENFORCE(id == Symbols::Opus());
    id = synthesizeClass(core::Names::Constants::T(), Symbols::todo()._id, true);
    ENFORCE(id == Symbols::T());
    id = synthesizeClass(core::Names::Constants::Class(), 0);
    ENFORCE(id == Symbols::Class());
    id = synthesizeClass(core::Names::Constants::BasicObject(), 0);
    ENFORCE(id == Symbols::BasicObject());
    id = synthesizeClass(core::Names::Constants::Kernel(), 0, true);
    ENFORCE(id == Symbols::Kernel());
    id = synthesizeClass(core::Names::Constants::Range());
    ENFORCE(id == Symbols::Range());
    id = synthesizeClass(core::Names::Constants::Regexp());
    ENFORCE(id == Symbols::Regexp());
    id = synthesizeClass(core::Names::Constants::Magic());
    ENFORCE(id == Symbols::Magic());
    id = Symbols::Magic().data(*this)->singletonClass(*this);
    ENFORCE(id == Symbols::MagicSingleton());
    id = synthesizeClass(core::Names::Constants::Module());
    ENFORCE(id == Symbols::Module());
    id = synthesizeClass(core::Names::Constants::StandardError());
    ENFORCE(id == Symbols::StandardError());
    id = synthesizeClass(core::Names::Constants::Complex());
    ENFORCE(id == Symbols::Complex());
    id = synthesizeClass(core::Names::Constants::Rational());
    ENFORCE(id == Symbols::Rational());
    id = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Array());
    ENFORCE(id == Symbols::T_Array());
    id = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Hash());
    ENFORCE(id == Symbols::T_Hash());
    id = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Proc());
    ENFORCE(id == Symbols::T_Proc());
    id = synthesizeClass(core::Names::Constants::Proc());
    ENFORCE(id == Symbols::Proc());
    id = synthesizeClass(core::Names::Constants::Enumerable(), 0, true);
    ENFORCE(id == Symbols::Enumerable());
    id = synthesizeClass(core::Names::Constants::Set());
    ENFORCE(id == Symbols::Set());
    id = synthesizeClass(core::Names::Constants::Struct());
    ENFORCE(id == Symbols::Struct());
    id = synthesizeClass(core::Names::Constants::File());
    ENFORCE(id == Symbols::File());
    id = synthesizeClass(core::Names::Constants::Sorbet());
    ENFORCE(id == Symbols::Sorbet());
    id = enterClassSymbol(Loc::none(), Symbols::Sorbet(), core::Names::Constants::Private());
    ENFORCE(id == Symbols::Sorbet_Private());
    id = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private(), core::Names::Constants::Static());
    ENFORCE(id == Symbols::Sorbet_Private_Static());
    id = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::StubModule());
    ENFORCE(id == Symbols::StubModule());
    id = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::StubAncestor());
    ENFORCE(id == Symbols::StubAncestor());
    id = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Enumerable());
    ENFORCE(id == Symbols::T_Enumerable());
    id = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Range());
    ENFORCE(id == Symbols::T_Range());
    id = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Set());
    ENFORCE(id == Symbols::T_Set());
    id = synthesizeClass(core::Names::Constants::Configatron());
    ENFORCE(id == Symbols::Configatron());
    id = enterClassSymbol(Loc::none(), Symbols::Configatron(), core::Names::Constants::Store());
    ENFORCE(id == Symbols::Configatron_Store());
    id = enterClassSymbol(Loc::none(), Symbols::Configatron(), core::Names::Constants::RootStore());
    ENFORCE(id == Symbols::Configatron_RootStore());
    id = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::Void());
    ENFORCE(id == Symbols::void_());
    id = synthesizeClass(core::Names::Constants::TypeAlias(), 0);
    ENFORCE(id == Symbols::typeAliasTemp());
    id = synthesizeClass(core::Names::Constants::Chalk(), 0, true);
    ENFORCE(id == Symbols::Chalk());
    id = enterClassSymbol(Loc::none(), Symbols::Chalk(), core::Names::Constants::Tools());
    ENFORCE(id == Symbols::Chalk_Tools());
    id = enterClassSymbol(Loc::none(), Symbols::Chalk_Tools(), core::Names::Constants::Accessible());
    ENFORCE(id == Symbols::Chalk_Tools_Accessible());
    id = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Generic());
    ENFORCE(id == Symbols::T_Generic());
    id = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::Tuple());
    ENFORCE(id == Symbols::Tuple());
    id = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::Shape());
    ENFORCE(id == Symbols::Shape());
    id = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::Subclasses());
    ENFORCE(id == Symbols::Subclasses());
    id = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(),
                          core::Names::Constants::ImplicitModuleSuperclass());
    ENFORCE(id == Symbols::Sorbet_Private_Static_ImplicitModuleSuperClass());
    id = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::ReturnTypeInference());
    ENFORCE(id == Symbols::Sorbet_Private_Static_ReturnTypeInference());
    id =
        enterMethodSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::guessedTypeTypeParameterHolder());
    ENFORCE(id == Symbols::Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder());
    {
        auto &arg = enterMethodArgumentSymbol(
            Loc::none(), Symbols::Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder(),
            Names::blkArg());
        arg.flags.isBlock = true;
    }
    id = enterTypeArgument(
        Loc::none(), Symbols::Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder(),
        freshNameUnique(core::UniqueNameKind::TypeVarName, core::Names::Constants::InferredReturnType(), 1),
        core::Variance::ContraVariant);
    id.data(*this)->resultType = make_type<core::TypeVar>(id);
    ENFORCE(
        id ==
        Symbols::Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_contravariant());
    id = enterTypeArgument(
        Loc::none(), Symbols::Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder(),
        freshNameUnique(core::UniqueNameKind::TypeVarName, core::Names::Constants::InferredArgumentType(), 1),
        core::Variance::CoVariant);
    id.data(*this)->resultType = make_type<core::TypeVar>(id);
    ENFORCE(id ==
            Symbols::Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_covariant());
    id = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Sig());
    ENFORCE(id == Symbols::T_Sig());

    // A magic non user-creatable class with methods to keep state between passes
    id = enterFieldSymbol(Loc::none(), Symbols::Magic(), core::Names::Constants::UndeclaredFieldStub());
    ENFORCE(id == Symbols::Magic_undeclaredFieldStub());

    // Sorbet::Private::Static#badAliasMethodStub(*arg0 : T.untyped) => T.untyped
    id = enterMethodSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::badAliasMethodStub());
    ENFORCE(id == Symbols::Sorbet_Private_Static_badAliasMethodStub());
    id.data(*this)->resultType = Types::untyped(*this, id);
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), Symbols::Sorbet_Private_Static_badAliasMethodStub(),
                                              core::Names::arg0());
        arg.flags.isRepeated = true;
        arg.type = Types::untyped(*this, id);
    }
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), Symbols::Sorbet_Private_Static_badAliasMethodStub(),
                                              core::Names::blkArg());
        arg.flags.isBlock = true;
        arg.type = Types::untyped(*this, id);
    }

    // T::Helpers
    id = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Helpers());
    ENFORCE(id == Symbols::T_Helpers());

    // SigBuilder magic class
    id = synthesizeClass(core::Names::Constants::DeclBuilderForProcs());
    ENFORCE(id == Symbols::DeclBuilderForProcs());
    id = Symbols::DeclBuilderForProcs().data(*this)->singletonClass(*this);
    ENFORCE(id == Symbols::DeclBuilderForProcsSingleton());

    // Ruby 2.5 Hack
    id = synthesizeClass(core::Names::Constants::Net(), 0, true);
    ENFORCE(id == Symbols::Net());
    id = enterClassSymbol(Loc::none(), Symbols::Net(), core::Names::Constants::IMAP());
    Symbols::Net_IMAP().data(*this)->setIsModule(false);
    ENFORCE(id == Symbols::Net_IMAP());
    id = enterClassSymbol(Loc::none(), Symbols::Net(), core::Names::Constants::Protocol());
    ENFORCE(id == Symbols::Net_Protocol());
    Symbols::Net_Protocol().data(*this)->setIsModule(false);

    // A magic symbol to cause CFGs to be exported
    id = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::CFGExport());
    ENFORCE(id == Symbols::T_CFGExport());

    id = enterClassSymbol(Loc::none(), Symbols::T_Sig(), core::Names::Constants::WithoutRuntime());
    ENFORCE(id == Symbols::T_Sig_WithoutRuntime());

    id = synthesizeClass(core::Names::Constants::Enumerator());
    ENFORCE(id == Symbols::Enumerator());
    id = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Enumerator());
    ENFORCE(id == Symbols::T_Enumerator());

    id = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Struct());
    ENFORCE(id == Symbols::T_Struct());

    // Root members
    Symbols::root().dataAllowingNone(*this)->members()[core::Names::Constants::NoSymbol()] = Symbols::noSymbol();
    Symbols::root().dataAllowingNone(*this)->members()[core::Names::Constants::Top()] = Symbols::top();
    Symbols::root().dataAllowingNone(*this)->members()[core::Names::Constants::Bottom()] = Symbols::bottom();
    Context ctx(*this, Symbols::root());

    // Synthesize <Magic>#build_hash(*vs : T.untyped) => Hash
    SymbolRef method = enterMethodSymbol(Loc::none(), Symbols::MagicSingleton(), Names::buildHash());
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
        arg.flags.isRepeated = true;
        arg.type = Types::untyped(*this, method);
    }
    method.data(*this)->resultType = Types::hashOfUntyped();
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
        arg.flags.isBlock = true;
    }
    // Synthesize <Magic>#build_array(*vs : T.untyped) => Array
    method = enterMethodSymbol(Loc::none(), Symbols::MagicSingleton(), Names::buildArray());
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
        arg.flags.isRepeated = true;
        arg.type = Types::untyped(*this, method);
    }
    method.data(*this)->resultType = Types::arrayOfUntyped();
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
        arg.flags.isBlock = true;
    }
    // Synthesize <Magic>#<splat>(a: Array) => Untyped
    method = enterMethodSymbol(Loc::none(), Symbols::MagicSingleton(), Names::splat());
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
        arg.type = Types::arrayOfUntyped();
    }
    method.data(*this)->resultType = Types::untyped(*this, method);

    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
        arg.flags.isBlock = true;
    }

    // Synthesize <Magic>#<defined>(arg0: Object) => Boolean
    method = enterMethodSymbol(Loc::none(), Symbols::MagicSingleton(), Names::defined_p());
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
        arg.type = Types::Object();
    }
    method.data(*this)->resultType = Types::any(ctx, Types::nilClass(), Types::String());
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
        arg.flags.isBlock = true;
    }

    // Synthesize <Magic>#<expandSplat>(arg0: T.untyped, arg1: Integer, arg2: Integer) => T.untyped
    method = enterMethodSymbol(Loc::none(), Symbols::MagicSingleton(), Names::expandSplat());
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
        arg.type = Types::untyped(*this, method);
    }
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg1());
        arg.type = Types::Integer();
    }
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg2());
        arg.type = Types::Integer();
    }
    method.data(*this)->resultType = Types::untyped(*this, method);
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
        arg.flags.isBlock = true;
    }
    // Synthesize <Magic>#<call-with-splat>(args: *T.untyped) => T.untyped
    method = enterMethodSymbol(Loc::none(), Symbols::MagicSingleton(), Names::callWithSplat());
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
        arg.type = Types::untyped(*this, method);
        arg.flags.isRepeated = true;
    }
    method.data(*this)->resultType = Types::untyped(*this, method);
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
        arg.flags.isBlock = true;
    }
    // Synthesize <Magic>#<call-with-block>(args: *T.untyped) => T.untyped
    method = enterMethodSymbol(Loc::none(), Symbols::MagicSingleton(), Names::callWithBlock());
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
        arg.type = Types::untyped(*this, method);
        arg.flags.isRepeated = true;
    }
    method.data(*this)->resultType = Types::untyped(*this, method);
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
        arg.flags.isBlock = true;
    }
    // Synthesize <Magic>#<call-with-splat-and-block>(args: *T.untyped) => T.untyped
    method = enterMethodSymbol(Loc::none(), Symbols::MagicSingleton(), Names::callWithSplatAndBlock());
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
        arg.type = Types::untyped(*this, method);
        arg.flags.isRepeated = true;
    }
    method.data(*this)->resultType = Types::untyped(*this, method);
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
        arg.flags.isBlock = true;
    }
    // Synthesize <DeclBuilderForProcs>#<params>(args: Hash) => DeclBuilderForProcs
    method = enterMethodSymbol(Loc::none(), Symbols::DeclBuilderForProcsSingleton(), Names::params());
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
        arg.flags.isDefault = true;
        arg.type = Types::hashOfUntyped();
    }
    method.data(*this)->resultType = Types::declBuilderForProcsSingletonClass();
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
        arg.flags.isBlock = true;
    }
    // Synthesize <DeclBuilderForProcs>#<bind>(args: T.untyped) =>
    // DeclBuilderForProcs
    method = enterMethodSymbol(Loc::none(), Symbols::DeclBuilderForProcsSingleton(), Names::bind());
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
        arg.type = Types::untyped(*this, method);
    }
    method.data(*this)->resultType = Types::declBuilderForProcsSingletonClass();
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
        arg.flags.isBlock = true;
    }
    // Synthesize <DeclBuilderForProcs>#<returns>(args: T.untyped) => DeclBuilderForProcs
    method = enterMethodSymbol(Loc::none(), Symbols::DeclBuilderForProcsSingleton(), Names::returns());
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
        arg.type = Types::untyped(*this, method);
    }
    method.data(*this)->resultType = Types::declBuilderForProcsSingletonClass();
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
        arg.flags.isBlock = true;
    }
    // Synthesize <DeclBuilderForProcs>#<type_parameters>(args: T.untyped) =>
    // DeclBuilderForProcs
    method = enterMethodSymbol(Loc::none(), Symbols::DeclBuilderForProcsSingleton(), Names::typeParameters());
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::arg0());
        arg.type = Types::untyped(*this, method);
    }
    method.data(*this)->resultType = Types::declBuilderForProcsSingletonClass();
    {
        auto &arg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
        arg.flags.isBlock = true;
    }
    // Some of these are Modules
    Symbols::StubModule().data(*this)->setIsModule(true);
    Symbols::T().data(*this)->setIsModule(true);
    Symbols::StubAncestor().data(*this)->setIsModule(true);

    // Synthesize T::Utils
    id = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Utils());
    id.data(*this)->setIsModule(true);

    // Synthesize T::Utils::RuntimeProfiled
    id = enterStaticFieldSymbol(Loc::none(), id, core::Names::Constants::RuntimeProfiled());
    id.data(*this)->resultType = make_type<core::AliasType>(Symbols::untyped());

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

    // This fills in all the way up to MAX_SYNTHETIC_SYMBOLS
    ENFORCE(symbols.size() < Symbols::Proc0()._id);
    while (symbols.size() < Symbols::Proc0()._id) {
        string name = absl::StrCat("<RESERVED_", reservedCount, ">");
        synthesizeClass(enterNameConstant(name));
        reservedCount++;
    }

    for (int arity = 0; arity <= Symbols::MAX_PROC_ARITY; ++arity) {
        string name = absl::StrCat("Proc", arity);
        auto id = synthesizeClass(enterNameConstant(name), Symbols::Proc()._id);
        ENFORCE(id == Symbols::Proc(arity), "Proc creation failed for arity: {} got: {} expected: {}", arity, id._id,
                Symbols::Proc(arity)._id);
        id.data(*this)->singletonClass(*this);
    }

    ENFORCE(symbols.size() == Symbols::last_synthetic_sym()._id + 1,
            "Too many synthetic symbols? have: {} expected: {}", symbols.size(), Symbols::last_synthetic_sym()._id + 1);

    installIntrinsics();

    Symbols::top().data(*this)->resultType = Types::top();
    Symbols::bottom().data(*this)->resultType = Types::bottom();
    Symbols::NilClass().data(*this)->resultType = Types::nilClass();
    Symbols::untyped().data(*this)->resultType = Types::untypedUntracked();
    Symbols::FalseClass().data(*this)->resultType = Types::falseClass();
    Symbols::TrueClass().data(*this)->resultType = Types::trueClass();
    Symbols::Integer().data(*this)->resultType = Types::Integer();
    Symbols::String().data(*this)->resultType = Types::String();
    Symbols::Symbol().data(*this)->resultType = Types::Symbol();
    Symbols::Float().data(*this)->resultType = Types::Float();
    Symbols::Object().data(*this)->resultType = Types::Object();
    Symbols::Class().data(*this)->resultType = Types::classClass();

    // First file is used to indicate absence of a file
    files.emplace_back();
    freezeNameTable();
    freezeSymbolTable();
    freezeFileTable();
    sanityCheck();

    isInitialized = true;
}

void GlobalState::installIntrinsics() {
    for (auto &entry : intrinsicMethods) {
        auto symbol = entry.symbol;
        if (entry.singleton) {
            symbol = symbol.data(*this)->singletonClass(*this);
        }
        auto countBefore = symbolsUsed();
        SymbolRef method = enterMethodSymbol(Loc::none(), symbol, entry.method);
        method.data(*this)->intrinsic = entry.impl;
        if (countBefore != symbolsUsed()) {
            auto &blkArg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
            blkArg.flags.isBlock = true;
        }
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
    u8 allocated = (sizeof(Name) + sizeof(decltype(namesByHash)::value_type)) * names.capacity() +
                   sizeof(Symbol) * symbols.capacity();
    u8 want = 1024 * kb;
    if (allocated > want) {
        return;
    }
    u4 scale = nextPowerOfTwo(want / allocated);
    symbols.reserve(symbols.capacity() * scale);
    expandNames(scale);
    sanityCheck();

    allocated = (sizeof(Name) + sizeof(decltype(namesByHash)::value_type)) * names.capacity() +
                sizeof(Symbol) * symbols.capacity();

    trace(absl::StrCat("Reserved ", allocated / 1024, "KiB of memory. symbols=", symbols.capacity(),
                       " names=", names.capacity()));
}

constexpr decltype(GlobalState::STRINGS_PAGE_SIZE) GlobalState::STRINGS_PAGE_SIZE;

SymbolRef GlobalState::enterSymbol(Loc loc, SymbolRef owner, NameRef name, u4 flags) {
    ENFORCE(owner.exists(), "entering symbol in to non-existing owner");
    ENFORCE(name.exists(), "entering symbol with non-existing name");
    SymbolData ownerScope = owner.dataAllowingNone(*this);
    histogramInc("symbol_enter_by_name", ownerScope->members().size());

    auto &store = ownerScope->members()[name];
    if (store.exists()) {
        ENFORCE((store.data(*this)->flags & flags) == flags, "existing symbol has wrong flags");
        counterInc("symbols.hit");
        return store;
    }

    ENFORCE(!symbolTableFrozen);

    SymbolRef ret = SymbolRef(this, symbols.size());
    store = ret; // DO NOT MOVE this assignment down. emplace_back on symbol invalidates `store`
    symbols.emplace_back();
    SymbolData data = ret.dataAllowingNone(*this);
    data->name = name;
    data->flags = flags;
    data->owner = owner;
    data->addLoc(*this, loc);
    DEBUG_ONLY(
        if (data->isClass()) { categoryCounterInc("symbols", "class"); } else if (data->isMethod()) {
            categoryCounterInc("symbols", "method");
        } else if (data->isField()) { categoryCounterInc("symbols", "field"); } else if (data->isStaticField()) {
            categoryCounterInc("symbols", "static_field");
        } else if (data->isTypeArgument()) {
            categoryCounterInc("symbols", "type_argument");
        } else if (data->isTypeMember()) { categoryCounterInc("symbols", "type_member"); } else {
            Exception::notImplemented();
        });

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

SymbolRef GlobalState::enterNewMethodOverload(Loc sigLoc, SymbolRef original, core::NameRef originalName, u2 num,
                                              const vector<int> &argsToKeep) {
    NameRef name = num == 0 ? originalName : freshNameUnique(UniqueNameKind::Overload, originalName, num);
    core::Loc loc = num == 0 ? original.data(*this)->loc()
                             : sigLoc; // use original Loc for main overload so that we get right jump-to-def for it.
    auto owner = original.data(*this)->owner;
    SymbolRef res = enterMethodSymbol(loc, owner, name);
    ENFORCE(res != original);
    if (res.data(*this)->arguments().size() != original.data(*this)->arguments().size()) {
        ENFORCE(res.data(*this)->arguments().empty());
        res.data(*this)->arguments().reserve(original.data(*this)->arguments().size());
        const auto &originalArguments = original.data(*this)->arguments();
        int i = -1;
        for (auto &arg : originalArguments) {
            i += 1;
            Loc loc = arg.loc;
            if (!absl::c_linear_search(argsToKeep, i)) {
                if (arg.flags.isBlock) {
                    loc = Loc::none();
                } else {
                    continue;
                }
            }
            NameRef nm = arg.name;
            auto &newArg = enterMethodArgumentSymbol(loc, res, nm);
            newArg = arg.deepCopy();
            newArg.loc = loc;
        }
    }
    return res;
}

SymbolRef GlobalState::enterFieldSymbol(Loc loc, SymbolRef owner, NameRef name) {
    ENFORCE(owner.data(*this)->isClass(), "entering field symbol into not-a-class");
    return enterSymbol(loc, owner, name, Symbol::Flags::FIELD);
}

SymbolRef GlobalState::enterStaticFieldSymbol(Loc loc, SymbolRef owner, NameRef name) {
    ENFORCE(owner.data(*this)->isClass());
    return enterSymbol(loc, owner, name, Symbol::Flags::STATIC_FIELD);
}

ArgInfo &GlobalState::enterMethodArgumentSymbol(Loc loc, SymbolRef owner, NameRef name) {
    ENFORCE(owner.exists(), "entering symbol in to non-existing owner");
    ENFORCE(owner.data(*this)->isMethod(), "entering method argument symbol into not-a-method");
    ENFORCE(name.exists(), "entering symbol with non-existing name");
    SymbolData ownerScope = owner.dataAllowingNone(*this);

    for (auto &arg : ownerScope->arguments()) {
        if (arg.name == name) {
            return arg;
        }
    }
    auto &store = ownerScope->arguments().emplace_back();

    ENFORCE(!symbolTableFrozen);

    store.name = name;
    store.loc = loc;
    DEBUG_ONLY(categoryCounterInc("symbols", "argument"););

    wasModified_ = true;
    return store;
}

string_view GlobalState::enterString(string_view nm) {
    DEBUG_ONLY(if (isInitialized) {
        if (nm != "<" && nm != "<<" && nm != "<=" && nm != "<=>" && nm != ">" && nm != ">>" && nm != ">=") {
            ENFORCE(nm.find("<") == string::npos);
            ENFORCE(nm.find(">") == string::npos);
        }
    });
    char *from = nullptr;
    if (nm.size() > GlobalState::STRINGS_PAGE_SIZE) {
        auto &inserted = strings.emplace_back(make_unique<vector<char>>(nm.size()));
        from = inserted->data();
        if (strings.size() > 1) {
            // last page wasn't full, keep it in the end
            swap(*(strings.end() - 1), *(strings.end() - 2));
        }
    } else {
        if (stringsLastPageUsed + nm.size() > GlobalState::STRINGS_PAGE_SIZE) {
            strings.emplace_back(make_unique<vector<char>>(GlobalState::STRINGS_PAGE_SIZE));
            // printf("Wasted %i space\n", STRINGS_PAGE_SIZE - stringsLastPageUsed);
            stringsLastPageUsed = 0;
        }
        from = strings.back()->data() + stringsLastPageUsed;
    }

    counterInc("strings");
    memcpy(from, nm.data(), nm.size());
    stringsLastPageUsed += nm.size();
    return string_view(from, nm.size());
}

NameRef GlobalState::enterNameUTF8(string_view nm) {
    const auto hs = _hash(nm);
    unsigned int hashTableSize = namesByHash.size();
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probeCount = 1;

    while (namesByHash[bucketId].second != 0u) {
        auto &bucket = namesByHash[bucketId];
        if (bucket.first == hs) {
            auto nameId = bucket.second;
            auto &nm2 = names[nameId];
            if (nm2.kind == NameKind::UTF8 && nm2.raw.utf8 == nm) {
                counterInc("names.utf8.hit");
                return nm2.ref(*this);
            } else {
                counterInc("names.hash_collision.utf8");
            }
        }
        bucketId = (bucketId + probeCount) & mask;
        probeCount++;
    }
    ENFORCE(!nameTableFrozen);

    ENFORCE(probeCount != hashTableSize, "Full table?");

    if (names.size() == names.capacity()) {
        expandNames();
        hashTableSize = namesByHash.size();
        mask = hashTableSize - 1;
        bucketId = hs & mask; // look for place in the new size
        probeCount = 1;
        while (namesByHash[bucketId].second != 0) {
            bucketId = (bucketId + probeCount) & mask;
            probeCount++;
        }
    }

    auto idx = names.size();
    auto &bucket = namesByHash[bucketId];
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
    ENFORCE(original.exists(), "making a constant name over non-existing name");
    ENFORCE(original.data(*this)->kind == UTF8 ||
                (original.data(*this)->kind == UNIQUE &&
                 original.data(*this)->unique.uniqueNameKind == UniqueNameKind::ResolverMissingClass),
            "making a constant name over wrong name kind");

    const auto hs = _hash_mix_constant(CONSTANT, original.id());
    unsigned int hashTableSize = namesByHash.size();
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probeCount = 1;

    while (namesByHash[bucketId].second != 0 && probeCount < hashTableSize) {
        auto &bucket = namesByHash[bucketId];
        if (bucket.first == hs) {
            auto &nm2 = names[bucket.second];
            if (nm2.kind == CONSTANT && nm2.cnst.original == original) {
                counterInc("names.constant.hit");
                return nm2.ref(*this);
            } else {
                counterInc("names.hash_collision.constant");
            }
        }
        bucketId = (bucketId + probeCount) & mask;
        probeCount++;
    }
    if (probeCount == hashTableSize) {
        Exception::raise("Full table?");
    }
    ENFORCE(!nameTableFrozen);

    if (names.size() == names.capacity()) {
        expandNames();
        hashTableSize = namesByHash.size();
        mask = hashTableSize - 1;

        bucketId = hs & mask; // look for place in the new size
        probeCount = 1;
        while (namesByHash[bucketId].second != 0) {
            bucketId = (bucketId + probeCount) & mask;
            probeCount++;
        }
    }

    auto &bucket = namesByHash[bucketId];
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
    vector<pair<unsigned int, unsigned int>> new_namesByHash(namesByHash.capacity() * growBy);
    moveNames(namesByHash.data(), new_namesByHash.data(), namesByHash.size(), new_namesByHash.capacity());
    namesByHash.swap(new_namesByHash);
}

NameRef GlobalState::getNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num) const {
    ENFORCE(num > 0, "num == 0, name overflow");
    const auto hs = _hash_mix_unique((u2)uniqueNameKind, UNIQUE, num, original.id());
    unsigned int hashTableSize = namesByHash.size();
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probeCount = 1;

    while (namesByHash[bucketId].second != 0 && probeCount < hashTableSize) {
        auto &bucket = namesByHash[bucketId];
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
        bucketId = (bucketId + probeCount) & mask;
        probeCount++;
    }
    Exception::raise("should never happen");
}

NameRef GlobalState::freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num) {
    ENFORCE(num > 0, "num == 0, name overflow");
    const auto hs = _hash_mix_unique((u2)uniqueNameKind, UNIQUE, num, original.id());
    unsigned int hashTableSize = namesByHash.size();
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probeCount = 1;

    while (namesByHash[bucketId].second != 0 && probeCount < hashTableSize) {
        auto &bucket = namesByHash[bucketId];
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
        bucketId = (bucketId + probeCount) & mask;
        probeCount++;
    }
    if (probeCount == hashTableSize) {
        Exception::raise("Full table?");
    }
    ENFORCE(!nameTableFrozen);

    if (names.size() == names.capacity()) {
        expandNames();
        hashTableSize = namesByHash.size();
        mask = hashTableSize - 1;

        bucketId = hs & mask; // look for place in the new size
        probeCount = 1;
        while (namesByHash[bucketId].second != 0) {
            bucketId = (bucketId + probeCount) & mask;
            probeCount++;
        }
    }

    auto &bucket = namesByHash[bucketId];
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

FileRef GlobalState::enterNewFileAt(const shared_ptr<File> &file, FileRef id) {
    ENFORCE(!fileTableFrozen);
    ENFORCE(id.id() < this->files.size());
    ENFORCE(this->files[id.id()]->sourceType == File::Type::NotYetRead);
    ENFORCE(this->files[id.id()]->path() == file->path());

    // was a tombstone before.
    this->files[id.id()] = file;
    FileRef result(id);
    return result;
}

FileRef GlobalState::reserveFileRef(string path) {
    return GlobalState::enterFile(make_shared<File>(move(path), "", File::Type::NotYetRead));
}

void GlobalState::mangleRenameSymbol(SymbolRef what, NameRef origName) {
    auto whatData = what.data(*this);
    auto owner = whatData->owner;
    auto ownerData = owner.data(*this);
    auto &ownerMembers = ownerData->members();
    auto fnd = ownerMembers.find(origName);
    ENFORCE(fnd != ownerMembers.end());
    ENFORCE(fnd->second == what);
    ENFORCE(whatData->name == origName);
    u2 collisionCount = 1;
    NameRef name;
    do {
        name = freshNameUnique(UniqueNameKind::MangleRename, origName, collisionCount++);
    } while (ownerData->findMember(*this, name).exists());
    ownerMembers.erase(fnd);
    ownerMembers[name] = what;
    whatData->name = name;
    if (whatData->isClass()) {
        auto singleton = whatData->lookupSingletonClass(*this);
        if (singleton.exists()) {
            mangleRenameSymbol(singleton, singleton.data(*this)->name);
        }
    }
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

string GlobalState::toStringWithOptions(bool showFull, bool showRaw) const {
    return Symbols::root().data(*this)->toStringWithOptions(*this, 0, showFull, showRaw);
}

void GlobalState::sanityCheck() const {
    if (!debug_mode) {
        return;
    }
    if (fuzz_mode) {
        // it's very slow to check this and it didn't find bugs
        return;
    }

    Timer timeit(tracer(), "GlobalState::sanityCheck");
    ENFORCE(!names.empty(), "empty name table size");
    ENFORCE(!strings.empty(), "empty string table size");
    ENFORCE(!namesByHash.empty(), "empty name hash table size");
    ENFORCE((namesByHash.size() & (namesByHash.size() - 1)) == 0, "name hash table size is not a power of two");
    ENFORCE(names.capacity() * 2 == namesByHash.capacity(),
            "name table and hash name table sizes out of sync names.capacity={} namesByHash.capacity={}",
            names.capacity(), namesByHash.capacity());
    ENFORCE(namesByHash.size() == namesByHash.capacity(), "hash name table not at full capacity");
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
    for (auto &ent : namesByHash) {
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
    Timer timeit(tracer(), "GlobalState::deepCopy", this->creation);
    this->sanityCheck();
    auto result = make_unique<GlobalState>(this->errorQueue);

    result->silenceErrors = this->silenceErrors;
    result->autocorrect = this->autocorrect;
    result->suggestRuntimeProfiledType = this->suggestRuntimeProfiledType;
    result->isInitialized = this->isInitialized;
    result->runningUnderAutogen = this->runningUnderAutogen;

    if (keepId) {
        result->globalStateId = this->globalStateId;
    }
    result->deepCloneHistory = this->deepCloneHistory;
    result->deepCloneHistory.emplace_back(DeepCloneHistoryEntry{this->globalStateId, namesUsed()});

    result->strings = this->strings;
    result->stringsLastPageUsed = STRINGS_PAGE_SIZE;
    result->files = this->files;
    result->fileRefByPath = this->fileRefByPath;
    result->lspQuery = this->lspQuery;
    result->lspTypecheckCount = this->lspTypecheckCount;
    result->errorUrlBase = this->errorUrlBase;
    result->suppressedErrorClasses = this->suppressedErrorClasses;
    result->onlyErrorClasses = this->onlyErrorClasses;
    result->dslPlugins = this->dslPlugins;
    result->dslRubyExtraArgs = this->dslRubyExtraArgs;
    result->names.reserve(this->names.capacity());
    if (keepId) {
        result->names.resize(this->names.size());
        ::memcpy(result->names.data(), this->names.data(), this->names.size() * sizeof(Name));
    } else {
        for (auto &nm : this->names) {
            result->names.emplace_back(nm.deepCopy(*result));
        }
    }

    result->namesByHash.reserve(this->namesByHash.size());
    result->namesByHash = this->namesByHash;

    result->symbols.reserve(this->symbols.size());
    for (auto &sym : this->symbols) {
        result->symbols.emplace_back(sym.deepCopy(*result, keepId));
    }
    result->pathPrefix = this->pathPrefix;
    result->sanityCheck();
    {
        Timer timeit2(tracer(), "GlobalState::deepCopyOut");
        result->creation = timeit2.getFlowEdge();
    }
    return result;
}

string_view GlobalState::getPrintablePath(string_view path) const {
    // Only strip the path prefix if the path has it.
    if (path.substr(0, pathPrefix.length()) == pathPrefix) {
        return path.substr(pathPrefix.length());
    }
    return path;
}

int GlobalState::totalErrors() const {
    return errorQueue->nonSilencedErrorCount.load();
}

void GlobalState::_error(unique_ptr<Error> error) const {
    if (error->isCritical()) {
        errorQueue->hadCritical = true;
    }
    auto loc = error->loc;
    if (loc.file().exists() && error->what != errors::Infer::SuggestTyped &&
        error->what != core::errors::Resolver::SigInFileWithoutSigil) {
        loc.file().data(*this).minErrorLevel_ = min(loc.file().data(*this).minErrorLevel_, error->what.minLevel);
    }

    errorQueue->pushError(*this, move(error));
}

bool GlobalState::hadCriticalError() const {
    return errorQueue->hadCritical;
}

ErrorBuilder GlobalState::beginError(Loc loc, ErrorClass what) const {
    bool reportForFile = shouldReportErrorOn(loc, what);
    if (reportForFile) {
        prodHistogramAdd("error", what.code, 1);
    }
    if (what == errors::Internal::InternalError) {
        Exception::failInFuzzer();
    }
    bool report = (what == errors::Internal::InternalError) || (what == errors::Internal::FileNotFound) ||
                  (reportForFile && !this->silenceErrors);
    return ErrorBuilder(*this, report, loc, what);
}

void GlobalState::suppressErrorClass(int code) {
    ENFORCE(onlyErrorClasses.empty());
    suppressedErrorClasses.insert(code);
}

void GlobalState::onlyShowErrorClass(int code) {
    ENFORCE(suppressedErrorClasses.empty());
    onlyErrorClasses.insert(code);
}

void GlobalState::addDslPlugin(string_view method, string_view command) {
    auto ref = enterNameUTF8(method);
    auto [it, inserted] = dslPlugins.try_emplace(ref, command);
    if (!inserted) {
        if (auto e = beginError(Loc::none(), errors::Internal::InternalError)) {
            e.setHeader("Duplicate plugin trigger \"{}\". Previous definition: \"{}\"", method, it->second);
        }
    }
}

optional<string_view> GlobalState::findDslPlugin(NameRef method) const {
    const auto it = dslPlugins.find(method);
    if (it != dslPlugins.end()) {
        return it->second;
    }
    return nullopt;
}

bool GlobalState::hasAnyDslPlugin() const {
    return !dslPlugins.empty();
}

bool GlobalState::shouldReportErrorOn(Loc loc, ErrorClass what) const {
    StrictLevel level = StrictLevel::Strong;
    if (loc.file().exists()) {
        level = loc.file().data(*this).strictLevel;
    }
    if (what.code == errors::Internal::InternalError.code) {
        return true;
    }
    if (suppressedErrorClasses.count(what.code) != 0) {
        return false;
    }
    if (!onlyErrorClasses.empty() && onlyErrorClasses.count(what.code) == 0) {
        return false;
    }

    if (level >= StrictLevel::Max) {
        // Custom rules
        if (level == StrictLevel::Autogenerated) {
            level = StrictLevel::True;
            if (what == errors::Resolver::StubConstant || what == errors::Infer::UntypedMethod) {
                return false;
            }
        } else if (level == StrictLevel::Stdlib) {
            level = StrictLevel::True;
            if (what == errors::Resolver::OverloadNotAllowed || what == errors::Resolver::VariantTypeMemberInClass) {
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

void GlobalState::trace(string_view msg) const {
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
    ENFORCE(whatFile.dataAllowingUnsafe(*inWhat).path() == withWhat->path());
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

u4 patchHash(u4 hash) {
    if (hash == GlobalStateHash::HASH_STATE_NOT_COMPUTED) {
        hash = GlobalStateHash::HASH_STATE_NOT_COMPUTED_COLLISION_AVOID;
    } else if (hash == GlobalStateHash::HASH_STATE_INVALID) {
        hash = GlobalStateHash::HASH_STATE_INVALID_COLLISION_AVOID;
    }
    return hash;
}

unique_ptr<GlobalStateHash> GlobalState::hash() const {
    constexpr bool DEBUG_HASHING_TAIL = false;
    u4 hierarchyHash = 0;
    UnorderedMap<NameHash, u4> methodHashes;
    int counter = 0;
    for (const auto &sym : this->symbols) {
        if (!sym.ignoreInHashing(*this)) {
            if (sym.isMethod()) {
                auto &target = methodHashes[NameHash(*this, sym.name.data(*this))];
                target = mix(target, sym.hash(*this));
                hierarchyHash = mix(hierarchyHash, sym.methodShapeHash(*this));
            } else {
                hierarchyHash = mix(hierarchyHash, sym.hash(*this));
            }
        }
        counter++;
        if (DEBUG_HASHING_TAIL && counter > symbolsUsed() - 15) {
            errorQueue->logger.info("Hashing symbols: {}, {}", hierarchyHash, sym.name.show(*this));
        }
    }
    unique_ptr<GlobalStateHash> result = make_unique<GlobalStateHash>();
    for (const auto &e : methodHashes) {
        result->methodHashes[e.first] = patchHash(e.second);
    }
    result->hierarchyHash = patchHash(hierarchyHash);
    return result;
}

vector<shared_ptr<File>> GlobalState::getFiles() const {
    return files;
}

SymbolRef GlobalState::staticInitForClass(SymbolRef klass, Loc loc) {
    auto prevCount = symbolsUsed();
    auto sym = enterMethodSymbol(loc, klass.data(*this)->singletonClass(*this), core::Names::staticInit());
    if (prevCount != symbolsUsed()) {
        auto blkLoc = core::Loc::none(loc.file());
        auto &blkSym = enterMethodArgumentSymbol(blkLoc, sym, core::Names::blkArg());
        blkSym.flags.isBlock = true;
    }
    return sym;
}

SymbolRef GlobalState::lookupStaticInitForClass(SymbolRef klass) const {
    auto &classData = klass.data(*this);
    ENFORCE(classData->isClass());
    auto ref = classData->lookupSingletonClass(*this).data(*this)->findMember(*this, core::Names::staticInit());
    ENFORCE(ref.exists(), "looking up non-existent <static-init> for {}", klass.toString(*this));
    return ref;
}

SymbolRef GlobalState::staticInitForFile(Loc loc) {
    auto nm = freshNameUnique(core::UniqueNameKind::Namer, core::Names::staticInit(), loc.file().id());
    auto prevCount = this->symbolsUsed();
    auto sym = enterMethodSymbol(loc, core::Symbols::rootSingleton(), nm);
    auto blkLoc = core::Loc::none(loc.file());
    if (prevCount != this->symbolsUsed()) {
        auto &blkSym = this->enterMethodArgumentSymbol(blkLoc, sym, core::Names::blkArg());
        blkSym.flags.isBlock = true;
    }
    return sym;
}

SymbolRef GlobalState::lookupStaticInitForFile(Loc loc) const {
    auto nm = getNameUnique(core::UniqueNameKind::Namer, core::Names::staticInit(), loc.file().id());
    auto ref = core::Symbols::rootSingleton().data(*this)->findMember(*this, nm);
    ENFORCE(ref.exists(), "looking up non-existent <static-init> for {}", loc.toString(*this));
    return ref;
}

spdlog::logger &GlobalState::tracer() const {
    return errorQueue->tracer;
}
} // namespace sorbet::core
