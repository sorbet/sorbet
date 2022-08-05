# typed: true
# spacer for exclude from file update assertion

class A_08
  extend T::Sig
  sig {params(x: Integer).void}
  def initialize(x)
    T.reveal_type(x) # error: `Integer`
  end
end
