# typed: true
class Foo
  def initialize
    @ivar = T.let(0, Integer)
  end

  def foo
    @ivar = 2
    @ivar = "ss" # error: Expected `Integer` but found `String("ss")` for field
  end
end
