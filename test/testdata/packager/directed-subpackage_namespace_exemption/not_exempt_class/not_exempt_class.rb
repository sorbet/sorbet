# typed: true

class NotExemptClass < StandardError # error: `NotExemptClass` is part of a subpackage namespace, and must not have ancestors
end
