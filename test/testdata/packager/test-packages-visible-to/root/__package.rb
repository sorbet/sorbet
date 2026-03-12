# typed: strict
# enable-packager: true
# enable-test-packages: true

class Root < PackageSpec
  export Root::A

  visible_to B
end
