# typed: true
class C
  extend T::Sig
  sig {void}
  def foo
    return 3 # error: `C#foo` returns `void` but contains an explicit `return <expression>`
  end
end
