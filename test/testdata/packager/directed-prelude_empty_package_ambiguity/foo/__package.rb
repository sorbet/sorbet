# typed: strict

class Opus::Foo < PackageSpec
  BadConstant = 10 # error: Invalid expression in package: `Assign` not allowed
end
