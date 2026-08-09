# typed: strict

# stratum: 1

class Child < PackageSpec
  import Parent

  export Child::MyChild
end
