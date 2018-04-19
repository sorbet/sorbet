# typed: strict
class Test
  def test_case
    case a
    when foo1
      :a
    when foo2, foo3
      :b
    else
      :c
    end

    case
    when test1
      :a
    when test2, test3
      :b
    end
  end
end
