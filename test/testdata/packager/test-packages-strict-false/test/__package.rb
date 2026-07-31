# typed: strict

class Root::Test < PackageSpec
  test!
  layer 'a'

  import Root, uses_internals: true

  strict_dependencies 'dag' # error: Test packages must be at `strict_dependencies` level `false`
end
