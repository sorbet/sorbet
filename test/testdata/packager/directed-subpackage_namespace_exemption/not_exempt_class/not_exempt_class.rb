# typed: true

class NotExemptClass < StandardError # error: `NotExemptClass` can only have a superclass if package `NotExemptClass` is marked `outer_package_with_namespace_behavior!`
end
