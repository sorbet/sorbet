# typed: true

module StrayStatements # error: `StrayStatements` cannot define behavior, because packages are nested under `StrayStatements`
  class Hidden; end
  private_constant :Hidden
end
