# typed: strict

class Test::Root < PackageSpec
  test!

  test_import Root
# ^^^^^^^^^^^ error: Method `test_import` does not exist on `T.class_of(Test::Root)`
  #           ^^^^ error: Invalid expression in package: Arguments to functions must be literals

  test_import Root, only: "test_rb"
# ^^^^^^^^^^^ error: Method `test_import` does not exist on `T.class_of(Test::Root)`
  #           ^^^^ error: Invalid expression in package: Arguments to functions must be literals

end
