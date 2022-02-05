# typed: false

class A
  def test0
    x = if
  end # error: unexpected token "end"

  def test1
    x = if x
  end # error: Closing "end" token was not indented as far as "if" token

  def test2
    x = if x.
  end
# ^^^ error: unexpected token "end"
# ^^^ error: Closing "end" token was not indented as far as "if" token

  def test3
    x = if x.f
  end # error: Closing "end" token was not indented as far as "if" token

  def test3
    x = if x.f()
  end # error: Closing "end" token was not indented as far as "if" token
end
