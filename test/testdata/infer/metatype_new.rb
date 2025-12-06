# typed: true

class MyGeneric
  extend T::Generic

  X = type_member
end

class MyOtherGeneric
  extend T::Generic

  X = type_template
end

MyArrayString = T.type_alias {T::Array[String]}

# -- good --

String.new
T::Array[String].new
MyGeneric.new
MyGeneric[Integer].new
MyOtherGeneric.new

# -- bad, but we can't detect them yet --

MyArrayString.new                  # Type alias is unwound by the type we get to MetaType::dispatchCall

# -- bad --

T.any(Integer, String).new         # error: Call to method `new` on `T.any(Integer, String)` mistakes a type for a value
T.all(Kernel, Comparable).new      # error: Call to method `new` on `T.all(Kernel, Comparable)` mistakes a type for a value
T.nilable(String).new              # error: Call to method `new` on `T.nilable(String)` mistakes a type for a value

T.untyped.new                      # error: Call to method `new` on `T.untyped` mistakes a type for a value
T.self_type.new                    # error: Call to method `new` on `T.self_type (of T.class_of(<root>))` mistakes a type for a value

T.class_of(String).new             # error: Call to method `new` on `T.class_of(String)` mistakes a type for a value
T.class_of(MyGeneric).new          # error: Call to method `new` on `T.class_of(MyGeneric)` mistakes a type for a value

# Note: the following shouldn't actually parse, but does due to a bug (https://github.com/sorbet/sorbet/issues/4377).
# If you fix that bug, please feel free to delete the following line.
T.class_of(MyGeneric[Integer]).new # error: Call to method `new` on `T.class_of(MyGeneric)` mistakes a type for a value

T.proc.void.new                    # error: Call to method `new` on `T.proc.void` mistakes a type for a value

T.noreturn.new                     # error: Call to method `new` on `T.noreturn` mistakes a type for a value
