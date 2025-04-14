# typed: strict

class FooBar < PackageSpec
  import Lib # error: Package `Lib` includes explicit visibility modifiers and cannot be imported from `FooBar`
end
