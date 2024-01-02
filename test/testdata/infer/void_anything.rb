# typed: strict
extend T::Sig

# We used to implement the logic to allow methods typed as `.void` to return
# anything. The logic was buggy in such a way that methods returning `.void` or
# *any superclass of void* would be treated as `.returns(T.anything)`.

module M; end

sig { params(x: M).void }
def foo1(x)
  x
end

sig { params(x: M).returns(Object) }
def foo2(x)
  x # error: Expected `Object` but found `M` for method result type
end

sig { params(x: M).returns(Kernel) }
def foo3(x)
  x # error: Expected `Kernel` but found `M` for method result type
end

sig { params(x: M).returns(BasicObject) }
def foo4(x)
  # Sorbet treats modules as descending from BasicObject
  # We should fix that at some point, but that's unrelated to the `.void` bug
  x
end
