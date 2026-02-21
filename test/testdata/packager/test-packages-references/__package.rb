# typed: strict
# enable-packager: true
# enable-test-packages: true

class Root < PackageSpec
  #   ^^^^ def: rootpkg

  export Root::A
end
