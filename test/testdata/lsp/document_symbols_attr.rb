# typed: true

class A < T::Struct
  extend T::Sig

  const :foo, Integer
  prop :bar, String

  attr_reader :qux
  attr_accessor :zap

  sig {returns(Integer)}
  attr_reader :checked_qux

  sig {returns(String)}
  attr_reader :checked_zap
end
