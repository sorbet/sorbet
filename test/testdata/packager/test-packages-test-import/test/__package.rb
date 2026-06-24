# typed: strict

class Test::Root < PackageSpec
  test!

  test_import Root
# ^^^^^^^^^^^ error: Test imports must use `import`

  test_import Root, only: "test_rb"
# ^^^^^^^^^^^ error: Test imports must use `import`

end
