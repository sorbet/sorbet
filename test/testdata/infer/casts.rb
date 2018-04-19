# typed: strict
class TestCasts
  def untyped; end

  def test_casts
    t = T.assert_type!(untyped, Integer) # error: unable to infer the type
    t + 4

    t1 = T.assert_type!("hi", Integer) # error: does not have asserted type
    t1 + 1

    s = T.cast(untyped, String)
    s + "hi"

    s = T.cast(6, String)
    s + "hi"
    s + 3 # error: does not match expected type


    s = T.cast(6, 7) # error: Unsupported type syntax

    # s ends up as `untyped`, so these are all OK
    s + "hi"
    s + 3
  end
end
