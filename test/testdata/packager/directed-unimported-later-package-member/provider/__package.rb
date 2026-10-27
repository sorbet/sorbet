# typed: strict

# `Provider` imports `Consumer`, so it is processed in a later stratum.
# stratum: 3

class Provider < PackageSpec
  import Consumer

  export Provider::Flag
end
