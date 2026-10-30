# typed: strict

class HelperUnitImport < PackageSpec
  test_import Target, only: "test_rb"
end
