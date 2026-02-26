# typed: strict

module Root::B::Subpackage
  class Example
    extend T::Sig

    sig { void }
    def f
    end
  end
end
