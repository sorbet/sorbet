# typed: strict

class TestConsumer < PackageSpec
  test_import Target
  export TestConsumer::Value
end
