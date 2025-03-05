# typed: true

class ProcTypeErrorTest

  extend T::Sig

  sig {params(x: T.proc.params(x: Integer).returns(String)).void}
  def foo(x)
  end

  sig {void}
  def test_proc_type_error
    x = T.cast(nil, T.proc.params(x: String).returns(Integer))
    foo(x)
#       ^ error: Expected `T.proc.params(arg0: Integer).returns(String)` but found `T.proc.params(arg0: String).returns(Integer)` for argument `x`
#       Detailed explanation:
#         Integer is not a subtype of String for return type of Proc1
#         String is not a supertype of Integer for Arg0 of Proc1
  end

end
