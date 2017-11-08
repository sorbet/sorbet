class Parent
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

module Mixin
  declare_variables(
    :@@mixin_cvar => Integer,
  )
  @@mixin_cvar
end

class Child < Parent
  include Mixin

  @@class_var
  @@mixin_cvar
  @@undefined_cvar # error: Use of undeclared variable

  def child_method
    @@class_var
    @@mixin_cvar
    @ivar2
    @undefinedivar # error: Use of undeclared variable
  end
end
