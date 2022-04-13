# frozen_string_literal: true
# typed: true

# Use as a mixin with extend (`extend T::Generic`).
module T::Generic
  include T::Helpers
  include Kernel

  ### Class/Module Helpers ###

  def [](*types)
    self
  end

  # TODO(jez) Remove these keyword arguments once it's a static error
  def type_member(variance=:invariant, fixed: nil, lower: T.untyped, upper: BasicObject, &blk)
    T::Types::TypeMember.new(variance)
  end

  # TODO(jez) Remove these keyword arguments once it's a static error
  def type_template(variance=:invariant, fixed: nil, lower: T.untyped, upper: BasicObject, &blk)
    T::Types::TypeTemplate.new(variance)
  end
end
