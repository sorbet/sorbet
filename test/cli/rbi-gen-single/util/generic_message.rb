# typed: true

module Util

  class GenericMessage
    extend T::Sig
    extend T::Generic

    Elem = type_member

    sig {params(x: Elem).void}
    def initialize(x)
      @x = x
    end

    sig {returns(Elem)}
    def x; @x; end
  end

end
