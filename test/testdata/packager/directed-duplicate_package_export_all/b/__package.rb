# typed: strict

# stratum: 0

class A < PackageSpec # error: Redefinition of package `A`
  export_all!
end
