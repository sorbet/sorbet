# typed: true
class C
  extend T::Sig

  sig {void}
  def explicit_return_expr
    return 3 # error: `C#explicit_return_expr` has return type `void` but explicitly returns an expression
  end

  sig {void}
  def implicit_return_at_end
    3
  end

  sig {void}
  def explicit_return_nothing
    return
  end
end
