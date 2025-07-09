# typed: false

class A
  def test1
# ^^^ parser-error: Hint: this "def" token might not be properly closed
    if x.f
    end

  def test2
    if x.f
    end
  end
end # parser-error: unexpected token "end of file"
