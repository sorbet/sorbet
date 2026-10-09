# typed: strict

class Lib < PackageSpec
  test!

  # Importing a prelude package is what puts `Lib` in a stratum of its own.
  import Prelude
end
