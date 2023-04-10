# typed: true
# disable-fast-path: true

class Parent1
  extend T::Generic

  X = type_member
end
class Child1 < Parent1
  Elem = type_template
  X = Elem # error: Type variable `X` needs to be declared as a type_member or type_template, not a static-field
end

class Parent2
  extend T::Generic

  X = type_member
end

class Child2 < Parent2
  Elem = type_member
  X = Elem # error: Type variable `X` needs to be declared as a type_member or type_template, not a static-field
end
