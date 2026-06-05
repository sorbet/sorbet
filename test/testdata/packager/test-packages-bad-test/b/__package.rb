# typed: strict
# enable-packager: true

class Test::Root::B < PackageSpec
  test!
# ^^^^^ error: `test!` is only valid for packages with `/test/` in their path
end
