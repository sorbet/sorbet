# typed: strict
# enable-packager: true

class Root < PackageSpec
  import Root::Test
# ^^^^^^^^^^^^^^^^^ error: Package `Root` may not import `test!` packages

  export Root::A
end
