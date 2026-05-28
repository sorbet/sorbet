# typed: true

class Test
  attr_reader :a, :foo1, :foo2, :foo3, :test1, :test2, :test3
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
