# typed: strict

class MyPkg < PackageSpec
  custom_method 'abc'
  custom_method 'abc', 'too_many_args'
  #                    ^^^^^^^^^^^^^^^ error: Too many arguments provided for method `PackageSpec.custom_method`. Expected: `1`, got: `2`

  bad_method 'def'
# ^^^^^^^^^^ error: Method `bad_method` does not exist on `T.class_of(MyPkg)`
end
