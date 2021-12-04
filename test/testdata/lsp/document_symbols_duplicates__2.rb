# typed: true

module A; module B; module C; end; end; end
module A::B::C
  class D
    extend T::Sig

    sig {void}
    def initialize
      @ivar = T.let(0, Integer)
    end
  end
end
