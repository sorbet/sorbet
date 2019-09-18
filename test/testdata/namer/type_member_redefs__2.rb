# typed: true

class D
  extend T::Generic
  R = type_member # error: Duplicate type member `R`
end
