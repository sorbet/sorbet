# @typed
class Foo
  def initialize
    @ivar = T.let(0, Integer)
  end

  def foo
    @ivar = 2
    @ivar = "ss" # error: Reassigning field with a value of wrong type: String("ss") is not a subtype of Integer(2)
  end
end
