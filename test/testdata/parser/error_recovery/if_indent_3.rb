# typed: false

class A
  def test0
    x = if
  end # error: unexpected token "end"

  def test1
    x = if x
  end

  def test2
    x = if x.
  end
# ^^^ error: unexpected token "end"

  def test3
    x = if x.f
  end

  def test3
    x = if x.f()
  end
end # error: unexpected token "end of file"
