# typed: strict

class TestTop < PackageSpec
  test!
  import TestConsumer
  export TestTop::Value
end
