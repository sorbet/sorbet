# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class NonStringOption < PackageSpec
  sorbet min_typed_level: 'true', tests_min_typed_level: false # error: Argument to `tests_min_typed_level` must be one of: `ignore`, `false`, `true`, `strict`, or `strong`
  #                                                      ^^^^^ error: Expected `T.nilable(String)` but found `FalseClass` for argument `tests_min_typed_level`

  layer 'a'
  strict_dependencies 'false'
end
