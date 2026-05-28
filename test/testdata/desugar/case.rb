# typed: true

class Test
  attr_reader :a, :foo1, :foo2, :foo3, :test1, :test2, :test3
  def test_case
    case a
    when foo1
      :a
    when foo2, foo3
      :b
    else
      :c
    end

    case
    when test1
      :a
    when test2, test3
      :b
    end
  end

  @@cvar = T.let(123, T.nilable(Integer))

  # Test when Sorbet synthesizes a temporary variable, vs. when it skips it.
  def test_when_temporary_variables_are_used
    # Method calls can have side-effects. Extract the result to a synthetic local variable.
    case to_s()
    when 1
    when 2
    end

    # Reading from `self` is side-effect-free, so we can elide the synthetic local variable.
    T.bind(self, T.nilable(Integer))
    case self
    when Integer
    else T.absurd(self)
    #    ^^^^^^^^^^^^^^ error: Control flow could reach `T.absurd` because the type `NilClass` wasn't handled
    end

    # Local variables are ... already local variables. We can elide the synthetic local variable.
    lvar = T.let(123, T.nilable(Integer))
    case lvar
    when Integer
    else T.absurd(lvar)
    #    ^^^^^^^^^^^^^^ error: Control flow could reach `T.absurd` because the type `NilClass` wasn't handled
    end

    @ivar  = T.let(123, T.nilable(Integer))
    case @ivar
    when Integer
    else T.absurd(@ivar)
    #    ^^^^^^^^^^^^^^^ error: Control flow could reach `T.absurd` because the type `NilClass` wasn't handled
    end

    case @@cvar
    when Integer
    else T.absurd(@@cvar)
    #    ^^^^^^^^^^^^^^^^ error: Control flow could reach `T.absurd` because the type `NilClass` wasn't handled
    end

    $gvar = T.let(123, T.nilable(Integer))
    case $gvar
    when Integer
    else T.absurd($gvar)
    #    ^^^^^^^^^^^^^^^ error: Control flow could reach `T.absurd` because the type `NilClass` wasn't handled
    end

    # Incorrect Desugar tree until https://github.com/sorbet/sorbet/pull/10020 is merged.
    # tap do |_; block_local_var|
    #   block_local_var = T.let(123, T.nilable(Integer))
    #   case block_local_var
    #   when Integer
    #   when 2
    #   end
    # end
  end
end
