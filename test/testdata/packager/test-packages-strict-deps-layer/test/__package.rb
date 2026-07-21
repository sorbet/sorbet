# typed: strict

  class Test::Root < PackageSpec
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: This package does not declare a `layer`
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: This package does not declare a `strict_dependencies` level
  test!

  import Root, uses_internals: true

end
