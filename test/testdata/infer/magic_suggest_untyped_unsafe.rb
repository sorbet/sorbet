# typed: strict
# enable-suggest-unsafe: true

class A
  extend T::Sig

  X = T.unsafe(nil) # error: Constants must have type annotations

  sig {void}
  def initialize
    @x = T.unsafe(nil) # error: must be declared using `T.let`
  end
end
