# typed: strict
# enable-packager: true
# packager-layers: a

class Root < PackageSpec
  layer 'a'
  strict_dependencies 'false'
end
