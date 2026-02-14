# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class TooFewArgs < PackageSpec
  sorbet min_typed_level: 'true'
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `sorbet` requires values for both `min_typed_level` and `tests_min_typed_level`
  strict_dependencies 'false'
  layer 'a'
end
