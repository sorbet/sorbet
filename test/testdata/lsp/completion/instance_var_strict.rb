# typed: strict

# This file tests that we can produce completion results for instance
# variables and class variables that have declared types.  For cases where
# such variables do not have declared types, we handle things differently;
# instance_var.rb exists to test such cases.

class ::Module
  include T::Sig
end

class InstanceVariable
  sig {void}
  def some_method
    @my_ivar = T.let(5, T.nilable(Integer))
  end

  sig {void}
  def other_method
    my = 5
    @my # error: Use of undeclared variable
    #  ^ completion: @my_ivar
  end

  sig {void}
  def more_method
    @ # error: unexpected `@`
    #^ completion: @my_ivar
  end
end

class OtherInstanceVariable
  # Don't pick up completion results from `InstanceVariable`
  sig {void}
  def initialize
    @mo_ivar = T.let(5, T.nilable(Integer))
  end

  sig {void}
  def some_method
    @m # error: Use of undeclared variable
    # ^ completion: @mo_ivar
  end
end

class ClassVariable
  sig {void}
  def some_method
    @@my_cvar = T.let(5, T.nilable(Integer)) # error: must be declared at class scope
  end

  sig {void}
  def other_method
    @@my # error: Use of undeclared variable
    #   ^ completion: @@my_cvar
  end

  sig {void}
  def more_method
    # We parse each '@' as its own separate thing.
    @@ # error: unexpected `@@`
    # ^ completion: @@my_cvar
  end
end

class OtherClassVariable
  # Don't pick up completion results from `ClassVariable`
  sig {void}
  def some_method
    @@m # error: Use of undeclared variable
    #  ^ completion: (nothing)
  end
end

class OnlyInstanceVariables
  # Don't pick up completion results from a similarly-named class variable
  @@my_cvar = nil

  sig {void}
  def some_method
    @@my_cvar = T.let(5, T.nilable(Integer)) # error: must be declared at class scope
    @my # error: Use of undeclared variable
    #  ^ completion: (nothing)
  end

  sig {void}
  def other_method
    @my # error: Use of undeclared variable
    #  ^ completion: (nothing)
  end
end

class OnlyClassVariables
  # Don't pick up completion results from a similarly-named instance variable
  sig {void}
  def some_method
    @my_ivar = T.let(6, T.nilable(Integer))
    @@my # error: Use of undeclared variable
    #   ^ completion: (nothing)
  end

  sig {void}
  def other_method
    @@my # error: Use of undeclared variable
    #   ^ completion: (nothing)
  end
end

class NestedClassInstanceVariables
  sig {void}
  def some_method
    @nciv_ivar = T.let(6, T.nilable(Integer))
  end

  class Inside
    sig {void}
    def some_method
      @ncivi_ivar = T.let(7, T.nilable(Integer))
    end
  end

  sig {void}
  def other_method
    @nciv_ivar2 = T.let(8, T.nilable(Integer))
  end

  sig {void}
  def more_method
    @nciv # error: Use of undeclared variable
    #    ^ completion: @nciv_ivar, @nciv_ivar2
  end
end

# We don't provide completion for variables in a class's static-init code;
# they are uncommon, discouraged at Stripe, and require special-casing in
# the completion code.
class ClassVariablesInStaticInit
  @@static_init_cvar = T.let(5, T.nilable(Integer))
  @@stat # error: Use of undeclared variable
  #     ^ completion: (nothing)
end

class Superclass
  @@super_cvar = T.let(5, T.nilable(Integer))

  sig {void}
  def some_method
    @super_ivar = T.let(6, T.nilable(Integer))
  end
end

# See above comments about completing variables in static-init.
class Inheriting < Superclass
  @@s # error: Use of undeclared variable
  #  ^ completion: (nothing)

  sig {void}
  def other_method
    @super # error: Use of undeclared variable
    #     ^ completion: @super_ivar
  end
end
