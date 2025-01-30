# typed: strict
# enable-experimental-rbs-signatures: true

class Foo
  #: Integer
  attr_reader :foo

  #: Integer?
  attr_accessor :bar, :baz

  #: Integer?
  attr_writer :qux

  #: Integer
  attr_writer :quux1, :quux2 # error: RBS signatures for attr_writer do not support multiple arguments
# ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: The method `quux1=` does not have a `sig`
# ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: The method `quux2=` does not have a `sig`

  #: Integer
  attr_writer # error: RBS signatures do not support attr_writer without arguments

  #: -> Integer
  #  ^^ error: Failed to parse RBS type (unexpected token for simple type)
  attr_reader :quux3 # error: The method `quux3` does not have a `sig`

  #: -> void
  def initialize
    @foo = T.let(1, Integer)
    @bar = T.let(2, T.nilable(Integer))
    @baz = T.let(3, T.nilable(Integer))
    @qux = T.let(4, T.nilable(Integer))
    @quux1 = T.let(5, Integer)
    @quux2 = T.let(6, Integer)
    @quux3 = T.let(7, Integer)
  end
end

x = Foo.new
T.reveal_type(x.foo) # error: Revealed type: `Integer`
T.reveal_type(x.bar) # error: Revealed type: `T.nilable(Integer)`
T.reveal_type(x.baz) # error: Revealed type: `T.nilable(Integer)`
x.qux = "" # error: Assigning a value to `qux` that does not match expected type `T.nilable(Integer)`
