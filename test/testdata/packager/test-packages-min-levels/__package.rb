# typed: strict
# enable-packager: true
# enable-test-packages: true

class Root < PackageSpec
  sorbet min_typed_level: 'true', tests_min_typed_level: 'true'
  #                               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Invalid expression in package

  export Root::A
end
