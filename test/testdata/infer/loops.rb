class HasLoops
  def variable_only_inside_loop
    while(true)
      a = 1
    end
  end

  def incorrect_assignment
    a = "s"
    while(false)
      a = 1 # error: Changing type of pinned argument
    end
  end

  def correct_assignment
    a = "s"
    while(false)
      a = "a"
    end
  end
end
