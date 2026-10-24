# typed: true

class NotExemptClass < StandardError # error: `NotExemptClass` can only be a `class` if package `NotExemptClass` is marked `outer_package_with_namespace_behavior!`
end
