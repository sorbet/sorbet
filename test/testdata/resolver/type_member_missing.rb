# typed: true
class Base
  extend T::Generic

  Elem = type_member
end

class Child < Base # error: Type `Elem` declared by parent `Base` should be declared again
  # The resolver should error and copy Elem into this class, as shown
  # by the name-table expectation
end
