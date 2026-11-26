# typed: true

module StrayStatements # error: `StrayStatements` cannot define behavior, because packages are nested under `StrayStatements`
  x = 1
  Y = x
end
