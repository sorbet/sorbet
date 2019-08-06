# typed: strict

x = 3

class C
  extend T::Sig

  sig {returns(NilClass)}
  def ret_nil
    nil
  end
end
