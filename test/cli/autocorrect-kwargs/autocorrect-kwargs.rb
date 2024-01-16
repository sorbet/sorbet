# typed: true

extend T::Sig

# Wrap curly braces around the keyword args in returns to make them into a shape
sig {returns(foo: Integer, bar: String)}
def test1
  {foo: 10, bar: "hi"}
end

# Wrap curly braces around the keyword args to [] to make the element of the
# array into a shape
sig {returns(T::Array[foo: Integer])}
def test2
  []
end

foo = T::Array[foo: Integer, bar: String].new

# Remove the curly braces from the params list
sig {params({a: Integer, b: String}).void}
def test3(a, b)
end
