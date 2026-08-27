# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a, b

class Root < PackageSpec
  strict_dependencies 'false'
  layer 'a'
end
