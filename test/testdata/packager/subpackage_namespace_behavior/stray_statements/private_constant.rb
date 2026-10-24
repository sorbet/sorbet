# typed: true

module StrayStatements # error: `StrayStatements` can only define behavior if package `StrayStatements` is marked `outer_package_with_namespace_behavior!`
  class Hidden; end
  private_constant :Hidden
end
