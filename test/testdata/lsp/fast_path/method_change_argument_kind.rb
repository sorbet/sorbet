# typed: true

class A extend T::Sig
  sig {params(x: Integer).returns(String)}
  def bar(x)
    x.to_s
  end
end

A.new.bar
#        ^ error: Not enough arguments provided for method `A#bar`. Expected: `1`, got: `0`
A.new.bar(0)
A.new.bar(x: 0)
#         ^^^^ error: Expected `Integer` but found `{x: Integer(0)}` for argument `x`
