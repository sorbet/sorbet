# typed: strict

class MyPkg < PackageSpec
  prelude!

  custom_method 'abc'
  custom_method 'abc', 'too_many_args'
  #                    ^^^^^^^^^^^^^^^ error: Too many arguments provided for method `Sorbet::Private::Static::PackageSpec.custom_method`. Expected: `1`, got: `2`

  bad_method 'def'
# ^^^^^^^^^^ error: Method `bad_method` does not exist on `T.class_of(MyPkg)`
  import MissingImport # error: Unable to resolve constant `MissingImport`
  visible_to MissingVisibility # error: Unable to resolve constant `MissingVisibility`

  strict_dependencies false # error: Argument to `strict_dependencies` must be one of:
  #                   ^^^^^ error: Expected `String` but found `FalseClass`
  layer 42 # error: Argument to `layer` must be one of:
  #     ^^ error: Expected `String` but found `Integer(42)`
  sorbet min_typed_level: 'true', tests_min_typed_level: false # error: Argument to `tests_min_typed_level` must be one of:
  #                                                      ^^^^^ error: Expected `T.nilable(String)` but found `FalseClass`
end
