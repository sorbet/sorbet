# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class TooManyArgs < PackageSpec
  strict_dependencies 'false', 'true' # error: Too many arguments provided for method `Sorbet::Private::Static::PackageSpec.strict_dependencies`. Expected: `1`, got: `2`
  layer 'a'
end
