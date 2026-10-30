# typed: strict
# stratum: 4

class Consumer < PackageSpec
  import A
  import B
  import A::Patch
  import Mutex
end
