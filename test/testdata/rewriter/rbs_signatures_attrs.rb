# typed: strict

class Foo
  #: Integer
  attr_reader :foo

  #: Integer?
  attr_accessor :bar, :baz

  #: Integer?
  attr_writer :qux

  #: -> void
  def initialize
    @foo = T.let(1, Integer)
    @bar = T.let(2, T.nilable(Integer))
    @baz = T.let(3, T.nilable(Integer))
    @qux = T.let(4, T.nilable(Integer))
  end
end

x = Foo.new
T.reveal_type(x.foo) # error: Revealed type: `Integer`
T.reveal_type(x.bar) # error: Revealed type: `T.nilable(Integer)`
T.reveal_type(x.baz) # error: Revealed type: `T.nilable(Integer)`
x.qux = "" # error: Assigning a value to `qux` that does not match expected type `T.nilable(Integer)`
