# typed: strict
# frozen_string_literal: true

# stratum: 0

class Parent::MyGeneric
  extend T::Generic

  Elem = type_member
end

Parent::ParentAlias = T.type_alias {Parent::MyGeneric[Integer]}
