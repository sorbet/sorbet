# typed: strict
# enable-packager: true

class Root < PackageSpec
  sorbet min_typed_level: 'true', tests_min_typed_level: 'true'
  #                               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unrecognized keyword argument `tests_min_typed_level`

  export Root::A
end
