# typed: __STDLIB_INTERNAL
#
# These are Stripe-specific shims that are meant to please the typechecker.
# Additional referernces to these shims are not encouraged, but if absolutely
# needed, you'll need to make sure to use `defined?(...)` to ensure that you
# don't crash at runtime.

module Opus::Sensitivity; end
module Opus::Sensitivity::Utils
  def self.normalize_sensitivity_and_pii_annotation(value); end
end

class Opus::Ownership; end

class Opus::Enum; end
