# typed: true

module Foo
  A_PUBLIC_CONST = 'public'
  A_PRIVATE_CONST = 'private'
  private_constant :A_PRIVATE_CONST

  ANOTHER_PRIVATE_CONST = true
  private_constant 'ANOTHER_PRIVATE_CONST'

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
end


Foo::A_PUBLIC_CONST
Foo::A_PRIVATE_CONST # error: private constant Foo::A_PRIVATE_CONST referenced
Foo::PrivateClass # error: private constant Foo::PrivateClass referenced
Foo::PrivateModule # error: private constant Foo::PrivateModule referenced
Foo::PrivateModule::ClassInsidePrivateModule.also_ok_private_usage # error: private constant Foo::PrivateModule referenced