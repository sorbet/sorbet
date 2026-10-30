# typed: strict

class A < PackageSpec # error: Redefinition of package `A`
  export_all!
end
