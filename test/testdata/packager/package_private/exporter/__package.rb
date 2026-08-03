# typed: strict
# enable-packager: true

class Exporter < PackageSpec
  export Exporter::Private # error: Constant `Exporter::Private` is declared `package_private!` and cannot be exported
end
