# typed: strict
class TestLet
  def testit
    x = T.let(0, Integer)
    x = "hi" # error: Incompatible assignment

    y = T.let(0, String) # error: Argument does not have asserted type

    # TODO: we should probably ban this but it's not immediately
    # apparent where to do so.
    0 + T.let(1, Integer)
  end
end
