# typed: strict
class TestIsA
  class T1; end
  class T2; end

  def test_is_a?(x)
    case x
    when T1 || T2 # error: This code is unreachable
    end

    if x.is_a?(T1 || T2) # error: This code is unreachable
    end
  end
end
