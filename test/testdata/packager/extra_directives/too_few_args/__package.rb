# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a, b

class TooFewArgs < PackageSpec
  strict_dependencies 'false'
  layer 'a'
  extra # error: Not enough arguments provided for method `Sorbet::Private::Static::PackageSpec.extra`. Expected: `1`, got: `0`
end
