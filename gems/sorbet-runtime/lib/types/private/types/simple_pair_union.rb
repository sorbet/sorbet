# frozen_string_literal: true
# typed: true

# Specialization of Union for the common case of the union of two simple types.
#
# This covers e.g. T.nilable(SomeModule) and T::Boolean.
class T::Private::Types::SimplePairUnion < T::Types::Union
  # @param type_a [T::Types::Simple]
  # @param type_b [T::Types::Simple]
  def initialize(type_a, type_b)
    @raw_a = type_a.raw_type
    @raw_b = type_b.raw_type
  end

  # @override Union
  def recursively_valid?(obj)
    obj.is_a?(@raw_a) || obj.is_a?(@raw_b)
  end

  # @override Union
  def valid?(obj)
    obj.is_a?(@raw_a) || obj.is_a?(@raw_b)
  end

  # @override Union
  def types
    @types ||= [
      T::Types::Simple::Private::Pool.type_for_module(@raw_a),
      T::Types::Simple::Private::Pool.type_for_module(@raw_b),
    ]
  end
end
