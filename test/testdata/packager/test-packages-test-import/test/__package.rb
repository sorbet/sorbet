# typed: strict

class Root::Test < PackageSpec
  test!

  test_import Root
# ^^^^^^^^^^^ error: Test imports must use `import`

  test_import Root, only: "test_rb"
# ^^^^^^^^^^^ error: Test imports must use `import`

end
