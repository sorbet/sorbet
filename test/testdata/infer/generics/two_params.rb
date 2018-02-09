# @typed
class MyProc1
  Return = type_member
  Arg0 = type_member


  sig(arg0: Arg0).returns(Return)
  def call(arg0)
    _
  end

  def _; end
end

class UseProc1
  sig.returns(NilClass)
  def callit
    p = MyProc1[String, Integer].new

    T.assert_type!(p.call(7), String)

    nil
  end
end
