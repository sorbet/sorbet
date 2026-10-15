# typed: strict

class Root < PackageSpec
  prelude!

  custom_method 'one', 'extra'
  missing_method

  import MissingImport
  visible_to MissingVisibility

  strict_dependencies false
  layer 1
  sorbet tests_min_typed_level: false
end
