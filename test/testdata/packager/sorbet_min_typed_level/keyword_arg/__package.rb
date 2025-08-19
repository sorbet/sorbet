# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class NonKeywordArg < PackageSpec
  sorbet 'true', min_typed_level: 'true', tests_min_typed_level: 'true'
  #      ^^^^^^ error: Too many positional arguments provided for method `Sorbet::Private::Static::PackageSpec.sorbet`. Expected: `0`, got: `1`
  strict_dependencies 'false'
  layer 'a'
end
