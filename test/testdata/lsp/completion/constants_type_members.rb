# typed: true

class Box
  extend T::Sig
  extend T::Generic

  Elem = type_member

  sig {params(val: Ele).void} # error: Unable to resolve
  #                   ^ completion: Elem
  def initialize(val)
    @val = val
  end
end
