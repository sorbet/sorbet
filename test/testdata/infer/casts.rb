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
    s + 3 # error: Expected `String` but found `Integer(3)` for argument `arg0`

    s = T.cast(6, Integer) # error: Useless cast
    s = T.cast(6, T.untyped) # error: Please use `T.unsafe(...)`

    s = T.cast(6, 7) # error: Useless cast
                # ^ error: Unsupported literal in type syntax

    # s ends up as `Integer`
    s + "hi" # error: Expected `Integer` but found `String("hi")`
    s + 3
  end
end
