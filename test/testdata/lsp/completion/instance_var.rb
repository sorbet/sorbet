# typed: true

# This file tests that we can produce completion results for instance variables
# and class variables that have no declared type, with some limitations.  For
# cases where such variables do have declared types, we can do better;
# instance_var_strict.rb exists to test such cases.

class InstanceVariable
  def some_method
    @different_prefix = 10
    @my_ivar = 5
  end

  def other_method
    my = 5
    @my
    #  ^ completion: @my, @my_ivar
  end

  def more_method
    @ # error: unexpected `@`
    #^ completion: @different_prefix, @my, @my_ivar
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
    @@different_prefix = 9
    @@my_cvar = 5
  end

  def other_method
    @@my
    #   ^ completion: @@my, @@my_cvar
  end

  def more_method
    # We parse each '@' as its own separate thing.
    #
    # If the parser gets fixed to return some kind of node for bare `@@`, we
    # should be able to start producing completion results here.
    @@ # error-with-dupes: unexpected
    # ^ completion: @@different_prefix, @@my, @@my_cvar
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

# We don't provide completion for variables in a class's static-init code;
# they are uncommon, discouraged at Stripe, and require special-casing in
# the completion code.
class ClassVariablesInStaticInit
  @@static_init_cvar = 5
  @@stat
  #     ^ completion: (nothing)
end

class Superclass
  @@super_cvar = 5

  def some_method
    @super_ivar = 6
  end
end

# See above comments about completing variables in static-init.
class Inheriting < Superclass
  @@s
  #  ^ completion: (nothing)

  # Because our completion algorithm operates purely syntactically for
  # untyped instance variables, it misses picking up instance variables from
  # `Superclass`, since those instance variables are not declared with `T.let`.
  def other_method
    @super
    #     ^ completion: @super
  end
end
