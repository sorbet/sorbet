# typed: true

class ImmutableBox
  extend T::Sig
  extend T::Generic

  Elem = type_member(:out)

  sig {params(x: Elem).void}
  def initialize(x)
    @x = x
  end

  sig {returns(Elem)}
  def get_x; @x; end

  sig {params(x: Elem).void}
  #           ^ error: `type_member` `Elem` was defined as `:out` but is used in an `:in` context
  def set_x(x); @x = x; end

  sig {params(x: Elem).void}
  private def private_set_x(x); @x = x; end
end
