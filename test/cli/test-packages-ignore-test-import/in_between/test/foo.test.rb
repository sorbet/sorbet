# typed: true

module Test::InBetween
  class FooTest
    def test_1
      Migrated::Test::Helper.new
      InBetween::Foo.new
    end
  end
end
