# typed: strict
class TestHash
  def something; 17; end
  def test
    {
      something => :bar,
      1 + 2 => 2,
    }
  end

  def test_shaped
    {
      1 => 2,
      2 => 3,
      foo: :bar,
      baz: something,
    }
  end
end
