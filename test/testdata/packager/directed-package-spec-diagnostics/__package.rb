# typed: strict
# enable-packager: true
# enable-package-directed: true
# packager-layers: app

class Root < PackageSpec
  prelude!

  custom_method 'one', 'extra'
  #                    ^^^^^^^ error: Too many arguments provided for method `Sorbet::Private::Static::PackageSpec.custom_method`

  missing_method # error: Method `missing_method` does not exist on `T.class_of(Root)`

  import MissingImport
  #      ^^^^^^^^^^^^^ error: Unable to resolve constant `MissingImport`

  visible_to MissingVisibility
  #          ^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `MissingVisibility`

  strict_dependencies false
  #                   ^^^^^ error: Argument to `strict_dependencies` must be one of
  #                   ^^^^^ error: Expected `String` but found `FalseClass`

  layer 1
  #     ^ error: Argument to `layer` must be one of: app
  #     ^ error: Expected `String` but found `Integer(1)`

  sorbet min_typed_level: 'true', tests_min_typed_level: false # error: Argument to `tests_min_typed_level` must be one of
  #                                                      ^^^^^ error: Expected `T.nilable(String)` but found `FalseClass`
end
