# typed: strict

x = T.must(3)

class C
  extend T::Sig

  sig {returns(nil)}
  def ret_nil
    nil
  end

  sig {returns(Array[Integer])}
  def ret_arr
    []
  end
end
