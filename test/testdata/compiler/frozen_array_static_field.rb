# frozen_string_literal: true
# typed: true
# compiled: true

class B
  Foo = [:foo].freeze
  FooBar = [:foo, :bar].freeze
end

class A
  extend T::Sig

  private def external?
    false
  end

  sig {returns(T::Array[Symbol])}
  def provided_attributes
    if external?
      B::Foo
    else
      B::FooBar
    end
  end

  sig {returns(T::Array[Symbol])}
  def attributes
    provided_attributes
  end
end

p A.new.attributes
