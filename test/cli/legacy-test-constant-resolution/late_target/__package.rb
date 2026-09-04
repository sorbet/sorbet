# typed: strict

class LateTarget < PackageSpec
  # This edge guarantees that UnitLate's test stratum runs first.
  test_import UnitLate
  export Test::LateTarget::Value
end
