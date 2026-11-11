# typed: strict

class Test::Producer < PackageSpec
  test!

  import Producer, uses_internals: true

  export Test::Producer::ToApp::Nested
end
