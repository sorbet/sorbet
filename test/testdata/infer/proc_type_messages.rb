# typed: true

class ProcTypeErrorTest

  extend T::Sig

  sig {params(x: T.proc.params(x: Integer).returns(String)).void}
  def foo(x)
  end

  sig {params(x: T.proc.params(x: T::Array[Integer]).returns(T::Array[String])).void}
  def array_proc(x)
  end

  sig {params(x: T.proc.params(x: T::Hash[String, Integer]).returns(T::Hash[Integer, String])).void}
  def hash_proc(x)
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

  sig {void}
  def test_array_proc_error
    x = T.cast(nil, T.proc.params(x: T::Array[String]).returns(T::Array[Integer]))
    array_proc(x)
#              ^ error: Expected `T.proc.params(arg0: T::Array[Integer]).returns(T::Array[String])` but found `T.proc.params(arg0: T::Array[String]).returns(T::Array[Integer])` for argument `x`
#              Detailed explanation:
#                T::Array[Integer] is not a subtype of T::Array[String] for return type of Proc1
#                T::Array[String] is not a supertype of T::Array[Integer] for Arg0 of Proc1
  end

  sig {void}
  def test_hash_proc_error
    x = T.cast(nil, T.proc.params(x: T::Hash[Integer, String]).returns(T::Hash[String, Integer]))
    hash_proc(x)
#             ^ error: Expected `T.proc.params(arg0: T::Hash[String, Integer]).returns(T::Hash[Integer, String])` but found `T.proc.params(arg0: T::Hash[Integer, String]).returns(T::Hash[String, Integer])` for argument `x`
#             Detailed explanation:
#               T::Hash[String, Integer] is not a subtype of T::Hash[Integer, String] for return type of Proc1
#               T::Hash[Integer, String] is not a supertype of T::Hash[String, Integer] for Arg0 of Proc1
  end

end
