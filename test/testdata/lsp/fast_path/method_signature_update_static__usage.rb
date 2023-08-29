# typed: strict

module Bat extend T::Sig
  sig {returns(String)}
  def foo
    Foobar.bar("hello") # error: Expected `Integer`
    #      ^ hover: sig { params(s: Integer).returns(String) }
  end
end
