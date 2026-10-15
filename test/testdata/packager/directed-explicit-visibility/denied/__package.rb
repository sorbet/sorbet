# typed: strict

class Denied < PackageSpec
  import Lib # error: Package `Lib` includes explicit visibility modifiers and cannot be imported from `Denied`
end
