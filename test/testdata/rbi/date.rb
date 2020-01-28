# typed: strict

require "date"

T.assert_type!(Date.today, Date)
T.assert_type!(Date.today + 1, Date)

class Test
  extend T::Sig

  sig {params(x: Date).void}
  def test_date(x)
    T.reveal_type(x -  x) # error: Revealed type: `Rational`
    T.reveal_type(x - 10) # error: Revealed type: `Date`
  end

  sig {params(x: DateTime).void}
  def test_datetime(x)
    T.reveal_type(x - x) # error: Revealed type: `Rational`
    T.reveal_type(x - 10) # error: Revealed type: `DateTime`
  end
end
