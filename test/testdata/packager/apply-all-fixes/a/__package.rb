# typed: strict
# enable-packager: true
# packager-layers: lib, app

class A < PackageSpec
  layer 'lib'
  strict_dependencies 'layered'
end
