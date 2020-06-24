# frozen_string_literal: true
# typed: true
# compiled: true

# We have a SymbolBasedIntrinsic for T::Enum.new that does weird things to get
# it to work for things like `MyEnum::X$1.new`. Based on the way it's
# implemented, it's easy for that intrinsic to accidentally fire for this
# snippet, so we have a test to make sure it doesn't.

begin
  T::Enum.new
rescue RuntimeError => exn
  p exn.message
end
