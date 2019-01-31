# frozen_string_literal: true
# typed: true

module T::Props::Constants
  # NB: This is in its own file to break a require-time cycle.
  SCALAR_TYPES = Set.new(%w{
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
  }).freeze
end
