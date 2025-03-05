# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class TooFewArgs < PackageSpec
  strict_dependencies # error: Not enough arguments provided for method `Sorbet::Private::Static::PackageSpec.strict_dependencies`. Expected: `1`, got: `0`
  layer 'a'
end
