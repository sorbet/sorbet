# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

ENUM_VALUES = ["value1", "value2"]

sig {params(e: T.deprecated_enum(ENUM_VALUES)).void}
def f(e)
  p e
end

begin
  f("bad value")
rescue => e
  p e.class
else
  p "should have gotten an error"
end
