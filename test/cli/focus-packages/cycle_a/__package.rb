# typed: strict

class CycleA < PackageSpec
  import CycleB
  import Target
end
