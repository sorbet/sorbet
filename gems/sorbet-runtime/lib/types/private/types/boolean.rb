# frozen_string_literal: true
# typed: true

# Specialization of Union for the common case of the union of two simple types.
#
# This covers e.g. T.nilable(SomeModule), T.any(Integer, Float), and T::Boolean.
class T::Private::Types::Boolean < T::Types::Union
  class DuplicateType < RuntimeError; end

  def initialize
  end

  # @override Union
  def recursively_valid?(obj)
    return true if true == obj
    false == obj
  end

  # @override Union
  def valid?(obj)
    return true if true == obj
    false == obj
  end

  # @override Union
  def types
    # We reconstruct the simple types rather than just storing them because
    # (1) this is normally not a hot path and (2) we want to keep the instance
    # variable count <= 3 so that we can fit in a 40 byte heap entry along
    # with object headers.
    @types ||= [
      T::Types::Simple::Private::Pool.type_for_module(TrueClass),
      T::Types::Simple::Private::Pool.type_for_module(FalseClass),
    ]
  end

  # overrides Union
  def unwrap_nilable
    nil
  end
end
