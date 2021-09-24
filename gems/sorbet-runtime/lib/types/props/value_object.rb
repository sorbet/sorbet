# typed: strict
# frozen_string_literal: true

module T::Props::ValueObject
  extend T::Sig

  # A struct is equal to another if the classes and all prop definitions (attributes) and values are the same
  sig {params(other: T::Props::ValueObject).returns(T::Boolean)}
  def ==(other)
    return false unless self.class == other.class

    decorator = self.class.decorator
    return false unless decorator.all_props == other.class.decorator.all_props

    decorator.all_props.all? do |prop|
      decorator.get(self, prop) == decorator.get(other, prop)
    end
  end

  # Match Struct#hash by using both the class and all prop definitions (attributes) and values to calculate the hash
  sig {returns(Integer)}
  def hash
    decorator = self.class.decorator
    prop_values = decorator.all_props.map {|prop| decorator.get(self, prop)}

    [self.class, decorator.all_props, prop_values].hash
  end
end
