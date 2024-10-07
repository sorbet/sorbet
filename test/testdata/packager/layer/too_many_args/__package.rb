# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a, b

class TooManyArgs < PackageSpec
  strict_dependencies 'false'
  layer 'a', 'b' # error: Too many arguments provided for method `PackageSpec.layer`. Expected: `1`, got: `2`
end
