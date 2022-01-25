#include "GlobalState.h"

#include "common/Timer.h"
#include "common/sort.h"
#include "core/Error.h"
#include "core/NameHash.h"
#include "core/Names.h"
#include "core/Names_gen.h"
#include "core/NullFlusher.h"
#include "core/Types.h"
#include "core/Unfreeze.h"
#include "core/errors/errors.h"
#include "core/hashing/hashing.h"
#include "core/lsp/Task.h"
#include "core/lsp/TypecheckEpochManager.h"
#include <utility>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "core/ErrorQueue.h"
#include "core/errors/infer.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"

template class std::vector<std::pair<unsigned int, unsigned int>>;
template class std::shared_ptr<sorbet::core::GlobalState>;
template class std::unique_ptr<sorbet::core::GlobalState>;

using namespace std;

namespace sorbet::core {

namespace {
// Hash functions used to determine position in namesByHash.

inline unsigned int hashMixUnique(UniqueNameKind unk, unsigned int num, unsigned int rawId) {
    return mix(mix(num, static_cast<uint32_t>(unk)), rawId) * HASH_MULT2 + static_cast<uint32_t>(NameKind::UNIQUE);
}

inline unsigned int hashMixConstant(unsigned int id) {
    return id * HASH_MULT2 + static_cast<uint32_t>(NameKind::CONSTANT);
}

inline unsigned int hashNameRef(const GlobalState &gs, NameRef nref) {
    switch (nref.kind()) {
        case NameKind::UTF8:
            return _hash(nref.shortName(gs));
        case NameKind::CONSTANT:
            return hashMixConstant(nref.dataCnst(gs)->original.rawId());
        case NameKind::UNIQUE: {
            auto data = nref.dataUnique(gs);
            return hashMixUnique(data->uniqueNameKind, data->num, data->original.rawId());
        }
    }
}

struct MethodBuilder {
    GlobalState &gs;
    MethodRef method;

    MethodBuilder(GlobalState &gs, MethodRef m) : gs(gs), method(m) {}

    MethodBuilder &defaultArg(NameRef name) {
        auto &arg = gs.enterMethodArgumentSymbol(Loc::none(), method, name);
        arg.flags.isDefault = true;
        return *this;
    }

    MethodBuilder &typedArg(NameRef name, TypePtr &&type) {
        auto &arg = gs.enterMethodArgumentSymbol(Loc::none(), method, name);
        arg.type = std::move(type);
        return *this;
    }

    MethodBuilder &arg(NameRef name) {
        gs.enterMethodArgumentSymbol(Loc::none(), method, name);
        return *this;
    }

    MethodBuilder &untypedArg(NameRef name) {
        auto &arg = gs.enterMethodArgumentSymbol(Loc::none(), method, name);
        arg.type = Types::untyped(gs, method);
        return *this;
    }

    MethodBuilder &repeatedArg(NameRef name) {
        auto &arg = gs.enterMethodArgumentSymbol(Loc::none(), method, name);
        arg.flags.isRepeated = true;
        return *this;
    }

    MethodBuilder &repeatedTypedArg(NameRef name, TypePtr &&type) {
        auto &arg = gs.enterMethodArgumentSymbol(Loc::none(), method, name);
        arg.flags.isRepeated = true;
        arg.type = std::move(type);
        return *this;
    }

    MethodBuilder &repeatedUntypedArg(NameRef name) {
        auto &arg = gs.enterMethodArgumentSymbol(Loc::none(), method, name);
        arg.flags.isRepeated = true;
        arg.type = Types::untyped(gs, method);
        return *this;
    }

    MethodBuilder &kwsplatArg(NameRef name) {
        auto &arg = gs.enterMethodArgumentSymbol(Loc::none(), method, name);
        arg.flags.isKeyword = true;
        arg.flags.isRepeated = true;
        arg.type = Types::untyped(gs, method);
        return *this;
    }

    MethodRef build() {
        auto &arg = gs.enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
        arg.flags.isBlock = true;
        return method;
    }

    MethodRef buildWithResult(TypePtr &&type) {
        method.data(gs)->resultType = type;
        return build();
    }

    MethodRef buildWithResultUntyped() {
        return buildWithResult(Types::untyped(gs, method));
    }
};

MethodBuilder enterMethod(GlobalState &gs, ClassOrModuleRef klass, NameRef name) {
    return MethodBuilder{gs, gs.enterMethodSymbol(Loc::none(), klass, name)};
}

} // namespace

ClassOrModuleRef GlobalState::synthesizeClass(NameRef nameId, uint32_t superclass, bool isModule) {
    // This can't use enterClass since there is a chicken and egg problem.
    // These will be added to Symbols::root().members later.
    ClassOrModuleRef symRef = ClassOrModuleRef(*this, classAndModules.size());
    classAndModules.emplace_back();
    SymbolData data = symRef.dataAllowingNone(*this); // allowing noSymbol is needed because this enters noSymbol.
    data->name = nameId;
    data->owner = Symbols::root();
    data->flags = 0;
    data->setClassOrModule();
    data->setIsModule(isModule);
    data->setSuperClass(ClassOrModuleRef(*this, superclass));

    if (symRef.id() > Symbols::root().id()) {
        Symbols::root().data(*this)->members()[nameId] = symRef;
    }
    return symRef;
}

atomic<int> globalStateIdCounter(1);
const int Symbols::MAX_PROC_ARITY;

GlobalState::GlobalState(shared_ptr<ErrorQueue> errorQueue)
    : GlobalState(move(errorQueue), make_shared<lsp::TypecheckEpochManager>()) {}

GlobalState::GlobalState(shared_ptr<ErrorQueue> errorQueue, shared_ptr<lsp::TypecheckEpochManager> epochManager)
    : GlobalState(move(errorQueue), move(epochManager), globalStateIdCounter.fetch_add(1)) {}

GlobalState::GlobalState(shared_ptr<ErrorQueue> errorQueue, shared_ptr<lsp::TypecheckEpochManager> epochManager,
                         int globalStateId)
    : globalStateId(globalStateId), errorQueue(std::move(errorQueue)), lspQuery(lsp::Query::noQuery()),
      epochManager(move(epochManager)) {
    // Reserve memory in internal vectors for the contents of payload.
    utf8Names.reserve(PAYLOAD_MAX_UTF8_NAME_COUNT);
    constantNames.reserve(PAYLOAD_MAX_CONSTANT_NAME_COUNT);
    uniqueNames.reserve(PAYLOAD_MAX_UNIQUE_NAME_COUNT);
    classAndModules.reserve(PAYLOAD_MAX_CLASS_AND_MODULE_COUNT);
    methods.reserve(PAYLOAD_MAX_METHOD_COUNT);
    fields.reserve(PAYLOAD_MAX_FIELD_COUNT);
    typeArguments.reserve(PAYLOAD_MAX_TYPE_ARGUMENT_COUNT);
    typeMembers.reserve(PAYLOAD_MAX_TYPE_MEMBER_COUNT);

    int namesByHashSize = nextPowerOfTwo(
        2 * (PAYLOAD_MAX_UTF8_NAME_COUNT + PAYLOAD_MAX_CONSTANT_NAME_COUNT + PAYLOAD_MAX_UNIQUE_NAME_COUNT));
    namesByHash.resize(namesByHashSize);
    ENFORCE((namesByHashSize & (namesByHashSize - 1)) == 0, "namesByHashSize is not a power of 2");
}

unique_ptr<GlobalState> GlobalState::makeEmptyGlobalStateForHashing(spdlog::logger &logger) {
    // Note: Private constructor.
    unique_ptr<GlobalState> rv(
        new GlobalState(make_shared<core::ErrorQueue>(logger, logger, make_shared<core::NullFlusher>()),
                        make_shared<lsp::TypecheckEpochManager>(), -1));
    rv->initEmpty();
    rv->silenceErrors = true;
    return rv;
}

void GlobalState::initEmpty() {
    UnfreezeFileTable fileTableAccess(*this);
    UnfreezeNameTable nameTableAccess(*this);
    UnfreezeSymbolTable symTableAccess(*this);
    Names::registerNames(*this);

    ClassOrModuleRef klass;
    klass = synthesizeClass(core::Names::Constants::NoSymbol(), 0);
    ENFORCE(klass == Symbols::noClassOrModule());
    MethodRef method = enterMethodSymbol(Loc::none(), Symbols::noClassOrModule(), Names::noMethod());
    ENFORCE(method == Symbols::noMethod());
    FieldRef field = enterFieldSymbol(Loc::none(), Symbols::noClassOrModule(), Names::noFieldOrStaticField());
    ENFORCE(field == Symbols::noField());
    TypeArgumentRef typeArgument =
        enterTypeArgument(Loc::none(), Symbols::noMethod(), Names::Constants::NoTypeArgument(), Variance::CoVariant);
    ENFORCE(typeArgument == Symbols::noTypeArgument());
    TypeMemberRef typeMember =
        enterTypeMember(Loc::none(), Symbols::noClassOrModule(), Names::Constants::NoTypeMember(), Variance::CoVariant);
    ENFORCE(typeMember == Symbols::noTypeMember());

    klass = synthesizeClass(core::Names::Constants::Top(), 0);
    ENFORCE(klass == Symbols::top());
    klass = synthesizeClass(core::Names::Constants::Bottom(), 0);
    ENFORCE(klass == Symbols::bottom());
    klass = synthesizeClass(core::Names::Constants::Root(), 0);
    ENFORCE(klass == Symbols::root());

    klass = core::Symbols::root().data(*this)->singletonClass(*this);
    ENFORCE(klass == Symbols::rootSingleton());
    klass = synthesizeClass(core::Names::Constants::Todo(), 0);
    ENFORCE(klass == Symbols::todo());
    klass = synthesizeClass(core::Names::Constants::Object(), Symbols::BasicObject().id());
    ENFORCE(klass == Symbols::Object());
    klass = synthesizeClass(core::Names::Constants::Integer());
    ENFORCE(klass == Symbols::Integer());
    klass = synthesizeClass(core::Names::Constants::Float());
    ENFORCE(klass == Symbols::Float());
    klass = synthesizeClass(core::Names::Constants::String());
    ENFORCE(klass == Symbols::String());
    klass = synthesizeClass(core::Names::Constants::Symbol());
    ENFORCE(klass == Symbols::Symbol());
    klass = synthesizeClass(core::Names::Constants::Array());
    ENFORCE(klass == Symbols::Array());
    klass = synthesizeClass(core::Names::Constants::Hash());
    ENFORCE(klass == Symbols::Hash());
    klass = synthesizeClass(core::Names::Constants::TrueClass());
    ENFORCE(klass == Symbols::TrueClass());
    klass = synthesizeClass(core::Names::Constants::FalseClass());
    ENFORCE(klass == Symbols::FalseClass());
    klass = synthesizeClass(core::Names::Constants::NilClass());
    ENFORCE(klass == Symbols::NilClass());
    klass = synthesizeClass(core::Names::Constants::Untyped(), 0);
    ENFORCE(klass == Symbols::untyped());
    klass = synthesizeClass(core::Names::Constants::Opus(), 0, true);
    ENFORCE(klass == Symbols::Opus());
    klass = synthesizeClass(core::Names::Constants::T(), Symbols::todo().id(), true);
    ENFORCE(klass == Symbols::T());
    klass = synthesizeClass(core::Names::Constants::Class(), 0);
    ENFORCE(klass == Symbols::Class());
    klass = synthesizeClass(core::Names::Constants::BasicObject(), 0);
    ENFORCE(klass == Symbols::BasicObject());
    klass = synthesizeClass(core::Names::Constants::Kernel(), 0, true);
    ENFORCE(klass == Symbols::Kernel());
    klass = synthesizeClass(core::Names::Constants::Range());
    ENFORCE(klass == Symbols::Range());
    klass = synthesizeClass(core::Names::Constants::Regexp());
    ENFORCE(klass == Symbols::Regexp());
    klass = synthesizeClass(core::Names::Constants::Magic());
    ENFORCE(klass == Symbols::Magic());
    klass = Symbols::Magic().data(*this)->singletonClass(*this);
    ENFORCE(klass == Symbols::MagicSingleton());
    klass = synthesizeClass(core::Names::Constants::Module());
    ENFORCE(klass == Symbols::Module());
    klass = synthesizeClass(core::Names::Constants::StandardError());
    ENFORCE(klass == Symbols::StandardError());
    klass = synthesizeClass(core::Names::Constants::Complex());
    ENFORCE(klass == Symbols::Complex());
    klass = synthesizeClass(core::Names::Constants::Rational());
    ENFORCE(klass == Symbols::Rational());
    klass = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Array());
    ENFORCE(klass == Symbols::T_Array());
    klass = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Hash());
    ENFORCE(klass == Symbols::T_Hash());
    klass = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Proc());
    ENFORCE(klass == Symbols::T_Proc());
    klass = synthesizeClass(core::Names::Constants::Proc());
    ENFORCE(klass == Symbols::Proc());
    klass = synthesizeClass(core::Names::Constants::Enumerable(), 0, true);
    ENFORCE(klass == Symbols::Enumerable());
    klass = synthesizeClass(core::Names::Constants::Set());
    ENFORCE(klass == Symbols::Set());
    klass = synthesizeClass(core::Names::Constants::Struct());
    ENFORCE(klass == Symbols::Struct());
    klass = synthesizeClass(core::Names::Constants::File());
    ENFORCE(klass == Symbols::File());
    klass = synthesizeClass(core::Names::Constants::Sorbet());
    ENFORCE(klass == Symbols::Sorbet());
    klass = enterClassSymbol(Loc::none(), Symbols::Sorbet(), core::Names::Constants::Private());
    ENFORCE(klass == Symbols::Sorbet_Private());
    klass = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private(), core::Names::Constants::Static());
    ENFORCE(klass == Symbols::Sorbet_Private_Static());
    klass = Symbols::Sorbet_Private_Static().data(*this)->singletonClass(*this);
    ENFORCE(klass == Symbols::Sorbet_Private_StaticSingleton());
    klass = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::StubModule());
    ENFORCE(klass == Symbols::StubModule());
    klass = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::StubMixin());
    ENFORCE(klass == Symbols::StubMixin());
    klass = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::PlaceholderMixin());
    ENFORCE(klass == Symbols::PlaceholderMixin());
    klass = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::StubSuperClass());
    ENFORCE(klass == Symbols::StubSuperClass());
    klass = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Enumerable());
    ENFORCE(klass == Symbols::T_Enumerable());
    klass = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Range());
    ENFORCE(klass == Symbols::T_Range());
    klass = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Set());
    ENFORCE(klass == Symbols::T_Set());
    klass = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::Void());
    klass.data(*this)->setIsModule(false);
    ENFORCE(klass == Symbols::void_());
    klass = synthesizeClass(core::Names::Constants::TypeAlias(), 0);
    ENFORCE(klass == Symbols::typeAliasTemp());
    klass = enterClassSymbol(Loc::none(), Symbols::T(), Names::Constants::Configuration());
    ENFORCE(klass == Symbols::T_Configuration());
    klass = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Generic());
    ENFORCE(klass == Symbols::T_Generic());
    klass = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::Tuple());
    ENFORCE(klass == Symbols::Tuple());
    klass = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::Shape());
    ENFORCE(klass == Symbols::Shape());
    klass = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::Subclasses());
    ENFORCE(klass == Symbols::Subclasses());
    klass = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(),
                             core::Names::Constants::ImplicitModuleSuperclass());
    ENFORCE(klass == Symbols::Sorbet_Private_Static_ImplicitModuleSuperClass());
    klass =
        enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::ReturnTypeInference());
    ENFORCE(klass == Symbols::Sorbet_Private_Static_ReturnTypeInference());
    method =
        enterMethod(*this, Symbols::Sorbet_Private_Static(), core::Names::guessedTypeTypeParameterHolder()).build();
    ENFORCE(method == Symbols::Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder());
    typeArgument = enterTypeArgument(
        Loc::none(), Symbols::Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder(),
        freshNameUnique(core::UniqueNameKind::TypeVarName, core::Names::Constants::InferredReturnType(), 1),
        core::Variance::ContraVariant);
    typeArgument.data(*this)->resultType = make_type<core::TypeVar>(typeArgument);
    ENFORCE(
        typeArgument ==
        Symbols::Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_contravariant());
    typeArgument = enterTypeArgument(
        Loc::none(), Symbols::Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder(),
        freshNameUnique(core::UniqueNameKind::TypeVarName, core::Names::Constants::InferredArgumentType(), 1),
        core::Variance::CoVariant);
    typeArgument.data(*this)->resultType = make_type<core::TypeVar>(typeArgument);
    ENFORCE(typeArgument ==
            Symbols::Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_covariant());
    klass = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Sig());
    ENFORCE(klass == Symbols::T_Sig());

    // A magic non user-creatable class with methods to keep state between passes
    field = enterFieldSymbol(Loc::none(), Symbols::Magic(), core::Names::Constants::UndeclaredFieldStub());
    ENFORCE(field == Symbols::Magic_undeclaredFieldStub());

    // Sorbet::Private::Static#badAliasMethodStub(*arg0 : T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::Sorbet_Private_Static(), core::Names::badAliasMethodStub())
                 .repeatedUntypedArg(Names::arg0())
                 .build();
    ENFORCE(method == Symbols::Sorbet_Private_Static_badAliasMethodStub());

    // T::Helpers
    klass = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Helpers());
    ENFORCE(klass == Symbols::T_Helpers());

    // SigBuilder magic class
    klass = synthesizeClass(core::Names::Constants::DeclBuilderForProcs());
    ENFORCE(klass == Symbols::DeclBuilderForProcs());
    klass = Symbols::DeclBuilderForProcs().data(*this)->singletonClass(*this);
    ENFORCE(klass == Symbols::DeclBuilderForProcsSingleton());

    // Ruby 2.5 Hack
    klass = synthesizeClass(core::Names::Constants::Net(), 0, true);
    ENFORCE(klass == Symbols::Net());
    klass = enterClassSymbol(Loc::none(), Symbols::Net(), core::Names::Constants::IMAP());
    Symbols::Net_IMAP().data(*this)->setIsModule(false);
    ENFORCE(klass == Symbols::Net_IMAP());
    klass = enterClassSymbol(Loc::none(), Symbols::Net(), core::Names::Constants::Protocol());
    ENFORCE(klass == Symbols::Net_Protocol());
    Symbols::Net_Protocol().data(*this)->setIsModule(false);

    klass = enterClassSymbol(Loc::none(), Symbols::T_Sig(), core::Names::Constants::WithoutRuntime());
    ENFORCE(klass == Symbols::T_Sig_WithoutRuntime());

    klass = synthesizeClass(core::Names::Constants::Enumerator());
    ENFORCE(klass == Symbols::Enumerator());
    klass = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Enumerator());
    ENFORCE(klass == Symbols::T_Enumerator());

    klass = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Struct());
    ENFORCE(klass == Symbols::T_Struct());

    klass = synthesizeClass(core::Names::Constants::Singleton(), 0, true);
    ENFORCE(klass == Symbols::Singleton());

    klass = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Enum());
    klass.data(*this)->setIsModule(false);
    ENFORCE(klass == Symbols::T_Enum());

    // T::Sig#sig
    method = enterMethod(*this, Symbols::T_Sig(), Names::sig()).defaultArg(Names::arg0()).build();
    ENFORCE(method == Symbols::sig());

    // Enumerable::Lazy
    klass = enterClassSymbol(Loc::none(), Symbols::Enumerator(), core::Names::Constants::Lazy());
    ENFORCE(klass == Symbols::Enumerator_Lazy());

    klass = enterClassSymbol(Loc::none(), Symbols::T(), Names::Constants::Private());
    ENFORCE(klass == Symbols::T_Private());
    klass = enterClassSymbol(Loc::none(), Symbols::T_Private(), Names::Constants::Types());
    ENFORCE(klass == Symbols::T_Private_Types());
    klass = enterClassSymbol(Loc::none(), Symbols::T_Private_Types(), Names::Constants::Void());
    klass.data(*this)->setIsModule(false);
    ENFORCE(klass == Symbols::T_Private_Types_Void());
    klass = enterClassSymbol(Loc::none(), Symbols::T_Private_Types_Void(), Names::Constants::VOID());
    ENFORCE(klass == Symbols::T_Private_Types_Void_VOID());
    klass = klass.data(*this)->singletonClass(*this);
    ENFORCE(klass == Symbols::T_Private_Types_Void_VOIDSingleton());

    // T.class_of(T::Sig::WithoutRuntime)
    klass = Symbols::T_Sig_WithoutRuntime().data(*this)->singletonClass(*this);
    ENFORCE(klass == Symbols::T_Sig_WithoutRuntimeSingleton());

    // T::Sig::WithoutRuntime.sig
    method =
        enterMethod(*this, Symbols::T_Sig_WithoutRuntimeSingleton(), Names::sig()).defaultArg(Names::arg0()).build();
    ENFORCE(method == Symbols::sigWithoutRuntime());

    klass = enterClassSymbol(Loc::none(), Symbols::T(), Names::Constants::NonForcingConstants());
    ENFORCE(klass == Symbols::T_NonForcingConstants());

    method = enterMethod(*this, Symbols::Sorbet_Private_StaticSingleton(), Names::sig())
                 .arg(Names::arg0())
                 .defaultArg(Names::arg1())
                 .build();
    ENFORCE(method == Symbols::SorbetPrivateStaticSingleton_sig());

    klass = enterClassSymbol(Loc::none(), Symbols::root(), Names::Constants::PackageRegistry());
    ENFORCE(klass == Symbols::PackageRegistry());
    klass = enterClassSymbol(Loc::none(), Symbols::root(), Names::Constants::PackageTests());
    ENFORCE(klass == Symbols::PackageTests());

    // PackageSpec is a class that can be subclassed.
    klass = enterClassSymbol(Loc::none(), Symbols::root(), Names::Constants::PackageSpec());
    klass.data(*this)->setIsModule(false);
    ENFORCE(klass == Symbols::PackageSpec());

    klass = klass.data(*this)->singletonClass(*this);
    ENFORCE(klass == Symbols::PackageSpecSingleton());

    method = enterMethod(*this, Symbols::PackageSpecSingleton(), Names::import())
                 .typedArg(Names::arg0(), make_type<ClassType>(Symbols::PackageSpecSingleton()))
                 .build();
    ENFORCE(method == Symbols::PackageSpec_import());

    method = enterMethod(*this, Symbols::PackageSpecSingleton(), Names::test_import())
                 .typedArg(Names::arg0(), make_type<ClassType>(Symbols::PackageSpecSingleton()))
                 .build();
    ENFORCE(method == Symbols::PackageSpec_test_import());

    method = enterMethod(*this, Symbols::PackageSpecSingleton(), Names::export_()).arg(Names::arg0()).build();
    ENFORCE(method == Symbols::PackageSpec_export());
    method = enterMethod(*this, Symbols::PackageSpecSingleton(), Names::export_for_test()).arg(Names::arg0()).build();
    ENFORCE(method == Symbols::PackageSpec_export_for_test());
    method =
        enterMethod(*this, Symbols::PackageSpecSingleton(), Names::restrict_to_service()).arg(Names::arg0()).build();
    ENFORCE(method == Symbols::PackageSpec_restrict_to_service());

    klass = synthesizeClass(core::Names::Constants::Encoding());
    ENFORCE(klass == Symbols::Encoding());

    klass = synthesizeClass(core::Names::Constants::Thread());
    ENFORCE(klass == Symbols::Thread());

    // Class#new
    method = enterMethod(*this, Symbols::Class(), Names::new_()).repeatedArg(Names::args()).build();
    ENFORCE(method == Symbols::Class_new());

    method = enterMethodSymbol(Loc::none(), Symbols::noClassOrModule(), Names::TodoMethod());
    enterMethodArgumentSymbol(Loc::none(), method, Names::args());
    ENFORCE(method == Symbols::todoMethod());

    klass = enterClassSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), core::Names::Constants::ResolvedSig());
    ENFORCE(klass == Symbols::Sorbet_Private_Static_ResolvedSig());
    klass = Symbols::Sorbet_Private_Static_ResolvedSig().data(*this)->singletonClass(*this);
    ENFORCE(klass == Symbols::Sorbet_Private_Static_ResolvedSigSingleton());

    klass = enterClassSymbol(Loc::none(), Symbols::T_Private(), core::Names::Constants::Compiler());
    ENFORCE(klass == Symbols::T_Private_Compiler());
    klass = Symbols::T_Private_Compiler().data(*this)->singletonClass(*this);
    ENFORCE(klass == Symbols::T_Private_CompilerSingleton());

    typeArgument =
        enterTypeArgument(Loc::none(), Symbols::noMethod(), Names::Constants::TodoTypeArgument(), Variance::CoVariant);
    ENFORCE(typeArgument == Symbols::todoTypeArgument());
    typeArgument.data(*this)->resultType = make_type<core::TypeVar>(typeArgument);

    // Root members
    Symbols::root().data(*this)->members()[core::Names::Constants::NoSymbol()] = Symbols::noSymbol();
    Symbols::root().data(*this)->members()[core::Names::Constants::Top()] = Symbols::top();
    Symbols::root().data(*this)->members()[core::Names::Constants::Bottom()] = Symbols::bottom();

    // Sorbet::Private::Static::VERSION
    field = enterStaticFieldSymbol(Loc::none(), Symbols::Sorbet_Private_Static(), Names::Constants::VERSION());
    field.data(*this)->resultType =
        make_type<LiteralType>(Symbols::String(), enterNameUTF8(sorbet_full_version_string));

    // ::<ErrorNode>
    field = enterStaticFieldSymbol(Loc::none(), Symbols::root(), Names::Constants::ErrorNode());

    // Synthesize <Magic>.<build-hash>(*vs : T.untyped) => Hash
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::buildHash())
                 .repeatedUntypedArg(Names::arg0())
                 .buildWithResult(Types::hashOfUntyped());
    // Synthesize <Magic>.<build-array>(*vs : T.untyped) => Array
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::buildArray())
                 .repeatedUntypedArg(Names::arg0())
                 .buildWithResult(Types::arrayOfUntyped());

    // Synthesize <Magic>.<build-range>(from: T.untyped, to: T.untyped) => Range
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::buildRange())
                 .untypedArg(Names::arg0())
                 .untypedArg(Names::arg1())
                 .untypedArg(Names::arg2())
                 .buildWithResult(Types::rangeOfUntyped());

    // Synthesize <Magic>.<regex-backref>(arg0: T.untyped) => T.nilable(String)
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::regexBackref())
                 .untypedArg(Names::arg0())
                 .buildWithResult(Types::any(*this, Types::nilClass(), Types::String()));

    // Synthesize <Magic>.<splat>(a: T.untyped) => Untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::splat())
                 .untypedArg(Names::arg0())
                 .buildWithResultUntyped();

    // Synthesize <Magic>.<defined>(*arg0: String) => Boolean
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::defined_p())
                 .repeatedTypedArg(Names::arg0(), Types::String())
                 .buildWithResult(Types::any(*this, Types::nilClass(), Types::String()));

    // Synthesize <Magic>.<expandSplat>(arg0: T.untyped, arg1: Integer, arg2: Integer) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::expandSplat())
                 .untypedArg(Names::arg0())
                 .untypedArg(Names::arg1())
                 .untypedArg(Names::arg2())
                 .buildWithResultUntyped();
    // Synthesize <Magic>.<call-with-splat>(args: *T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::callWithSplat())
                 .repeatedUntypedArg(Names::arg0())
                 .buildWithResultUntyped();
    // Synthesize <Magic>.<call-with-block>(args: *T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::callWithBlock())
                 .repeatedUntypedArg(Names::arg0())
                 .buildWithResultUntyped();
    // Synthesize <Magic>.<call-with-splat-and-block>(args: *T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::callWithSplatAndBlock())
                 .repeatedUntypedArg(Names::arg0())
                 .buildWithResultUntyped();
    // Synthesize <Magic>.<suggest-type>(arg: *T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::suggestType())
                 .untypedArg(Names::arg0())
                 .buildWithResultUntyped();
    // Synthesize <Magic>.<self-new>(arg: *T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::selfNew())
                 .repeatedUntypedArg(Names::arg0())
                 .buildWithResultUntyped();
    // Synthesize <Magic>.<nil-for-safe-navigation>(recv: T.untyped) => NilClass
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::nilForSafeNavigation())
                 .untypedArg(Names::arg0())
                 .buildWithResult(Types::nilClass());
    // Synthesize <Magic>.<string-interpolate>(arg: *T.untyped) => String
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::stringInterpolate())
                 .repeatedUntypedArg(Names::arg0())
                 .buildWithResult(Types::String());
    // Synthesize <Magic>.<define-top-class-or-module>(arg: T.untyped) => Void
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::defineTopClassOrModule())
                 .untypedArg(Names::arg0())
                 .buildWithResult(Types::void_());
    // Synthesize <Magic>.<keep-for-cfg>(arg: T.untyped) => Void
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::keepForCfg())
                 .untypedArg(Names::arg0())
                 .buildWithResult(Types::void_());
    // Synthesize <Magic>.<retry>() => Void
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::retry()).buildWithResult(Types::void_());

    // Synthesize <Magic>.<blockBreak>(args: T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::blockBreak())
                 .untypedArg(Names::arg0())
                 .buildWithResultUntyped();

    // Synthesize <Magic>.<getEncoding>() => Encoding
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::getEncoding())
                 .buildWithResult(core::make_type<core::ClassType>(core::Symbols::Encoding()));

    // Synthesize <Magic>.mixes_in_class_methods(self: T.untyped, arg: T.untyped) => Void
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::mixesInClassMethods())
                 .untypedArg(Names::selfLocal())
                 .untypedArg(Names::arg0())
                 .buildWithResult(Types::void_());

    // Synthesize <Magic>.<check-match-array>(pattern: T.untyped, splatArray: T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::checkMatchArray())
                 .untypedArg(Names::arg0())
                 .untypedArg(Names::arg1())
                 .buildWithResultUntyped();

    // Synthesize <Magic>.<to-hash-dup>(arg: T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::toHashDup())
                 .untypedArg(Names::arg0())
                 .buildWithResultUntyped();

    // Synthesize <Magic>.<to-hash-nodup>(arg: T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::toHashNoDup())
                 .untypedArg(Names::arg0())
                 .buildWithResultUntyped();

    // Synthesize <Magic>.<merge-hash>(*arg: T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::mergeHash())
                 .repeatedUntypedArg(Names::arg0())
                 .buildWithResultUntyped();

    // Synthesize <Magic>.<merge-hash-values>(arg0: T.untyped, *arg1: T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::mergeHashValues())
                 .untypedArg(Names::arg0())
                 .repeatedUntypedArg(Names::arg())
                 .buildWithResultUntyped();

    // Synthesize <Magic>.<defined-class-var>(arg0: T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::definedClassVar())
                 .untypedArg(Names::arg0())
                 .buildWithResultUntyped();

    // Synthesize <Magic>.<defined-instance-var>(arg0: T.untyped) => T.untyped
    method = enterMethod(*this, Symbols::MagicSingleton(), Names::definedInstanceVar())
                 .untypedArg(Names::arg0())
                 .buildWithResultUntyped();

    // Synthesize <DeclBuilderForProcs>.<params>(args: T.untyped) => DeclBuilderForProcs
    method = enterMethod(*this, Symbols::DeclBuilderForProcsSingleton(), Names::params())
                 .kwsplatArg(Names::arg0())
                 .buildWithResult(Types::declBuilderForProcsSingletonClass());
    // Synthesize <DeclBuilderForProcs>.<bind>(args: T.untyped) =>
    // DeclBuilderForProcs
    method = enterMethod(*this, Symbols::DeclBuilderForProcsSingleton(), Names::bind())
                 .untypedArg(Names::arg0())
                 .buildWithResult(Types::declBuilderForProcsSingletonClass());
    // Synthesize <DeclBuilderForProcs>.<returns>(args: T.untyped) => DeclBuilderForProcs
    method = enterMethod(*this, Symbols::DeclBuilderForProcsSingleton(), Names::returns())
                 .untypedArg(Names::arg0())
                 .buildWithResult(Types::declBuilderForProcsSingletonClass());
    // Synthesize <DeclBuilderForProcs>.<type_parameters>(args: T.untyped) =>
    // DeclBuilderForProcs
    method = enterMethod(*this, Symbols::DeclBuilderForProcsSingleton(), Names::typeParameters())
                 .untypedArg(Names::arg0())
                 .buildWithResult(Types::declBuilderForProcsSingletonClass());
    // Some of these are Modules
    Symbols::StubModule().data(*this)->setIsModule(true);
    Symbols::T().data(*this)->setIsModule(true);
    Symbols::StubMixin().data(*this)->setIsModule(true);

    // Some of these are Classes
    Symbols::StubSuperClass().data(*this)->setIsModule(false);
    Symbols::StubSuperClass().data(*this)->setSuperClass(Symbols::Object());

    // Synthesize T::Utils
    klass = enterClassSymbol(Loc::none(), Symbols::T(), core::Names::Constants::Utils());
    klass.data(*this)->setIsModule(true);

    int reservedCount = 0;

    // Set the correct resultTypes for all synthesized classes
    // Collect size prior to loop since singletons will cause vector to grow.
    size_t classAndModulesSize = classAndModules.size();
    for (uint32_t i = 1; i < classAndModulesSize; i++) {
        classAndModules[i].singletonClass(*this);
    }

    // This fills in all the way up to MAX_SYNTHETIC_CLASS_SYMBOLS
    ENFORCE(classAndModules.size() < Symbols::Proc0().id());
    while (classAndModules.size() < Symbols::Proc0().id()) {
        string name = absl::StrCat("<RESERVED_", reservedCount, ">");
        synthesizeClass(enterNameConstant(name));
        reservedCount++;
    }

    for (int arity = 0; arity <= Symbols::MAX_PROC_ARITY; ++arity) {
        string name = absl::StrCat("Proc", arity);
        auto id = synthesizeClass(enterNameConstant(name), Symbols::Proc().id());
        ENFORCE(id == Symbols::Proc(arity), "Proc creation failed for arity: {} got: {} expected: {}", arity, id.id(),
                Symbols::Proc(arity).id());
        id.data(*this)->singletonClass(*this);
    }

    ENFORCE(classAndModules.size() == Symbols::last_synthetic_class_sym().id() + 1,
            "Too many synthetic class symbols? have: {} expected: {}", classAndModules.size(),
            Symbols::last_synthetic_class_sym().id() + 1);

    ENFORCE(methods.size() == Symbols::MAX_SYNTHETIC_METHOD_SYMBOLS,
            "Too many synthetic method symbols? have: {} expected: {}", methods.size(),
            Symbols::MAX_SYNTHETIC_METHOD_SYMBOLS);
    ENFORCE(fields.size() == Symbols::MAX_SYNTHETIC_FIELD_SYMBOLS,
            "Too many synthetic field symbols? have: {} expected: {}", fields.size(),
            Symbols::MAX_SYNTHETIC_FIELD_SYMBOLS);
    ENFORCE(typeMembers.size() == Symbols::MAX_SYNTHETIC_TYPEMEMBER_SYMBOLS,
            "Too many synthetic typeMember symbols? have: {} expected: {}", typeMembers.size(),
            Symbols::MAX_SYNTHETIC_TYPEMEMBER_SYMBOLS);
    ENFORCE(typeArguments.size() == Symbols::MAX_SYNTHETIC_TYPEARGUMENT_SYMBOLS,
            "Too many synthetic typeArgument symbols? have: {} expected: {}", typeArguments.size(),
            Symbols::MAX_SYNTHETIC_TYPEARGUMENT_SYMBOLS);

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
}

void GlobalState::installIntrinsics() {
    for (auto &entry : intrinsicMethods) {
        ClassOrModuleRef symbol;
        switch (entry.singleton) {
            case Intrinsic::Kind::Instance:
                symbol = entry.symbol;
                break;
            case Intrinsic::Kind::Singleton:
                symbol = entry.symbol.data(*this)->singletonClass(*this);
                break;
        }
        auto countBefore = methodsUsed();
        auto method = enterMethodSymbol(Loc::none(), symbol, entry.method);
        method.data(*this)->intrinsic = entry.impl;
        if (countBefore != methodsUsed()) {
            auto &blkArg = enterMethodArgumentSymbol(Loc::none(), method, Names::blkArg());
            blkArg.flags.isBlock = true;
        }
    }
}

void GlobalState::preallocateTables(uint32_t classAndModulesSize, uint32_t methodsSize, uint32_t fieldsSize,
                                    uint32_t typeArgumentsSize, uint32_t typeMembersSize, uint32_t utf8NameSize,
                                    uint32_t constantNameSize, uint32_t uniqueNameSize) {
    uint32_t classAndModulesSizeScaled = nextPowerOfTwo(classAndModulesSize);
    uint32_t methodsSizeScaled = nextPowerOfTwo(methodsSize);
    uint32_t fieldsSizeScaled = nextPowerOfTwo(fieldsSize);
    uint32_t typeArgumentsSizeScaled = nextPowerOfTwo(typeArgumentsSize);
    uint32_t typeMembersSizeScaled = nextPowerOfTwo(typeMembersSize);
    uint32_t utf8NameSizeScaled = nextPowerOfTwo(utf8NameSize);
    uint32_t constantNameSizeScaled = nextPowerOfTwo(constantNameSize);
    uint32_t uniqueNameSizeScaled = nextPowerOfTwo(uniqueNameSize);

    // When preallocating in release builds, large initial reservations aren't necessarily a problem on hosts with lots
    // of cores available as we use jemalloc as the allocator. An effect of this is that larger allocations will be
    // mapped with MAP_NORESERVE, and will only be backed by real memory if they're used. As a result, the large number
    // of threads spawned during indexing (where no symbol table entries are created) won't end up allocating memory to
    // back the symbol tables when the global state is copied in each thread.

    // Note: reserve is a no-op if size is < current capacity.
    classAndModules.reserve(classAndModulesSizeScaled);
    methods.reserve(methodsSizeScaled);
    fields.reserve(fieldsSizeScaled);
    typeArguments.reserve(typeArgumentsSizeScaled);
    typeMembers.reserve(typeMembersSizeScaled);
    expandNames(utf8NameSizeScaled, constantNameSizeScaled, uniqueNameSizeScaled);
    sanityCheck();

    trace(fmt::format("Preallocated symbol and name tables. classAndModules={} methods={} fields={} typeArguments={} "
                      "typeMembers={} utf8Names={} constantNames={} uniqueNames={}",
                      classAndModules.capacity(), methods.capacity(), fields.capacity(), typeArguments.capacity(),
                      typeMembers.capacity(), utf8Names.capacity(), constantNames.capacity(), uniqueNames.capacity()));
}

constexpr decltype(GlobalState::STRINGS_PAGE_SIZE) GlobalState::STRINGS_PAGE_SIZE;

MethodRef GlobalState::lookupMethodSymbolWithHash(ClassOrModuleRef owner, NameRef name,
                                                  const vector<uint32_t> &methodHash) const {
    ENFORCE(owner.exists(), "looking up symbol from non-existing owner");
    ENFORCE(name.exists(), "looking up symbol with non-existing name");
    auto ownerScope = owner.dataAllowingNone(*this);
    histogramInc("symbol_lookup_by_name", ownerScope->members().size());

    NameRef lookupName = name;
    uint32_t unique = 1;
    auto res = ownerScope->members().find(lookupName);
    while (res != ownerScope->members().end()) {
        ENFORCE(res->second.exists());
        auto resSym = res->second;
        if (resSym.isMethod()) {
            auto resMethod = resSym.asMethodRef().data(*this);
            if (resMethod->methodArgumentHash(*this) == methodHash ||
                (resMethod->intrinsic != nullptr && !resMethod->hasSig())) {
                return resSym.asMethodRef();
            }
        }
        lookupName = lookupNameUnique(UniqueNameKind::MangleRename, name, unique);
        if (!lookupName.exists()) {
            break;
        }
        res = ownerScope->members().find(lookupName);
        unique++;
    }
    return Symbols::noMethod();
}

// look up a symbol whose flags match the desired kind (or ignores the kind filter if `ignoreKind` is `true`).
// This might look through mangled names to discover one whose flags match.
// If no such symbol exists, then it will return defaultReturnValue.
SymbolRef GlobalState::lookupSymbolWithKind(ClassOrModuleRef owner, NameRef name, SymbolRef::Kind kind,
                                            SymbolRef defaultReturnValue, bool ignoreKind) const {
    ENFORCE(owner.exists(), "looking up symbol from non-existing owner");
    ENFORCE(name.exists(), "looking up symbol with non-existing name");
    auto ownerScope = owner.dataAllowingNone(*this);
    histogramInc("symbol_lookup_by_name", ownerScope->members().size());

    NameRef lookupName = name;
    uint32_t unique = 1;
    auto res = ownerScope->members().find(lookupName);
    while (res != ownerScope->members().end()) {
        ENFORCE(res->second.exists());
        if (ignoreKind || res->second.kind() == kind) {
            return res->second;
        }
        lookupName = lookupNameUnique(UniqueNameKind::MangleRename, name, unique);
        if (!lookupName.exists()) {
            break;
        }
        res = ownerScope->members().find(lookupName);
        unique++;
    }
    return defaultReturnValue;
}

SymbolRef GlobalState::findRenamedSymbol(ClassOrModuleRef owner, SymbolRef sym) const {
    // This method works by knowing how to replicate the logic of renaming in order to find whatever
    // the previous name was: for `x$n` where `n` is larger than 2, it'll be `x$(n-1)`, for bare `x`,
    // it'll be whatever the largest `x$n` that exists is, if any; otherwise, there will be none.
    ENFORCE(sym.exists(), "lookup up previous name of non-existing symbol");
    NameRef name = sym.name(*this);
    auto ownerScope = owner.dataAllowingNone(*this);

    if (name.kind() == NameKind::UNIQUE) {
        auto uniqueData = name.dataUnique(*this);
        if (uniqueData->uniqueNameKind != UniqueNameKind::MangleRename) {
            return Symbols::noSymbol();
        }
        if (uniqueData->num == 1) {
            return Symbols::noSymbol();
        } else {
            ENFORCE(uniqueData->num > 1);
            auto nm = lookupNameUnique(UniqueNameKind::MangleRename, uniqueData->original, uniqueData->num - 1);
            if (!nm.exists()) {
                return Symbols::noSymbol();
            }
            auto res = ownerScope->members().find(nm);
            ENFORCE(res != ownerScope->members().end());
            return res->second;
        }
    } else {
        uint32_t unique = 1;
        NameRef lookupName = lookupNameUnique(UniqueNameKind::MangleRename, name, unique);
        auto res = ownerScope->members().find(lookupName);
        while (res != ownerScope->members().end()) {
            ENFORCE(res->second.exists());
            unique++;
            lookupName = lookupNameUnique(UniqueNameKind::MangleRename, name, unique);
            if (!lookupName.exists()) {
                return res->second;
            }
            res = ownerScope->members().find(lookupName);
        }
        return Symbols::noSymbol();
    }
}

ClassOrModuleRef GlobalState::enterClassSymbol(Loc loc, ClassOrModuleRef owner, NameRef name) {
    // ENFORCE_NO_TIMER(!owner.exists()); // Owner may not exist on purely synthetic symbols.
    ENFORCE_NO_TIMER(name.isClassName(*this));
    SymbolData ownerScope = owner.dataAllowingNone(*this);
    histogramInc("symbol_enter_by_name", ownerScope->members().size());

    auto &store = ownerScope->members()[name];
    if (store.exists()) {
        ENFORCE_NO_TIMER(store.isClassOrModule(), "existing symbol is not a class or module");
        counterInc("symbols.hit");
        return store.asClassOrModuleRef();
    }

    ENFORCE_NO_TIMER(!symbolTableFrozen);
    auto ret = ClassOrModuleRef(*this, classAndModules.size());
    store = ret; // DO NOT MOVE this assignment down. emplace_back on classAndModules invalidates `store`
    classAndModules.emplace_back();
    SymbolData data = ret.data(*this);
    data->name = name;
    data->flags = Symbol::Flags::CLASS_OR_MODULE;
    data->owner = owner;
    data->addLoc(*this, loc);
    DEBUG_ONLY(categoryCounterInc("symbols", "class"));
    wasModified_ = true;

    return ret;
}

TypeMemberRef GlobalState::enterTypeMember(Loc loc, ClassOrModuleRef owner, NameRef name, Variance variance) {
    uint32_t flags;
    ENFORCE(owner.exists() || name == Names::Constants::NoTypeMember());
    ENFORCE(name.exists());
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

    SymbolData ownerScope = owner.dataAllowingNone(*this);
    histogramInc("symbol_enter_by_name", ownerScope->members().size());

    auto &store = ownerScope->members()[name];
    if (store.exists()) {
        ENFORCE(store.isTypeMember() && (store.asTypeMemberRef().data(*this)->flags & flags) == flags,
                "existing symbol has wrong flags");
        counterInc("symbols.hit");
        return store.asTypeMemberRef();
    }

    ENFORCE(!symbolTableFrozen);
    auto result = TypeMemberRef(*this, typeMembers.size());
    store = result; // DO NOT MOVE this assignment down. emplace_back on typeMembers invalidates `store`
    typeMembers.emplace_back();

    SymbolData data = result.dataAllowingNone(*this);
    data->name = name;
    data->flags = flags;
    data->owner = owner;
    data->addLoc(*this, loc);
    DEBUG_ONLY(categoryCounterInc("symbols", "type_member"));
    wasModified_ = true;

    auto &members = owner.dataAllowingNone(*this)->typeMembers();
    if (!absl::c_linear_search(members, result)) {
        members.emplace_back(result);
    }
    return result;
}

TypeArgumentRef GlobalState::enterTypeArgument(Loc loc, MethodRef owner, NameRef name, Variance variance) {
    ENFORCE(owner.exists() || name == Names::Constants::NoTypeArgument() ||
            name == Names::Constants::TodoTypeArgument());
    ENFORCE(name.exists());
    uint32_t flags;
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

    auto ownerScope = owner.dataAllowingNone(*this);
    histogramInc("symbol_enter_by_name", ownerScope->typeArguments.size());

    for (auto typeArg : ownerScope->typeArguments) {
        if (typeArg.dataAllowingNone(*this)->name == name) {
            ENFORCE((typeArg.dataAllowingNone(*this)->flags & flags) == flags, "existing symbol has wrong flags");
            counterInc("symbols.hit");
            return typeArg;
        }
    }

    ENFORCE(!symbolTableFrozen);
    auto result = TypeArgumentRef(*this, typeArguments.size());
    typeArguments.emplace_back();

    SymbolData data = result.dataAllowingNone(*this);
    data->name = name;
    data->flags = flags;
    data->owner = owner;
    data->addLoc(*this, loc);
    DEBUG_ONLY(categoryCounterInc("symbols", "type_argument"));
    wasModified_ = true;

    owner.dataAllowingNone(*this)->typeArguments.emplace_back(result);
    return result;
}

MethodRef GlobalState::enterMethodSymbol(Loc loc, ClassOrModuleRef owner, NameRef name) {
    SymbolData ownerScope = owner.dataAllowingNone(*this);
    histogramInc("symbol_enter_by_name", ownerScope->members().size());

    auto &store = ownerScope->members()[name];
    if (store.exists()) {
        ENFORCE(store.isMethod(), "existing symbol is not a method");
        counterInc("symbols.hit");
        return store.asMethodRef();
    }

    ENFORCE(!symbolTableFrozen);

    auto result = MethodRef(*this, methods.size());
    store = result; // DO NOT MOVE this assignment down. emplace_back on methods invalidates `store`
    methods.emplace_back();

    MethodData data = result.dataAllowingNone(*this);
    data->name = name;
    data->owner = owner;
    data->addLoc(*this, loc);
    DEBUG_ONLY(categoryCounterInc("symbols", "method"));
    wasModified_ = true;

    return result;
}

MethodRef GlobalState::enterNewMethodOverload(Loc sigLoc, MethodRef original, core::NameRef originalName, uint32_t num,
                                              const vector<bool> &argsToKeep) {
    NameRef name = num == 0 ? originalName : freshNameUnique(UniqueNameKind::Overload, originalName, num);
    core::Loc loc = num == 0 ? original.data(*this)->loc()
                             : sigLoc; // use original Loc for main overload so that we get right jump-to-def for it.
    auto owner = original.data(*this)->owner;
    auto res = enterMethodSymbol(loc, owner, name);
    ENFORCE(res != original);
    if (res.data(*this)->arguments.size() != original.data(*this)->arguments.size()) {
        ENFORCE(res.data(*this)->arguments.empty());
        res.data(*this)->arguments.reserve(original.data(*this)->arguments.size());
        const auto &originalArguments = original.data(*this)->arguments;
        int i = -1;
        for (auto &arg : originalArguments) {
            i += 1;
            Loc loc = arg.loc;
            if (!argsToKeep[i]) {
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

FieldRef GlobalState::enterFieldSymbol(Loc loc, ClassOrModuleRef owner, NameRef name) {
    ENFORCE(name.exists());

    SymbolData ownerScope = owner.dataAllowingNone(*this);
    histogramInc("symbol_enter_by_name", ownerScope->members().size());

    auto &store = ownerScope->members()[name];
    if (store.exists()) {
        ENFORCE(store.isField(*this), "existing symbol is not a field");
        counterInc("symbols.hit");
        return store.asFieldRef();
    }

    ENFORCE(!symbolTableFrozen);

    auto result = FieldRef(*this, fields.size());
    store = result; // DO NOT MOVE this assignment down. emplace_back on fields invalidates `store`
    fields.emplace_back();

    FieldData data = result.dataAllowingNone(*this);
    data->name = name;
    data->flags.isField = true;
    data->owner = owner;
    data->addLoc(*this, loc);

    DEBUG_ONLY(categoryCounterInc("symbols", "field"));
    wasModified_ = true;

    return result;
}

FieldRef GlobalState::enterStaticFieldSymbol(Loc loc, ClassOrModuleRef owner, NameRef name) {
    ENFORCE(name.exists());

    SymbolData ownerScope = owner.dataAllowingNone(*this);
    histogramInc("symbol_enter_by_name", ownerScope->members().size());

    auto &store = ownerScope->members()[name];
    if (store.exists()) {
        ENFORCE(store.isStaticField(*this), "existing symbol is not a static field");
        counterInc("symbols.hit");
        return store.asFieldRef();
    }

    ENFORCE(!symbolTableFrozen);

    auto ret = FieldRef(*this, fields.size());
    store = ret; // DO NOT MOVE this assignment down. emplace_back on fields invalidates `store`
    fields.emplace_back();

    FieldData data = ret.dataAllowingNone(*this);
    data->name = name;
    data->flags.isStaticField = true;
    data->owner = owner;
    data->addLoc(*this, loc);

    DEBUG_ONLY(categoryCounterInc("symbols", "static_field"));
    wasModified_ = true;

    return ret;
}

ArgInfo &GlobalState::enterMethodArgumentSymbol(Loc loc, MethodRef owner, NameRef name) {
    ENFORCE(owner.exists(), "entering symbol in to non-existing owner");
    ENFORCE(name.exists(), "entering symbol with non-existing name");
    MethodData ownerScope = owner.data(*this);

    for (auto &arg : ownerScope->arguments) {
        if (arg.name == name) {
            return arg;
        }
    }
    auto &store = ownerScope->arguments.emplace_back();

    ENFORCE(!symbolTableFrozen);

    store.name = name;
    store.loc = loc;
    DEBUG_ONLY(categoryCounterInc("symbols", "argument"););

    wasModified_ = true;
    return store;
}

string_view GlobalState::enterString(string_view nm) {
    DEBUG_ONLY(if (ensureCleanStrings) {
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

            // NOTE: we do not update `stringsLastPageUsed` here because it refers to the offset into the last page,
            // which is swapped in by the line above.
        } else {
            // Insert a new empty page at the end to enforce the invariant that inserting a huge string will always
            // leave a page that can be written to at the end of the table.
            strings.emplace_back(make_unique<vector<char>>(GlobalState::STRINGS_PAGE_SIZE));
            stringsLastPageUsed = 0;
        }
    } else {
        if (stringsLastPageUsed + nm.size() > GlobalState::STRINGS_PAGE_SIZE) {
            strings.emplace_back(make_unique<vector<char>>(GlobalState::STRINGS_PAGE_SIZE));
            // printf("Wasted %i space\n", STRINGS_PAGE_SIZE - stringsLastPageUsed);
            stringsLastPageUsed = 0;
        }
        from = strings.back()->data() + stringsLastPageUsed;
        stringsLastPageUsed += nm.size();
    }

    counterInc("strings");
    memcpy(from, nm.data(), nm.size());
    return string_view(from, nm.size());
}

NameRef GlobalState::lookupNameUTF8(string_view nm) const {
    const auto hs = _hash(nm);
    unsigned int hashTableSize = namesByHash.size();
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probeCount = 1;

    while (namesByHash[bucketId].second != 0u) {
        auto &bucket = namesByHash[bucketId];
        if (bucket.first == hs) {
            auto name = NameRef::fromRaw(*this, bucket.second);
            if (name.kind() == NameKind::UTF8 && name.dataUtf8(*this)->utf8 == nm) {
                counterInc("names.utf8.hit");
                return name;
            } else {
                counterInc("names.hash_collision.utf8");
            }
        }
        bucketId = (bucketId + probeCount) & mask;
        probeCount++;
    }

    return core::NameRef::noName();
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
            auto name = NameRef::fromRaw(*this, bucket.second);
            if (name.kind() == NameKind::UTF8 && name.dataUtf8(*this)->utf8 == nm) {
                counterInc("names.utf8.hit");
                return name;
            } else {
                counterInc("names.hash_collision.utf8");
            }
        }
        bucketId = (bucketId + probeCount) & mask;
        probeCount++;
    }
    ENFORCE(!nameTableFrozen);

    ENFORCE(probeCount != hashTableSize, "Full table?");

    if (utf8Names.size() == utf8Names.capacity()) {
        expandNames(utf8Names.capacity() * 2, constantNames.capacity(), uniqueNames.capacity());
        hashTableSize = namesByHash.size();
        mask = hashTableSize - 1;
        bucketId = hs & mask; // look for place in the new size
        probeCount = 1;
        while (namesByHash[bucketId].second != 0) {
            bucketId = (bucketId + probeCount) & mask;
            probeCount++;
        }
    }

    auto name = NameRef(*this, NameKind::UTF8, utf8Names.size());
    auto &bucket = namesByHash[bucketId];
    bucket.first = hs;
    bucket.second = name.rawId();
    utf8Names.emplace_back(UTF8Name{enterString(nm)});

    ENFORCE(hashNameRef(*this, name) == hs);
    categoryCounterInc("names", "utf8");

    wasModified_ = true;
    return name;
}

NameRef GlobalState::enterNameConstant(NameRef original) {
    ENFORCE(original.exists(), "making a constant name over non-existing name");
    ENFORCE(original.isValidConstantName(*this), "making a constant name over wrong name kind");

    const auto hs = hashMixConstant(original.rawId());
    unsigned int hashTableSize = namesByHash.size();
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probeCount = 1;

    while (namesByHash[bucketId].second != 0 && probeCount < hashTableSize) {
        auto &bucket = namesByHash[bucketId];
        if (bucket.first == hs) {
            auto name = NameRef::fromRaw(*this, bucket.second);
            if (name.kind() == NameKind::CONSTANT && name.dataCnst(*this)->original == original) {
                counterInc("names.constant.hit");
                return name;
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

    if (constantNames.size() == constantNames.capacity()) {
        expandNames(utf8Names.capacity(), constantNames.capacity() * 2, uniqueNames.capacity());
        hashTableSize = namesByHash.size();
        mask = hashTableSize - 1;

        bucketId = hs & mask; // look for place in the new size
        probeCount = 1;
        while (namesByHash[bucketId].second != 0) {
            bucketId = (bucketId + probeCount) & mask;
            probeCount++;
        }
    }

    auto name = NameRef(*this, NameKind::CONSTANT, constantNames.size());
    auto &bucket = namesByHash[bucketId];
    bucket.first = hs;
    bucket.second = name.rawId();

    constantNames.emplace_back(ConstantName{original});
    ENFORCE(hashNameRef(*this, name) == hs);
    wasModified_ = true;
    categoryCounterInc("names", "constant");
    return name;
}

NameRef GlobalState::enterNameConstant(string_view original) {
    return enterNameConstant(enterNameUTF8(original));
}

NameRef GlobalState::lookupNameConstant(NameRef original) const {
    if (!original.exists()) {
        return core::NameRef::noName();
    }
    ENFORCE(original.isValidConstantName(*this), "looking up a constant name over wrong name kind");

    const auto hs = hashMixConstant(original.rawId());
    unsigned int hashTableSize = namesByHash.size();
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probeCount = 1;

    while (namesByHash[bucketId].second != 0 && probeCount < hashTableSize) {
        auto &bucket = namesByHash[bucketId];
        if (bucket.first == hs) {
            auto name = NameRef::fromRaw(*this, bucket.second);
            if (name.kind() == NameKind::CONSTANT && name.dataCnst(*this)->original == original) {
                counterInc("names.constant.hit");
                return name;
            } else {
                counterInc("names.hash_collision.constant");
            }
        }
        bucketId = (bucketId + probeCount) & mask;
        probeCount++;
    }

    return core::NameRef::noName();
}

NameRef GlobalState::lookupNameConstant(string_view original) const {
    auto utf8 = lookupNameUTF8(original);
    if (!utf8.exists()) {
        return core::NameRef::noName();
    }
    return lookupNameConstant(utf8);
}

void moveNames(pair<unsigned int, uint32_t> *from, pair<unsigned int, uint32_t> *to, unsigned int szFrom,
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

void GlobalState::expandNames(uint32_t utf8NameSize, uint32_t constantNameSize, uint32_t uniqueNameSize) {
    sanityCheck();
    utf8Names.reserve(utf8NameSize);
    constantNames.reserve(constantNameSize);
    uniqueNames.reserve(uniqueNameSize);

    uint32_t hashTableSize = 2 * nextPowerOfTwo(utf8NameSize + constantNameSize + uniqueNameSize);

    if (hashTableSize > namesByHash.size()) {
        vector<pair<unsigned int, unsigned int>> new_namesByHash(hashTableSize);
        moveNames(namesByHash.data(), new_namesByHash.data(), namesByHash.size(), new_namesByHash.capacity());
        namesByHash.swap(new_namesByHash);
    }
}

NameRef GlobalState::lookupNameUnique(UniqueNameKind uniqueNameKind, NameRef original, uint32_t num) const {
    ENFORCE(num > 0, "num == 0, name overflow");
    const auto hs = hashMixUnique(uniqueNameKind, num, original.rawId());
    unsigned int hashTableSize = namesByHash.size();
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probeCount = 1;

    while (namesByHash[bucketId].second != 0 && probeCount < hashTableSize) {
        auto &bucket = namesByHash[bucketId];
        if (bucket.first == hs) {
            auto name = NameRef::fromRaw(*this, bucket.second);
            if (name.kind() == NameKind::UNIQUE && name.dataUnique(*this)->uniqueNameKind == uniqueNameKind &&
                name.dataUnique(*this)->num == num && name.dataUnique(*this)->original == original) {
                counterInc("names.unique.hit");
                return name;
            } else {
                counterInc("names.hash_collision.unique");
            }
        }
        bucketId = (bucketId + probeCount) & mask;
        probeCount++;
    }
    return core::NameRef::noName();
}

NameRef GlobalState::freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original, uint32_t num) {
    ENFORCE(num > 0, "num == 0, name overflow");
    const auto hs = hashMixUnique(uniqueNameKind, num, original.rawId());
    unsigned int hashTableSize = namesByHash.size();
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probeCount = 1;

    while (namesByHash[bucketId].second != 0 && probeCount < hashTableSize) {
        auto &bucket = namesByHash[bucketId];
        if (bucket.first == hs) {
            auto name = NameRef::fromRaw(*this, bucket.second);
            if (name.kind() == NameKind::UNIQUE && name.dataUnique(*this)->uniqueNameKind == uniqueNameKind &&
                name.dataUnique(*this)->num == num && name.dataUnique(*this)->original == original) {
                counterInc("names.unique.hit");
                return name;
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

    if (uniqueNames.size() == uniqueNames.capacity()) {
        expandNames(utf8Names.capacity(), constantNames.capacity(), uniqueNames.capacity() * 2);
        hashTableSize = namesByHash.size();
        mask = hashTableSize - 1;

        bucketId = hs & mask; // look for place in the new size
        probeCount = 1;
        while (namesByHash[bucketId].second != 0) {
            bucketId = (bucketId + probeCount) & mask;
            probeCount++;
        }
    }

    auto name = NameRef(*this, NameKind::UNIQUE, uniqueNames.size());
    auto &bucket = namesByHash[bucketId];
    bucket.first = hs;
    bucket.second = name.rawId();

    uniqueNames.emplace_back(UniqueName{original, num, uniqueNameKind});
    ENFORCE(hashNameRef(*this, name) == hs);
    wasModified_ = true;
    categoryCounterInc("names", "unique");
    return name;
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
    auto owner = what.owner(*this).asClassOrModuleRef();
    auto ownerData = owner.data(*this);
    auto &ownerMembers = ownerData->members();
    auto fnd = ownerMembers.find(origName);
    ENFORCE(fnd != ownerMembers.end());
    ENFORCE(fnd->second == what);
    ENFORCE(what.name(*this) == origName);
    uint32_t collisionCount = 1;
    NameRef name;
    do {
        name = freshNameUnique(UniqueNameKind::MangleRename, origName, collisionCount++);
    } while (ownerData->findMember(*this, name).exists());
    ownerMembers.erase(fnd);
    ownerMembers[name] = what;
    switch (what.kind()) {
        case SymbolRef::Kind::ClassOrModule: {
            auto whatKlass = what.asClassOrModuleRef().data(*this);
            whatKlass->name = name;
            auto singleton = whatKlass->lookupSingletonClass(*this);
            if (singleton.exists()) {
                mangleRenameSymbol(singleton, singleton.data(*this)->name);
            }
            break;
        }
        case SymbolRef::Kind::Method:
            what.asMethodRef().data(*this)->name = name;
            break;
        case SymbolRef::Kind::FieldOrStaticField:
            what.asFieldRef().data(*this)->name = name;
            break;
        case SymbolRef::Kind::TypeArgument:
            what.asTypeArgumentRef().data(*this)->name = name;
            break;
        case SymbolRef::Kind::TypeMember:
            what.asTypeMemberRef().data(*this)->name = name;
            break;
    }
}

unsigned int GlobalState::classAndModulesUsed() const {
    return classAndModules.size();
}

unsigned int GlobalState::methodsUsed() const {
    return methods.size();
}

unsigned int GlobalState::fieldsUsed() const {
    return fields.size();
}

unsigned int GlobalState::typeArgumentsUsed() const {
    return typeArguments.size();
}

unsigned int GlobalState::typeMembersUsed() const {
    return typeMembers.size();
}

unsigned int GlobalState::filesUsed() const {
    return files.size();
}

unsigned int GlobalState::namesUsedTotal() const {
    return utf8Names.size() + constantNames.size() + uniqueNames.size();
}

unsigned int GlobalState::utf8NamesUsed() const {
    return utf8Names.size();
}

unsigned int GlobalState::constantNamesUsed() const {
    return constantNames.size();
}

unsigned int GlobalState::uniqueNamesUsed() const {
    return uniqueNames.size();
}

unsigned int GlobalState::symbolsUsedTotal() const {
    return classAndModulesUsed() + methodsUsed() + fieldsUsed() + typeArgumentsUsed() + typeMembersUsed();
}

string GlobalState::toStringWithOptions(bool showFull, bool showRaw) const {
    return Symbols::root().toStringWithOptions(*this, 0, showFull, showRaw);
}

void GlobalState::sanityCheck() const {
    if (!debug_mode) {
        return;
    }
    if (fuzz_mode) {
        // it's very slow to check this and it didn't find bugs
        return;
    }

    Timer timeit("GlobalState::sanityCheck");
    ENFORCE(namesUsedTotal() > 0, "empty name table size");
    ENFORCE(!strings.empty(), "empty string table size");
    ENFORCE(!namesByHash.empty(), "empty name hash table size");
    ENFORCE((namesByHash.size() & (namesByHash.size() - 1)) == 0, "name hash table size is not a power of two");
    ENFORCE(nextPowerOfTwo(utf8Names.capacity() + constantNames.capacity() + uniqueNames.capacity()) * 2 ==
                namesByHash.capacity(),
            "name table and hash name table sizes out of sync names.capacity={} namesByHash.capacity={}",
            namesUsedTotal(), namesByHash.capacity());
    ENFORCE(namesByHash.size() == namesByHash.capacity(), "hash name table not at full capacity");

    for (uint32_t i = 0; i < utf8Names.size(); i++) {
        NameRef(*this, NameKind::UTF8, i).sanityCheck(*this);
    }

    for (uint32_t i = 0; i < constantNames.size(); i++) {
        NameRef(*this, NameKind::CONSTANT, i).sanityCheck(*this);
    }

    for (uint32_t i = 0; i < uniqueNames.size(); i++) {
        NameRef(*this, NameKind::UNIQUE, i).sanityCheck(*this);
    }

    int i = -1;
    for (auto &sym : classAndModules) {
        i++;
        if (i != 0) {
            sym.sanityCheck(*this);
        }
    }

    i = -1;
    for (auto &sym : methods) {
        i++;
        if (i != 0) {
            sym.sanityCheck(*this);
        }
    }

    i = -1;
    for (auto &sym : fields) {
        i++;
        if (i != 0) {
            sym.sanityCheck(*this);
        }
    }

    i = -1;
    for (auto &sym : typeArguments) {
        i++;
        if (i != 0) {
            sym.sanityCheck(*this);
        }
    }

    i = -1;
    for (auto &sym : typeMembers) {
        i++;
        if (i != 0) {
            sym.sanityCheck(*this);
        }
    }
    for (auto &ent : namesByHash) {
        if (ent.second == 0) {
            continue;
        }
        ENFORCE_NO_TIMER(ent.first == hashNameRef(*this, NameRef::fromRaw(*this, ent.second)),
                         "name hash table corruption");
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
    Timer timeit("GlobalState::deepCopy", this->creation);
    this->sanityCheck();
    auto result = make_unique<GlobalState>(this->errorQueue, this->epochManager);

    result->silenceErrors = this->silenceErrors;
    result->autocorrect = this->autocorrect;
    result->ensureCleanStrings = this->ensureCleanStrings;
    result->runningUnderAutogen = this->runningUnderAutogen;
    result->censorForSnapshotTests = this->censorForSnapshotTests;
    result->sleepInSlowPath = this->sleepInSlowPath;
    result->requiresAncestorEnabled = this->requiresAncestorEnabled;

    if (keepId) {
        result->globalStateId = this->globalStateId;
    }
    result->deepCloneHistory = this->deepCloneHistory;
    result->deepCloneHistory.emplace_back(
        DeepCloneHistoryEntry{this->globalStateId, utf8NamesUsed(), constantNamesUsed(), uniqueNamesUsed()});

    result->strings = this->strings;
    result->stringsLastPageUsed = STRINGS_PAGE_SIZE;
    result->files = this->files;
    result->fileRefByPath = this->fileRefByPath;
    result->lspQuery = this->lspQuery;
    result->kvstoreUuid = this->kvstoreUuid;
    result->lspTypecheckCount = this->lspTypecheckCount;
    result->errorUrlBase = this->errorUrlBase;
    result->includeErrorSections = this->includeErrorSections;
    result->ignoredForSuggestTypedErrorClasses = this->ignoredForSuggestTypedErrorClasses;
    result->suppressedErrorClasses = this->suppressedErrorClasses;
    result->onlyErrorClasses = this->onlyErrorClasses;
    result->suggestUnsafe = this->suggestUnsafe;
    result->utf8Names.reserve(this->utf8Names.capacity());
    result->constantNames.reserve(this->constantNames.capacity());
    result->uniqueNames.reserve(this->uniqueNames.capacity());
    if (keepId) {
        result->utf8Names.resize(this->utf8Names.size());
        ::memcpy(result->utf8Names.data(), this->utf8Names.data(), this->utf8Names.size() * sizeof(UTF8Name));
        result->constantNames.resize(this->constantNames.size());
        ::memcpy(result->constantNames.data(), this->constantNames.data(),
                 this->constantNames.size() * sizeof(ConstantName));
        result->uniqueNames.resize(this->uniqueNames.size());
        ::memcpy(result->uniqueNames.data(), this->uniqueNames.data(), this->uniqueNames.size() * sizeof(UniqueName));
    } else {
        for (auto &utf8Name : this->utf8Names) {
            result->utf8Names.emplace_back(utf8Name.deepCopy(*result));
        }
        for (auto &constantName : this->constantNames) {
            result->constantNames.emplace_back(constantName.deepCopy(*result));
        }
        for (auto &uniqueName : this->uniqueNames) {
            result->uniqueNames.emplace_back(uniqueName.deepCopy(*result));
        }
    }

    result->namesByHash.reserve(this->namesByHash.size());
    result->namesByHash = this->namesByHash;

    result->classAndModules.reserve(this->classAndModules.capacity());
    for (auto &sym : this->classAndModules) {
        result->classAndModules.emplace_back(sym.deepCopy(*result, keepId));
    }
    result->methods.reserve(this->methods.capacity());
    for (auto &sym : this->methods) {
        result->methods.emplace_back(sym.deepCopy(*result));
    }
    result->fields.reserve(this->fields.capacity());
    for (auto &sym : this->fields) {
        result->fields.emplace_back(sym.deepCopy(*result));
    }
    result->typeArguments.reserve(this->typeArguments.capacity());
    for (auto &sym : this->typeArguments) {
        result->typeArguments.emplace_back(sym.deepCopy(*result, keepId));
    }
    result->typeMembers.reserve(this->typeMembers.capacity());
    for (auto &sym : this->typeMembers) {
        result->typeMembers.emplace_back(sym.deepCopy(*result, keepId));
    }
    result->pathPrefix = this->pathPrefix;
    for (auto &semanticExtension : this->semanticExtensions) {
        result->semanticExtensions.emplace_back(semanticExtension->deepCopy(*this, *result));
    }
    result->packageDB_ = packageDB_.deepCopy();
    result->sanityCheck();
    {
        Timer timeit2("GlobalState::deepCopyOut");
        result->creation = timeit2.getFlowEdge();
    }
    return result;
}

unique_ptr<GlobalState> GlobalState::copyForIndex() const {
    auto result = make_unique<GlobalState>(this->errorQueue, this->epochManager);

    result->initEmpty();

    // Options that might be used during indexing are manually copied over here
    result->files = this->files;
    result->fileRefByPath = this->fileRefByPath;
    result->silenceErrors = this->silenceErrors;
    result->autocorrect = this->autocorrect;
    result->ensureCleanStrings = this->ensureCleanStrings;
    result->runningUnderAutogen = this->runningUnderAutogen;
    result->censorForSnapshotTests = this->censorForSnapshotTests;
    result->sleepInSlowPath = this->sleepInSlowPath;
    result->requiresAncestorEnabled = this->requiresAncestorEnabled;
    result->kvstoreUuid = this->kvstoreUuid;
    result->errorUrlBase = this->errorUrlBase;
    result->suppressedErrorClasses = this->suppressedErrorClasses;
    result->onlyErrorClasses = this->onlyErrorClasses;
    result->suggestUnsafe = this->suggestUnsafe;
    result->pathPrefix = this->pathPrefix;

    return result;
}

void GlobalState::mergeFileTable(const core::GlobalState &from) {
    UnfreezeFileTable unfreezeFiles(*this);
    // id 0 is for non-existing FileRef
    for (int fileIdx = 1; fileIdx < from.filesUsed(); fileIdx++) {
        if (from.files[fileIdx]->sourceType == File::Type::NotYetRead) {
            continue;
        }
        if (fileIdx < this->filesUsed() && from.files[fileIdx].get() == this->files[fileIdx].get()) {
            continue;
        }
        ENFORCE(fileIdx >= this->filesUsed() || this->files[fileIdx]->sourceType == File::Type::NotYetRead);
        this->enterNewFileAt(from.files[fileIdx], fileIdx);
    }
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
    if (loc.file().exists() && ignoredForSuggestTypedErrorClasses.count(error->what.code) == 0) {
        loc.file().data(*this).minErrorLevel_ = min(loc.file().data(*this).minErrorLevel_, error->what.minLevel);
    }

    errorQueue->pushError(*this, move(error));
}

bool GlobalState::hadCriticalError() const {
    return errorQueue->hadCritical;
}

ErrorBuilder GlobalState::beginError(Loc loc, ErrorClass what) const {
    if (what == errors::Internal::InternalError) {
        Exception::failInFuzzer();
    }
    return ErrorBuilder(*this, shouldReportErrorOn(loc, what), loc, what);
}

void GlobalState::ignoreErrorClassForSuggestTyped(int code) {
    ignoredForSuggestTypedErrorClasses.insert(code);
}

void GlobalState::suppressErrorClass(int code) {
    ENFORCE(onlyErrorClasses.empty());
    suppressedErrorClasses.insert(code);
}

void GlobalState::onlyShowErrorClass(int code) {
    ENFORCE(suppressedErrorClasses.empty());
    onlyErrorClasses.insert(code);
}

bool GlobalState::shouldReportErrorOn(Loc loc, ErrorClass what) const {
    if (what.minLevel == StrictLevel::Internal) {
        return true;
    }
    if (this->silenceErrors) {
        return false;
    }
    if (suppressedErrorClasses.count(what.code) != 0) {
        return false;
    }
    if (!onlyErrorClasses.empty() && onlyErrorClasses.count(what.code) == 0) {
        return false;
    }
    if (!lspQuery.isEmpty()) {
        // LSP queries throw away the errors anyways (only cares about the QueryResponses)
        // so it's no use spending time computing better error messages.
        return false;
    }

    StrictLevel level = StrictLevel::Strong;
    if (loc.file().exists()) {
        level = loc.file().data(*this).strictLevel;
    }
    if (level >= StrictLevel::Max) {
        // Custom rules
        if (level == StrictLevel::Autogenerated) {
            level = StrictLevel::True;
            if (what == errors::Resolver::StubConstant || what == errors::Infer::UntypedMethod) {
                return false;
            }
        } else if (level == StrictLevel::Stdlib) {
            level = StrictLevel::Strict;
            if (what == errors::Resolver::OverloadNotAllowed || what == errors::Resolver::VariantTypeMemberInClass ||
                what == errors::Infer::UntypedMethod) {
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

void GlobalState::replaceFile(FileRef whatFile, const shared_ptr<File> &withWhat) {
    ENFORCE(whatFile.id() < filesUsed());
    ENFORCE(whatFile.dataAllowingUnsafe(*this).path() == withWhat->path());
    files[whatFile.id()] = withWhat;
}

FileRef GlobalState::findFileByPath(string_view path) const {
    auto fnd = fileRefByPath.find(string(path));
    if (fnd != fileRefByPath.end()) {
        return fnd->second;
    }
    return FileRef();
}

const packages::PackageDB &GlobalState::packageDB() const {
    return packageDB_;
}

void GlobalState::setPackagerOptions(const std::vector<std::string> &secondaryTestPackageNamespaces,
                                     const std::vector<std::string> &extraPackageFilesDirectoryPrefixes,
                                     std::string errorHint) {
    ENFORCE(packageDB_.secondaryTestPackageNamespaceRefs_.size() == 0);
    ENFORCE(!packageDB_.frozen);

    for (const string &ns : secondaryTestPackageNamespaces) {
        packageDB_.secondaryTestPackageNamespaceRefs_.emplace_back(enterNameConstant(ns));
    }

    packageDB_.extraPackageFilesDirectoryPrefixes_ = extraPackageFilesDirectoryPrefixes;
    packageDB_.errorHint_ = errorHint;
}

packages::UnfreezePackages GlobalState::unfreezePackages() {
    return packageDB_.unfreeze();
}

unique_ptr<GlobalState> GlobalState::markFileAsTombStone(unique_ptr<GlobalState> what, FileRef fref) {
    ENFORCE(fref.id() < what->filesUsed());
    what->files[fref.id()]->sourceType = File::Type::TombStone;
    return what;
}

uint32_t patchHash(uint32_t hash) {
    if (hash == GlobalStateHash::HASH_STATE_NOT_COMPUTED) {
        hash = GlobalStateHash::HASH_STATE_NOT_COMPUTED_COLLISION_AVOID;
    } else if (hash == GlobalStateHash::HASH_STATE_INVALID) {
        hash = GlobalStateHash::HASH_STATE_INVALID_COLLISION_AVOID;
    }
    return hash;
}

unique_ptr<GlobalStateHash> GlobalState::hash() const {
    constexpr bool DEBUG_HASHING_TAIL = false;
    uint32_t hierarchyHash = 0;
    UnorderedMap<NameHash, uint32_t> methodHashes;
    int counter = 0;

    for (const auto *symbolType : {&this->classAndModules, &this->typeArguments, &this->typeMembers}) {
        counter = 0;
        for (const auto &sym : *symbolType) {
            if (!sym.ignoreInHashing(*this)) {
                hierarchyHash = mix(hierarchyHash, sym.hash(*this));
                counter++;
                if (DEBUG_HASHING_TAIL && counter > symbolType->size() - 15) {
                    errorQueue->logger.info("Hashing symbols: {}, {}", hierarchyHash, sym.name.show(*this));
                }
            }
        }
    }

    counter = 0;
    for (const auto &field : this->fields) {
        counter++;
        // No fields are ignored in hashing.
        hierarchyHash = mix(hierarchyHash, field.hash(*this));

        if (DEBUG_HASHING_TAIL && counter > this->fields.size() - 15) {
            errorQueue->logger.info("Hashing symbols: {}, {}", hierarchyHash, field.name.show(*this));
        }
    }

    counter = 0;
    for (const auto &sym : this->methods) {
        if (!sym.ignoreInHashing(*this)) {
            auto &target = methodHashes[NameHash(*this, sym.name)];
            target = mix(target, sym.hash(*this));
            hierarchyHash = mix(hierarchyHash, sym.methodShapeHash(*this));
            counter++;
            if (DEBUG_HASHING_TAIL && counter > this->methods.size() - 15) {
                errorQueue->logger.info("Hashing method symbols: {}, {}", hierarchyHash, sym.name.show(*this));
            }
        }
    }

    unique_ptr<GlobalStateHash> result = make_unique<GlobalStateHash>();
    result->methodHashes.reserve(methodHashes.size());
    for (const auto &e : methodHashes) {
        result->methodHashes.emplace_back(e.first, patchHash(e.second));
    }
    // Sort the hashes. Semantically important for quickly diffing hashes.
    fast_sort(result->methodHashes);

    result->hierarchyHash = patchHash(hierarchyHash);
    return result;
}

const vector<shared_ptr<File>> &GlobalState::getFiles() const {
    return files;
}

MethodRef GlobalState::staticInitForClass(ClassOrModuleRef klass, Loc loc) {
    auto prevCount = methodsUsed();
    auto sym = enterMethodSymbol(loc, klass.data(*this)->singletonClass(*this), core::Names::staticInit());
    if (prevCount != methodsUsed()) {
        auto blkLoc = core::Loc::none(loc.file());
        auto &blkSym = enterMethodArgumentSymbol(blkLoc, sym, core::Names::blkArg());
        blkSym.flags.isBlock = true;
    }
    return sym;
}

MethodRef GlobalState::lookupStaticInitForClass(ClassOrModuleRef klass) const {
    auto classData = klass.data(*this);
    auto ref = classData->lookupSingletonClass(*this).data(*this)->findMethod(*this, core::Names::staticInit());
    ENFORCE(ref.exists(), "looking up non-existent <static-init> for {}", klass.toString(*this));
    return ref;
}

MethodRef GlobalState::staticInitForFile(Loc loc) {
    auto nm = freshNameUnique(core::UniqueNameKind::Namer, core::Names::staticInit(), loc.file().id());
    auto prevCount = this->methodsUsed();
    auto sym = enterMethodSymbol(loc, core::Symbols::rootSingleton(), nm);
    if (prevCount != this->methodsUsed()) {
        auto blkLoc = core::Loc::none(loc.file());
        auto &blkSym = this->enterMethodArgumentSymbol(blkLoc, sym, core::Names::blkArg());
        blkSym.flags.isBlock = true;
    }
    return sym;
}

MethodRef GlobalState::lookupStaticInitForFile(Loc loc) const {
    auto nm = lookupNameUnique(core::UniqueNameKind::Namer, core::Names::staticInit(), loc.file().id());
    auto ref = core::Symbols::rootSingleton().data(*this)->findMember(*this, nm);
    ENFORCE(ref.exists(), "looking up non-existent <static-init> for {}", loc.toString(*this));
    return ref.asMethodRef();
}

spdlog::logger &GlobalState::tracer() const {
    return errorQueue->tracer;
}

} // namespace sorbet::core
