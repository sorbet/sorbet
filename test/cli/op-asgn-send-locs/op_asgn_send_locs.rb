# typed: true
extend T::Sig

class A < T::Struct
  prop :b, T.nilable(Integer)
end

sig {params(x: Integer).void}
def takes_integer(x); end

sig {params(a: A, h: T::Hash[Symbol, T.nilable(Integer)], x: T.nilable(Integer)).void}
def example(a, h, x)
  takes_integer(a.b ||= x)
  takes_integer(h[:k] ||= x)
  takes_integer(a.b &&= x)
end

sig {params(s: T.nilable(String)).void}
def setter_errors(s)
  s.length ||= 0
  s.length += 1
  s.length = 0
end

sig {params(a: A, x: Integer).void}
def misspelled_setter(a, x)
  a.bb ||= x
  a.bb = x
end
