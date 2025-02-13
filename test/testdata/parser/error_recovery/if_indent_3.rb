# typed: false

class A
  def test0
    x = if # parser-error: unexpected token "if"
  end

  def test1
    x = if x # parser-error: Hint: this "if" token might not be properly closed
  end

  def test2
    x = if x. # parser-error: Hint: this "if" token might not be properly closed
  end
# ^^^ parser-error: unexpected token "end"

  def test3
    x = if x.f # parser-error: Hint: this "if" token might not be properly closed
  end

  def test3
    x = if x.f() # parser-error: Hint: this "if" token might not be properly closed
  end
end # parser-error: unexpected token "end of file"
