# typed: true
class C
  extend T::Sig
  sig {void}
  def foo
    return 3 # error: `C#foo` has return type `void` but explicitly returns an expression
  end
end
