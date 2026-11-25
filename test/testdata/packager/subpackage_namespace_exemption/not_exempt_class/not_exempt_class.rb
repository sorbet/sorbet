# typed: true

class NotExemptClass < StandardError # error: `NotExemptClass` cannot have a superclass, because packages are nested under `NotExemptClass`
end
