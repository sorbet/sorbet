# typed: strict

class Prelude < PackageSpec
  prelude!

  import Application # error: `Prelude` may not `import` non-prelude package `Application`
end
