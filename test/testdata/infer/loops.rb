# typed: strict
class HasLoops
  def variable_only_inside_loop
    while(true)
      a = 1
    end
  end

  def incorrect_assignment
    a = "s"
    while(true)
      a = 1 # error: Changing the type of a variable in a loop is not permitted
    end
  end

  def correct_assignment
    a = "s"
    while(true)
      a = "a"
    end
  end
end
