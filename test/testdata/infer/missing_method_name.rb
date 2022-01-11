# typed: true
extend T::Sig

class A
end

sig {params(x: A).void}
def foo(x)
  puts x
  x. # error: Method `<method-name-missing>` does not exist on `A`
end # error: unexpected token "end"
