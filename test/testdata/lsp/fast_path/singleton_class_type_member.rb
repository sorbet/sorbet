# typed: true

class Parent
  class << self
    extend T::Generic

    XYZ = type_member
  end
end

class Child < Parent
  extend T::Generic

  XYZ = type_template
end
