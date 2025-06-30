# typed: true

class Animal
  extend T::Sig
end

class Vehicle
  extend T::Sig
  
  # This should error: String is not related to Vehicle
  sig { returns(T::Boolean).narrows_to(String) } # error: Malformed `sig`: `narrows_to(String)` must be a type that is related to the receiver type `Vehicle`
  def string_like?
    false
  end
  
  # This should error: Integer is not related to Vehicle
  sig { returns(T::Boolean).narrows_to(Integer) } # error: Malformed `sig`: `narrows_to(Integer)` must be a type that is related to the receiver type `Vehicle`
  def numeric?
    false
  end
  
  # This should error: Animal is not related to Vehicle
  sig { returns(T::Boolean).narrows_to(Animal) } # error: Malformed `sig`: `narrows_to(Animal)` must be a type that is related to the receiver type `Vehicle`
  def animal?
    false
  end
end

class Car < Vehicle
  # This should be OK: Car is a subtype of Vehicle
  sig { returns(T::Boolean).narrows_to(Car) }
  def car?
    true
  end
  
  # This should be OK: Vehicle is a supertype of Car
  sig { returns(T::Boolean).narrows_to(Vehicle) }
  def vehicle?
    true
  end
end

# Test with modules
module Flyable
  extend T::Sig
  
  # This should error: String is not related to module Flyable
  sig { returns(T::Boolean).narrows_to(String) } # error: Malformed `sig`: `narrows_to(String)` must be a type that is related to the receiver type `Flyable`
  def can_fly_like_string?
    false
  end
end

class Bird < Animal
  include Flyable
  
  # This should be OK: Bird includes Flyable
  sig { returns(T::Boolean).narrows_to(Bird) }
  def bird?
    true
  end
end

# Test with abstract classes
class User
  extend T::Sig
  extend T::Helpers
  abstract!
  
  # This should error: Document is not related to User
  sig { abstract.returns(T::Boolean).narrows_to(Document) } # error: Malformed `sig`: `narrows_to(Document)` must be a type that is related to the receiver type `User`
  def document?; end
end

class Document
  extend T::Sig
end