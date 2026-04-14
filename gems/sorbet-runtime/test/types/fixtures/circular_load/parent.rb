# typed: true

module Opus::Types::Test::Fixtures::CircularLoad
  class Parent
    extend T::Sig
    sig { params(x: OneOrTwo).void }
    def self.foo(x)
      @@foo_invocations ||= []
      @@foo_invocations << x
    end

    def self.foo_invocations = @@foo_invocations
  end
end
