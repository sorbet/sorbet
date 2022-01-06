# frozen_string_literal: true
# typed: strict
# enable-packager: true

class BadSorbetSpec < PackageSpec
  sorbet(
    min_typed_level: false, # error: All package type levels must be specified as string literals
    tests_min_typed_level: "other" # error: Invalid typed level `"other"`
  )
end
