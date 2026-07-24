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
# ^ hover: # StaticField = T.let(…, T.class_of(Foo))
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

# Nested static fields render a reconstructed definition in context.
module Foo2
  NESTED_FIELD = T.let(1, Integer)
  # ^ hover: # module Foo2
  # ^ hover: #   NESTED_FIELD = T.let(…, Integer)
  # ^ hover: # end
  # ^ hover: Integer
end

# Multi-line definitions render the same reconstructed form.
class MultilineField
  LONG = T.let(
  # ^ hover: # class MultilineField
  # ^ hover: #   LONG = T.let(…, T::Array[Integer])
  # ^ hover: # end
    [1, 2, 3],
    T::Array[Integer]
  )
end

# Static fields declared inside `class << self` render a nested singleton block.
class Settings
  class << self
    DEFAULT_LIMIT = T.let(100, Integer)
    # ^ hover: # class Settings
    # ^ hover: #   class << self
    # ^ hover: #     DEFAULT_LIMIT = T.let(…, Integer)
    # ^ hover: #   end
    # ^ hover: # end
    # ^ hover: Integer
  end
end

# Usage-site hovers (not just definition sites) get the same headers.
usage1 = Generic::Elem
#                 ^ hover: # class Generic
#                 ^ hover: #   Elem = type_member
#                 ^ hover: # end

usage2 = Foo2::NESTED_FIELD
#              ^ hover: # module Foo2
#              ^ hover: #   NESTED_FIELD = T.let(…, Integer)
#              ^ hover: # end
