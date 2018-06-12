# typed: strict
class Main
  def initialize
    @s = T.let(nil, T.nilable(Integer))
  end

  def foo
    @s = 1 unless @s
  end

  def post_pin
    while foo
        if foo
            a = 1 # error: Changing the type of a variable in a loop is not permitted
        else
            a = "1" # error: Changing the type of a variable in a loop is not permitted
        end
    end
    a
  end

  def metatype_pin
    a = 1
    while foo
        a = T::Array[Integer] # error: MULTI
    end
    a
  end

  def unpin(cond)
      a = 1
      while cond.run
          a = 2
      end

      if cond.bla
          a = "2"
      end
      puts a
  end

  def unpin_double(cond)
      a = 1
      while cond.run
          a = 2
          while cond.run2
              a = 3
          end
      end

      if cond.bla
          a = "2"
      end
      puts a
  end

  def unify(cond)
      a = cond ? 1 : "1"
      while cond
          a = 2
      end
  end

  def unify_with_metatype(cond)
      a = cond ? 1 : T::Array[Integer]
      while cond
          a = 2
      end
  end
end
