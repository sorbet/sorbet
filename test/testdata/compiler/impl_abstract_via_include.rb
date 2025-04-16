# frozen_string_literal: true
# typed: true
# compiled: true

module IFoo
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.returns(Integer)}
  def foo; end
end


module FooImpl
  extend T::Sig
  sig {returns(Float)}
  def foo; 0.1617; end
end

  class Foo
# ^^^^^^^^^ error: Return type `Float` does not match return type of abstract method `IFoo#foo`
    include FooImpl
    include IFoo
  end

p Foo.new.foo
