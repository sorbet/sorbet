# typed: true

module Root::Test
  class ATest
    def test_a
      Root::A.new.foo
      Root::A.foo
    end
  end
end
