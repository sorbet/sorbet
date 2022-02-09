# typed: false

class A
  def test0
    x = if # error: unexpected token "if"
  end

  def test1
    x = if x # error: Hint: this "if" token might not be properly closed
  end

  def test2
    x = if x. # error: Hint: this "if" token might not be properly closed
  end
# ^^^ error: unexpected token "end"

  def test3
    x = if x.f # error: Hint: this "if" token might not be properly closed
  end

  def test3
    x = if x.f() # error: Hint: this "if" token might not be properly closed
  end
end # error: unexpected token "end of file"
