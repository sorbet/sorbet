# typed: strict

class UnitEarly < PackageSpec
  # This transitively schedules EarlyTarget first without importing it here.
  test_import EarlyBridge
end
