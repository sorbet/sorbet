# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a, b

class TooFewArgs < PackageSpec
  strict_dependencies 'false'
  layer # error: Not enough arguments provided for method `PackageSpec.layer`. Expected: `1`, got: `0`
end
