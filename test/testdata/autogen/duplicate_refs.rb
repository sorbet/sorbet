# typed: true
class Module; include T::Sig; end

class A < T::Struct
  prop :foo, Integer
end

class B
  # AttrReader rewriter doesn't run in autogen
  sig { returns(T.nilable(Integer)) }
  attr_reader :bar
end
