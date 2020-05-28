# typed: strict

class A
  extend T::Sig

  sig(:final) {returns(T.nilable(Integer))}
  attr_accessor :x
end
