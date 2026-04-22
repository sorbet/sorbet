# typed: strict

# Stratum 0 despite depending on A because we too are a prelude package

# stratum: 0

class C < PackageSpec
  prelude_package
  import A
end
