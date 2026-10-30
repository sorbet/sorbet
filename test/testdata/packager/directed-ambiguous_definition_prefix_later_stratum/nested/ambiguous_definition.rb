# typed: true

module Nested
  # `Prefix` is an intermediate namespace in the package registry rather than
  # a package itself. Its child package is in a later stratum.
  module Prefix::B # error: Definition of `B` is possibly ambiguous
  end
end
