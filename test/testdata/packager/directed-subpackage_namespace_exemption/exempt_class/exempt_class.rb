# typed: true

class ExemptClass < StandardError # error: `ExemptClass` is part of a subpackage namespace, and must not have ancestors
end
