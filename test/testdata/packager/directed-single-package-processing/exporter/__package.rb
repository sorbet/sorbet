# typed: strict
# enable-packager: true
# enable-package-directed: true

class Exporter < PackageSpec
  export Exporter::Public
  export Exporter::Alias
  export Exporter::StaticField
  export Exporter::TypeAlias
  export Test::Exporter::TestPublic
end
