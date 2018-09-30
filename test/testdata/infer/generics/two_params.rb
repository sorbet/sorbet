# typed: true
class MyProc1
  extend T::Generic

  Return = type_member
  Arg0 = type_member


  sig {params(arg0: Arg0).returns(Return)}
  def call(arg0)
    _
  end

  def _; end
end

class UseProc1
  extend T::Helpers

  sig {returns(NilClass)}
  def callit
    p = MyProc1[String, Integer].new

    T.assert_type!(p.call(7), String)

    nil
  end
end
