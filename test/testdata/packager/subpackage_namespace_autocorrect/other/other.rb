# typed: true

module Other
  include Comparable # error: `Other` can only have ancestors if package `Other` is marked `outer_package_with_namespace_behavior!`
end
