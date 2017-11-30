# @typed
class Foo
  declare_variables(
    :@ivar => Integer
  )

  def foo
    @ivar = 2
    @ivar = "ss" # error: Changing type of pinned argument
  end
end
