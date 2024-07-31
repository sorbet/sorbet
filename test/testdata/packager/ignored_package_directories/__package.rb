# typed: strict
# enable-packager: true
# ignore-packaging-in: test/testdata/packager/ignored_package_directories/ignored/

class Root < PackageSpec
  export Root::Included
end
