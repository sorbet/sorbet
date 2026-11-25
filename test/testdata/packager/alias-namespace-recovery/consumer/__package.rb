# typed: strict
# stratum: 3

class Consumer < PackageSpec
  import A
  import B
  import A::Patch
  import Mutex
end
