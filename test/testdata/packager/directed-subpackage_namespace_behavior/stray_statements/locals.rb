# typed: true

module StrayStatements # error: `StrayStatements` can only define behavior if package `StrayStatements` is marked `outer_package_with_namespace_behavior!`
  x = 1
  Y = x
end
