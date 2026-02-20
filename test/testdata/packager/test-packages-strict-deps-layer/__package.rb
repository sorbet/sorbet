# typed: strict
# enable-packager: true
# enable-test-packages: true
# packager-layers: a

  class Root < PackageSpec
# ^^^^^^^^^^^^^^^^^^^^^^^^ error: This package does not declare a `layer`
# ^^^^^^^^^^^^^^^^^^^^^^^^ error: This package does not declare a `strict_dependencies` level
  export Root::A
end
