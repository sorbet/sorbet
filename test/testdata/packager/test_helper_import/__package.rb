# typed: strict
# enable-packager: true

class RootPkg < PackageSpec
  import A
  test_import B
  test_import C, only: "test_rb"
end
