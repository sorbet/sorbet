# typed: true

# class Parent1
#   extend T::Generic

#   X = type_member
# end
# class Child1 < Parent1
#   Elem = type_template
#   X = Elem
# end

class Parent2
  extend T::Generic

  X = type_member
end

class Child2 < Parent2
  Elem = type_member
  X = Elem
end
