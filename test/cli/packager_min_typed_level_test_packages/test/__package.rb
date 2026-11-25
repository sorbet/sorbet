# typed: strict
# enable-packager: true

class Project::MyPackage::Test < PackageSpec
  test!

  sorbet min_typed_level: 'true'
end
