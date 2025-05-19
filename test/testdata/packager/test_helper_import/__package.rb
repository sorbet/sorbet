# typed: strict
# enable-packager: true

class RootPkg < PackageSpec
  import A
  test_import B
  test_import C, for: :TEST_RB_ONLY
end
