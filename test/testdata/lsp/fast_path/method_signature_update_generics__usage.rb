# typed: strict

module Bat extend T::Sig
  sig {returns(String)}
  def foo
    Foobar.bar(["hello"]) # error: Expected `T::Array[Integer]`
    #      ^ hover: sig { params(s: T::Array[Integer]).returns(String) }
  end
end
