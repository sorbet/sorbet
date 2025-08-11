# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class TooFewArgs < PackageSpec
  sorbet min_typed_level: 'true' # error: Missing required keyword argument `tests_min_typed_level` for method `Sorbet::Private::Static::PackageSpec.sorbet`
  strict_dependencies 'false'
  layer 'a'
end
