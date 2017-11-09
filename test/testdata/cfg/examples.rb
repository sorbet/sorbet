class Integer
  standard_method(
    {
      o: Integer,
    },
    returns: Opus::Types.any(Integer)
  )
  def +(o)
  end
end

class Examples
  def i_like_ifs()
    if true
      return 1
    else
      return 2
    end
  end

    def i_like_exps()
      if (true)
        1
      else
        2
      end
    end

    def return_in_one_branch1()
      if (true)
        return 1
      else
        2
      end
    end

    def return_in_one_branch2()
      if (true)
        1
      else
        return 2
      end
    end


    def variables()
      if (true)
        a = 1
      else
        a = 2
      end

      if (false)
        b = 1
      else
        b = 2
      end

      a + b
    end


    def variables_and_loop()
      if (true)
        a = 1
      else
        a = 2
      end

      while true
        if (false)
          b = 1 # error: pinned
        else
          b = 2 # error: pinned
        end
      end

      b
    end


    def variables_loop_if()
      while true
        if (false)
          b = 1 # error: pinned
        else
          b = 2 # error: pinned
        end
      end

      b
    end

    def take_arguments(i)
      if(false)
       2
      else
       i
      end
    end

end
