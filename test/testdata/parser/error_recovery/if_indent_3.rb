# typed: false

class A
  def test0
    x = if
  end # error: unexpected token "end"

  def test1
    x = if x
  end # error: Hint: closing "end" token was not indented as far as "if" token

  def test2
    x = if x.
  end
# ^^^ error: unexpected token "end"
# ^^^ error: Hint: closing "end" token was not indented as far as "if" token

  def test3
    x = if x.f
  end # error: Hint: closing "end" token was not indented as far as "if" token

  def test3
    x = if x.f()
  end # error: Hint: closing "end" token was not indented as far as "if" token
end # error: unexpected token "end of file"
