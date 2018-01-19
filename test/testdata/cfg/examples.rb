# @typed
class Examples
  def i_like_ifs()
    if true
      return 1
    else
      return 2 # error: This code is unreachable
    end
  end

    def i_like_exps()
      if (true)
        1
      else
        2 # error: This code is unreachable
      end
    end

    def return_in_one_branch1()
      if (true)
        return 1
      else
        2 # error: This code is unreachable
      end
    end

    def return_in_one_branch2()
      if (true)
        1
      else
        return 2 # error: This code is unreachable
      end
    end


    def variables()
      if (true)
        a = 1
      else
        a = 2 # error: This code is unreachable
      end

      if (false)
        b = 1 # not-yet-an-error: This code is unreachable
      else
        b = 2
      end

      a + b
    end


    def variables_and_loop(cond)
      if (true)
        a = 1
      else
        a = 2 # error: This code is unreachable
      end

      while true
        if (cond)
          b = 1 # error: Changing type of pinned argument
        else
          b = 2 # error: Changing type of pinned argument
        end
      end

      b # error: This code is unreachable
    end


    def variables_loop_if(cond)
      while true
        if (cond)
          b = 1 # error: Changing type of pinned argument
        else
          b = 2 # error: Changing type of pinned argument
        end
      end

      b # error: This code is unreachable
    end

    def take_arguments(i)
      if(false)
       2 # not-yet-an-error: This code is unreachable
      else
       i
      end
    end

end
