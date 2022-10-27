# typed: strict

extend T::Sig

# This test will infinitely loop if we do not return equal references in glb.

sig {params(x: T::Hash[Symbol, T.untyped]).void}
def foo(x)
end

sig {params(x: T::Hash[Symbol, T.untyped]).void}
def bar(x)
end

def qux(x) # error: The method `qux` does not have a `sig`
  if T.unsafe(nil)
    foo(x)
  end
  bar(x)
end

sig {params(x: {y: Integer}).void}
def wtf(x)
end

sig {params(x: {y: Integer}).void}
def bbq(x)
end

def abc(x) # error: The method `abc` does not have a `sig`
  if T.unsafe(nil)
    wtf(x)
  end
  bbq(x)
end
