# typed: true
extend T::Sig

class A < T::Struct
  prop :b, T.nilable(Integer)
end

sig {params(a: A, y: T.nilable(Integer)).returns(Integer)}
def or_assign_attr(a, y)
  a.b ||= y
end

sig {params(h: T::Hash[Symbol, T.nilable(Integer)], y: T.nilable(Integer)).returns(Integer)}
def or_assign_index(h, y)
  h[:k] ||= y
end
