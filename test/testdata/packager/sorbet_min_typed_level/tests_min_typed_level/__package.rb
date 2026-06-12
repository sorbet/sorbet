# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class TestsMinTypedLevel < PackageSpec
  sorbet min_typed_level: 'true', tests_min_typed_level: 'true'
  #                               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unrecognized keyword argument `tests_min_typed_level` passed for method `Sorbet::Private::Static::PackageSpec.sorbet`
  strict_dependencies 'false'
  layer 'a'
end
