# typed: strict

class MyClass2
  extend T::Sig
  sig {returns(String)}
  def returns_integer
    result = T.let('', T.nilable(String))
    result # error: Expected `String` but found `T.nilable(String)` for method result type
  end
end
