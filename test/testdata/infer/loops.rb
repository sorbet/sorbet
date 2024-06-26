# typed: true
class HasLoops
  def variable_only_inside_loop
    while(true)
      a = 1
    end
  end

  def incorrect_assignment
    a = "s"
    while(true)
      a = 1 # error: Changing the type of a variable is not permitted in loops and blocks
    end
  end

  def correct_assignment
    a = "s"
    while(true)
      a = "a"
    end
  end
end
