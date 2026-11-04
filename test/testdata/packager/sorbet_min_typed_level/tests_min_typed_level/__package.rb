# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class TestsMinTypedLevel < PackageSpec
  # `tests_min_typed_level` no longer has any effect, but is still accepted.
  sorbet min_typed_level: 'true', tests_min_typed_level: 'true'
  strict_dependencies 'false'
  layer 'a'
end
