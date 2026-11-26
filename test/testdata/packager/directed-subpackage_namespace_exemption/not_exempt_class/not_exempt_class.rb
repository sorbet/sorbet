# typed: true

class NotExemptClass < StandardError # error: `NotExemptClass` cannot be a `class`, because packages are nested under `NotExemptClass`
end
