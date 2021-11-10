# frozen_string_literal: true
# typed: true
# compiled: true

module X
  extend T::Sig

  sig {params(maps: T::Hash[T.untyped, T.untyped]).void}
  def self.f(*maps)
    p maps
  end
end

X.f({}, {a: 5})
begin
  X.f(T.unsafe([]), {a: 6})
rescue => e
  p "got error"
end

