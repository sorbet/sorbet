# typed: strict

class TestConsumer < PackageSpec
  test!
  import Target
  export TestConsumer::Value
end
