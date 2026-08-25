# typed: strict
# enable-packager: true

class Root < PackageSpec
  test!
# ^^^^^ error: `test!` is only valid for packages with `/test/` in their path

  export Root::A
end
