# typed: true

class User
  extend T::Sig
  extend T::Helpers
  abstract!

  # This should cause an error: FalseClass with narrows_to
  sig { abstract.returns(FalseClass).narrows_to(User) } # error: Malformed `sig`: `narrows_to(User)` cannot be used with `returns(FalseClass)` as type narrowing only occurs when the method returns a truthy value
  def invalid_type_guard?; end
end

class Admin < User
  # Override the abstract method properly
  sig { override.returns(FalseClass) }
  def invalid_type_guard?; false; end
end

# Test with class that doesn't inherit appropriately
class Document
  extend T::Sig

  sig { returns(T::Boolean).narrows_to(String) } # This might be questionable but not necessarily an error
  def string_like?; false; end
end