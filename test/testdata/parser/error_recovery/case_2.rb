# typed: false

class A
  puts 'before'

  def test1
    case # error: "case" statement must at least have one "when" clause
    end
  end

  def test2
    case x # error: "case" statement must at least have one "when" clause
    end
  end

  puts 'after'
end
