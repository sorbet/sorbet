# typed: strict

class TestTop < PackageSpec
  test_import TestConsumer, only: "test_rb"
  export TestTop::Value
end
