# typed: true

class A extend T::Sig
  sig {params(x: Integer).returns(String)}
  def bar(x)
    x.to_s
  end
end

A.new.bar # error: Not enough arguments provided for method `A#bar`. Expected: `1`, got: `0`
A.new.foo.nope
#     ^^^ error: Method `foo` does not exist on `A`
