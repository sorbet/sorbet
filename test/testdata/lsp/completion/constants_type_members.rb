# typed: true

class Box
  extend T::Sig
  extend T::Generic

  Elem = type_member

  # We get a duplicate constant resolution error here because of the Initializer rewrite pass.
  sig {params(val: Ele).void} # error-with-dupes: Unable to resolve
  #                   ^ completion: Elem
  def initialize(val)
    @val = val
  end
end
