#include "common/common.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "absl/strings/escaping.h"

using namespace std;

enum class NameKind : uint8_t {
    UTF8 = 1,
    UNIQUE = 2,
    CONSTANT = 3,
};

struct NameDef {
    int id;
    string srcName;
    string val;
    NameKind kind;

    NameDef(string_view srcName, string_view val, NameKind kind)
        : srcName(string(srcName)), val(string(val)), kind(kind) {}
    NameDef(string_view srcName, string_view val) : srcName(string(srcName)), val(string(val)), kind(NameKind::UTF8) {
        if (srcName == val) {
            throw std::logic_error("Only pass one arg for '" + string(val) + "'");
        }
    }
    NameDef(string_view srcName) : srcName(string(srcName)), val(string(srcName)), kind(NameKind::UTF8){};
};

NameDef names[] = {
    {"initialize"},
    {"andAnd", "&&"},
    {"orOr", "||"},
    {"toS", "to_s"},
    {"toA", "to_a"},
    {"toAry", "to_ary"},
    {"toH", "to_h"},
    {"toHash", "to_hash"},
    {"toProc", "to_proc"},
    {"concat"},
    {"key_p", "key?"},
    {"intern"},
    {"call"},
    {"bang", "!"},
    {"squareBrackets", "[]"},
    {"squareBracketsEq", "[]="},
    {"unaryPlus", "+@"},
    {"unaryMinus", "-@"},
    {"plus", "+"},
    {"minus", "-"},
    {"star", "*"},
    {"starStar", "**"},
    {"ampersand", "&"},
    {"tripleEq", "==="},
    {"orOp", "|"},
    {"backtick", "`"},
    {"defined_p", "defined?"},
    {"undef"},
    {"each"},
    {"subclasses"},
    {"transpose"},
    {"caseWhen"},

    // Used in parser for error recovery
    {"methodNameMissing", "<method-name-missing>"},
    {"methodDefNameMissing", "<method-def-name-missing>"},
    {"ivarNameMissing", "@<ivar-name-missing>"},
    {"cvarNameMissing", "@@<cvar-name-missing>"},
    {"ConstantNameMissing", "<ConstantNameMissing>", NameKind::CONSTANT},
    {"ErrorNode", "<ErrorNode>", NameKind::CONSTANT},
    {"dynamicConstAssign", "<dynamic-const-assign>"},

    // used in CFG for temporaries
    {"whileTemp", "<whileTemp>"},
    {"ifTemp", "<ifTemp>"},
    {"returnTemp", "<returnTemp>"},
    {"statTemp", "<statTemp>"},
    {"assignTemp", "<assignTemp>"},
    {"returnMethodTemp", "<returnMethodTemp>"},
    {"debugEnvironmentTemp", "<debugEnvironmentTemp>"},
    {"blockReturnTemp", "<blockReturnTemp>"},
    {"nextTemp", "<nextTemp>"},
    {"selfMethodTemp", "<selfMethodTemp>"},
    {"selfLocal", "<self>"},
    {"selfRestore", "<selfRestore>"}, // used to restore self in case block changes it
    {"hashTemp", "<hashTemp>"},
    {"arrayTemp", "<arrayTemp>"},
    {"rescueTemp", "<rescueTemp>"},
    {"exceptionValue", "<exceptionValue>"},
    {"gotoDeadTemp", "<gotoDeadTemp>"},
    {"exceptionClassTemp", "<exceptionClassTemp>"},
    {"isaCheckTemp", "<isaCheckTemp>"},
    {"keepForCfgTemp", "<keepForCfgTemp>"},
    {"retryTemp", "<retryTemp>"},
    {"throwAwayTemp", "<throwAwayTemp>"},
    {"castTemp", "<castTemp>"},
    {"finalReturn", "<finalReturn>"},
    {"cfgAlias", "<cfgAlias>"},
    {"magic", "<magic>"},
    {"unconditional", "<unconditional>"},
    {"argPresent", "<argPresent>"},
    // end CFG temporaries

    {"include"},
    {"extend"},
    {"currentFile", "__FILE__"},
    {"merge"},

    // T keywords
    {"sig"},
    {"typeParameters", "type_parameters"},
    {"typeParameter", "type_parameter"},
    {"abstract"},
    {"implementation"},
    {"override_", "override"},
    {"overridable"},
    {"allowIncompatible", "allow_incompatible"},
    {"sigForMethod"},

    // Sig builders
    {"bind"},
    {"params"},
    {"final_", "final"},
    {"returns"},
    {"packagePrivate", "package_private"},
    {"packagePrivateClassMethod", "package_private_class_method"},
    {"void_", "void"},
    {"VOID", "VOID", NameKind::CONSTANT},
    {"checked"},
    {"never"},
    {"onFailure", "on_failure"},

    {"all"},
    {"any"},
    {"enum_", "enum"},
    {"deprecatedEnum", "deprecated_enum"},
    {"enums"},
    {"serialize"},
    {"nilable"},
    {"proc"},
    {"untyped"},
    {"noreturn"},
    {"anything"},
    {"singletonClass", "singleton_class"},
    {"class_", "class"},
    {"classOf", "class_of"},
    {"selfType", "self_type"},
    {"experimentalAttachedClass", "experimental_attached_class"},
    {"attachedClass", "attached_class"},
    {"coerce"},

    {"assertType", "assert_type!"},
    {"cast"},
    {"let"},
    {"uncheckedLet", "<unchecked_let>"},
    {"syntheticBind", "<synthetic bind>"},
    {"assumeType", "<assume type>"},
    {"unsafe"},
    {"must"},
    {"mustBecause", "must_because"},
    {"declareInterface", "interface!"},
    {"declareAbstract", "abstract!"},
    {"declareFinal", "final!"},
    {"declareSealed", "sealed!"},
    {"revealType", "reveal_type"},
    {"absurd"},
    {"nonForcingIsA_p", "non_forcing_is_a?"},
    {"valid_p", "valid?"},
    {"recursivelyValid_p", "recursively_valid?"},
    {"subtypeOf_p", "subtype_of?"},
    {"describeObj", "describe_obj"},
    {"errorMessageForObj", "error_message_for_obj"},
    {"errorMessageForObjRecursive", "error_message_for_obj_recursive"},
    {"validate_bang", "validate!"},
    // end T keywords

    // Ruby DSL methods which we understand
    {"attr"},
    {"attrAccessor", "attr_accessor"},
    {"attrWriter", "attr_writer"},
    {"attrReader", "attr_reader"},
    {"private_", "private"},
    {"protected_", "protected"},
    {"public_", "public"},
    {"privateClassMethod", "private_class_method"},
    {"publicClassMethod", "public_class_method"},
    {"privateConstant", "private_constant"},
    {"moduleFunction", "module_function"},
    {"aliasMethod", "alias_method"},

    {"flatfile"},
    {"from"},
    {"field"},
    {"pattern"},

    // type alias names
    {"typeAlias", "type_alias"},
    {"typeMember", "type_member"},
    {"typeTemplate", "type_template"},
    {"covariant", "out"},
    {"contravariant", "in"},
    {"fixed"},
    {"lower"},
    {"upper"},
    {"declareHasAttachedClass", "has_attached_class!"},

    {"prop"},
    {"tokenProp", "token_prop"},
    {"timestampedTokenProp", "timestamped_token_prop"},
    {"createdProp", "created_prop"},
    {"updatedProp", "updated_prop"},
    {"merchantProp", "merchant_prop"},
    {"merchantTokenProp", "merchant_token_prop"},
    {"name"},
    {"encryptedProp", "encrypted_prop"},
    {"array"},
    {"defDelegator", "def_delegator"},
    {"defDelegators", "def_delegators"},
    {"delegate"},
    {"type"},
    {"optional"},
    {"immutable"},
    {"migrate"},
    {"default_", "default"},
    {"const_", "const"},
    {"token"},
    {"created"},
    {"updated"},
    {"merchant"},
    {"foreign"},
    {"allowDirectMutation", "allow_direct_mutation"},
    {"ifunset"},
    {"withoutAccessors", "without_accessors"},
    {"instanceVariableGet", "instance_variable_get"},
    {"instanceVariableSet", "instance_variable_set"},
    {"decorator"},
    {"propGetLogic", "prop_get_logic"},
    {"propFreezeHandler", "prop_freeze_handler"},
    {"computedBy", "computed_by"},
    {"factory"},
    {"InexactStruct", "InexactStruct", NameKind::CONSTANT},
    {"ImmutableStruct", "ImmutableStruct", NameKind::CONSTANT},
    {"Private", "Private", NameKind::CONSTANT},
    {"Types", "Types", NameKind::CONSTANT},
    {"Methods", "Methods", NameKind::CONSTANT},
    {"DeclBuilder", "DeclBuilder", NameKind::CONSTANT},

    {"prefix"},
    {"to"},

    {"mattrAccessor", "mattr_accessor"},
    {"mattrReader", "mattr_reader"},
    {"mattrWriter", "mattr_writer"},
    {"cattrAccessor", "cattr_accessor"},
    {"cattrReader", "cattr_reader"},
    {"cattrWriter", "cattr_writer"},
    {"threadMattrAccessor", "thread_mattr_accessor"},
    {"threadCattrAccessor", "thread_cattr_accessor"},
    {"instanceReader", "instance_reader"},
    {"instanceWriter", "instance_writer"},
    {"instanceAccessor", "instance_accessor"},
    {"instancePredicate", "instance_predicate"},
    {"classAttribute", "class_attribute"},

    {"describe"},
    {"it"},
    {"before"},
    {"beforeAngles", "<before>"},
    {"after"},
    {"afterAngles", "<after>"},
    {"testEach", "test_each"},
    {"testEachHash", "test_each_hash"},
    {"constSet", "const_set"},
    {"collect"},

    {"dslOptional", "dsl_optional"},
    {"dslRequired", "dsl_required"},
    {"implied"},
    {"skipGetter", "skip_getter"},
    {"skipSetter", "skip_setter"},

    {"registered"},
    {"instanceRegistered", "<instance_registered>"},
    {"helpers"},

    {"keywordInit", "keyword_init"},

    {"DB", "DB", NameKind::CONSTANT},
    {"Model", "Model", NameKind::CONSTANT},
    {"Mixins", "Mixins", NameKind::CONSTANT},
    {"Encryptable", "Encryptable", NameKind::CONSTANT},
    {"EncryptedValue", "EncryptedValue", NameKind::CONSTANT},
    {"Command", "Command", NameKind::CONSTANT},
    {"Enum", "Enum", NameKind::CONSTANT},

    {"ActiveRecord", "ActiveRecord", NameKind::CONSTANT},
    {"Migration", "Migration", NameKind::CONSTANT},
    {"Compatibility", "Compatibility", NameKind::CONSTANT},
    {"ActiveSupport", "ActiveSupport", NameKind::CONSTANT},
    {"Concern", "Concern", NameKind::CONSTANT},

    {"instance"},
    {"singletonClassInstance", "singleton class instance"},
    {"normal"},
    {"genericPropGetter"},

    {"raise"},
    {"fail"},
    {"rewriterRaiseUnimplemented", "Sorbet rewriter pass partially unimplemented"},

    {"test"},
    {"setup"},
    {"teardown"},
    // end DSL methods

    // The next two names are used as keys in SymbolInfo::members to store
    // pointers up and down the singleton-class hierarchy. If A's singleton
    // class is B, then A will have a `singletonClass` entry in its members
    // table which references B, and B will have an `attachedClass` entry
    // pointing at A.
    //
    // The "attached class" terminology is borrowed from MRI, which refers
    // to the unique instance attached to a singleton class as the "attached
    // object"
    {"singleton", "<singleton class>"},
    {"attached", "<attached class>"},

    // Requires ancestor
    {"requiredAncestors", "<required-ancestor>"},
    {"requiredAncestorsLin", "<required-ancestor-lin>"},
    {"requiresAncestor", "requires_ancestor"},

    // This behaves like the above two names, in the sense that we use a member
    // on a class to lookup an associated symbol with some extra info.
    {"sealedSubclasses", "sealed_subclasses"},

    // Used to store arguments to a "mixes_in_class_methods()" call
    {"mixedInClassMethods", "<mixed_in_class_methods>"},
    {"mixesInClassMethods", "mixes_in_class_methods"},
    {"ClassMethods", "ClassMethods", NameKind::CONSTANT},
    {"classMethods", "class_methods"},

    {"blockTemp", "<block>"},
    {"blockPreCallTemp", "<block-pre-call-temp>"},
    {"blockPassTemp", "<block-pass>"},
    {"forTemp"},
    {"new_", "new"},
    {"blockCall", "<block-call>"},
    {"blockBreakAssign", "<block-break-assign>"},
    {"arg", "<arg>"},
    {"kwargs", "<kwargs>"},
    {"blkArg", "<blk>"},
    {"blockGiven_p", "block_given?"},
    {"anonymousBlock", "<anonymous-block>"},

    // Method names known to Data
    {"define"},

    // Used to generate temporary names for destructuring arguments ala proc do
    //  |(x,y)|; end
    {"destructureArg", "<destructure>"},

    {"lambda"},
    {"lambdaTLet", "<lambda T.let>"},
    {"returnType", "return_type"},
    {"nil_p", "nil?"},
    {"blank_p", "blank?"},
    {"present_p", "present?"},
    {"nil"},
    {"super", "<super>"},
    {"untypedSuper", "<untypedSuper>"},
    {"empty", ""},

    {"buildHash", "<build-hash>"},
    {"buildArray", "<build-array>"},
    {"buildRange", "<build-range>"},
    {"mergeHash", "<merge-hash>"},
    {"mergeHashValues", "<merge-hash-values>"},
    {"toHashDup", "<to-hash-dup>"},
    {"toHashNoDup", "<to-hash-nodup>"},
    {"splat", "<splat>"},
    {"expandSplat", "<expand-splat>"},
    {"suggestConstantType", "<suggest-constant-type>"},
    {"suggestFieldType", "<suggest-field-type>"},
    {"checkMatchArray", "<check-match-array>"},
    {"definedInstanceVar", "<defined-instance-var>"},
    {"definedClassVar", "<defined-class-var>"},
    {"arg0"},
    {"arg1"},
    {"arg2"},
    {"arg3"},
    {"opts"},
    {"args"},
    {"Elem", "Elem", NameKind::CONSTANT},
    {"keepForIde", "keep_for_ide"},
    {"keepDef", "keep_def"},
    {"keepSelfDef", "keep_self_def"},
    {"keepForCfg", "<keep-for-cfg>"},
    {"retry", "<retry>"},
    {"unresolvedAncestors", "<unresolved-ancestors>"},
    {"defineTopClassOrModule", "<define-top-class-or-module>"},
    {"nilForSafeNavigation", "<nil-for-safe-navigation>"},
    {"checkAndAnd", "<check-and-and>"},

    {"isA_p", "is_a?"},
    {"kindOf_p", "kind_of?"},
    {"lessThan", "<"},
    {"greaterThan", ">"},
    {"equal_p", "equal?"},
    {"eqeq", "=="},
    {"neq", "!="},
    {"leq", "<="},
    {"geq", ">="},

    // methods that are known by tuple and\or shape types
    {"freeze"},
    {"last"},
    {"first"},
    {"min"},
    {"max"},
    {"sum"},
    {"sample"},
    {"at"},

    // Argument forwarding
    {"fwdArgs", "<fwd-args>"},
    {"fwdKwargs", "<fwd-kwargs>"},
    {"fwdBlock", "<fwd-block>"},

    // Enumerable#flat_map has special-case logic in Infer
    {"flatMap", "flat_map"},

    // Array#dig, Array#flatten, #product, #compact and #zip are also custom-implemented
    {"dig"},
    {"flatten"},
    {"product"},
    {"compact"},
    {"zip"},

    // Pattern matching
    {"patternMatch", "<pattern-match>"},

    {"regexBackref", "<regex-backref>"},

    {"staticInit", "<static-init>"},

    {"require"},
    {"callWithSplat", "<call-with-splat>"},
    {"callWithBlock", "<call-with-block>"},
    {"callWithSplatAndBlock", "<call-with-splat-and-block>"},
    {"enumerableToH", "enumerable_to_h"},
    {"blockBreak", "<block-break>"},
    {"stringInterpolate", "<string-interpolate>"},

    // Packager
    {"import"},
    {"testImport", "test_import"},
    {"export_", "export"},
    {"restrictToService", "restrict_to_service"},
    {"legacy"},
    {"strict"},
    {"visibleTo", "visible_to"},
    {"tests"},
    {"exportAll", "export_all!"},
    {"PackageSpec", "PackageSpec", NameKind::CONSTANT},
    {"PackageSpecRegistry", "<PackageSpecRegistry>", NameKind::CONSTANT},

    // GlobalState initEmpty()
    {"Top", "T.anything", NameKind::CONSTANT},
    {"Bottom", "T.noreturn", NameKind::CONSTANT},
    {"Untyped", "T.untyped", NameKind::CONSTANT},
    {"Root", "<root>", NameKind::CONSTANT},
    {"Object", "Object", NameKind::CONSTANT},
    {"String", "String", NameKind::CONSTANT},
    {"Integer", "Integer", NameKind::CONSTANT},
    {"Float", "Float", NameKind::CONSTANT},
    {"Numeric", "Numeric", NameKind::CONSTANT},
    {"Symbol", "Symbol", NameKind::CONSTANT},
    {"Array", "Array", NameKind::CONSTANT},
    {"Hash", "Hash", NameKind::CONSTANT},
    {"Proc", "Proc", NameKind::CONSTANT},
    {"TrueClass", "TrueClass", NameKind::CONSTANT},
    {"FalseClass", "FalseClass", NameKind::CONSTANT},
    {"Boolean", "Boolean", NameKind::CONSTANT},
    {"NilClass", "NilClass", NameKind::CONSTANT},
    {"Class", "Class", NameKind::CONSTANT},
    {"Module", "Module", NameKind::CONSTANT},
    {"Time", "Time", NameKind::CONSTANT},
    {"Todo", "<todo sym>", NameKind::CONSTANT},
    {"TodoMethod", "<todo method>", NameKind::UTF8},
    {"TodoTypeArgument", "<todo typeargument>", NameKind::CONSTANT},
    {"NoSymbol", "<none>", NameKind::CONSTANT},
    {"noFieldOrStaticField", "<no-field-or-static-field>", NameKind::UTF8},
    {"noMethod", "<no-method>", NameKind::UTF8},
    {"NoTypeArgument", "<no-type-argument>", NameKind::CONSTANT},
    {"NoTypeMember", "<no-type-member>", NameKind::CONSTANT},
    {"Opus", "Opus", NameKind::CONSTANT},
    {"T", "T", NameKind::CONSTANT},
    {"BasicObject", "BasicObject", NameKind::CONSTANT},
    {"Kernel", "Kernel", NameKind::CONSTANT},
    {"Range", "Range", NameKind::CONSTANT},
    {"Regexp", "Regexp", NameKind::CONSTANT},
    {"Exception", "Exception", NameKind::CONSTANT},
    {"StandardError", "StandardError", NameKind::CONSTANT},
    {"Complex", "Complex", NameKind::CONSTANT},
    {"Rational", "Rational", NameKind::CONSTANT},
    // A magic non user-creatable class with methods to keep state between passes
    {"Magic", "<Magic>", NameKind::CONSTANT},
    // A magic non user-creatable class to attach symbols for blaming untyped to
    {"UntypedSource", "<UntypedSource>", NameKind::CONSTANT},
    {"tupleUnderlying", "<tupleUnderlying>", NameKind::CONSTANT},
    {"shapeUnderlying", "<shapeUnderlying>", NameKind::CONSTANT},
    {"tupleLub", "<tupleLub>", NameKind::CONSTANT},
    {"shapeLub", "<shapeLub>", NameKind::CONSTANT},
    {"YieldLoadArg", "<YieldLoadArg>", NameKind::CONSTANT},
    {"GetCurrentException", "<GetCurrentException>", NameKind::CONSTANT},
    {"LoadYieldParams", "<LoadYieldParams>", NameKind::CONSTANT},
    {"shapeSquareBracketsEq", "<shapeSquareBracketsEq>", NameKind::CONSTANT},
    // A magic non user-creatable class for binding procs to attached_class
    {"BindToAttachedClass", "<BindToAttachedClass>", NameKind::CONSTANT},
    // A magic non user-creatable class for binding procs to self_type
    {"BindToSelfType", "<BindToSelfType>", NameKind::CONSTANT},
    // A magic non user-creatable class for mimicking the decl builder during cfg
    // construction
    {"DeclBuilderForProcs", "<DeclBuilderForProcs>", NameKind::CONSTANT},
    {"Enumerable", "Enumerable", NameKind::CONSTANT},
    {"Enumerator", "Enumerator", NameKind::CONSTANT},
    {"Lazy", "Lazy", NameKind::CONSTANT},
    {"Chain", "Chain", NameKind::CONSTANT},
    {"Set", "Set", NameKind::CONSTANT},
    {"Struct", "Struct", NameKind::CONSTANT},
    {"Data", "Data", NameKind::CONSTANT},
    {"File", "File", NameKind::CONSTANT},
    {"Encoding", "Encoding", NameKind::CONSTANT},
    {"getEncoding", "<get-encoding>"},
    {"Static", "Static", NameKind::CONSTANT},
    {"StubModule", "<StubModule>", NameKind::CONSTANT},
    {"StubSuperClass", "<StubSuperClass>", NameKind::CONSTANT},
    {"StubMixin", "<StubMixin>", NameKind::CONSTANT},
    {"PlaceholderMixin", "<PlaceholderMixin>", NameKind::CONSTANT},
    {"Base", "Base", NameKind::CONSTANT},
    {"Void", "Void", NameKind::CONSTANT},
    {"TypeAlias", "<TypeAlias>", NameKind::CONSTANT},
    {"Generic", "Generic", NameKind::CONSTANT},
    {"Tuple", "Tuple", NameKind::CONSTANT},
    {"Shape", "Shape", NameKind::CONSTANT},
    {"Subclasses", "SUBCLASSES", NameKind::CONSTANT},
    {"Sorbet", "Sorbet", NameKind::CONSTANT},
    {"ReturnTypeInference", "ReturnTypeInference", NameKind::CONSTANT},
    {"ResolvedSig", "ResolvedSig", NameKind::CONSTANT},
    {"InferredReturnType", "INFERRED_RETURN_TYPE", NameKind::CONSTANT},
    {"InferredArgumentType", "INFERRED_ARGUMENT_TYPE", NameKind::CONSTANT},
    {"ImplicitModuleSuperclass", "ImplicitModuleSuperclass", NameKind::CONSTANT},
    {"guessedTypeTypeParameterHolder", "guessed_type_type_parameter_holder"},
    {"Builder", "Builder", NameKind::CONSTANT},
    {"Sig", "Sig", NameKind::CONSTANT},
    {"Utils", "Utils", NameKind::CONSTANT},
    {"UndeclaredFieldStub", "<undeclared-field-stub>", NameKind::CONSTANT},
    {"badAliasMethodStub", "<bad-method-alias-stub>"},
    {"Helpers", "Helpers", NameKind::CONSTANT},
    {"Net", "Net", NameKind::CONSTANT},
    {"IMAP", "IMAP", NameKind::CONSTANT},
    {"Protocol", "Protocol", NameKind::CONSTANT},
    {"WithoutRuntime", "WithoutRuntime", NameKind::CONSTANT},
    {"Singleton", "Singleton", NameKind::CONSTANT},
    {"AttachedClass", "<AttachedClass>", NameKind::CONSTANT},
    {"NonForcingConstants", "NonForcingConstants", NameKind::CONSTANT},
    {"VERSION", "VERSION", NameKind::CONSTANT},
    {"Thread", "Thread", NameKind::CONSTANT},
    {"Configuration", "Configuration", NameKind::CONSTANT},
    {"Test", "Test", NameKind::CONSTANT},
    {"Autogen", "Autogen", NameKind::CONSTANT},
    {"Tokens", "Tokens", NameKind::CONSTANT},
    {"AccountModelMerchant", "AccountModelMerchant", NameKind::CONSTANT},
    {"Token", "Token", NameKind::CONSTANT},
    {"Account", "Account", NameKind::CONSTANT},
    {"Merchant", "Merchant", NameKind::CONSTANT},

    // Typos
    {"Int", "Int", NameKind::CONSTANT},
    {"Timestamp", "Timestamp", NameKind::CONSTANT},
    {"Bool", "Bool", NameKind::CONSTANT},
};

string_view kindToString(NameKind kind) {
    switch (kind) {
        case NameKind::UTF8:
            return "UTF8"sv;
        case NameKind::UNIQUE:
            return "UNIQUE"sv;
        case NameKind::CONSTANT:
            return "CONSTANT"sv;
    }
}

void emit_name_header(ostream &out, NameDef &name) {
    out << "#ifndef NAME_" << name.srcName << '\n';
    out << "#define NAME_" << name.srcName << '\n';
    out << "    // \"" << name.val << "\"" << '\n';
    out << "    static inline constexpr NameRef " << name.srcName << "() {" << '\n';
    out << "        return NameRef(NameRef::WellKnown{}, NameKind::" << (kindToString(name.kind)) << ", " << name.id
        << ");" << '\n';
    out << "    }" << '\n';
    out << "#endif" << '\n';
    out << '\n';
}

void emit_name_string(ostream &out, NameDef &name) {
    out << "const char *" << name.srcName << " = \"";
    out << absl::CEscape(name.val) << "\";" << '\n';

    out << "std::string_view " << name.srcName << "_DESC{(char*)";
    out << name.srcName << "," << name.val.size() << "};" << '\n';
    out << '\n';
}

string_view kindToEnterMethod(NameKind kind) {
    switch (kind) {
        case NameKind::UTF8:
            return "enterNameUTF8"sv;
        case NameKind::CONSTANT:
            return "enterNameConstant"sv;
        case NameKind::UNIQUE:
            return "freshNameUnique"sv;
    }
}

string_view kindToIndexMethod(NameKind kind) {
    switch (kind) {
        case NameKind::UTF8:
            return "utf8Index"sv;
        case NameKind::CONSTANT:
            return "constantIndex"sv;
        case NameKind::UNIQUE:
            return "uniqueIndex"sv;
    }
}

void emit_register(ostream &out) {
    out << "void registerNames(GlobalState &gs) {" << '\n';
    for (auto &name : names) {
        auto fun = kindToEnterMethod(name.kind);
        out << "    NameRef " << name.srcName << "_id = gs." << fun << "(" << name.srcName << "_DESC";
        if (name.kind == NameKind::UNIQUE) {
            out << ", UniqueNameKind::WellKnown, 1);";
        }
        out << ");" << '\n';
    }
    out << '\n';
    for (auto &name : names) {
        out << "    ENFORCE(" << name.srcName << "_id." << kindToIndexMethod(name.kind) << "() == " << name.id
            << "); /* " << name.srcName << "() */" << '\n';
    }
    out << '\n';
    out << "}" << '\n';
}

int main(int argc, char **argv) {
    int constantI = 0;
    int utf8I = 0;
    int uniqueI = 0;
    for (auto &name : names) {
        switch (name.kind) {
            case NameKind::UTF8:
                name.id = utf8I++;
                break;
            case NameKind::UNIQUE:
                // Does not increment utf8I. Unique names must pre-declare the utf8 name backing them.
                name.id = uniqueI++;
                break;
            case NameKind::CONSTANT:
                utf8I++;
                name.id = constantI++;
                break;
        }
    }
    int lastConstantId = constantI;
    int lastUtf8Id = utf8I;
    int lastUniqueId = uniqueI;

    // emit header file
    {
        ofstream header(argv[1], ios::trunc);
        if (!header.good()) {
            cerr << "unable to open " << argv[1] << '\n';
            return 1;
        }
        header << "#include \"core/NameRef.h\"" << '\n' << '\n';
        header << "namespace sorbet {" << '\n';
        header << "namespace core {" << '\n';
        header << "class GlobalState;" << '\n';
        header << "namespace Names {" << '\n';

        for (auto &name : names) {
            if (name.kind == NameKind::UTF8) {
                emit_name_header(header, name);
            }
        }

        header << "namespace Constants {" << '\n';
        for (auto &name : names) {
            if (name.kind == NameKind::CONSTANT) {
                emit_name_header(header, name);
            }
        }
        header << "}" << '\n';

        header << "namespace Uniques {" << '\n';
        for (auto &name : names) {
            if (name.kind == NameKind::UNIQUE) {
                emit_name_header(header, name);
            }
        }
        header << "}" << '\n';

        header << "#ifndef NAME_LAST_WELL_KNOWN_CONSTANT_NAME" << '\n';
        header << "#define NAME_LAST_WELL_KNOWN_CONSTANT_NAME" << '\n';
        header << "constexpr int LAST_WELL_KNOWN_CONSTANT_NAME = " << lastConstantId << ";" << '\n';
        header << "#endif" << '\n';

        header << "#ifndef NAME_LAST_WELL_KNOWN_UTF8_NAME" << '\n';
        header << "#define NAME_LAST_WELL_KNOWN_UTF8_NAME" << '\n';
        header << "constexpr int LAST_WELL_KNOWN_UTF8_NAME = " << lastUtf8Id << ";" << '\n';
        header << "#endif" << '\n';

        header << "#ifndef NAME_LAST_WELL_KNOWN_UNIQUE_NAME" << '\n';
        header << "#define NAME_LAST_WELL_KNOWN_UNIQUE_NAME" << '\n';
        header << "constexpr int LAST_WELL_KNOWN_UNIQUE_NAME = " << lastUniqueId << ";" << '\n';
        header << "#endif" << '\n';

        header << "    void registerNames(GlobalState &gs);" << '\n';
        header << "}" << '\n';
        header << "}" << '\n';
        header << "}" << '\n';
    }

    // emit initialization .cc file
    {
        ofstream classfile(argv[2], ios::trunc);
        if (!classfile.good()) {
            cerr << "unable to open " << argv[2] << '\n';
            return 1;
        }
        classfile << "#include \"core/GlobalState.h\"" << '\n' << '\n';
        classfile << "#include \"core/Names.h\"" << '\n' << '\n';
        classfile << "#include \"core/Names_gen.h\"" << '\n' << '\n';
        classfile << "namespace sorbet {" << '\n';
        classfile << "namespace core {" << '\n';
        classfile << "namespace Names {" << '\n';
        classfile << "namespace {" << '\n';
        for (auto &name : names) {
            emit_name_string(classfile, name);
        }
        classfile << "}" << '\n';
        classfile << '\n';

        emit_register(classfile);

        classfile << "}" << '\n';
        classfile << "}" << '\n';
        classfile << "}" << '\n';
    }

    return 0;
}
