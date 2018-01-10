# @typed
class TestIsA
  class T1; end
  class T2; end

  def test_is_a?(x)
    case x
    when T1 || T2
      # This is equivalent to `T1`, since Ruby evaluates the `||`
      # before the `match, but it exhibited a bug in the typechecker.
    end

    if x.is_a?(T1 || T2)
    end
  end
end
