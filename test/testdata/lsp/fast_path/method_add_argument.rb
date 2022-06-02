# typed: true

class A extend T::Sig
  sig {params(x: Integer).returns(String)}
  def bar(x)
    res = x.to_s
    puts(y)
    #    ^ error: Method `y` does not exist on `A`
    res
  end
end

A.new.bar # error: Expected: `1`, got: `0`
A.new.bar(0)
A.new.bar(0, 0) # error: Expected: `1`, got: `2`
