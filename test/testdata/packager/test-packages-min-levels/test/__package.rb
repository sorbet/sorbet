# typed: strict

class Test::Root < PackageSpec
  test!

  sorbet tests_min_typed_level: 'strict', min_typed_level: 'strict'
  #      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unrecognized keyword argument `tests_min_typed_level`


  import Root, uses_internals: true

end
