# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class TooManyArgs < PackageSpec
  sorbet min_typed_level: 'true', tests_min_typed_level: 'true', foo: 'bar' # error: Unrecognized keyword argument `foo` passed for method `Sorbet::Private::Static::PackageSpec.sorbet`
  strict_dependencies 'false'
  layer 'a'
end
