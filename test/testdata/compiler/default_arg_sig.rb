# frozen_string_literal: true
# typed: true
# compiled: true

class A
  extend T::Sig

  def self.returns_78
    puts '💥 side effect'
    78
  end

  sig {params(raise_on_error: Integer).void}
  def self.foo(raise_on_error: returns_78)
    puts raise_on_error
  end
end

A.foo
A.foo
