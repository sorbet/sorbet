# typed: strict

class Test::Producer < PackageSpec
  test!

  import Producer, uses_internals: true

  export Test::Producer::ToApp::Nested # error: Cannot export through class alias
end
