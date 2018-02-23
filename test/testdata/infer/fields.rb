# @typed
class Foo
  declare_variables(
    :@ivar => Integer
  )

  def foo
    @ivar = 2
    @ivar = "ss" # error: Reassigning field with a value of wrong type: String is not a subtype of Integer
  end
end
