
# typed: true

extend T::Sig

sig {params(x: Integer, y: String).void}
def f(x, y)
  puts (x+y)
  puts (x+y)
  puts (x+y)
  puts (x+y)
  puts (x+y)
end
