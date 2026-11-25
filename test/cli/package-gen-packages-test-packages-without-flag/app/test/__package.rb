# typed: strict

class App::Test < PackageSpec
  test!

  import App, uses_internals: true
end
