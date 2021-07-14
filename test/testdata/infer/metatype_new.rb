# typed: true

class MyGeneric
  extend T::Generic

  X = type_member
end

MyArrayString = T.type_alias {T::Array[String]}

# -- good --

String.new
T::Array[String].new
MyGeneric.new
MyGeneric[Integer].new

# -- bad, but we can't detect them yet --

MyArrayString.new                  # Type alias is unwound by the type we get to MetaType::dispatchCall

# -- bad --

T.any(Integer, String).new         # error: Cannot call `new` on type `T.any(Integer, String)`
T.all(Kernel, Comparable).new      # error: Cannot call `new` on type `T.all(Kernel, Comparable)`
T.nilable(String).new              # error: Cannot call `new` on type `T.nilable(String)`

T.untyped.new                      # error: Cannot call `new` on type `T.untyped`
T.self_type.new                    # error: Cannot call `new` on type `T.untyped`

T.class_of(String).new             # error: Cannot call `new` on type `T.class_of(String)`
T.class_of(MyGeneric).new          # error: Cannot call `new` on type `T.class_of(MyGeneric)`
T.class_of(MyGeneric[Integer]).new # error: Cannot call `new` on type `T.class_of(MyGeneric)`

T.proc.void.new                    # error: Cannot call `new` on declaration builder type

T.noreturn.new                     # error: Cannot call `new` on type `T.noreturn`
