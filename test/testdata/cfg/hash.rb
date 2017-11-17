class Integer
  standard_method(
    {o: Integer,},
    returns: Integer
  )
  def +(o)
  end
end

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
    }
  end
end
