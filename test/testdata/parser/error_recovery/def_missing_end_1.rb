# typed: false

class A
  def test1
    if x.f
    end

  def test2
    if x.f
    end
  end
end # error: unexpected token "end of file"
