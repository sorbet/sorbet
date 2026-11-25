# typed: strict

class Consumer < PackageSpec
  # This import is an error, but it also gives `Lib` (a `test!` package) a
  # non-test node in the package condensation graph.
  import Lib
# ^^^^^^^^^^ error: Package `Consumer` may not import `test!` packages
end
