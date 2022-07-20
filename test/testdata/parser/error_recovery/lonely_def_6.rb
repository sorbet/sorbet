# typed: false

class A
  def # error: Hint: this "def" token might be unmatched
  end

  sig {void}
  def foo
  end
end # error: unexpected token "end of file"
