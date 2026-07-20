# typed: strict

class Root::Test < PackageSpec
  test!

  sorbet tests_min_typed_level: 'strict', min_typed_level: 'strict'
  #      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Invalid expression in package


  import Root, uses_internals: true

end
