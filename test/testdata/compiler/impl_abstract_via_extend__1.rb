# frozen_string_literal: true
# typed: strict
# compiled: false

module Foo
  extend IFoo
  extend T::Sig
  sig {override.returns(Integer)}
  def self.foo; 64; end
end

p Foo.foo
