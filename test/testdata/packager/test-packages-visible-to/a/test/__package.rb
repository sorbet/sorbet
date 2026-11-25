# typed: strict

class A::Test < PackageSpec
  test!

  import A # allowed because A has `visible_to "tests"`
  import Root
# ^^^^^^^^^^^ error: Package `Root` includes explicit visibility modifiers
end
