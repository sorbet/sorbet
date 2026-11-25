# typed: true

module Opus::Types::Test::Fixtures::CircularLoad
  class Parent
    extend T::Sig
    sig { params(x: OneOrTwo).void }
    def self.foo(x)
      @foo_invocations ||= []
      @foo_invocations << x
    end

    class << self
      attr_reader :foo_invocations
    end
  end
end
