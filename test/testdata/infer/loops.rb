class HasLoops
  def variable_only_inside_loop
    while(true)
      a = 1
    end
  end

  def incorrect_assignment
    a = "s"
    while(false)
      a = 1 # error: pinned
    end
  end

  def correct_assignment
    a = "s"
    while(false)
      a = "a"
    end
  end
end
