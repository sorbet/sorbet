# typed: strict

class Test::Root < PackageSpec
  test!

  test_import Root
  #           ^^^^ error: Invalid expression in package
  #           ^^^^ error: Expected `T.class_of(Sorbet::Private::Static::PackageSpec)`

  test_import Root, only: "test_rb"
  #           ^^^^ error: Invalid expression in package
  #           ^^^^ error: Expected `T.class_of(Sorbet::Private::Static::PackageSpec)`

end
