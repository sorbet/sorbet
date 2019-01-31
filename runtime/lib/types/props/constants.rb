# frozen_string_literal: true
# typed: true

module T::Props::Constants
  # NB: This is in its own file to break a require-time cycle.
  SCALAR_TYPES = Set.new(%w[
    NilClass
    TrueClass
    FalseClass
    Integer
    Float
    String
    Symbol
    Time
    BSON::Binary
    Opus::Enum
    BSON::Timestamp
  ]).freeze

  # A bit silly, but in Ruby, if you use any? with a Set you get a deoptimized version which uses allocations.
  # While if you use an Array, you get the nice version. Providing this for CustomType.scalar_type?
  # which doesn't care about Sets, but does need to use any? in an optimized fashion.
  SCALAR_TYPES_ARRAY = SCALAR_TYPES.to_a.freeze
end
