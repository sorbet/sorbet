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
    end

    class ClassInsidePrivateModule
      def self.also_ok_private_usage; end
    end
  end
  private_constant :PrivateModule

  module AnotherPrivateModule; end
  private_constant 'AnotherPrivateModule'

  sig { returns(::Foo::PrivateClass) }
  def self.not_ok_private_usage
  end

  sig { params(x: PrivateIntTypeAlias, y: PrivateInt).void }
  def self.using_private_ints(x, y); end

  sig { params(x: PrivateClass).void }
  def self.using_private_class(x); end

  sig { params(x: T.class_of(PrivateModule)).void }
  def self.using_private_module(x); end
end

Foo::A_PUBLIC_CONST
Foo::A_PRIVATE_CONST
Foo::PrivateClass
Foo::PrivateModule
Foo::PrivateModule::ClassInsidePrivateModule
