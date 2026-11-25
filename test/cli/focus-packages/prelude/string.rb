# typed: true

class ::String
  extend T::Sig

  sig {returns(Integer)}
  def prelude_value
    "wrong"
  end
end
