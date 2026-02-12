# typed: strict

class Test::Root < PackageSpec
  test!

  test_import Root
  #           ^^^^ error: Invalid expression in package

  test_import Root, only: "test_rb"
  #           ^^^^ error: Invalid expression in package

end
