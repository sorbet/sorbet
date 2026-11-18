# typed: true
extend T::Sig

class A < T::Struct
  prop :b, T.nilable(Integer)
end

sig {params(a: A, h: T::Hash[Symbol, T.nilable(Integer)], x: Integer).void}
def example(a, h, x)
  y = a.b ||= x
  #   ^ hover: A
  #     ^ hover-line: 2 # A#b:
  #     ^ hover-line: 3 sig { returns(T.nilable(Integer)) }
  #     ^ hover-line: 4 def b; end
  #       ^ hover-line: 2 Integer
  #           ^ hover: Integer
  z = h[:k] ||= x
  #   ^ hover: T::Hash[Symbol, T.nilable(Integer)]
  #    ^ hover: T::Hash[Symbol, T.nilable(Integer)]
  #         ^ hover-line: 2 Integer
  # For comparison, the setter call of a plain assignment:
  w = a.b = x
  #     ^ hover-line: 2 # A#b=:
  #       ^ hover: (nothing)
end
