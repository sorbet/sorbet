# typed: strict

class Test::Root < PackageSpec
  test!

  import Root, uses_internals: true
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Package `Root` includes explicit visibility modifiers
end
