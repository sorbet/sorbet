# typed: true

class A
  extend T::Generic
  R = type_member
  R = type_member # error: Duplicate type member `R`
end

class B
  extend T::Generic
  R = type_member
  R = 1 # error: Redefining constant `R` as a static field
end

class C
  extend T::Generic
  R = 1
  R = type_member # error: Redefining constant `R` as a type member or type template
end

class D
  extend T::Generic
  R = type_member # error: Duplicate type member `R`
end
