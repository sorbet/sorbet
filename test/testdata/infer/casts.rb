# @typed
class TestCasts
  def untyped; end

  def test_casts
    t = Opus::Types.assert_type!(untyped, Integer) # error: unable to infer the type
    t + 4

    t1 = Opus::Types.assert_type!("hi", Integer) # error: does not have asserted type
    t1 + 1

    s = Opus::Types.cast(untyped, String)
    s + "hi"

    s = Opus::Types.cast(6, String)
    s + "hi"
    s + 3 # error: does not match expected type


    s = Opus::Types.cast(6, 7) # error: Unsupported type syntax

    # s ends up as `untyped`, so these are all OK
    s + "hi"
    s + 3
  end
end
