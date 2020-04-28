# frozen_string_literal: true
# typed: true

# Use as a mixin with extend (`extend T::Generic`).
# Docs at https://hackpad.corp.stripe.com/Type-Validation-in-pay-server-1JaoTHir5Mo.
module T::Generic
  include T::Helpers
  include Kernel

  ### Class/Module Helpers ###

  def [](*types)
    self
  end

  def type_member(variance=:invariant, fixed: nil, lower: T.untyped, upper: BasicObject)
    T::Types::TypeMember.new(variance, fixed) # rubocop:disable PrisonGuard/UseOpusTypesShortcut
  end

  def type_template(variance=:invariant, fixed: nil, lower: T.untyped, upper: BasicObject)
    T::Types::TypeTemplate.new(variance, fixed) # rubocop:disable PrisonGuard/UseOpusTypesShortcut
  end
end
