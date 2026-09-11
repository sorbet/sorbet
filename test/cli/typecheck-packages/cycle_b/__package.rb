# typed: strict

class CycleB < PackageSpec
  import CycleA
  export CycleB::Value
end
