# typed: true

class Foo; end
module Bar; end

x = Foo
#   ^ hover: # class
#   ^ hover: T.class_of(Foo)

y = Bar
#   ^ hover: # module
#   ^ hover: T.class_of(Bar)

Aliased = Foo
# ^ hover: # class alias
# ^ hover: Aliased = Foo

StaticField = T.let(Foo, T.class_of(Foo))
# ^ hover: # static field
# ^ hover: T.class_of(Foo)

MyType = T.type_alias { Integer }
# ^ hover: # type alias
# ^ hover: T.type_alias {Integer}

class Generic
  extend T::Generic
  Elem = type_member
  # ^ hover: # type member
end
