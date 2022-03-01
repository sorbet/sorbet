# typed: true
extend T::Sig

class InstanceVariable
  def some_method
    @my_ivar = 5
  end

  def other_method
    my = 5
    @my
    #  ^ completion: @my, @my_ivar
  end
end

class OtherInstanceVariable
  # Don't pick up completion results from `InstanceVariable`
  def some_method
    @m
    # ^ completion: @m
  end
end

class ClassVariable
  def some_method
    @@my_cvar = 5
  end

  def other_method
    @@my
    #   ^ completion: @@my, @@my_cvar
  end
end

class OtherClassVariable
  # Don't pick up completion results from `ClassVariable`
  def some_method
    @@m
    #  ^ completion: @@m
  end
end

class OnlyInstanceVariables
  # Don't pick up completion results from a similarly-named class variable
  def some_method
    @@my_cvar = 5
    @my
    #  ^ completion: @my
  end

  def other_method
    @my
    #  ^ completion: @my
  end
end

class OnlyClassVariables
  # Don't pick up completion results from a similarly-named instance variable
  def some_method
    @my_ivar = 6
    @@my
    #   ^ completion: @@my
  end

  def other_method
    @@my
    #   ^ completion: @@my
  end
end

class NestedClassInstanceVariables
  def some_method
    @nciv_ivar = 6
  end

  class Inside
    def some_method
      @ncivi_ivar = 7
    end
  end

  def other_method
    @nciv_ivar2 = 8
  end

  def more_method
    @nciv
    #    ^ completion: @nciv, @nciv_ivar, @nciv_ivar2
  end
end
