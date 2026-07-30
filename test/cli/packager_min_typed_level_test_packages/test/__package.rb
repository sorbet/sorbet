# typed: strict
# enable-packager: true

class Test::Project::MyPackage < PackageSpec
  test!

  sorbet min_typed_level: 'true'
end
