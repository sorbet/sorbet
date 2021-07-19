# typed: true

extend T::Sig

sig {params(x: Integer).void}
def foo(x)
  puts(x + 1)
end
