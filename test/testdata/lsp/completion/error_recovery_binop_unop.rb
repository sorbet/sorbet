# typed: true

class A
  def f(a, b, c)
    if a + # error: missing arg to "+" operator
    #     ^ completion: a, b, c, ...
    #      ^ completion: a, b, c, ...
    end

    if a == # error: missing arg to "==" operator
    #      ^ completion: a, b, c, ...
    #       ^ completion: a, b, c, ...
    end
  end
end
