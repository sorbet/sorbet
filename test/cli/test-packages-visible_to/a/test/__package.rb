# typed: strict

class Test::A < PackageSpec
  test!

  import A # allowed because A has `visible_to "tests"`
  import Root
end
