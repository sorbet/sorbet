# typed: true
extend T::Sig

# Type tests are not allowed to have T.untyped in them.
# the "Not enough arguments" error below introduces `T.untyped`
# This caused an ENFORCE to fail halfway through implementing support for `x ==
# C`, so I figured it'd be a good test to have.

class C
  extend T::Generic
  Elem = type_template
end

class D
  extend T::Generic
  Elem = type_template
end

sig {params(x: T.any(T.class_of(C), T.class_of())).void} # error: Not enough arguments provided for method `T.class_of`
def test5(x)
  if x == C
    T.reveal_type(x) # error: `T.class_of(C)[C, T.untyped]`
  else
    T.reveal_type(x) # error: `T.untyped`
  end
end
