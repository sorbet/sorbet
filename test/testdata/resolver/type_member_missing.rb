# typed: true

class Base
  extend T::Generic

  Elem = type_member
end

class Child < Base # error: Type `Elem` declared by parent `Base` must be re-declared in `Child`
  # The resolver should error and copy Elem into this class, as shown
  # by the symbol-table expectation
end
