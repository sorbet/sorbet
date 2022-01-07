# frozen_string_literal: true
# typed: strict
# enable-packager: true

class BadSorbetSpec < PackageSpec
  sorbet(
    min_typed_level: false, # error: All package type levels must be specified as string literals
    tests_min_typed_level: "other" # error: Invalid typed level `"other"`
  )
  sorbet min_typed_level: "false" # error: Missing required keyword argument `tests_min_typed_level`
  sorbet tests_min_typed_level: "true" # error: Missing required keyword argument `min_typed_level`
  sorbet other_typed_level: "true"
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Missing required keyword argument `min_typed_level` for method
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Missing required keyword argument `tests_min_typed_level` for method
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unrecognized keyword argument `other_typed_level` passed for method
end
