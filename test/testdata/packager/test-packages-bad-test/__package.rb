# typed: strict
# enable-packager: true

class Root < PackageSpec
  test!

  export Root::A
end
