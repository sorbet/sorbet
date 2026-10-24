# typed: true

module PreludePkg
  include Comparable # error: `PreludePkg` can only have ancestors if package `PreludePkg` is marked `outer_package_with_namespace_behavior!`
end
