# typed: strict
class TestCasts
  def untyped; end # error: does not have a `sig`

  def test_casts # error: does not have a `sig`
    t = T.assert_type!(untyped, Integer) # error: unable to infer the type
    t + 4

    t1 = T.assert_type!("hi", Integer) # error: does not have asserted type
    t1 + 1

    s = T.cast(untyped, String)
    s + "hi"

    s = T.cast(6, String)
    s + "hi"
    s + 3 # error: `Integer(3)` doesn't match `String` for argument `arg0`

    s = T.cast(6, Integer) # error: Useless cast
    s = T.cast(6, T.untyped) # error: Please use `T.unsafe(...)`

    s = T.cast(6, 7) # error: MULTI

    # s ends up as `untyped`, so these are all OK
    s + "hi"
    s + 3
  end
end
