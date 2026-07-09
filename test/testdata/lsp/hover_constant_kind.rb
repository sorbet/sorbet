# typed: true

class Foo; end
module Bar; end

x = Foo
#   ^ hover: # class Foo
#   ^ hover: T.class_of(Foo)

y = Bar
#   ^ hover: # module Bar
#   ^ hover: T.class_of(Bar)

Aliased = Foo
# ^ hover: Aliased = Foo
# ^ hover-line: 2 Aliased = Foo

StaticField = T.let(Foo, T.class_of(Foo))
# ^ hover: # static field StaticField
# ^ hover: T.class_of(Foo)

MyType = T.type_alias { Integer }
# ^ hover: # type alias MyType
# ^ hover: T.type_alias {Integer}

class Generic
  extend T::Generic
  Elem = type_member
  # ^ hover: # class Generic
  # ^ hover: #   Elem = type_member
  # ^ hover: # end
  # ^ hover: Generic::Elem
end

class GenericTemplate
  extend T::Generic
  Tmpl = type_template
  # ^ hover: # class GenericTemplate
  # ^ hover: #   Tmpl = type_template
  # ^ hover: # end
  # ^ hover: T.class_of(GenericTemplate)::Tmpl
end

# Nested namespaces should render the fully-qualified path (elliottt's examples).
module Foo2
  class Bar; end
end

nested = Foo2::Bar
#              ^ hover: # class Foo2::Bar
#              ^ hover: T.class_of(Foo2::Bar)

module Wrapper
  class Box
    extend T::Generic
    Elem = type_member
    # ^ hover: # class Wrapper::Box
    # ^ hover: #   Elem = type_member
    # ^ hover: # end
    # ^ hover: Wrapper::Box::Elem
  end
end

# Variance and bounds should be rendered in the skeleton.
class Variances
  extend T::Generic
  Cov = type_member(:out)
  # ^ hover: #   Cov = type_member(:out)
  Contra = type_member(:in)
  # ^ hover: #   Contra = type_member(:in)
  Bounded = type_member { {upper: Numeric} }
  # ^ hover: #   Bounded = type_member { {upper: Numeric} }
  Fixed = type_member { {fixed: Integer} }
  # ^ hover: #   Fixed = type_member { {fixed: Integer} }
  Lower = type_member { {lower: Integer} }
  # ^ hover: #   Lower = type_member { {lower: Integer} }
  Both = type_member(:out) { {lower: Integer, upper: Numeric} }
  # ^ hover: #   Both = type_member(:out) { {lower: Integer, upper: Numeric} }
end

# Type members declared inside a module render `module` in the skeleton.
module GenericModule
  extend T::Generic
  Elem = type_member
  # ^ hover: # module GenericModule
  # ^ hover: #   Elem = type_member
  # ^ hover: # end
  Tmpl = type_template
  # ^ hover: # module GenericModule
  # ^ hover: #   Tmpl = type_template
  # ^ hover: # end
end

# Nested static fields render their fully-qualified name.
module Foo2
  NESTED_FIELD = T.let(1, Integer)
  # ^ hover: # static field Foo2::NESTED_FIELD
  # ^ hover: Integer
end

# Usage-site hovers (not just definition sites) get the same headers.
usage1 = Generic::Elem
#                 ^ hover: # class Generic
#                 ^ hover: #   Elem = type_member
#                 ^ hover: # end

usage2 = Foo2::NESTED_FIELD
#              ^ hover: # static field Foo2::NESTED_FIELD
