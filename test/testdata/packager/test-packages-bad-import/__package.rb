# typed: strict
# enable-packager: true
# enable-test-packages: true

class Root < PackageSpec
  import Test::Root
# ^^^^^^^^^^^^^^^^^ error: Package `Root` may not import `test!` packages

  export Root::A
end
