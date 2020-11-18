# typed: true

module Foo
  extend T::Sig

  A_PUBLIC_CONST = 'public'
  A_PRIVATE_CONST = 'private'
  private_constant :A_PRIVATE_CONST

  ANOTHER_PRIVATE_CONST = true
  private_constant 'ANOTHER_PRIVATE_CONST'

  PrivateIntTypeAlias = T.type_alias { Integer }
  private_constant :PrivateIntTypeAlias

  PrivateInt = Integer
  private_constant :PrivateInt

  class PrivateClass; end
  private_constant :PrivateClass

  class AnotherPrivateClass; end
  private_constant 'AnotherPrivateClass'

  module PrivateModule
    def self.ok_private_usage
      A_PUBLIC_CONST
      A_PRIVATE_CONST
      PrivateClass
      PrivateModule
    end

    class ClassInsidePrivateModule
      def self.also_ok_private_usage; end
    end
  end
  private_constant :PrivateModule

  module AnotherPrivateModule; end
  private_constant 'AnotherPrivateModule'

  def self.ok_private_usage
    A_PUBLIC_CONST
    A_PRIVATE_CONST
    PrivateClass
    PrivateModule.ok_private_usage
    PrivateModule::ClassInsidePrivateModule.also_ok_private_usage
  end

  def self.not_ok_private_usage
    ::Foo::A_PUBLIC_CONST
    ::Foo::A_PRIVATE_CONST # error: private constant Foo::A_PRIVATE_CONST referenced
    ::Foo::PrivateClass # error: private constant Foo::PrivateClass referenced
    ::Foo::PrivateModule # error: private constant Foo::PrivateModule referenced
  end

  sig { params(x: PrivateIntTypeAlias, y: PrivateInt).void }
  def self.using_private_ints(x, y); end

  sig { params(x: PrivateClass).void }
  def self.using_private_class(x); end

  sig { params(x: T.class_of(PrivateModule)).void }
  def self.using_private_module(x); end
end


Foo::A_PUBLIC_CONST
Foo::A_PRIVATE_CONST # error: private constant Foo::A_PRIVATE_CONST referenced
Foo::PrivateClass # error: private constant Foo::PrivateClass referenced
Foo::PrivateModule # error: private constant Foo::PrivateModule referenced
Foo::PrivateModule::ClassInsidePrivateModule.also_ok_private_usage # error: private constant Foo::PrivateModule referenced

Foo.using_private_ints(1, 2)
Foo.using_private_class(1) # error: Expected `Foo::PrivateClass` but found `Integer(1)` for argument `x`
Foo.using_private_module(1) # error: Expected `T.class_of(Foo::PrivateModule)` but found `Integer(1)` for argument `x`

extend T::Sig

class PrivateTypeMember
  extend T::Generic
  extend T::Sig

  Elem = type_member
  private_constant :Elem

  sig {params(x: Elem).void}
  def foo(x); end

  class B
    extend T::Sig
    sig {params(x: Elem).void}
    #              ^^^^ error: `type_member` type `PrivateTypeMember::Elem` used outside of the class definition
    def bar(x); end
  end
end

sig {params(x: PrivateTypeMember::Elem).void}
#              ^^^^^^^^^^^^^^^^^^^^^^^ error: `type_member` type `PrivateTypeMember::Elem` used outside of the class definition
def foo(x); end