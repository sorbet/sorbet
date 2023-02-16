# typed: strict

module Foo::Bar::Baz
  class Quux
    extend T::Sig

    sig { void }
    def example; end
  end
end
