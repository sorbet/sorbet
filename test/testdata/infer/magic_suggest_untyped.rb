# typed: strict

class A
  extend T::Sig

  X = T.unsafe(nil)

  sig {void}
  def initialize
    @x = T.unsafe(nil)
  end
end
