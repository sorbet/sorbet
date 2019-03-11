# typed: true

class A
  extend T::Generic

  Elem = type_template(fixed: Integer)
end
