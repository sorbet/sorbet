# typed: true

module StrayStatements # error: `StrayStatements` can only define behavior if package `StrayStatements` is marked `outer_package_with_namespace_behavior!`
  class Main; end

  Main.new if __FILE__ == $PROGRAM_NAME
end
