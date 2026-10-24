# typed: true

module Mid
  class Inner # error: `Mid::Inner` can only be a `class` if package `Mid` is marked `outer_package_with_namespace_behavior!`
  end
end
