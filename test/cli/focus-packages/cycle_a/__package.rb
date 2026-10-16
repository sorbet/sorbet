# typed: strict

class CycleA < PackageSpec
  import CycleB
  import Target
  export CycleA::Value
end
