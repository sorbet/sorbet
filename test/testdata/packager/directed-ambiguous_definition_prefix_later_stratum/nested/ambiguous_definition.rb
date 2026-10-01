# typed: true

module Nested
  # `Prefix` is an intermediate namespace in the package registry rather than
  # a package itself. Its child package is in a later stratum, so
  # package-directed mode does not report the ambiguity.
  module Prefix::B
  end
end
