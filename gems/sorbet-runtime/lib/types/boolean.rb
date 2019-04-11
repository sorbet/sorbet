# typed: true
# frozen_string_literal: true

module T
  # T::Boolean is a type alias helper for the common `T::Boolean`.
  # Defined separately from _types.rb because it has a dependency on T::Types::Union.
  Boolean = T.type_alias(T::Boolean)
end
