# typed: strict
# enable-packager: true
# enable-test-packages: true
# packager-layers: a

class Root < PackageSpec
  layer 'a'
  strict_dependencies 'false'
end
