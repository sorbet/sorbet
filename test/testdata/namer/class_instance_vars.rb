class A
  declare_variables(
    :@ivar => Integer,
    :@ivar2 => Integer,
    :@@class_var => String,
  )
  @@class_var = "hi"

  def hi
    @ivar = 1
    @ivar2
  end
end
