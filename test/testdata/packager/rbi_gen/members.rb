# typed: strict

# Cheating
module ::Opus::Flatfiles
  class Record
    extend T::Sig

    sig {params(name: T.untyped, type: T.untyped).void}
    def self.dsl_required(name, type); end
    
    sig {params(blk: T.proc.void).void}
    def self.flatfile(&blk); end

    sig {params(name: T.untyped).void}
    def self.field(name); end

    sig {params(pattern: T.untyped, name: T.untyped).void}
    def self.pattern(pattern, name); end

    sig {params(pattern: T.untyped, name: T.untyped).void}
    def self.from(pattern, name); end
  end

  class MarkupLanguageNodeStruct
    extend T::Sig
    
    sig {params(blk: T.proc.void).void}
    def self.flatfile(&blk); end

    sig {params(name: T.untyped).void}
    def self.field(name); end
  end
end

module RBIGen::Private
  class PrivateClass; end
  class PrivateClassPulledInByClassAlias; end
  class PrivateClassPulledInByTypeTemplate; end
  class PrivateClassPulledInByPrivateMethod; end

  class PrivateClassNotReferenced; end

  class PrivateClassForTests; end
end

module RBIGen::DirectlyExported
  MyString = String
end

module RBIGen::Public
  class RefersToPrivateTypes
    extend T::Sig

    sig {params(a: RBIGen::Private::PrivateClass).void}
    def method(a)
    end

    ClassAlias = RBIGen::Private::PrivateClassPulledInByClassAlias
  end

  class FinalClass
    extend T::Helpers
    extend T::Sig
    final!

    sig(:final) {void}
    def final_method; end
  end

  class SealedClass
    extend T::Helpers
    sealed!
  end

  class AbstractClass
    extend T::Helpers
    extend T::Sig
    abstract!

    sig {abstract.returns(String)}
    def abstract_method; end
  end

  module InterfaceModule
    extend T::Helpers
    extend T::Sig
    interface!

    sig {abstract.returns(String)}
    def interface_method; end
  end

  class AbstractAndInterfaceImplementor < AbstractClass
    include InterfaceModule

    sig {override.returns(String)}
    def abstract_method
      ""
    end

    sig {override.returns(String)}
    def interface_method
      ""
    end
  end

  class MyEnum < T::Enum
    extend T::Sig
    
    enums do
      Spades = new
      Hearts = new
      Clubs = new
      Diamonds = new
    end

    sig {returns(String)}
    def to_string
      ""
    end
  end

  class MyStruct < T::Struct
    extend T::Sig
    
    prop :foo, Integer
    const :bar, T.nilable(String)
    const :quz, Float, default: 0.5
    const :singleton_type, T.class_of(MyStruct)
    const :singleton_type_with_type_params, T.class_of(ClassWithTypeParams)

    @field = T.let(10, Integer)

    sig {returns(Integer)}
    def extra_method
      @internal_field = T.let(10, T.nilable(Integer))
      10
    end

    sig {void}
    def later_method; end
  end

  class StructWithInitializer < T::Struct
    extend T::Sig

    prop :foo, T.nilable(String)

    sig {params(foo: T.nilable(String)).void}
    def initialize(foo: nil)
      @extra_prop = T.let("", String)
    end
  end

  class CustomStruct
    include T::Props
    extend T::Sig

    # The default value does not matter to the output.
    prop :foo, Integer, default: 10
    const :bar, T.nilable(String)

    sig {void}
    def my_method; end
  end

  class FieldCheck
    extend T::Sig
  
    Alias = RBIGen::Public::FieldCheck
    Constant = T.let(0, Integer)
    AliasConstant = T.let(RBIGen::Public::FieldCheck, T.class_of(RBIGen::Public::FieldCheck))

    @@static_field = T.let(10, Integer)

    @statically_declared_field = T.let(0, Integer)

    sig {void}
    def initialize
      @field = T.let(0, Integer)
    end

    sig {void}
    def empty_method; end
  end

  class AliasMethod
    extend T::Sig

    alias_method :eql?, :==
    alias_method :bad_alias, :method_not_found # error: Can't make method alias

    class << self
      extend T::Sig

      sig {void}
      def some_method; end

      alias_method :some_method_alias, :some_method
    end

    sig {params(other: BasicObject).returns(T::Boolean)}
    def ==(other)
      false
    end
  end

  class ClassWithTypeParams
    extend T::Generic
    
    A = type_template(fixed: RBIGen::Private::PrivateClassPulledInByTypeTemplate)
    B = type_template()
    C = type_member()
  end

  module ModuleWithTypeParams
    extend T::Generic

    A = type_member(:in)
    B = type_member(:out)
  end

  module VariousMethods
    extend T::Sig

    MyString = T.type_alias {String}

    sig {returns(Integer)}
    attr_accessor :if

    sig {returns(String)}
    attr_accessor :unless

    sig {void}
    def initialize
      @if = T.let(10, Integer)
      @unless = T.let("", String)
    end

    sig {params(a: RBIGen::Private::PrivateClassPulledInByPrivateMethod).void}
    private def my_method(a); end

    sig {returns(T::Array[String])}
    def returns_generic_type
      [""]
    end

    sig {returns({'str' => Integer, symb: Integer})}
    def returns_shape_type
      {
        'str' => 10,
        symb: 10,
      }
    end

    sig {returns(MyString)}
    def returns_type_alias
      ""
    end

    sig {returns(T.any(String, [T.deprecated_enum([:D]), Integer], Integer))}
    def returns_any_with_literals
      "A"
    end

    sig {returns(T.deprecated_enum(["hello"]))}
    def dep_enum
      T.let(nil, T.untyped)
    end

    sig {void}
    private_class_method def self.kls_method; end

    sig {void}
    module_function def sample_mod_fcn; end

    sig {void}
    def some_mod_fcn; end

    module_function :some_mod_fcn # error: This function does not have a `sig`

    def dotdotdot(...); end # error: This function does not have a `sig`
  end

  module DefDelegator
    extend T::Sig
    extend T::Helpers
    extend Forwardable

    sig {void}
    def initialize
      @field = T.let("", String)
    end

    @static_field = T.let(10, Integer)
    class << self
      extend Forwardable
      
      def_delegators :@static_field, :method1, :method2
      def_delegator :@static_field, :method3
    end


    def_delegator :@field, :length
    def_delegator :@field, :concat, :aliased_concat
    def_delegators :@field, :size, :empty
  end

  MaybeString = T.type_alias {T.nilable(String)}
  ShapeType = T.type_alias{{:$str => String}}
  
  class AttachedClassType
    extend T::Sig

    sig {params(a: T.proc.params(arg: T.attached_class).void).void}
    def self.method(a)
    end
  end

  class HasInitializer
    extend T::Sig

    sig {params(a: Integer).void}
    def initialize(a); end
  end

  class InheritsInitializer < HasInitializer
    extend T::Sig
    extend T::Helpers

    abstract!

    # Ensures that we don't put field decls into abstract methods!
    sig {abstract.void}
    def abstract_method; end

    sig {void}
    def declares_field
      @field = T.let(10, T.nilable(Integer))
    end
  end

  module MixesInClassMethods
    extend T::Generic

    module ClassMethods
      extend T::Sig

      sig {void}
      def method; end
    end

    mixes_in_class_methods(ClassMethods)
  end

  class Flatfile < Opus::Flatfiles::Record
    dsl_required :deprecated?, String

    flatfile do
      from   1..2, :foo
      pattern(/A-Za-z/, :bar)
      field :baz
    end
  end

  class XMLNode < Opus::Flatfiles::MarkupLanguageNodeStruct
    flatfile do
      field :boo
    end
  end
end