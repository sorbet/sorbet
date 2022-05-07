# typed: strict

class Opus::Util  < PackageSpec
  export Opus::Util::UtilClass
  export Test::Opus::Util::TestUtil

  # Allowed export for test a prefix of a public export
  export Opus::Util::Nesting::Public
  export_for_test Opus::Util::Nesting
# ^^^^^^^^^^^^^^^ error: Method `export_for_test` does not exist
  #               ^^^^^^^^^^^^^^^^^^^ error: Invalid expression in package: Arguments to functions must be literals
end
