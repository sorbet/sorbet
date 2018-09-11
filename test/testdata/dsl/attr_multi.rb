# typed: true

class Test
  extend T::Helpers

  sig.returns(T.nilable(String))
  attr_accessor :a

  sig.returns(T.nilable(Integer))
  attr_accessor :b, :c

  sig.returns(String)
  attr_reader :d

  sig.returns(String)
  attr_reader :e, :f

  sig(g: String).returns(String)
  attr_writer :g

  sig(h: String).returns(String)
  attr_writer :h, :i
end
