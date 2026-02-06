# typed: strict
# enable-packager: true

class Project::MyPackage < PackageSpec
  sorbet min_typed_level: 'strict', tests_min_typed_level: 'true'
end
