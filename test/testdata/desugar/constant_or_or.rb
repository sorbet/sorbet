# typed: true

class A
  class X; end

  # TODO(jez) The Class.new rewriter does not fire for these assignments
  X ||= Class.new
# ^^^^^^^^^^^^^^^ error: Redefining constant `X` as a static field

  T.reveal_type(X) # error: `T.untyped`
end

class B
  X = T.let(nil, Integer)
  #   ^^^^^^^^^^^^^^^^^^^ error: Argument does not have asserted type `Integer`
  X ||= 0

  T.reveal_type(X) # error: `Integer`
end

class C
  X = T.let(nil, T.nilable(String))
  X ||= 0
  #     ^ error: Expected `T.nilable(String)` but found `Integer(0)` for field

  T.reveal_type(X) # error: `T.nilable(String)`
end
