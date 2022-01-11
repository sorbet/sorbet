# typed: true
extend T::Sig

class A
end

sig {params(x: A).void}
def foo(x)
  puts x
  x.
end # error: unexpected token "end"
